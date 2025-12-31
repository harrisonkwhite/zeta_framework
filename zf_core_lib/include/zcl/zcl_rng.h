#pragma once

#include <zcl/zcl_mem.h>

namespace zf {
    struct s_rng;

    s_rng *rng_create(const t_u64 seed, s_arena *const arena);

    t_u32 rng_gen_u32(s_rng *const rng);

    inline t_i32 rng_gen_i32(s_rng *const rng) {
        return static_cast<t_i32>(rng_gen_u32(rng));
    }

    t_u32 rng_gen_u32_in_range(s_rng *const rng, const t_u32 min_incl, const t_u32 max_excl);

    t_i32 rng_gen_i32_in_range(s_rng *const rng, const t_i32 min_incl, const t_i32 max_excl);

    // Generates a random F32 in the range [0, 1).
    t_f32 rng_gen_perc(s_rng *const rng);

    inline t_f32 rng_gen_f32_in_range(s_rng *const rng, const t_f32 min_incl, const t_f32 max_excl) {
        return min_incl + (rng_gen_perc(rng) * (max_excl - min_incl));
    }
}
