#include <zgl/zgl_rng.h>

namespace zf {
    struct s_pcg32 {
    public:
        void Seed(const t_u64 init_state, const t_u64 init_seq) {
            m_seeded = true;

            m_state = 0U;
            m_inc = (init_seq << 1u) | 1u;

            Next();

            m_state += init_state;

            Next();
        }

        // Generates a uniformly distributed random U32.
        t_u32 Next() {
            ZF_ASSERT(m_seeded);

            const t_u64 oldstate = m_state;
            m_state = oldstate * 6364136223846793005ULL + m_inc;
            const auto xorshifted = static_cast<t_u32>(((oldstate >> 18u) ^ oldstate) >> 27u);
            const auto rot = static_cast<t_u32>(oldstate >> 59u);
            return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
        }

        // Generates a uniformly distributed U32 strictly less than the bound.
        // The bound must be greater than 0.
        t_u32 NextBounded(const t_u32 bound) {
            ZF_ASSERT(m_seeded);
            ZF_ASSERT(bound > 0);

            const t_u32 threshold = -bound % bound;

            for (;;) {
                const t_u32 r = Next();

                if (r >= threshold) {
                    return r % bound;
                }
            }
        }

    private:
        t_u64 m_state = 0; // RNG state. All values are possible.
        t_u64 m_inc = 0;   // Controls which RNG sequence (stream) is selected. Must ALWAYS be odd.

        t_b8 m_seeded = false;
    };

    struct s_rng {
        s_pcg32 pcg32 = {};
    };

    static struct {
        t_b8 initted = false;
        s_rng rng = {};
    } g_state;

    void InitRNGModule() {
        ZF_ASSERT(!g_state.initted);

        g_state.initted = true;
        g_state.rng.pcg32.Seed(42u, 54u); // @todo: Truly randomise, use time or something.
    }

    void ShutdownRNGModule() {
        ZF_ASSERT(g_state.initted);
        g_state = {};
    }

    s_rng &GlobalRNG() {
        ZF_ASSERT(g_state.initted);
        return g_state.rng;
    }

    s_rng &CreateRNG(const t_u64 init_state, const t_u64 init_seq, s_mem_arena &mem_arena) {
        ZF_ASSERT(g_state.initted);

        s_rng &rng = Alloc<s_rng>(mem_arena);
        rng.pcg32.Seed(init_state, init_seq);
        return rng;
    }

    t_f32 RandPerc(s_rng &rng) {
        ZF_ASSERT(g_state.initted);
        return static_cast<t_f32>(rng.pcg32.Next()) / static_cast<t_f32>(g_u32_max); // @todo: Wrong range! Can't include 1!
    }

}
