#pragma once

#include <zcl/zcl_mem.h>

namespace zf::rand {
    struct RNG;

    RNG *create_rng(const t_u64 seed, t_arena *const arena);

    t_u32 gen_u32(RNG *const rng);

    inline t_i32 gen_i32(RNG *const rng) {
        return static_cast<t_i32>(gen_u32(rng));
    }

    t_u32 gen_u32_in_range(RNG *const rng, const t_u32 min_incl, const t_u32 max_excl);

    t_i32 gen_i32_in_range(RNG *const rng, const t_i32 min_incl, const t_i32 max_excl);

    // Generates a random F32 in the range [0, 1).
    t_f32 gen_perc(RNG *const rng);

    inline t_f32 gen_f32_in_range(RNG *const rng, const t_f32 min_incl, const t_f32 max_excl) {
        return min_incl + (gen_perc(rng) * (max_excl - min_incl));
    }
}
