#ifndef ACO_H
#define ACO_H

#include "scheduling_types.h"

#include <stddef.h>

void aco_default_params(AcoParams *params);

int solve_aco(
    const SchedulingInstance *instance,
    const size_t *job_ids,
    size_t job_count,
    const int *initial_machine_ready,
    Objective objective,
    const AcoParams *params,
    AssignmentSequence *out_sequence
);

#endif
