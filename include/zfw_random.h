#ifndef ZFW_RANDOM
#define ZFW_RANDOM

#include <assert.h>

void InitRNG();
float RandPerc();
int RandRangeI(const int beg, const int end);

static inline float RandRange(const float beg, const float end) {
    assert(beg <= end);
    return beg + ((end - beg) * RandPerc());
}

#endif
