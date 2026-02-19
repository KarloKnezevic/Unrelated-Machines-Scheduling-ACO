#include "benchmark.h"
#include "dynamic_scheduler.h"
#include "evaluator.h"
#include "input_loader.h"
#include "instance.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define COMPARISON_EPSILON 1e-9

static double now_seconds(void) {
#if defined(CLOCK_MONOTONIC)
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
    }
#endif
    return (double)clock() / (double)CLOCKS_PER_SEC;
}

static int run_algorithm_for_mode(
    const SchedulingInstance *instance,
    AlgorithmKind algorithm,
    SchedulingMode mode,
    Objective objective,
    const AcoParams *aco_params,
    ScheduleMetrics *metrics
) {
    if (mode == MODE_DYNAMIC) {
        return run_dynamic_schedule(instance, algorithm, objective, aco_params, metrics);
    }
    return run_static_schedule(instance, algorithm, objective, aco_params, metrics);
}

static void print_configuration(
    const BenchmarkOptions *options,
    size_t start_set,
    size_t end_set
) {
    printf("=======================================================================\n");
    printf(" Unrelated Machines Scheduling Benchmark\n");
    printf("=======================================================================\n");
    printf(" Input root   : %s\n", options->input_root);
    printf(" Sets         : %zu..%zu\n", start_set, end_set);
    printf(" Mode         : %s\n", mode_name(options->mode));
    printf(" Objective    : %s\n", objective_name(options->objective));
    printf(" ACO params   : ants=%zu iter=%zu alpha=%.2f beta=%.2f rho=%.2f tau0=%.2f q0=%.2f\n",
        options->aco_params.ant_count,
        options->aco_params.iterations,
        options->aco_params.alpha,
        options->aco_params.beta,
        options->aco_params.rho,
        options->aco_params.tau0,
        options->aco_params.q0
    );
    printf(" Heuristic    : %s\n", heuristic_name(options->aco_params.heuristic));
    printf(" Seed         : %u\n", options->aco_params.seed);
    if (options->csv_path != NULL) {
        printf(" CSV output   : %s\n", options->csv_path);
    }
    printf("=======================================================================\n");
}

