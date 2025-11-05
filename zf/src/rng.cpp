#include <zf/rng.h>

#include <ctime>
#include <cstdlib>

namespace zf {
    static bool g_rng_initted;

    void InitRNG() {
        ZF_ASSERT(!g_rng_initted);

        srand(time(NULL));
        g_rng_initted = true;
    }

    float RandPerc() {
        ZF_ASSERT(g_rng_initted);
        return static_cast<float>(rand()) / (RAND_MAX + 1.0f);
    }

    float RandPercIncl() {
        ZF_ASSERT(g_rng_initted);
        return static_cast<float>(rand()) / RAND_MAX;
    }

    int RandRangeInt(const int beg, const int end) {
        ZF_ASSERT(g_rng_initted);
        ZF_ASSERT(beg < end);
        return beg + (rand() % (end - beg));
    }

    int RandRangeIntIncl(const int beg, const int end_incl) {
        ZF_ASSERT(g_rng_initted);
        ZF_ASSERT(beg <= end_incl);
        return beg + (rand() % (end_incl - beg + 1));
    }
}
