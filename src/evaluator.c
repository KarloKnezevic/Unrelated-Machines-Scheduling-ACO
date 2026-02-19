#include "evaluator.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static int string_equals_ci(const char *left, const char *right) {
    while (*left != '\0' && *right != '\0') {
        if (tolower((unsigned char)*left) != tolower((unsigned char)*right)) {
            return 0;
        }
        ++left;
        ++right;
    }

    return *left == '\0' && *right == '\0';
}

int evaluate_sequence(
    const SchedulingInstance *instance,
    const AssignmentSequence *sequence,
    const int *initial_machine_ready,
    ScheduleMetrics *metrics,
    int *start_times,
    int *completion_times,
    size_t *assigned_machine
) {
    size_t i;
    size_t n;
    size_t m;
    int makespan = 0;
    double weighted_flow = 0.0;
    double weighted_tardiness = 0.0;
    double weighted_tardy_jobs = 0.0;
    int *machine_ready = NULL;
    unsigned char *seen = NULL;

    if (instance == NULL || sequence == NULL || metrics == NULL) {
        return -1;
    }

    n = instance->job_count;
    m = instance->machine_count;
    if (sequence->length == 0 || sequence->length > n) {
        return -1;
    }

    machine_ready = (int *)calloc(m, sizeof(int));
    seen = (unsigned char *)calloc(n, sizeof(unsigned char));
    if (machine_ready == NULL || seen == NULL) {
        free(machine_ready);
        free(seen);
        return -1;
    }

    if (initial_machine_ready != NULL) {
        for (i = 0; i < m; ++i) {
            machine_ready[i] = initial_machine_ready[i];
            if (machine_ready[i] > makespan) {
                makespan = machine_ready[i];
            }
        }
    }

    if (start_times != NULL) {
        for (i = 0; i < n; ++i) {
            start_times[i] = -1;
        }
    }

    if (completion_times != NULL) {
        for (i = 0; i < n; ++i) {
            completion_times[i] = -1;
        }
    }

    if (assigned_machine != NULL) {
        for (i = 0; i < n; ++i) {
            assigned_machine[i] = (size_t)-1;
        }
    }

    for (i = 0; i < sequence->length; ++i) {
        const Assignment assignment = sequence->items[i];
        const Job *job;
        int start;
        int completion;
        double tardiness;

        if (assignment.job_id >= n || assignment.machine_id >= m) {
            free(machine_ready);
            free(seen);
            return -1;
        }

        if (seen[assignment.job_id] != 0U) {
            free(machine_ready);
            free(seen);
            return -1;
        }
        seen[assignment.job_id] = 1U;

        job = &instance->jobs[assignment.job_id];
        start = machine_ready[assignment.machine_id];
        if (start < job->release_time) {
            start = job->release_time;
        }

        completion = start + job->processing_times[assignment.machine_id];
        machine_ready[assignment.machine_id] = completion;

        if (completion > makespan) {
            makespan = completion;
        }

        weighted_flow += job->weight * (double)(completion - job->release_time);
        tardiness = (double)(completion - job->due_date);
        if (tardiness < 0.0) {
            tardiness = 0.0;
        }
        weighted_tardiness += job->weight * tardiness;
        if (completion > job->due_date) {
            weighted_tardy_jobs += job->weight;
        }

        if (start_times != NULL) {
            start_times[assignment.job_id] = start;
        }

        if (completion_times != NULL) {
            completion_times[assignment.job_id] = completion;
        }

        if (assigned_machine != NULL) {
            assigned_machine[assignment.job_id] = assignment.machine_id;
        }
    }

    metrics->makespan = makespan;
    metrics->weighted_flow_time = weighted_flow;
    metrics->weighted_tardiness = weighted_tardiness;
    metrics->weighted_tardy_jobs = weighted_tardy_jobs;

    free(machine_ready);
    free(seen);
    return 0;
}

