#include "aco.h"
#include "benchmark.h"
#include "evaluator.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage(const char *program_name) {
    printf(
        "Usage: %s [options]\n\n"
        "Compares Min-Min and ACO for unrelated-machines scheduling input sets.\n\n"
        "Options:\n"
        "  --input-root PATH       Input dataset folder (default: data/input)\n"
        "  --set-start N           First set index to evaluate (default: 0)\n"
        "  --set-end N             Last set index to evaluate (default: all)\n"
        "  --mode MODE             Scheduling mode: static | dynamic (default: static)\n"
        "  --objective OBJ         Objective: makespan | weighted_flow |\n"
        "                          weighted_tardiness | weighted_tardy_jobs\n"
        "                          (default: makespan)\n"
        "  --heuristic H           ACO heuristic: edd | spt | wspt | mon (default: mon)\n"
        "  --ants N                Number of ants per iteration\n"
        "  --iterations N          Number of ACO iterations\n"
        "  --alpha X               Pheromone influence\n"
        "  --beta X                Heuristic influence\n"
        "  --rho X                 Evaporation rate in (0,1]\n"
        "  --tau0 X                Initial pheromone value\n"
        "  --q0 X                  Greedy probability in [0,1]\n"
        "  --seed N                Random seed\n"
        "  --csv PATH              Write per-set results to CSV\n"
        "  --verbose               Print each evaluated set\n"
        "  --help                  Show this message\n",
        program_name
    );
}

static int parse_size_t(const char *value, size_t *out) {
    char *endptr = NULL;
    unsigned long long parsed;

    errno = 0;
    parsed = strtoull(value, &endptr, 10);
    if (errno != 0 || endptr == value || *endptr != '\0') {
        return -1;
    }

    if (parsed > (unsigned long long)SIZE_MAX) {
        return -1;
    }

    *out = (size_t)parsed;
    return 0;
}

static int parse_uint(const char *value, unsigned int *out) {
    char *endptr = NULL;
    unsigned long parsed;

    errno = 0;
    parsed = strtoul(value, &endptr, 10);
    if (errno != 0 || endptr == value || *endptr != '\0' || parsed > UINT_MAX) {
        return -1;
    }

    *out = (unsigned int)parsed;
    return 0;
}

static int parse_double(const char *value, double *out) {
    char *endptr = NULL;
    double parsed;

    errno = 0;
    parsed = strtod(value, &endptr);
    if (errno != 0 || endptr == value || *endptr != '\0') {
        return -1;
    }

    *out = parsed;
    return 0;
}

