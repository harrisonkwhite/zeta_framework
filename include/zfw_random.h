#ifndef ZFW_RANDOM
#define ZFW_RANDOM

#include <assert.h>

void ZFWInitRNG();
float ZFWRandPerc();
float ZFWRandPercIncl();
int ZFWRandRangeI(const int beg, const int end);

static inline float ZFWRandRange(const float beg, const float end) {
    assert(beg <= end);
    return beg + ((end - beg) * ZFWRandPerc());
}

static inline float ZFWRandRangeIncl(const float beg, const float end_incl) {
    assert(beg <= end_incl);
    return beg + ((end_incl - beg) * ZFWRandPercIncl());
}

#endif