double metrics_value(const ScheduleMetrics *metrics, Objective objective) {
    if (metrics == NULL) {
        return 0.0;
    }

    switch (objective) {
        case OBJECTIVE_MAKESPAN:
            return (double)metrics->makespan;
        case OBJECTIVE_WEIGHTED_FLOW:
            return metrics->weighted_flow_time;
        case OBJECTIVE_WEIGHTED_TARDINESS:
            return metrics->weighted_tardiness;
        case OBJECTIVE_WEIGHTED_TARDY_JOBS:
            return metrics->weighted_tardy_jobs;
        default:
            return (double)metrics->makespan;
    }
}

const char *objective_name(Objective objective) {
    switch (objective) {
        case OBJECTIVE_MAKESPAN:
            return "makespan";
        case OBJECTIVE_WEIGHTED_FLOW:
            return "weighted_flow";
        case OBJECTIVE_WEIGHTED_TARDINESS:
            return "weighted_tardiness";
        case OBJECTIVE_WEIGHTED_TARDY_JOBS:
            return "weighted_tardy_jobs";
        default:
            return "unknown";
    }
}

Objective objective_from_string(const char *value, int *ok) {
    if (ok != NULL) {
        *ok = 1;
    }

    if (value == NULL) {
        if (ok != NULL) {
            *ok = 0;
        }
        return OBJECTIVE_MAKESPAN;
    }

    if (string_equals_ci(value, "makespan") || string_equals_ci(value, "cmax")) {
        return OBJECTIVE_MAKESPAN;
    }
    if (string_equals_ci(value, "weighted_flow") || string_equals_ci(value, "fw")) {
        return OBJECTIVE_WEIGHTED_FLOW;
    }
    if (string_equals_ci(value, "weighted_tardiness") || string_equals_ci(value, "tw")) {
        return OBJECTIVE_WEIGHTED_TARDINESS;
    }
    if (string_equals_ci(value, "weighted_tardy_jobs") || string_equals_ci(value, "uw")) {
        return OBJECTIVE_WEIGHTED_TARDY_JOBS;
    }

    if (ok != NULL) {
        *ok = 0;
    }
    return OBJECTIVE_MAKESPAN;
}

SchedulingMode mode_from_string(const char *value, int *ok) {
    if (ok != NULL) {
        *ok = 1;
    }

    if (value == NULL) {
        if (ok != NULL) {
            *ok = 0;
        }
        return MODE_STATIC;
    }

    if (string_equals_ci(value, "static")) {
        return MODE_STATIC;
    }

    if (string_equals_ci(value, "dynamic")) {
        return MODE_DYNAMIC;
    }

    if (ok != NULL) {
        *ok = 0;
    }
    return MODE_STATIC;
}

const char *mode_name(SchedulingMode mode) {
    return mode == MODE_DYNAMIC ? "dynamic" : "static";
}

AcoHeuristic heuristic_from_string(const char *value, int *ok) {
    if (ok != NULL) {
        *ok = 1;
    }

    if (value == NULL) {
        if (ok != NULL) {
            *ok = 0;
        }
        return HEURISTIC_MON;
    }

    if (string_equals_ci(value, "edd")) {
        return HEURISTIC_EDD;
    }
    if (string_equals_ci(value, "spt")) {
        return HEURISTIC_SPT;
    }
    if (string_equals_ci(value, "wspt")) {
        return HEURISTIC_WSPT;
    }
    if (string_equals_ci(value, "mon") || string_equals_ci(value, "montagne")) {
        return HEURISTIC_MON;
    }

    if (ok != NULL) {
        *ok = 0;
    }
    return HEURISTIC_MON;
}

const char *heuristic_name(AcoHeuristic heuristic) {
    switch (heuristic) {
        case HEURISTIC_EDD:
            return "EDD";
        case HEURISTIC_SPT:
            return "SPT";
        case HEURISTIC_WSPT:
            return "WSPT";
        case HEURISTIC_MON:
            return "MON";
        default:
            return "MON";
    }
}
