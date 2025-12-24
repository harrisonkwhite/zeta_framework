#pragma once

#include <zcl.h>

namespace zf {
    void InitRNGModule();
    void ShutdownRNGModule();

    struct s_rng;

    s_rng &GlobalRNG();

    s_rng &CreateRNG(const t_u64 init_state, const t_u64 init_seq, s_mem_arena &mem_arena);
    t_f32 RandPerc(s_rng &rng = GlobalRNG());
}
