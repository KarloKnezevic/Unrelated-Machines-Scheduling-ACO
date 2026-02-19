#include "input_loader.h"
#include "instance.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TOKEN_MAX 256

static int join_path(char *buffer, size_t buffer_size, const char *root, const char *filename) {
    int written = snprintf(buffer, buffer_size, "%s/%s", root, filename);
    return (written >= 0 && (size_t)written < buffer_size) ? 0 : -1;
}

static int parse_header_file(
    const char *fitness_path,
    InputDatasetConfig *config,
    char *ready_name,
    char *duration_name,
    char *deadline_name,
    char *weight_name
) {
    FILE *file = fopen(fitness_path, "r");
    char token[TOKEN_MAX];
    int set_count = -1;
    int jobs_loaded = 0;
    int machines_loaded = 0;

    if (file == NULL) {
        return -1;
    }

    while (fscanf(file, "%255s", token) == 1) {
        if (token[0] == '/' && token[1] == '/') {
            int ch;
            do {
                ch = fgetc(file);
            } while (ch != '\n' && ch != EOF);
            continue;
        }

        if (strcmp(token, "sets") == 0) {
            if (fscanf(file, "%d", &set_count) != 1 || set_count <= 0) {
                fclose(file);
                return -1;
            }
            config->set_count = (size_t)set_count;
            continue;
        }

        if (strcmp(token, "max_jobs") == 0) {
            int value;
            if (fscanf(file, "%d", &value) != 1 || value <= 0) {
                fclose(file);
                return -1;
            }
            config->max_jobs = (size_t)value;
            continue;
        }

        if (strcmp(token, "max_machines") == 0) {
            int value;
            if (fscanf(file, "%d", &value) != 1 || value <= 0) {
                fclose(file);
                return -1;
            }
            config->max_machines = (size_t)value;
            continue;
        }

        if (strcmp(token, "jobs") == 0) {
            size_t i;
            if (config->set_count == 0) {
                fclose(file);
                return -1;
            }

            config->jobs_per_set = (int *)calloc(config->set_count, sizeof(int));
            if (config->jobs_per_set == NULL) {
                fclose(file);
                return -1;
            }

            for (i = 0; i < config->set_count; ++i) {
                if (fscanf(file, "%d", &config->jobs_per_set[i]) != 1 || config->jobs_per_set[i] <= 0) {
                    fclose(file);
                    return -1;
                }
            }
            jobs_loaded = 1;
            continue;
        }

        if (strcmp(token, "machines") == 0) {
            size_t i;
            if (config->set_count == 0) {
                fclose(file);
                return -1;
            }

            config->machines_per_set = (int *)calloc(config->set_count, sizeof(int));
            if (config->machines_per_set == NULL) {
                fclose(file);
                return -1;
            }

            for (i = 0; i < config->set_count; ++i) {
                if (fscanf(file, "%d", &config->machines_per_set[i]) != 1 || config->machines_per_set[i] <= 0) {
                    fclose(file);
                    return -1;
                }
            }
            machines_loaded = 1;
            continue;
        }

        if (strcmp(token, "ready") == 0) {
            if (fscanf(file, "%255s", ready_name) != 1) {
                fclose(file);
                return -1;
            }
            continue;
        }

        if (strcmp(token, "duration") == 0) {
            if (fscanf(file, "%255s", duration_name) != 1) {
                fclose(file);
                return -1;
            }
            continue;
        }

        if (strcmp(token, "deadline") == 0) {
            if (fscanf(file, "%255s", deadline_name) != 1) {
                fclose(file);
                return -1;
            }
            continue;
        }

        if (strcmp(token, "weight_F") == 0) {
            if (fscanf(file, "%255s", weight_name) != 1) {
                fclose(file);
                return -1;
            }
            continue;
        }
    }

    fclose(file);

    if (config->set_count == 0 || config->max_jobs == 0 || config->max_machines == 0) {
        return -1;
    }

    if (!jobs_loaded || !machines_loaded) {
        return -1;
    }

    if (ready_name[0] == '\0' || duration_name[0] == '\0' || deadline_name[0] == '\0' || weight_name[0] == '\0') {
        return -1;
    }

    return 0;
}

static int read_row_ints(
    const char *path,
    size_t row_index,
    int *output,
    size_t output_count
) {
    FILE *file = fopen(path, "r");
    int rows;
    int cols;
    size_t row;
    size_t col;

    if (file == NULL) {
        return -1;
    }

    if (fscanf(file, "%d %d", &rows, &cols) != 2 || rows <= 0 || cols <= 0) {
        fclose(file);
        return -1;
    }

    if (row_index >= (size_t)rows || output_count > (size_t)cols) {
        fclose(file);
        return -1;
    }

    for (row = 0; row <= row_index; ++row) {
        for (col = 0; col < (size_t)cols; ++col) {
            int value;
            if (fscanf(file, "%d", &value) != 1) {
                fclose(file);
                return -1;
            }
            if (row == row_index && col < output_count) {
                output[col] = value;
            }
        }
    }

    fclose(file);
    return 0;
}

