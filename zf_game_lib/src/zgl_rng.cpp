#include <zgl/zgl_rng.h>

namespace zf {
    struct s_pcg32 {
    public:
        s_pcg32(const t_u64 init_state, const t_u64 init_seq) {
            m_state = 0U;
            m_inc = (init_seq << 1u) | 1u;
            Next();
            m_state += init_state;
            Next();
        }

        // Generates a uniformly distributed random U32.
        t_u32 Next() {
            const t_u64 oldstate = m_state;
            m_state = oldstate * 6364136223846793005ULL + m_inc;
            const auto xorshifted = static_cast<t_u32>(((oldstate >> 18u) ^ oldstate) >> 27u);
            const auto rot = static_cast<t_u32>(oldstate >> 59u);
            return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
        }

        // Generates a uniformly distributed U32 strictly less than the bound.
        // The bound must be greater than 0.
        t_u32 NextBounded(const t_u32 bound) {
            ZF_ASSERT(bound > 0);

            const t_u32 threshold = -bound % bound;

            for (;;) {
                t_u32 r = Next();

                if (r >= threshold) {
                    return r % bound;
                }
            }
        }

    private:
        t_u64 m_state; // RNG state. All values are possible.
        t_u64 m_inc;   // Controls which RNG sequence (stream) is selected. Must ALWAYS be odd.
    };

    struct s_rng {
        s_pcg32 pcg32;
    };
}
