#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "scheduling_types.h"

#include <stddef.h>

int evaluate_sequence(
    const SchedulingInstance *instance,
    const AssignmentSequence *sequence,
    const int *initial_machine_ready,
    ScheduleMetrics *metrics,
    int *start_times,
    int *completion_times,
    size_t *assigned_machine
);

double metrics_value(const ScheduleMetrics *metrics, Objective objective);

const char *objective_name(Objective objective);

Objective objective_from_string(const char *value, int *ok);

SchedulingMode mode_from_string(const char *value, int *ok);

const char *mode_name(SchedulingMode mode);

AcoHeuristic heuristic_from_string(const char *value, int *ok);

const char *heuristic_name(AcoHeuristic heuristic);

#endif
