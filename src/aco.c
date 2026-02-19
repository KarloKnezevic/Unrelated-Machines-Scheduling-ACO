#include "aco.h"
#include "evaluator.h"
#include "instance.h"
#include "minmin.h"
#include "rng.h"

#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define EPSILON 1e-12

static double positive_floor(double value) {
    return value > EPSILON ? value : EPSILON;
}

static int sequence_clone(AssignmentSequence *destination, const AssignmentSequence *source) {
    if (destination->items == NULL || destination->length != source->length) {
        assignment_sequence_destroy(destination);
        if (assignment_sequence_create(destination, source->length) != 0) {
            return -1;
        }
    }

    memcpy(destination->items, source->items, source->length * sizeof(Assignment));
    return 0;
}

static int evaluate_objective(
    const SchedulingInstance *instance,
    const AssignmentSequence *sequence,
    const int *initial_machine_ready,
    Objective objective,
    double *value
) {
    ScheduleMetrics metrics;
    if (evaluate_sequence(
            instance,
            sequence,
            initial_machine_ready,
            &metrics,
            NULL,
            NULL,
            NULL
        ) != 0) {
        return -1;
    }
    *value = metrics_value(&metrics, objective);
    return 0;
}

static int local_search_improve(
    const SchedulingInstance *instance,
    AssignmentSequence *sequence,
    const int *initial_machine_ready,
    Objective objective
) {
    double best_value;
    size_t i;

    if (evaluate_objective(
            instance,
            sequence,
            initial_machine_ready,
            objective,
            &best_value
        ) != 0) {
        return -1;
    }

    for (i = 0; i < sequence->length; ++i) {
        size_t original_machine = sequence->items[i].machine_id;
        size_t best_machine = original_machine;
        size_t machine_id;
        double best_for_position = best_value;

        for (machine_id = 0; machine_id < instance->machine_count; ++machine_id) {
            double candidate_value;
            if (machine_id == original_machine) {
                continue;
            }

            sequence->items[i].machine_id = machine_id;
            if (evaluate_objective(
                    instance,
                    sequence,
                    initial_machine_ready,
                    objective,
                    &candidate_value
                ) == 0 &&
                candidate_value + EPSILON < best_for_position) {
                best_for_position = candidate_value;
                best_machine = machine_id;
            }
        }

        sequence->items[i].machine_id = best_machine;
        best_value = best_for_position;
    }

    for (i = 0; i + 1 < sequence->length; ++i) {
        Assignment left = sequence->items[i];
        Assignment right = sequence->items[i + 1];
        double candidate_value;

        sequence->items[i] = right;
        sequence->items[i + 1] = left;
        if (evaluate_objective(
                instance,
                sequence,
                initial_machine_ready,
                objective,
                &candidate_value
            ) == 0 &&
            candidate_value + EPSILON < best_value) {
            best_value = candidate_value;
        } else {
            sequence->items[i] = left;
            sequence->items[i + 1] = right;
        }
    }

    return 0;
}

static double objective_cost(
    Objective objective,
    const Job *job,
    int completion_time
) {
    switch (objective) {
        case OBJECTIVE_MAKESPAN:
            return (double)completion_time;
        case OBJECTIVE_WEIGHTED_FLOW:
            return job->weight * (double)(completion_time - job->release_time);
        case OBJECTIVE_WEIGHTED_TARDINESS: {
            double tardiness = (double)(completion_time - job->due_date);
            if (tardiness < 0.0) {
                tardiness = 0.0;
            }
            return job->weight * tardiness;
        }
        case OBJECTIVE_WEIGHTED_TARDY_JOBS:
            return completion_time > job->due_date ? job->weight : 0.1 * job->weight;
        default:
            return (double)completion_time;
    }
}

