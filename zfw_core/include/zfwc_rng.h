#ifndef ZFWC_RNG_H
#define ZFWC_RNG_H

// NOTE: This is all very much temporary and will be replaced with a more robust RNG system later on.

#include <assert.h>

void InitRNG();

float RandPerc(); // Generates a random float in the range [0.0f, 1.0f).
float RandPercIncl(); // Generates a random float in the range [0.0f, 1.0f].

int RandRangeInt(const int beg, const int end);
int RandRangeIntIncl(const int beg, const int end_incl);

static inline float RandRange(const float beg, const float end) {
    assert(beg < end);
    return beg + (RandPerc() * (end - beg));
}

static inline float RandRangeIncl(const float beg, const float end_incl) {
    assert(beg <= end_incl);
    return beg + (RandPercIncl() * (end_incl - beg));
}

#endif
