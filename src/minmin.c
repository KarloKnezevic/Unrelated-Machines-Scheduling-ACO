#include "minmin.h"
#include "instance.h"

#include <limits.h>
#include <stdlib.h>

int solve_minmin(
    const SchedulingInstance *instance,
    const size_t *job_ids,
    size_t job_count,
    const int *initial_machine_ready,
    AssignmentSequence *out_sequence
) {
    size_t machine_count;
    int *machine_ready = NULL;
    unsigned char *scheduled = NULL;
    size_t step;

    if (instance == NULL || job_ids == NULL || out_sequence == NULL || job_count == 0) {
        return -1;
    }

    machine_count = instance->machine_count;
    if (machine_count == 0 || job_count > instance->job_count) {
        return -1;
    }

    out_sequence->items = NULL;
    out_sequence->length = 0;
    if (assignment_sequence_create(out_sequence, job_count) != 0) {
        return -1;
    }

    machine_ready = (int *)calloc(machine_count, sizeof(int));
    scheduled = (unsigned char *)calloc(job_count, sizeof(unsigned char));
    if (machine_ready == NULL || scheduled == NULL) {
        free(machine_ready);
        free(scheduled);
        assignment_sequence_destroy(out_sequence);
        return -1;
    }

    if (initial_machine_ready != NULL) {
        size_t m;
        for (m = 0; m < machine_count; ++m) {
            machine_ready[m] = initial_machine_ready[m];
        }
    }

    for (step = 0; step < job_count; ++step) {
        size_t chosen_local_job = (size_t)-1;
        size_t chosen_machine = (size_t)-1;
        int chosen_completion = INT_MAX;
        int chosen_processing = INT_MAX;

        size_t local_job_index;
        for (local_job_index = 0; local_job_index < job_count; ++local_job_index) {
            size_t global_job_id;
            const Job *job;
            size_t machine_id;
            size_t best_machine_for_job = (size_t)-1;
            int best_completion_for_job = INT_MAX;
            int best_processing_for_job = INT_MAX;

            if (scheduled[local_job_index] != 0U) {
                continue;
            }

            global_job_id = job_ids[local_job_index];
            if (global_job_id >= instance->job_count) {
                free(machine_ready);
                free(scheduled);
                assignment_sequence_destroy(out_sequence);
                return -1;
            }

            job = &instance->jobs[global_job_id];
            for (machine_id = 0; machine_id < machine_count; ++machine_id) {
                int start_time = machine_ready[machine_id];
                int completion_time;
                int processing_time = job->processing_times[machine_id];

                if (start_time < job->release_time) {
                    start_time = job->release_time;
                }

                completion_time = start_time + processing_time;
                if (completion_time < best_completion_for_job ||
                    (completion_time == best_completion_for_job && processing_time < best_processing_for_job) ||
                    (completion_time == best_completion_for_job &&
                     processing_time == best_processing_for_job &&
                     machine_id < best_machine_for_job)) {
                    best_completion_for_job = completion_time;
                    best_processing_for_job = processing_time;
                    best_machine_for_job = machine_id;
                }
            }

            if (best_machine_for_job == (size_t)-1) {
                free(machine_ready);
                free(scheduled);
                assignment_sequence_destroy(out_sequence);
                return -1;
            }

            if (best_completion_for_job < chosen_completion ||
                (best_completion_for_job == chosen_completion &&
                 best_processing_for_job < chosen_processing) ||
                (best_completion_for_job == chosen_completion &&
                 best_processing_for_job == chosen_processing &&
                 global_job_id < job_ids[chosen_local_job])) {
                chosen_local_job = local_job_index;
                chosen_machine = best_machine_for_job;
                chosen_completion = best_completion_for_job;
                chosen_processing = best_processing_for_job;
            }
        }

        if (chosen_local_job == (size_t)-1 || chosen_machine == (size_t)-1) {
            free(machine_ready);
            free(scheduled);
            assignment_sequence_destroy(out_sequence);
            return -1;
        }

        out_sequence->items[step].job_id = job_ids[chosen_local_job];
        out_sequence->items[step].machine_id = chosen_machine;
        machine_ready[chosen_machine] = chosen_completion;
        scheduled[chosen_local_job] = 1U;
    }

    free(machine_ready);
    free(scheduled);
    return 0;
}
