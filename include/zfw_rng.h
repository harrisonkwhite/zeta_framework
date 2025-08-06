#ifndef ZFW_RNG_H
#define ZFW_RNG_H

// NOTE: This is all very much temporary and will be replaced with a more robust RNG system later on.

#include <assert.h>

void ZFW_InitRNG();

float ZFW_RandPerc(); // Generates a random float in the range [0.0f, 1.0f).
float ZFW_RandPercIncl(); // Generates a random float in the range [0.0f, 1.0f].

int ZFW_RandInt(const int beg, const int end);
int ZFW_RandIntIncl(const int beg, const int end_incl);

static inline float ZFW_RandRange(const float beg, const float end) {
    assert(beg < end);
    return beg + (ZFW_RandPerc() * (end - beg));
}

static inline float ZFW_RandRangeIncl(const float beg, const float end_incl) {
    assert(beg <= end_incl);
    return beg + (ZFW_RandPercIncl() * (end_incl - beg));
}

#endif
