#pragma once

#include <zcl.h>

namespace zf {
    void InitRNGModule();
    void ShutdownRNGModule();

    struct s_rng;

    s_rng &GlobalRNG();

    s_rng &CreateRNG(const t_u64 init_state, const t_u64 init_seq, s_mem_arena &mem_arena);
    void ResetRNGSeed(s_rng &rng);

    t_u32 RandU32(s_rng &rng, const t_u64 new_state, const t_u64 new_seq);

    inline t_i32 RandI32(s_rng &rng = GlobalRNG()) {
        return static_cast<t_i32>(RandU32(rng));
    }

    // Generates a random U32 in the range [min, max).
    t_u32 RandU32InRange(const t_u32 min, t_u32 max, s_rng &rng = GlobalRNG());

    inline t_i32 RandI32InRange(const t_u32 min, t_u32 max, s_rng &rng = GlobalRNG()) {
        ZF_ASSERT(false); // @todo
        return 0;
    }

    // Generates a random F32 in the range [min, max).
    inline t_f32 RandF32InRange(const t_f32 min, const t_f32 max, s_rng &rng = GlobalRNG()) {
        ZF_ASSERT(false); // @todo
        return 0;
    }

    // Generates a random F32 in the range [0, 1).
    inline t_f32 RandPerc(s_rng &rng = GlobalRNG()) {
        return RandF32InRange(0.0f, 1.0f, rng);
    }
}
