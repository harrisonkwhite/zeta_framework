#pragma once

#include <cassert>
#include <zc.h>

namespace zf {
    void InitRNG();

    float RandPerc(); // Generates a random float in the range [0.0f, 1.0f).
    float RandPercIncl(); // Generates a random float in the range [0.0f, 1.0f].

    t_s32 RandRangeS32(const t_s32 beg, const t_s32 end);
    t_s32 RandRangeS32Incl(const t_s32 beg, const t_s32 end_incl);

    static inline float RandRange(const float beg, const float end) {
        assert(beg < end);
        return beg + (RandPerc() * (end - beg));
    }

    static inline float RandRangeIncl(const float beg, const float end_incl) {
        assert(beg <= end_incl);
        return beg + (RandPercIncl() * (end_incl - beg));
    }
}
