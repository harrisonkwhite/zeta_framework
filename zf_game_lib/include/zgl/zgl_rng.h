#pragma once

#include <zcl.h>

namespace zf {
    void InitRNGModule();
    void ShutdownRNGModule();

    struct s_rng;

    s_rng &GlobalRNG();

    s_rng &CreateRNG(const t_u64 init_state, const t_u64 init_seq, s_mem_arena &mem_arena);

#if 0
    t_u32 RandU32(s_rng &rng = GlobalRNG());

    inline t_i32 RandI32(s_rng &rng = GlobalRNG()) {
        return static_cast<t_i32>(RandU32(rng));
    }
#endif

    // Generates a random F32 in the range [0, 1).
    t_f32 RandPerc(s_rng &rng = GlobalRNG());
}
