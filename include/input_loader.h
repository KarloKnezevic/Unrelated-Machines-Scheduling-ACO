#ifndef INPUT_LOADER_H
#define INPUT_LOADER_H

#include "scheduling_types.h"

#include <stddef.h>

#define INPUT_PATH_MAX 1024

typedef struct {
    size_t set_count;
    size_t max_jobs;
    size_t max_machines;
    int *jobs_per_set;
    int *machines_per_set;
    char root_dir[INPUT_PATH_MAX];
    char ready_file[INPUT_PATH_MAX];
    char duration_file[INPUT_PATH_MAX];
    char deadline_file[INPUT_PATH_MAX];
    char weight_file[INPUT_PATH_MAX];
} InputDatasetConfig;

int input_config_load(const char *root_dir, InputDatasetConfig *config);

void input_config_destroy(InputDatasetConfig *config);

int input_load_instance(
    const InputDatasetConfig *config,
    size_t set_index,
    SchedulingInstance *instance
);

#endif
