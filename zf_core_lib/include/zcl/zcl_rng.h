#pragma once

#include <zcl/zcl_mem.h>

namespace zf {
    struct s_rng;

    s_rng *rng_create(const t_u64 seed, s_arena *const arena);

    t_u32 RandU32(s_rng *const rng);

    inline t_i32 RandI32(s_rng *const rng) {
        return static_cast<t_i32>(RandU32(rng));
    }

    t_u32 RandU32InRange(s_rng *const rng, const t_u32 min_incl, const t_u32 max_excl);

    t_i32 RandI32InRange(s_rng *const rng, const t_i32 min_incl, const t_i32 max_excl);

    // Generates a random F32 in the range [0, 1).
    t_f32 RandPerc(s_rng *const rng);

    inline t_f32 RandF32InRange(s_rng *const rng, const t_f32 min_incl, const t_f32 max_excl) {
        return min_incl + (RandPerc(rng) * (max_excl - min_incl));
    }
}
