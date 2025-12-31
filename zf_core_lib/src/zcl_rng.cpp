#include <zcl/zcl_rng.h>

namespace zf {
    void c_rng::c_pcg32::Seed(const t_u64 init_state, const t_u64 seq) {
        m_state = 0;
        m_inc = (seq << 1u) | 1u;

        CalcNext();

        m_state += init_state;

        CalcNext();
    }

    t_u32 c_rng::c_pcg32::CalcNext() {
        const t_u64 oldstate = m_state;
        m_state = (oldstate * 6364136223846793005ull) + m_inc;
        const auto xorshifted = static_cast<t_u32>(((oldstate >> 18u) ^ oldstate) >> 27u);
        const auto rot = static_cast<t_u32>(oldstate >> 59u);
        return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
    }

    t_u32 c_rng::c_pcg32::CalcNextBounded(const t_u32 bound) {
        ZF_ASSERT(bound > 0);

        const t_u32 threshold = -bound % bound;

        while (true) {
            const t_u32 r = CalcNext();

            if (r >= threshold) {
                return r % bound;
            }
        }
    }
}
