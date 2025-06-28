#ifndef ZFW_RANDOM
#define ZFW_RANDOM

#include <assert.h>

void InitRNG();
float RandPerc();

static inline float RandRange(const float min, const float max) {
    assert(min <= max);
    return min + ((max - min) * RandPerc());
}

#endif
