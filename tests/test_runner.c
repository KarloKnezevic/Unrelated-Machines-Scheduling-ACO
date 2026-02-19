#include "aco.h"
#include "dynamic_scheduler.h"
#include "evaluator.h"
#include "input_loader.h"
#include "instance.h"
#include "minmin.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ASSERT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "Assertion failed at %s:%d: %s\n", __FILE__, __LINE__, #condition); \
            return 0; \
        } \
    } while (0)

static int approx_equal(double a, double b) {
    return fabs(a - b) < 1e-9;
}

static int setup_base_instance(SchedulingInstance *instance) {
    if (instance_create(instance, "base", 3, 2) != 0) {
        return -1;
    }

    instance->jobs[0].release_time = 0;
    instance->jobs[0].due_date = 4;
    instance->jobs[0].weight = 1.0;
    instance->jobs[0].processing_times[0] = 2;
    instance->jobs[0].processing_times[1] = 3;

    instance->jobs[1].release_time = 0;
    instance->jobs[1].due_date = 5;
    instance->jobs[1].weight = 2.0;
    instance->jobs[1].processing_times[0] = 4;
    instance->jobs[1].processing_times[1] = 1;

    instance->jobs[2].release_time = 1;
    instance->jobs[2].due_date = 6;
    instance->jobs[2].weight = 1.5;
    instance->jobs[2].processing_times[0] = 3;
    instance->jobs[2].processing_times[1] = 2;

    return 0;
}

static int test_evaluator_basics(void) {
    SchedulingInstance instance;
    AssignmentSequence sequence;
    ScheduleMetrics metrics;
    int initial_ready[2] = {0, 0};

    if (setup_base_instance(&instance) != 0) {
        return 0;
    }

    ASSERT_TRUE(assignment_sequence_create(&sequence, 3) == 0);

    sequence.items[0].job_id = 0;
    sequence.items[0].machine_id = 0;
    sequence.items[1].job_id = 1;
    sequence.items[1].machine_id = 1;
    sequence.items[2].job_id = 2;
    sequence.items[2].machine_id = 1;

    ASSERT_TRUE(evaluate_sequence(&instance, &sequence, initial_ready, &metrics, NULL, NULL, NULL) == 0);
    ASSERT_TRUE(metrics.makespan == 3);
    ASSERT_TRUE(approx_equal(metrics.weighted_flow_time, 7.0));
    ASSERT_TRUE(approx_equal(metrics.weighted_tardiness, 0.0));
    ASSERT_TRUE(approx_equal(metrics.weighted_tardy_jobs, 0.0));

    assignment_sequence_destroy(&sequence);
    instance_destroy(&instance);
    return 1;
}

static int test_minmin_validity(void) {
    SchedulingInstance instance;
    AssignmentSequence sequence;
    ScheduleMetrics metrics;
    size_t job_ids[3] = {0, 1, 2};
    int seen[3] = {0, 0, 0};
    size_t i;

    if (setup_base_instance(&instance) != 0) {
        return 0;
    }

    ASSERT_TRUE(solve_minmin(&instance, job_ids, 3, NULL, &sequence) == 0);
    ASSERT_TRUE(sequence.length == 3);

    for (i = 0; i < sequence.length; ++i) {
        ASSERT_TRUE(sequence.items[i].job_id < 3);
        ASSERT_TRUE(sequence.items[i].machine_id < 2);
        ASSERT_TRUE(seen[sequence.items[i].job_id] == 0);
        seen[sequence.items[i].job_id] = 1;
    }

    ASSERT_TRUE(evaluate_sequence(&instance, &sequence, NULL, &metrics, NULL, NULL, NULL) == 0);
    ASSERT_TRUE(metrics.makespan <= 6);

    assignment_sequence_destroy(&sequence);
    instance_destroy(&instance);
    return 1;
}

static int test_aco_validity(void) {
    SchedulingInstance instance;
    AssignmentSequence sequence;
    ScheduleMetrics metrics;
    size_t job_ids[3] = {0, 1, 2};
    int seen[3] = {0, 0, 0};
    AcoParams params;
    size_t i;

    if (setup_base_instance(&instance) != 0) {
        return 0;
    }

    aco_default_params(&params);
    params.ant_count = 12;
    params.iterations = 30;
    params.seed = 42U;
    params.heuristic = HEURISTIC_MON;

    ASSERT_TRUE(solve_aco(
        &instance,
        job_ids,
        3,
        NULL,
        OBJECTIVE_MAKESPAN,
        &params,
        &sequence
    ) == 0);

    ASSERT_TRUE(sequence.length == 3);
    for (i = 0; i < sequence.length; ++i) {
        ASSERT_TRUE(sequence.items[i].job_id < 3);
        ASSERT_TRUE(sequence.items[i].machine_id < 2);
        ASSERT_TRUE(seen[sequence.items[i].job_id] == 0);
        seen[sequence.items[i].job_id] = 1;
    }

    ASSERT_TRUE(evaluate_sequence(&instance, &sequence, NULL, &metrics, NULL, NULL, NULL) == 0);
    ASSERT_TRUE(metrics.makespan <= 6);

    assignment_sequence_destroy(&sequence);
    instance_destroy(&instance);
    return 1;
}

