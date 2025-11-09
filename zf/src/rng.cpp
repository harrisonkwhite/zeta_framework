#include <zf/rng.h>

#include <ctime>
#include <cstdlib>

namespace zf {
    static t_b8 g_rng_initted;

    void InitRNG() {
        ZF_ASSERT(!g_rng_initted);

        srand(static_cast<t_u32>(time(nullptr)));
        g_rng_initted = true;
    }

    t_f32 RandPerc() {
        ZF_ASSERT(g_rng_initted);
        return static_cast<t_f32>(rand()) / (RAND_MAX + 1.0f);
    }

    t_f32 RandPercIncl() {
        ZF_ASSERT(g_rng_initted);
        return static_cast<t_f32>(rand()) / RAND_MAX;
    }

    t_s32 RandRangeInt(const t_s32 beg, const t_s32 end) {
        ZF_ASSERT(g_rng_initted);
        ZF_ASSERT(beg < end);
        return beg + (rand() % (end - beg));
    }

    t_s32 RandRangeIntIncl(const t_s32 beg, const t_s32 end_incl) {
        ZF_ASSERT(g_rng_initted);
        ZF_ASSERT(beg <= end_incl);
        return beg + (rand() % (end_incl - beg + 1));
    }
}
