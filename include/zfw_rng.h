#ifndef ZFW_RNG_H
#define ZFW_RNG_H

#include <assert.h>

void ZFW_InitRNG();
float ZFW_RandPerc();
float ZFW_RandPercIncl();
int ZFW_RandRangeI(const int beg, const int end);

static inline float ZFW_RandRange(const float beg, const float end) {
    assert(beg <= end);
    return beg + ((end - beg) * ZFW_RandPerc());
}

static inline float ZFW_RandRangeIncl(const float beg, const float end_incl) {
    assert(beg <= end_incl);
    return beg + ((end_incl - beg) * ZFW_RandPercIncl());
}

#endif
