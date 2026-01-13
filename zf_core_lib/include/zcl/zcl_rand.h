#pragma once

#include <zcl/zcl_basic.h>
#include <zcl/zcl_arenas.h>

namespace zcl {
    struct t_rng;

    t_rng *rng_create(const t_u64 seed, t_arena *const arena);

    t_u32 rand_gen_u32(t_rng *const rng);

    inline t_i32 rand_gen_i32(t_rng *const rng) {
        return static_cast<t_i32>(rand_gen_u32(rng));
    }

    t_u32 rand_gen_u32_in_range(t_rng *const rng, const t_u32 min_incl, const t_u32 max_excl);

    t_i32 rand_gen_i32_in_range(t_rng *const rng, const t_i16 min_incl, const t_i16 max_excl);

    // Generates a random F32 in the range [0, 1).
    t_f32 rand_gen_perc(t_rng *const rng);

    inline t_f32 rand_gen_f32_in_range(t_rng *const rng, const t_f32 min_incl, const t_f32 max_excl) {
        return min_incl + (rand_gen_perc(rng) * (max_excl - min_incl));
    }

    t_u64 rand_gen_seed();

    // Returns a seemingly random value from x, and updates x to a new seemingly random value.
    // This is useful for seed generation.
    t_u64 scramble(t_u64 *const x);
}
