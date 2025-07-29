#include "zfw_random.h"

#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <cu.h>

static bool g_rng_initted;

void ZFW_InitRNG() {
    assert(!g_rng_initted);

    srand(time(NULL));
    g_rng_initted = true;
}

float ZFW_RandPerc() {
    assert(g_rng_initted);
    return (float)rand() / (RAND_MAX + 1.0f);
}

float ZFW_RandPercIncl() {
    assert(g_rng_initted);
    return (float)rand() / RAND_MAX;
}

int ZFW_RandRangeI(const int beg, const int end) {
    assert(g_rng_initted);
    assert(beg <= end);
    return beg + (rand() % (end - beg));
}
