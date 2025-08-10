#include "zfwc_rng.h"

#include <time.h>
#include <stdlib.h>
#include <cu.h>

static bool g_rng_initted;

void InitRNG() {
    assert(!g_rng_initted);

    srand(time(NULL));
    g_rng_initted = true;
}

t_r32 RandPerc() {
    assert(g_rng_initted);
    return (t_r32)rand() / (RAND_MAX + 1.0f);
}

t_r32 RandPercIncl() {
    assert(g_rng_initted);
    return (t_r32)rand() / RAND_MAX;
}

t_s32 RandRangeS32(const t_s32 beg, const t_s32 end) {
    assert(g_rng_initted);
    assert(beg < end);

    return beg + ((end - 1 - beg) % rand());
}

t_s32 RandRangeS32Incl(const t_s32 beg, const t_s32 end_incl) {
    assert(g_rng_initted);
    assert(beg <= end_incl);

    return beg + ((end_incl - beg) % rand());
}
