#ifndef INSTANCE_H
#define INSTANCE_H

#include "scheduling_types.h"

int instance_create(
    SchedulingInstance *instance,
    const char *name,
    size_t job_count,
    size_t machine_count
);

void instance_destroy(SchedulingInstance *instance);

int assignment_sequence_create(AssignmentSequence *sequence, size_t length);

void assignment_sequence_destroy(AssignmentSequence *sequence);

#endif
