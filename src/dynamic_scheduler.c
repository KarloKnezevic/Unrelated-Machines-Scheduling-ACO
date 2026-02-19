#include "dynamic_scheduler.h"
#include "aco.h"
#include "evaluator.h"
#include "instance.h"
#include "minmin.h"

#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static int run_algorithm(
    const SchedulingInstance *instance,
    AlgorithmKind algorithm,
    Objective objective,
    const AcoParams *aco_params,
    const size_t *job_ids,
    size_t job_count,
    const int *initial_machine_ready,
    AssignmentSequence *out_sequence
) {
    switch (algorithm) {
        case ALGORITHM_MINMIN:
            return solve_minmin(
                instance,
                job_ids,
                job_count,
                initial_machine_ready,
                out_sequence
            );
        case ALGORITHM_ACO:
            return solve_aco(
                instance,
                job_ids,
                job_count,
                initial_machine_ready,
                objective,
                aco_params,
                out_sequence
            );
        default:
            return -1;
    }
}

static void fill_all_job_ids(const SchedulingInstance *instance, size_t *job_ids) {
    size_t i;
    for (i = 0; i < instance->job_count; ++i) {
        job_ids[i] = i;
    }
}

int run_static_schedule(
    const SchedulingInstance *instance,
    AlgorithmKind algorithm,
    Objective objective,
    const AcoParams *aco_params,
    ScheduleMetrics *out_metrics
) {
    size_t *job_ids = NULL;
    int *initial_machine_ready = NULL;
    AssignmentSequence sequence = {0};
    int result = -1;

    if (instance == NULL || out_metrics == NULL || instance->job_count == 0 || instance->machine_count == 0) {
        return -1;
    }

    job_ids = (size_t *)malloc(instance->job_count * sizeof(size_t));
    initial_machine_ready = (int *)calloc(instance->machine_count, sizeof(int));
    if (job_ids == NULL || initial_machine_ready == NULL) {
        free(job_ids);
        free(initial_machine_ready);
        return -1;
    }

    fill_all_job_ids(instance, job_ids);
    if (run_algorithm(
            instance,
            algorithm,
            objective,
            aco_params,
            job_ids,
            instance->job_count,
            initial_machine_ready,
            &sequence
        ) != 0) {
        free(job_ids);
        free(initial_machine_ready);
        return -1;
    }

    result = evaluate_sequence(
        instance,
        &sequence,
        initial_machine_ready,
        out_metrics,
        NULL,
        NULL,
        NULL
    );

    assignment_sequence_destroy(&sequence);
    free(job_ids);
    free(initial_machine_ready);
    return result;
}