int main(int argc, char **argv) {
    BenchmarkOptions options;
    int i;

    options.input_root = "data/input";
    options.set_start = 0;
    options.set_end = SIZE_MAX;
    options.mode = MODE_STATIC;
    options.objective = OBJECTIVE_MAKESPAN;
    aco_default_params(&options.aco_params);
    options.csv_path = NULL;
    options.verbose = 0;

    for (i = 1; i < argc; ++i) {
        const char *arg = argv[i];

        if (strcmp(arg, "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }

        if (strcmp(arg, "--verbose") == 0) {
            options.verbose = 1;
            continue;
        }

        if (strcmp(arg, "--input-root") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Missing value for --input-root.\n");
                return 1;
            }
            options.input_root = argv[++i];
            continue;
        }

        if (strcmp(arg, "--legacy-root") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Missing value for --legacy-root.\n");
                return 1;
            }
            options.input_root = argv[++i];
            continue;
        }

        if (strcmp(arg, "--set-start") == 0) {
            if (i + 1 >= argc || parse_size_t(argv[++i], &options.set_start) != 0) {
                fprintf(stderr, "Invalid value for --set-start.\n");
                return 1;
            }
            continue;
        }

        if (strcmp(arg, "--set-end") == 0) {
            if (i + 1 >= argc || parse_size_t(argv[++i], &options.set_end) != 0) {
                fprintf(stderr, "Invalid value for --set-end.\n");
                return 1;
            }
            continue;
        }

        if (strcmp(arg, "--mode") == 0) {
            int ok = 0;
            if (i + 1 >= argc) {
                fprintf(stderr, "Missing value for --mode.\n");
                return 1;
            }
            options.mode = mode_from_string(argv[++i], &ok);
            if (!ok) {
                fprintf(stderr, "Unsupported mode. Use static or dynamic.\n");
                return 1;
            }
            continue;
        }

        if (strcmp(arg, "--objective") == 0) {
            int ok = 0;
            if (i + 1 >= argc) {
                fprintf(stderr, "Missing value for --objective.\n");
                return 1;
            }
            options.objective = objective_from_string(argv[++i], &ok);
            if (!ok) {
                fprintf(stderr, "Unsupported objective.\n");
                return 1;
            }
            continue;
        }

        if (strcmp(arg, "--heuristic") == 0) {
            int ok = 0;
            if (i + 1 >= argc) {
                fprintf(stderr, "Missing value for --heuristic.\n");
                return 1;
            }
            options.aco_params.heuristic = heuristic_from_string(argv[++i], &ok);
            if (!ok) {
                fprintf(stderr, "Unsupported heuristic.\n");
                return 1;
            }
            continue;
        }

        if (strcmp(arg, "--ants") == 0) {
            size_t value;
            if (i + 1 >= argc || parse_size_t(argv[++i], &value) != 0) {
                fprintf(stderr, "Invalid value for --ants.\n");
                return 1;
            }
            options.aco_params.ant_count = value;
            continue;
        }

        if (strcmp(arg, "--iterations") == 0) {
            size_t value;
            if (i + 1 >= argc || parse_size_t(argv[++i], &value) != 0) {
                fprintf(stderr, "Invalid value for --iterations.\n");
                return 1;
            }
            options.aco_params.iterations = value;
            continue;
        }

        if (strcmp(arg, "--alpha") == 0) {
            if (i + 1 >= argc || parse_double(argv[++i], &options.aco_params.alpha) != 0) {
                fprintf(stderr, "Invalid value for --alpha.\n");
                return 1;
            }
            continue;
        }

        if (strcmp(arg, "--beta") == 0) {
            if (i + 1 >= argc || parse_double(argv[++i], &options.aco_params.beta) != 0) {
                fprintf(stderr, "Invalid value for --beta.\n");
                return 1;
            }
            continue;
        }

        if (strcmp(arg, "--rho") == 0) {
            if (i + 1 >= argc || parse_double(argv[++i], &options.aco_params.rho) != 0) {
                fprintf(stderr, "Invalid value for --rho.\n");
                return 1;
            }
            continue;
        }

        if (strcmp(arg, "--tau0") == 0) {
            if (i + 1 >= argc || parse_double(argv[++i], &options.aco_params.tau0) != 0) {
                fprintf(stderr, "Invalid value for --tau0.\n");
                return 1;
            }
            continue;
        }

        if (strcmp(arg, "--q0") == 0) {
            if (i + 1 >= argc || parse_double(argv[++i], &options.aco_params.q0) != 0) {
                fprintf(stderr, "Invalid value for --q0.\n");
                return 1;
            }
            continue;
        }

        if (strcmp(arg, "--seed") == 0) {
            if (i + 1 >= argc || parse_uint(argv[++i], &options.aco_params.seed) != 0) {
                fprintf(stderr, "Invalid value for --seed.\n");
                return 1;
            }
            continue;
        }

        if (strcmp(arg, "--csv") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Missing value for --csv.\n");
                return 1;
            }
            options.csv_path = argv[++i];
            continue;
        }

        fprintf(stderr, "Unknown option: %s\n", arg);
        print_usage(argv[0]);
        return 1;
    }

    if (run_dataset_benchmark(&options) != 0) {
        return 1;
    }

    return 0;
}
