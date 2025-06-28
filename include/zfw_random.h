#ifndef GCE_RANDOM
#define GCE_RANDOM

void InitRNG();
float RandPerc();

static inline float RandRange(const float min, const float max) {
    return min + ((max - min) * RandPerc());
}

#endif
