#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "scheduling_types.h"

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    const char *input_root;
    size_t set_start;
    size_t set_end;
    SchedulingMode mode;
    Objective objective;
    AcoParams aco_params;
    const char *csv_path;
    bool verbose;
} BenchmarkOptions;

int run_dataset_benchmark(const BenchmarkOptions *options);

#endif