static int test_dynamic_scheduler_runs(void) {
    SchedulingInstance instance;
    ScheduleMetrics minmin_metrics;
    ScheduleMetrics aco_metrics;
    AcoParams params;

    if (instance_create(&instance, "dynamic", 4, 2) != 0) {
        return 0;
    }

    instance.jobs[0].release_time = 0;
    instance.jobs[0].due_date = 20;
    instance.jobs[0].weight = 1.0;
    instance.jobs[0].processing_times[0] = 4;
    instance.jobs[0].processing_times[1] = 6;

    instance.jobs[1].release_time = 0;
    instance.jobs[1].due_date = 20;
    instance.jobs[1].weight = 1.0;
    instance.jobs[1].processing_times[0] = 5;
    instance.jobs[1].processing_times[1] = 2;

    instance.jobs[2].release_time = 3;
    instance.jobs[2].due_date = 20;
    instance.jobs[2].weight = 1.0;
    instance.jobs[2].processing_times[0] = 1;
    instance.jobs[2].processing_times[1] = 4;

    instance.jobs[3].release_time = 5;
    instance.jobs[3].due_date = 20;
    instance.jobs[3].weight = 1.0;
    instance.jobs[3].processing_times[0] = 2;
    instance.jobs[3].processing_times[1] = 2;

    aco_default_params(&params);
    params.ant_count = 10;
    params.iterations = 25;
    params.seed = 7U;

    ASSERT_TRUE(run_dynamic_schedule(
        &instance,
        ALGORITHM_MINMIN,
        OBJECTIVE_MAKESPAN,
        &params,
        &minmin_metrics
    ) == 0);

    ASSERT_TRUE(run_dynamic_schedule(
        &instance,
        ALGORITHM_ACO,
        OBJECTIVE_MAKESPAN,
        &params,
        &aco_metrics
    ) == 0);

    ASSERT_TRUE(minmin_metrics.makespan > 0);
    ASSERT_TRUE(aco_metrics.makespan > 0);

    instance_destroy(&instance);
    return 1;
}

static int test_repository_input_files(void) {
    InputDatasetConfig config;
    SchedulingInstance first_instance;
    SchedulingInstance last_instance;
    ScheduleMetrics minmin_metrics;
    ScheduleMetrics aco_metrics;
    AcoParams params;
    size_t last_set_index;

    ASSERT_TRUE(input_config_load("data/input", &config) == 0);
    ASSERT_TRUE(config.set_count > 0);

    ASSERT_TRUE(input_load_instance(&config, 0, &first_instance) == 0);
    ASSERT_TRUE(first_instance.job_count > 0);
    ASSERT_TRUE(first_instance.machine_count > 0);

    aco_default_params(&params);
    params.ant_count = 6;
    params.iterations = 12;
    params.seed = 123U;

    ASSERT_TRUE(run_static_schedule(
        &first_instance,
        ALGORITHM_MINMIN,
        OBJECTIVE_MAKESPAN,
        &params,
        &minmin_metrics
    ) == 0);
    ASSERT_TRUE(run_static_schedule(
        &first_instance,
        ALGORITHM_ACO,
        OBJECTIVE_MAKESPAN,
        &params,
        &aco_metrics
    ) == 0);
    ASSERT_TRUE(minmin_metrics.makespan > 0);
    ASSERT_TRUE(aco_metrics.makespan > 0);

    ASSERT_TRUE(run_dynamic_schedule(
        &first_instance,
        ALGORITHM_MINMIN,
        OBJECTIVE_WEIGHTED_TARDINESS,
        &params,
        &minmin_metrics
    ) == 0);
    ASSERT_TRUE(run_dynamic_schedule(
        &first_instance,
        ALGORITHM_ACO,
        OBJECTIVE_WEIGHTED_TARDINESS,
        &params,
        &aco_metrics
    ) == 0);

    last_set_index = config.set_count - 1;
    ASSERT_TRUE(input_load_instance(&config, last_set_index, &last_instance) == 0);
    ASSERT_TRUE(run_static_schedule(
        &last_instance,
        ALGORITHM_MINMIN,
        OBJECTIVE_MAKESPAN,
        &params,
        &minmin_metrics
    ) == 0);

    instance_destroy(&first_instance);
    instance_destroy(&last_instance);
    input_config_destroy(&config);
    return 1;
}

int main(void) {
    int passed = 0;
    int total = 0;

    ++total;
    passed += test_evaluator_basics();

    ++total;
    passed += test_minmin_validity();

    ++total;
    passed += test_aco_validity();

    ++total;
    passed += test_dynamic_scheduler_runs();

    ++total;
    passed += test_repository_input_files();

    printf("Tests passed: %d/%d\n", passed, total);
    return passed == total ? 0 : 1;
}
