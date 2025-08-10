#include "zfwc_rng.h"

#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <cu.h>

static bool g_rng_initted;

void InitRNG() {
    assert(!g_rng_initted);

    srand(time(NULL));
    g_rng_initted = true;
}

float RandPerc() {
    assert(g_rng_initted);
    return (float)rand() / (RAND_MAX + 1.0f);
}

float RandPercIncl() {
    assert(g_rng_initted);
    return (float)rand() / RAND_MAX;
}

int RandRangeInt(const int beg, const int end) {
    assert(g_rng_initted);
    assert(beg < end);

    return beg + ((end - 1 - beg) % rand());
}

int RandRangeIntIncl(const int beg, const int end_incl) {
    assert(g_rng_initted);
    assert(beg <= end_incl);

    return beg + ((end_incl - beg) % rand());
}
