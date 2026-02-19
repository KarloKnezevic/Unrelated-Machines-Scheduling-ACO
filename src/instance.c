#include "instance.h"

#include <stdlib.h>
#include <string.h>

int instance_create(
    SchedulingInstance *instance,
    const char *name,
    size_t job_count,
    size_t machine_count
) {
    size_t i;

    if (instance == NULL || job_count == 0 || machine_count == 0) {
        return -1;
    }

    memset(instance, 0, sizeof(*instance));
    if (name != NULL) {
        strncpy(instance->name, name, SCHED_INSTANCE_NAME_MAX - 1);
        instance->name[SCHED_INSTANCE_NAME_MAX - 1] = '\0';
    }

    instance->job_count = job_count;
    instance->machine_count = machine_count;
    instance->jobs = (Job *)calloc(job_count, sizeof(Job));
    if (instance->jobs == NULL) {
        return -1;
    }

    for (i = 0; i < job_count; ++i) {
        instance->jobs[i].id = i;
        instance->jobs[i].weight = 1.0;
        instance->jobs[i].processing_times = (int *)calloc(machine_count, sizeof(int));
        if (instance->jobs[i].processing_times == NULL) {
            instance_destroy(instance);
            return -1;
        }
    }

    return 0;
}

void instance_destroy(SchedulingInstance *instance) {
    size_t i;

    if (instance == NULL) {
        return;
    }

    if (instance->jobs != NULL) {
        for (i = 0; i < instance->job_count; ++i) {
            free(instance->jobs[i].processing_times);
            instance->jobs[i].processing_times = NULL;
        }
    }

    free(instance->jobs);
    instance->jobs = NULL;
    instance->job_count = 0;
    instance->machine_count = 0;
    memset(instance->name, 0, sizeof(instance->name));
}

int assignment_sequence_create(AssignmentSequence *sequence, size_t length) {
    if (sequence == NULL || length == 0) {
        return -1;
    }

    sequence->items = (Assignment *)calloc(length, sizeof(Assignment));
    if (sequence->items == NULL) {
        sequence->length = 0;
        return -1;
    }

    sequence->length = length;
    return 0;
}

void assignment_sequence_destroy(AssignmentSequence *sequence) {
    if (sequence == NULL) {
        return;
    }

    free(sequence->items);
    sequence->items = NULL;
    sequence->length = 0;
}