int run_dataset_benchmark(const BenchmarkOptions *options) {
    InputDatasetConfig config;
    size_t start_set;
    size_t end_set;
    FILE *csv_file = NULL;
    size_t set_index;
    size_t evaluated_sets = 0;
    size_t aco_better = 0;
    size_t equal = 0;
    size_t minmin_better = 0;
    double sum_minmin = 0.0;
    double sum_aco = 0.0;
    double total_start_time;

    if (options == NULL || options->input_root == NULL) {
        return -1;
    }

    total_start_time = now_seconds();

    if (input_config_load(options->input_root, &config) != 0) {
        fprintf(stderr, "Failed to load input dataset from '%s'.\n", options->input_root);
        return -1;
    }

    if (config.set_count == 0) {
        input_config_destroy(&config);
        fprintf(stderr, "Input dataset does not contain any sets.\n");
        return -1;
    }

    start_set = options->set_start;
    if (start_set >= config.set_count) {
        start_set = config.set_count - 1;
    }

    end_set = options->set_end;
    if (end_set >= config.set_count) {
        end_set = config.set_count - 1;
    }
    if (end_set < start_set) {
        end_set = start_set;
    }

    if (options->csv_path != NULL) {
        csv_file = fopen(options->csv_path, "w");
        if (csv_file == NULL) {
            input_config_destroy(&config);
            fprintf(stderr, "Failed to open CSV output '%s'.\n", options->csv_path);
            return -1;
        }

        fprintf(
            csv_file,
            "set_index,jobs,machines,mode,objective,minmin_value,aco_value,improvement_percent,"
            "set_runtime_ms,minmin_makespan,aco_makespan,minmin_weighted_flow,aco_weighted_flow,"
            "minmin_weighted_tardiness,aco_weighted_tardiness,minmin_weighted_tardy_jobs,"
            "aco_weighted_tardy_jobs\n"
        );
    }

    print_configuration(options, start_set, end_set);

    if (options->verbose) {
        printf("%-6s %-6s %-8s %-14s %-14s %-10s %-10s\n",
            "Set",
            "Jobs",
            "Machines",
            "Min-Min",
            "ACO",
            "Delta%",
            "Time(ms)"
        );
        printf("-----------------------------------------------------------------------\n");
    }

    for (set_index = start_set; set_index <= end_set; ++set_index) {
        SchedulingInstance instance;
        ScheduleMetrics minmin_metrics;
        ScheduleMetrics aco_metrics;
        double minmin_value;
        double aco_value;
        double improvement = 0.0;
        double set_start_time;
        double set_elapsed_ms;

        set_start_time = now_seconds();

        if (input_load_instance(&config, set_index, &instance) != 0) {
            fprintf(stderr, "Failed to load dataset set %zu.\n", set_index);
            if (csv_file != NULL) {
                fclose(csv_file);
            }
            input_config_destroy(&config);
            return -1;
        }

        if (run_algorithm_for_mode(
                &instance,
                ALGORITHM_MINMIN,
                options->mode,
                options->objective,
                &options->aco_params,
                &minmin_metrics
            ) != 0 ||
            run_algorithm_for_mode(
                &instance,
                ALGORITHM_ACO,
                options->mode,
                options->objective,
                &options->aco_params,
                &aco_metrics
            ) != 0) {
            fprintf(stderr, "Scheduling failed for set %zu.\n", set_index);
            instance_destroy(&instance);
            if (csv_file != NULL) {
                fclose(csv_file);
            }
            input_config_destroy(&config);
            return -1;
        }

        minmin_value = metrics_value(&minmin_metrics, options->objective);
        aco_value = metrics_value(&aco_metrics, options->objective);
        if (minmin_value > 0.0) {
            improvement = ((minmin_value - aco_value) / minmin_value) * 100.0;
        }

        if (aco_value + COMPARISON_EPSILON < minmin_value) {
            ++aco_better;
        } else if (minmin_value + COMPARISON_EPSILON < aco_value) {
            ++minmin_better;
        } else {
            ++equal;
        }

        set_elapsed_ms = (now_seconds() - set_start_time) * 1000.0;

        sum_minmin += minmin_value;
        sum_aco += aco_value;
        ++evaluated_sets;

        if (options->verbose) {
            printf("%-6zu %-6zu %-8zu %-14.4f %-14.4f %-10.2f %-10.2f\n",
                set_index,
                instance.job_count,
                instance.machine_count,
                minmin_value,
                aco_value,
                improvement,
                set_elapsed_ms
            );
        }

        if (csv_file != NULL) {
            fprintf(
                csv_file,
                "%zu,%zu,%zu,%s,%s,%.10f,%.10f,%.6f,%.3f,"
                "%d,%d,%.10f,%.10f,%.10f,%.10f,%.10f,%.10f\n",
                set_index,
                instance.job_count,
                instance.machine_count,
                mode_name(options->mode),
                objective_name(options->objective),
                minmin_value,
                aco_value,
                improvement,
                set_elapsed_ms,
                minmin_metrics.makespan,
                aco_metrics.makespan,
                minmin_metrics.weighted_flow_time,
                aco_metrics.weighted_flow_time,
                minmin_metrics.weighted_tardiness,
                aco_metrics.weighted_tardiness,
                minmin_metrics.weighted_tardy_jobs,
                aco_metrics.weighted_tardy_jobs
            );
        }

        instance_destroy(&instance);
    }

    if (csv_file != NULL) {
        fclose(csv_file);
    }

    if (evaluated_sets > 0) {
        double avg_minmin = sum_minmin / (double)evaluated_sets;
        double avg_aco = sum_aco / (double)evaluated_sets;
        double avg_improvement =
            sum_minmin > 0.0 ? ((sum_minmin - sum_aco) / sum_minmin) * 100.0 : 0.0;
        double total_elapsed = now_seconds() - total_start_time;

        printf("=======================================================================\n");
        printf(" Summary\n");
        printf("=======================================================================\n");
        printf(" Average Min-Min        : %.6f\n", avg_minmin);
        printf(" Average ACO            : %.6f\n", avg_aco);
        printf(" Average improvement    : %.2f%%\n", avg_improvement);
        printf(" ACO better / equal / worse sets : %zu / %zu / %zu\n",
            aco_better,
            equal,
            minmin_better
        );
        printf(" Total execution time   : %.3f s\n", total_elapsed);
        printf("=======================================================================\n");
    }

    input_config_destroy(&config);
    return 0;
}
