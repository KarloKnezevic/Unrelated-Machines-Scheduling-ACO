#ifndef RNG_H
#define RNG_H

#include <stdint.h>

typedef struct {
    uint64_t state;
} RandomState;

void rng_seed(RandomState *rng, uint64_t seed);

uint32_t rng_next_u32(RandomState *rng);

double rng_next_double(RandomState *rng);

#endif
