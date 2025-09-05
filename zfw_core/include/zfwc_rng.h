#pragma once

#include <cassert>
#include <cu.h>

void InitRNG();

t_r32 RandPerc(); // Generates a random t_r32 in the range [0.0f, 1.0f).
t_r32 RandPercIncl(); // Generates a random t_r32 in the range [0.0f, 1.0f].

t_s32 RandRangeS32(const t_s32 beg, const t_s32 end);
t_s32 RandRangeS32Incl(const t_s32 beg, const t_s32 end_incl);

static inline t_r32 RandRange(const t_r32 beg, const t_r32 end) {
    assert(beg < end);
    return beg + (RandPerc() * (end - beg));
}

static inline t_r32 RandRangeIncl(const t_r32 beg, const t_r32 end_incl) {
    assert(beg <= end_incl);
    return beg + (RandPercIncl() * (end_incl - beg));
}