int run_dynamic_schedule(
    const SchedulingInstance *instance,
    AlgorithmKind algorithm,
    Objective objective,
    const AcoParams *aco_params,
    ScheduleMetrics *out_metrics
) {
    size_t n;
    size_t m;
    unsigned char *is_scheduled = NULL;
    int *machine_ready = NULL;
    int *solver_machine_ready = NULL;
    int *initial_machine_ready = NULL;
    size_t *available_jobs = NULL;
    AssignmentSequence final_sequence = {0};
    size_t scheduled_count = 0;
    int current_time = 0;

    if (instance == NULL || out_metrics == NULL || instance->job_count == 0 || instance->machine_count == 0) {
        return -1;
    }

    n = instance->job_count;
    m = instance->machine_count;

    is_scheduled = (unsigned char *)calloc(n, sizeof(unsigned char));
    machine_ready = (int *)calloc(m, sizeof(int));
    solver_machine_ready = (int *)calloc(m, sizeof(int));
    initial_machine_ready = (int *)calloc(m, sizeof(int));
    available_jobs = (size_t *)malloc(n * sizeof(size_t));
    if (is_scheduled == NULL || machine_ready == NULL || solver_machine_ready == NULL ||
        initial_machine_ready == NULL || available_jobs == NULL) {
        free(is_scheduled);
        free(machine_ready);
        free(solver_machine_ready);
        free(initial_machine_ready);
        free(available_jobs);
        return -1;
    }

    if (assignment_sequence_create(&final_sequence, n) != 0) {
        free(is_scheduled);
        free(machine_ready);
        free(solver_machine_ready);
        free(initial_machine_ready);
        free(available_jobs);
        return -1;
    }

    while (scheduled_count < n) {
        int has_idle_machine = 0;
        size_t machine_index;
        size_t available_count = 0;

        for (machine_index = 0; machine_index < m; ++machine_index) {
            if (machine_ready[machine_index] <= current_time) {
                has_idle_machine = 1;
                break;
            }
        }

        if (!has_idle_machine) {
            int next_machine_time = INT_MAX;
            for (machine_index = 0; machine_index < m; ++machine_index) {
                if (machine_ready[machine_index] < next_machine_time) {
                    next_machine_time = machine_ready[machine_index];
                }
            }
            if (next_machine_time == INT_MAX) {
                assignment_sequence_destroy(&final_sequence);
                free(is_scheduled);
                free(machine_ready);
                free(solver_machine_ready);
                free(initial_machine_ready);
                free(available_jobs);
                return -1;
            }
            current_time = next_machine_time;
            continue;
        }

        {
            size_t job_id;
            for (job_id = 0; job_id < n; ++job_id) {
                const Job *job;
                if (is_scheduled[job_id] != 0U) {
                    continue;
                }

                job = &instance->jobs[job_id];
                if (job->release_time <= current_time) {
                    available_jobs[available_count++] = job_id;
                }
            }
        }

        if (available_count == 0) {
            int next_release_time = INT_MAX;
            size_t job_id;

            for (job_id = 0; job_id < n; ++job_id) {
                if (is_scheduled[job_id] == 0U) {
                    int release_time = instance->jobs[job_id].release_time;
                    if (release_time > current_time && release_time < next_release_time) {
                        next_release_time = release_time;
                    }
                }
            }

            if (next_release_time == INT_MAX) {
                int next_machine_time = INT_MAX;
                for (machine_index = 0; machine_index < m; ++machine_index) {
                    if (machine_ready[machine_index] > current_time &&
                        machine_ready[machine_index] < next_machine_time) {
                        next_machine_time = machine_ready[machine_index];
                    }
                }
                if (next_machine_time == INT_MAX) {
                    assignment_sequence_destroy(&final_sequence);
                    free(is_scheduled);
                    free(machine_ready);
                    free(solver_machine_ready);
                    free(initial_machine_ready);
                    free(available_jobs);
                    return -1;
                }
                current_time = next_machine_time;
            } else {
                current_time = next_release_time;
            }
            continue;
        }

        {
            AssignmentSequence candidate_sequence = {0};
            size_t chosen_index = (size_t)-1;
            size_t chosen_job_id;
            size_t chosen_machine_id;
            const Job *chosen_job;
            int start_time;
            int completion_time;

            for (machine_index = 0; machine_index < m; ++machine_index) {
                if (machine_ready[machine_index] <= current_time) {
                    solver_machine_ready[machine_index] = current_time;
                } else {
                    solver_machine_ready[machine_index] = machine_ready[machine_index];
                }
            }

            if (run_algorithm(
                    instance,
                    algorithm,
                    objective,
                    aco_params,
                    available_jobs,
                    available_count,
                    solver_machine_ready,
                    &candidate_sequence
                ) != 0) {
                assignment_sequence_destroy(&final_sequence);
                free(is_scheduled);
                free(machine_ready);
                free(solver_machine_ready);
                free(initial_machine_ready);
                free(available_jobs);
                return -1;
            }

            for (machine_index = 0; machine_index < candidate_sequence.length; ++machine_index) {
                size_t machine_id = candidate_sequence.items[machine_index].machine_id;
                if (machine_id < m && machine_ready[machine_id] <= current_time) {
                    chosen_index = machine_index;
                    break;
                }
            }

            if (chosen_index == (size_t)-1) {
                double best_completion = 1e300;
                size_t best_job = (size_t)-1;
                size_t best_machine = (size_t)-1;
                size_t available_index;

                for (available_index = 0; available_index < available_count; ++available_index) {
                    size_t job_id = available_jobs[available_index];
                    const Job *job = &instance->jobs[job_id];
                    size_t machine_id;

                    for (machine_id = 0; machine_id < m; ++machine_id) {
                        int machine_time = machine_ready[machine_id];
                        int completion;

                        if (machine_time > current_time) {
                            continue;
                        }

                        if (machine_time < current_time) {
                            machine_time = current_time;
                        }
                        if (machine_time < job->release_time) {
                            machine_time = job->release_time;
                        }

                        completion = machine_time + job->processing_times[machine_id];
                        if ((double)completion < best_completion) {
                            best_completion = (double)completion;
                            best_job = job_id;
                            best_machine = machine_id;
                        }
                    }
                }

                if (best_job == (size_t)-1 || best_machine == (size_t)-1) {
                    assignment_sequence_destroy(&candidate_sequence);
                    assignment_sequence_destroy(&final_sequence);
                    free(is_scheduled);
                    free(machine_ready);
                    free(solver_machine_ready);
                    free(initial_machine_ready);
                    free(available_jobs);
                    return -1;
                }

                chosen_job_id = best_job;
                chosen_machine_id = best_machine;
            } else {
                chosen_job_id = candidate_sequence.items[chosen_index].job_id;
                chosen_machine_id = candidate_sequence.items[chosen_index].machine_id;
            }

            if (chosen_job_id >= n || chosen_machine_id >= m || is_scheduled[chosen_job_id] != 0U) {
                assignment_sequence_destroy(&candidate_sequence);
                assignment_sequence_destroy(&final_sequence);
                free(is_scheduled);
                free(machine_ready);
                free(solver_machine_ready);
                free(initial_machine_ready);
                free(available_jobs);
                return -1;
            }

            chosen_job = &instance->jobs[chosen_job_id];
            start_time = machine_ready[chosen_machine_id];
            if (start_time < current_time) {
                start_time = current_time;
            }
            if (start_time < chosen_job->release_time) {
                start_time = chosen_job->release_time;
            }

            completion_time = start_time + chosen_job->processing_times[chosen_machine_id];
            machine_ready[chosen_machine_id] = completion_time;
            is_scheduled[chosen_job_id] = 1U;

            final_sequence.items[scheduled_count].job_id = chosen_job_id;
            final_sequence.items[scheduled_count].machine_id = chosen_machine_id;
            ++scheduled_count;

            assignment_sequence_destroy(&candidate_sequence);
        }
    }

    final_sequence.length = scheduled_count;

    if (evaluate_sequence(
            instance,
            &final_sequence,
            initial_machine_ready,
            out_metrics,
            NULL,
            NULL,
            NULL
        ) != 0) {
        assignment_sequence_destroy(&final_sequence);
        free(is_scheduled);
        free(machine_ready);
        free(solver_machine_ready);
        free(initial_machine_ready);
        free(available_jobs);
        return -1;
    }

    assignment_sequence_destroy(&final_sequence);
    free(is_scheduled);
    free(machine_ready);
    free(solver_machine_ready);
    free(initial_machine_ready);
    free(available_jobs);
    return 0;
}