static double heuristic_bias(
    AcoHeuristic heuristic,
    const Job *job,
    int processing_time,
    int start_time,
    double processing_reference
) {
    double weight = job->weight > 0.0 ? job->weight : 1.0;

    switch (heuristic) {
        case HEURISTIC_EDD:
            return 1.0 / (1.0 + (double)(job->due_date > 0 ? job->due_date : 0));
        case HEURISTIC_SPT:
            return 1.0 / (1.0 + (double)processing_time);
        case HEURISTIC_WSPT:
            return weight / (1.0 + (double)processing_time);
        case HEURISTIC_MON:
        default: {
            double wait_time = (double)(start_time - job->release_time);
            double urgency = 1.0 - ((double)job->due_date / (processing_reference + 1.0));
            double wspt_like;

            if (wait_time < 0.0) {
                wait_time = 0.0;
            }
            if (urgency < 0.05) {
                urgency = 0.05;
            }

            wspt_like = weight / (1.0 + (double)processing_time + wait_time);
            return wspt_like * urgency;
        }
    }
}

static int pick_random_unscheduled_job(
    RandomState *rng,
    const unsigned char *scheduled,
    size_t job_count,
    size_t *selected_job
) {
    size_t remaining = 0;
    size_t i;
    size_t pick;

    for (i = 0; i < job_count; ++i) {
        if (scheduled[i] == 0U) {
            ++remaining;
        }
    }

    if (remaining == 0) {
        return -1;
    }

    pick = (size_t)(rng_next_u32(rng) % (uint32_t)remaining);
    for (i = 0; i < job_count; ++i) {
        if (scheduled[i] == 0U) {
            if (pick == 0) {
                *selected_job = i;
                return 0;
            }
            --pick;
        }
    }

    return -1;
}

void aco_default_params(AcoParams *params) {
    if (params == NULL) {
        return;
    }

    params->ant_count = 24;
    params->iterations = 80;
    params->alpha = 1.0;
    params->beta = 3.0;
    params->rho = 0.10;
    params->tau0 = 1.0;
    params->q0 = 0.90;
    params->seed = 12345U;
    params->heuristic = HEURISTIC_MON;
}

