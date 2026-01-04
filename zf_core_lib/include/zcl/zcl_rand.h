#pragma once

#include <zcl/zcl_mem.h>

namespace zf::rand {
    struct RNG;

    RNG *create_rng(const U64 seed, s_arena *const arena);

    U32 gen_u32(RNG *const rng);

    inline I32 gen_i32(RNG *const rng) {
        return static_cast<I32>(gen_u32(rng));
    }

    U32 gen_u32_in_range(RNG *const rng, const U32 min_incl, const U32 max_excl);

    I32 gen_i32_in_range(RNG *const rng, const I32 min_incl, const I32 max_excl);

    // Generates a random F32 in the range [0, 1).
    F32 gen_perc(RNG *const rng);

    inline F32 gen_f32_in_range(RNG *const rng, const F32 min_incl, const F32 max_excl) {
        return min_incl + (gen_perc(rng) * (max_excl - min_incl));
    }
}
