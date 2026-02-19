#ifndef SCHEDULING_TYPES_H
#define SCHEDULING_TYPES_H

#include <stdbool.h>
#include <stddef.h>

#define SCHED_INSTANCE_NAME_MAX 128

typedef enum {
    OBJECTIVE_MAKESPAN = 0,
    OBJECTIVE_WEIGHTED_FLOW,
    OBJECTIVE_WEIGHTED_TARDINESS,
    OBJECTIVE_WEIGHTED_TARDY_JOBS
} Objective;

typedef enum {
    MODE_STATIC = 0,
    MODE_DYNAMIC
} SchedulingMode;

typedef enum {
    ALGORITHM_MINMIN = 0,
    ALGORITHM_ACO
} AlgorithmKind;

typedef enum {
    HEURISTIC_EDD = 0,
    HEURISTIC_SPT,
    HEURISTIC_WSPT,
    HEURISTIC_MON
} AcoHeuristic;

typedef struct {
    size_t id;
    int release_time;
    int due_date;
    double weight;
    int *processing_times;
} Job;

typedef struct {
    char name[SCHED_INSTANCE_NAME_MAX];
    size_t job_count;
    size_t machine_count;
    Job *jobs;
} SchedulingInstance;

typedef struct {
    size_t job_id;
    size_t machine_id;
} Assignment;

typedef struct {
    Assignment *items;
    size_t length;
} AssignmentSequence;

typedef struct {
    int makespan;
    double weighted_flow_time;
    double weighted_tardiness;
    double weighted_tardy_jobs;
} ScheduleMetrics;

typedef struct {
    size_t ant_count;
    size_t iterations;
    double alpha;
    double beta;
    double rho;
    double tau0;
    double q0;
    unsigned int seed;
    AcoHeuristic heuristic;
} AcoParams;

#endif
