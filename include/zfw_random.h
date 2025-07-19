#ifndef ZFW_RANDOM
#define ZFW_RANDOM

#include <assert.h>

void InitRNG();
float RandPerc();
float RandPercIncl();
int RandRangeI(const int beg, const int end);

static inline float RandRange(const float beg, const float end) {
    assert(beg <= end);
    return beg + ((end - beg) * RandPerc());
}

static inline float RandRangeIncl(const float beg, const float end_incl) {
    assert(beg <= end_incl);
    return beg + ((end_incl - beg) * RandPercIncl());
}

#endif
