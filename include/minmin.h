#ifndef MINMIN_H
#define MINMIN_H

#include "scheduling_types.h"

#include <stddef.h>

int solve_minmin(
    const SchedulingInstance *instance,
    const size_t *job_ids,
    size_t job_count,
    const int *initial_machine_ready,
    AssignmentSequence *out_sequence
);

#endif
