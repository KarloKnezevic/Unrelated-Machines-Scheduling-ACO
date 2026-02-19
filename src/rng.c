#include "rng.h"

void rng_seed(RandomState *rng, uint64_t seed) {
    if (seed == 0U) {
        seed = 0x9e3779b97f4a7c15ULL;
    }
    rng->state = seed;
}

uint32_t rng_next_u32(RandomState *rng) {
    uint64_t x = rng->state;

    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    rng->state = x;
    x *= 0x2545F4914F6CDD1DULL;

    return (uint32_t)(x >> 32);
}

double rng_next_double(RandomState *rng) {
    uint32_t raw = rng_next_u32(rng);
    return ((double)raw + 1.0) / 4294967297.0;
}