int solve_aco(
    const SchedulingInstance *instance,
    const size_t *job_ids,
    size_t job_count,
    const int *initial_machine_ready,
    Objective objective,
    const AcoParams *params,
    AssignmentSequence *out_sequence
) {
    AcoParams effective_params;
    size_t machine_count;
    size_t n;
    size_t edge_count;
    double *pheromone = NULL;
    int *machine_ready = NULL;
    unsigned char *scheduled = NULL;
    double processing_reference = 0.0;
    RandomState rng;
    AssignmentSequence global_best_sequence = {0};
    double global_best_value = HUGE_VAL;
    size_t *job_to_local = NULL;
    size_t iteration;
    AssignmentSequence seeded_sequence = {0};

    if (instance == NULL || job_ids == NULL || out_sequence == NULL || job_count == 0) {
        return -1;
    }

    machine_count = instance->machine_count;
    n = instance->job_count;
    if (machine_count == 0 || job_count > n) {
        return -1;
    }

    aco_default_params(&effective_params);
    if (params != NULL) {
        effective_params = *params;
    }

    if (effective_params.ant_count == 0) {
        effective_params.ant_count = 24;
    }
    if (effective_params.iterations == 0) {
        effective_params.iterations = 80;
    }
    if (effective_params.alpha <= 0.0) {
        effective_params.alpha = 1.0;
    }
    if (effective_params.beta <= 0.0) {
        effective_params.beta = 3.0;
    }
    if (effective_params.rho <= 0.0 || effective_params.rho > 1.0) {
        effective_params.rho = 0.10;
    }
    if (effective_params.tau0 <= 0.0) {
        effective_params.tau0 = 1.0;
    }
    if (effective_params.q0 < 0.0 || effective_params.q0 > 1.0) {
        effective_params.q0 = 0.90;
    }

    edge_count = job_count * machine_count;
    pheromone = (double *)calloc(edge_count, sizeof(double));
    machine_ready = (int *)calloc(machine_count, sizeof(int));
    scheduled = (unsigned char *)calloc(job_count, sizeof(unsigned char));
    job_to_local = (size_t *)malloc(n * sizeof(size_t));
    if (pheromone == NULL || machine_ready == NULL || scheduled == NULL || job_to_local == NULL) {
        free(pheromone);
        free(machine_ready);
        free(scheduled);
        free(job_to_local);
        return -1;
    }

    out_sequence->items = NULL;
    out_sequence->length = 0;

    {
        size_t i;
        for (i = 0; i < edge_count; ++i) {
            pheromone[i] = effective_params.tau0;
        }
        for (i = 0; i < n; ++i) {
            job_to_local[i] = (size_t)-1;
        }
        for (i = 0; i < job_count; ++i) {
            size_t global_job_id = job_ids[i];
            const Job *job;
            size_t machine_id;
            int min_processing = 0;

            if (global_job_id >= n || job_to_local[global_job_id] != (size_t)-1) {
                free(pheromone);
                free(machine_ready);
                free(scheduled);
                free(job_to_local);
                return -1;
            }

            job_to_local[global_job_id] = i;
            job = &instance->jobs[global_job_id];
            min_processing = job->processing_times[0];
            for (machine_id = 1; machine_id < machine_count; ++machine_id) {
                if (job->processing_times[machine_id] < min_processing) {
                    min_processing = job->processing_times[machine_id];
                }
            }
            processing_reference += (double)min_processing;
        }
    }

    if (processing_reference <= 0.0) {
        processing_reference = (double)job_count;
    }

    if (assignment_sequence_create(&global_best_sequence, job_count) != 0) {
        free(pheromone);
        free(machine_ready);
        free(scheduled);
        free(job_to_local);
        return -1;
    }

    rng_seed(&rng, (uint64_t)(effective_params.seed == 0U ? 12345U : effective_params.seed));

    if (solve_minmin(
            instance,
            job_ids,
            job_count,
            initial_machine_ready,
            &seeded_sequence
        ) == 0) {
        if (local_search_improve(
                instance,
                &seeded_sequence,
                initial_machine_ready,
                objective
            ) == 0 &&
            evaluate_objective(
                instance,
                &seeded_sequence,
                initial_machine_ready,
                objective,
                &global_best_value
            ) == 0) {
            size_t i;
            double delta;
            (void)sequence_clone(&global_best_sequence, &seeded_sequence);
            delta = 1.0 / (global_best_value + EPSILON);
            for (i = 0; i < seeded_sequence.length; ++i) {
                const Assignment assignment = seeded_sequence.items[i];
                size_t local_job_index = job_to_local[assignment.job_id];
                if (local_job_index == (size_t)-1 || assignment.machine_id >= machine_count) {
                    continue;
                }
                pheromone[local_job_index * machine_count + assignment.machine_id] += delta;
            }
        }
        assignment_sequence_destroy(&seeded_sequence);
    }

    for (iteration = 0; iteration < effective_params.iterations; ++iteration) {
        AssignmentSequence iteration_best_sequence = {0};
        double iteration_best_value = HUGE_VAL;
        size_t ant_index;

        for (ant_index = 0; ant_index < effective_params.ant_count; ++ant_index) {
            AssignmentSequence ant_sequence = {0};
            double ant_value;
            size_t step;

            if (assignment_sequence_create(&ant_sequence, job_count) != 0) {
                assignment_sequence_destroy(&iteration_best_sequence);
                assignment_sequence_destroy(&global_best_sequence);
                free(pheromone);
                free(machine_ready);
                free(scheduled);
                free(job_to_local);
                return -1;
            }

            if (initial_machine_ready != NULL) {
                size_t m;
                for (m = 0; m < machine_count; ++m) {
                    machine_ready[m] = initial_machine_ready[m];
                }
            } else {
                memset(machine_ready, 0, machine_count * sizeof(int));
            }
            memset(scheduled, 0, job_count * sizeof(unsigned char));

            for (step = 0; step < job_count; ++step) {
                size_t selected_local_job = (size_t)-1;
                size_t selected_machine = (size_t)-1;
                double sum_scores = 0.0;
                double best_score = -1.0;
                size_t local_job_index;

                for (local_job_index = 0; local_job_index < job_count; ++local_job_index) {
                    size_t global_job_id;
                    const Job *job;
                    size_t machine_id;

                    if (scheduled[local_job_index] != 0U) {
                        continue;
                    }

                    global_job_id = job_ids[local_job_index];
                    job = &instance->jobs[global_job_id];

                    for (machine_id = 0; machine_id < machine_count; ++machine_id) {
                        size_t edge_index = local_job_index * machine_count + machine_id;
                        int start_time = machine_ready[machine_id];
                        int completion_time;
                        int processing_time = job->processing_times[machine_id];
                        double objective_component;
                        double eta;
                        double tau;
                        double score;

                        if (start_time < job->release_time) {
                            start_time = job->release_time;
                        }
                        completion_time = start_time + processing_time;

                        objective_component = 1.0 /
                            (1.0 + objective_cost(objective, job, completion_time));
                        eta = objective_component *
                            heuristic_bias(
                                effective_params.heuristic,
                                job,
                                processing_time,
                                start_time,
                                processing_reference
                            );
                        tau = positive_floor(pheromone[edge_index]);
                        eta = positive_floor(eta);
                        score = pow(tau, effective_params.alpha) * pow(eta, effective_params.beta);

                        if (score > best_score) {
                            best_score = score;
                            selected_local_job = local_job_index;
                            selected_machine = machine_id;
                        }
                        sum_scores += score;
                    }
                }

                if (selected_local_job == (size_t)-1 || selected_machine == (size_t)-1) {
                    assignment_sequence_destroy(&ant_sequence);
                    assignment_sequence_destroy(&iteration_best_sequence);
                    assignment_sequence_destroy(&global_best_sequence);
                    free(pheromone);
                    free(machine_ready);
                    free(scheduled);
                    free(job_to_local);
                    return -1;
                }

                if (rng_next_double(&rng) > effective_params.q0) {
                    if (sum_scores > EPSILON) {
                        double threshold = rng_next_double(&rng) * sum_scores;
                        double cumulative = 0.0;
                        int found = 0;
                        size_t local_job_index2;

                        for (local_job_index2 = 0; local_job_index2 < job_count && !found; ++local_job_index2) {
                            size_t global_job_id;
                            const Job *job;
                            size_t machine_id;

                            if (scheduled[local_job_index2] != 0U) {
                                continue;
                            }

                            global_job_id = job_ids[local_job_index2];
                            job = &instance->jobs[global_job_id];

                            for (machine_id = 0; machine_id < machine_count; ++machine_id) {
                                size_t edge_index = local_job_index2 * machine_count + machine_id;
                                int start_time = machine_ready[machine_id];
                                int completion_time;
                                int processing_time = job->processing_times[machine_id];
                                double objective_component;
                                double eta;
                                double tau;
                                double score;

                                if (start_time < job->release_time) {
                                    start_time = job->release_time;
                                }
                                completion_time = start_time + processing_time;

                                objective_component = 1.0 /
                                    (1.0 + objective_cost(objective, job, completion_time));
                                eta = objective_component *
                                    heuristic_bias(
                                        effective_params.heuristic,
                                        job,
                                        processing_time,
                                        start_time,
                                        processing_reference
                                    );
                                tau = positive_floor(pheromone[edge_index]);
                                eta = positive_floor(eta);
                                score = pow(tau, effective_params.alpha) * pow(eta, effective_params.beta);

                                cumulative += score;
                                if (cumulative >= threshold) {
                                    selected_local_job = local_job_index2;
                                    selected_machine = machine_id;
                                    found = 1;
                                    break;
                                }
                            }
                        }
                    } else {
                        size_t fallback_job;
                        if (pick_random_unscheduled_job(&rng, scheduled, job_count, &fallback_job) == 0) {
                            selected_local_job = fallback_job;
                            selected_machine = (size_t)(rng_next_u32(&rng) % (uint32_t)machine_count);
                        }
                    }
                }

                {
                    size_t global_job_id = job_ids[selected_local_job];
                    const Job *job = &instance->jobs[global_job_id];
                    int start_time = machine_ready[selected_machine];
                    int completion_time;
                    size_t edge_index = selected_local_job * machine_count + selected_machine;

                    if (start_time < job->release_time) {
                        start_time = job->release_time;
                    }

                    completion_time = start_time + job->processing_times[selected_machine];
                    machine_ready[selected_machine] = completion_time;
                    scheduled[selected_local_job] = 1U;
                    ant_sequence.items[step].job_id = global_job_id;
                    ant_sequence.items[step].machine_id = selected_machine;

                    pheromone[edge_index] =
                        (1.0 - effective_params.rho) * pheromone[edge_index] +
                        effective_params.rho * effective_params.tau0;
                }
            }

            if (local_search_improve(
                    instance,
                    &ant_sequence,
                    initial_machine_ready,
                    objective
                ) != 0 ||
                evaluate_objective(
                    instance,
                    &ant_sequence,
                    initial_machine_ready,
                    objective,
                    &ant_value
                ) != 0) {
                assignment_sequence_destroy(&ant_sequence);
                assignment_sequence_destroy(&iteration_best_sequence);
                assignment_sequence_destroy(&global_best_sequence);
                free(pheromone);
                free(machine_ready);
                free(scheduled);
                free(job_to_local);
                return -1;
            }

            if (ant_value < iteration_best_value) {
                iteration_best_value = ant_value;
                if (sequence_clone(&iteration_best_sequence, &ant_sequence) != 0) {
                    assignment_sequence_destroy(&ant_sequence);
                    assignment_sequence_destroy(&iteration_best_sequence);
                    assignment_sequence_destroy(&global_best_sequence);
                    free(pheromone);
                    free(machine_ready);
                    free(scheduled);
                    free(job_to_local);
                    return -1;
                }
            }

            if (ant_value < global_best_value) {
                global_best_value = ant_value;
                if (sequence_clone(&global_best_sequence, &ant_sequence) != 0) {
                    assignment_sequence_destroy(&ant_sequence);
                    assignment_sequence_destroy(&iteration_best_sequence);
                    assignment_sequence_destroy(&global_best_sequence);
                    free(pheromone);
                    free(machine_ready);
                    free(scheduled);
                    free(job_to_local);
                    return -1;
                }
            }

            assignment_sequence_destroy(&ant_sequence);
        }

        {
            double delta = 1.0 / (global_best_value + EPSILON);
            size_t edge_index;
            size_t assignment_index;

            for (edge_index = 0; edge_index < edge_count; ++edge_index) {
                pheromone[edge_index] = (1.0 - effective_params.rho) * pheromone[edge_index];
                pheromone[edge_index] = positive_floor(pheromone[edge_index]);
            }

            for (assignment_index = 0; assignment_index < global_best_sequence.length; ++assignment_index) {
                const Assignment assignment = global_best_sequence.items[assignment_index];
                size_t local_job_index = job_to_local[assignment.job_id];

                if (local_job_index == (size_t)-1 || assignment.machine_id >= machine_count) {
                    continue;
                }

                edge_index = local_job_index * machine_count + assignment.machine_id;
                pheromone[edge_index] += effective_params.rho * delta;
            }
        }

        assignment_sequence_destroy(&iteration_best_sequence);
    }

    if (global_best_sequence.length != job_count) {
        assignment_sequence_destroy(&global_best_sequence);
        free(pheromone);
        free(machine_ready);
        free(scheduled);
        free(job_to_local);
        return -1;
    }

    if (assignment_sequence_create(out_sequence, global_best_sequence.length) != 0) {
        assignment_sequence_destroy(&global_best_sequence);
        free(pheromone);
        free(machine_ready);
        free(scheduled);
        free(job_to_local);
        return -1;
    }
    memcpy(
        out_sequence->items,
        global_best_sequence.items,
        global_best_sequence.length * sizeof(Assignment)
    );

    assignment_sequence_destroy(&global_best_sequence);
    free(pheromone);
    free(machine_ready);
    free(scheduled);
    free(job_to_local);
    return 0;
}
