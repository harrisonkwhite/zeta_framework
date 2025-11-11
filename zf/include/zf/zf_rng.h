#pragma once

#include <zc.h>

// @todo: Rework this entire thing. Could probably be moved over to core if we dropped the global state.

namespace zf {
    void InitRNG();

    t_f32 RandPerc(); // Generates a random float in the range [0.0f, 1.0f).
    t_f32 RandPercIncl(); // Generates a random float in the range [0.0f, 1.0f].

    t_s32 RandRangeInt(const t_s32 beg, const t_s32 end); // Generates a random integer in the range [beg, end).
    t_s32 RandRangeIntIncl(const t_s32 beg, const t_s32 end_incl); // Generates a random integer in the range [beg, end_incl].

    // Generates a random t_r32 in the range [beg, end).
    inline t_f32 RandRange(const t_f32 beg, const t_f32 end) {
        ZF_ASSERT(beg < end);
        return beg + (RandPerc() * (end - beg));
    }

    // Generates a random t_r32 in the range [beg, end_incl].
    inline t_f32 RandRangeIncl(const t_f32 beg, const t_f32 end_incl) {
        ZF_ASSERT(beg <= end_incl);
        return beg + (RandPercIncl() * (end_incl - beg));
    }
}
