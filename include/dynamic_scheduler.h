#ifndef DYNAMIC_SCHEDULER_H
#define DYNAMIC_SCHEDULER_H

#include "scheduling_types.h"

int run_static_schedule(
    const SchedulingInstance *instance,
    AlgorithmKind algorithm,
    Objective objective,
    const AcoParams *aco_params,
    ScheduleMetrics *out_metrics
);

int run_dynamic_schedule(
    const SchedulingInstance *instance,
    AlgorithmKind algorithm,
    Objective objective,
    const AcoParams *aco_params,
    ScheduleMetrics *out_metrics
);

#endif
