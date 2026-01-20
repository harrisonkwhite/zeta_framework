#pragma once

#include <zcl/zcl_basic.h>

namespace zcl {
    struct t_rng;

    t_rng *RNGCreate(const t_u64 seed, t_arena *const arena);
    void RNGReseed(t_rng *const rng, const t_u64 seed);

    t_u32 RandGenU32(t_rng *const rng);

    inline t_i32 RandGenI32(t_rng *const rng) {
        return static_cast<t_i32>(RandGenU32(rng));
    }

    t_u32 RandGenU32InRange(t_rng *const rng, const t_u32 min_incl, const t_u32 max_excl);

    t_i32 RandGenI32InRange(t_rng *const rng, const t_i16 min_incl, const t_i16 max_excl);

    // Generates a random F32 in the range [0, 1).
    t_f32 RandGenPerc(t_rng *const rng);

    inline t_f32 RandGenF32InRange(t_rng *const rng, const t_f32 min_incl, const t_f32 max_excl) {
        return min_incl + (RandGenPerc(rng) * (max_excl - min_incl));
    }

    t_u64 RandGenSeed();

    // Returns a seemingly random value from x, and updates x to a new seemingly random value.
    // This is useful for seed generation.
    t_u64 Scramble(t_u64 *const x);
}
