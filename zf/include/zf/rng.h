#pragma once

#include <zc/math.h>

namespace zf {
    void InitRNG();

    float RandPerc(); // Generates a random float in the range [0.0f, 1.0f).
    float RandPercIncl(); // Generates a random float in the range [0.0f, 1.0f].

    int RandRangeInt(const int beg, const int end); // Generates a random integer in the range [beg, end).
    int RandRangeIntIncl(const int beg, const int end_incl); // Generates a random integer in the range [beg, end_incl].

    // Generates a random float in the range [beg, end).
    inline float RandRange(const float beg, const float end) {
        ZF_ASSERT(beg < end);
        return beg + (RandPerc() * (end - beg));
    }

    // Generates a random float in the range [beg, end_incl].
    inline float RandRangeIncl(const float beg, const float end_incl) {
        ZF_ASSERT(beg <= end_incl);
        return beg + (RandPercIncl() * (end_incl - beg));
    }
}