static int read_row_doubles(
    const char *path,
    size_t row_index,
    double *output,
    size_t output_count
) {
    FILE *file = fopen(path, "r");
    int rows;
    int cols;
    size_t row;
    size_t col;

    if (file == NULL) {
        return -1;
    }

    if (fscanf(file, "%d %d", &rows, &cols) != 2 || rows <= 0 || cols <= 0) {
        fclose(file);
        return -1;
    }

    if (row_index >= (size_t)rows || output_count > (size_t)cols) {
        fclose(file);
        return -1;
    }

    for (row = 0; row <= row_index; ++row) {
        for (col = 0; col < (size_t)cols; ++col) {
            double value;
            if (fscanf(file, "%lf", &value) != 1) {
                fclose(file);
                return -1;
            }
            if (row == row_index && col < output_count) {
                output[col] = value;
            }
        }
    }

    fclose(file);
    return 0;
}

int input_config_load(const char *root_dir, InputDatasetConfig *config) {
    char fitness_path[INPUT_PATH_MAX];
    char ready_name[TOKEN_MAX] = {0};
    char duration_name[TOKEN_MAX] = {0};
    char deadline_name[TOKEN_MAX] = {0};
    char weight_name[TOKEN_MAX] = {0};

    if (root_dir == NULL || config == NULL) {
        return -1;
    }

    memset(config, 0, sizeof(*config));

    strncpy(config->root_dir, root_dir, INPUT_PATH_MAX - 1);
    config->root_dir[INPUT_PATH_MAX - 1] = '\0';

    if (join_path(fitness_path, sizeof(fitness_path), root_dir, "fitness.txt") != 0) {
        return -1;
    }

    if (parse_header_file(
        fitness_path,
        config,
        ready_name,
        duration_name,
        deadline_name,
        weight_name
    ) != 0) {
        input_config_destroy(config);
        return -1;
    }

    if (join_path(config->ready_file, sizeof(config->ready_file), root_dir, ready_name) != 0 ||
        join_path(config->duration_file, sizeof(config->duration_file), root_dir, duration_name) != 0 ||
        join_path(config->deadline_file, sizeof(config->deadline_file), root_dir, deadline_name) != 0 ||
        join_path(config->weight_file, sizeof(config->weight_file), root_dir, weight_name) != 0) {
        input_config_destroy(config);
        return -1;
    }

    return 0;
}

void input_config_destroy(InputDatasetConfig *config) {
    if (config == NULL) {
        return;
    }

    free(config->jobs_per_set);
    free(config->machines_per_set);
    config->jobs_per_set = NULL;
    config->machines_per_set = NULL;
    config->set_count = 0;
    config->max_jobs = 0;
    config->max_machines = 0;
}

int input_load_instance(
    const InputDatasetConfig *config,
    size_t set_index,
    SchedulingInstance *instance
) {
    size_t jobs;
    size_t machines;
    int *ready = NULL;
    int *deadline = NULL;
    double *weight = NULL;
    int *duration = NULL;
    size_t j;

    if (config == NULL || instance == NULL || set_index >= config->set_count) {
        return -1;
    }

    jobs = (size_t)config->jobs_per_set[set_index];
    machines = (size_t)config->machines_per_set[set_index];
    if (jobs == 0 || machines == 0) {
        return -1;
    }

    ready = (int *)calloc(jobs, sizeof(int));
    deadline = (int *)calloc(jobs, sizeof(int));
    weight = (double *)calloc(jobs, sizeof(double));
    duration = (int *)calloc(jobs * machines, sizeof(int));
    if (ready == NULL || deadline == NULL || weight == NULL || duration == NULL) {
        free(ready);
        free(deadline);
        free(weight);
        free(duration);
        return -1;
    }

    if (read_row_ints(config->ready_file, set_index, ready, jobs) != 0 ||
        read_row_ints(config->deadline_file, set_index, deadline, jobs) != 0 ||
        read_row_doubles(config->weight_file, set_index, weight, jobs) != 0 ||
        read_row_ints(config->duration_file, set_index, duration, jobs * machines) != 0) {
        free(ready);
        free(deadline);
        free(weight);
        free(duration);
        return -1;
    }

    {
        char instance_name[SCHED_INSTANCE_NAME_MAX];
        int written = snprintf(instance_name, sizeof(instance_name), "input_set_%zu", set_index);
        if (written < 0 || (size_t)written >= sizeof(instance_name) ||
            instance_create(instance, instance_name, jobs, machines) != 0) {
            free(ready);
            free(deadline);
            free(weight);
            free(duration);
            return -1;
        }
    }

    for (j = 0; j < jobs; ++j) {
        size_t m;
        Job *job = &instance->jobs[j];

        job->release_time = ready[j];
        job->due_date = deadline[j];
        job->weight = weight[j];
        if (job->weight <= 0.0) {
            job->weight = 1.0;
        }

        for (m = 0; m < machines; ++m) {
            int value = duration[j * machines + m];
            if (value <= 0) {
                value = 1;
            }
            job->processing_times[m] = value;
        }
    }

    free(ready);
    free(deadline);
    free(weight);
    free(duration);
    return 0;
}
