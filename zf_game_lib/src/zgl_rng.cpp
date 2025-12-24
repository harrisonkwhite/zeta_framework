#include <zgl/zgl_rng.h>

namespace zf {
    struct s_pcg32 {
    public:
        void Seed(const t_u64 init_state, const t_u64 seq) {
            m_seeded = true;

            m_state = 0;
            m_inc = (seq << 1u) | 1u;

            Next();

            m_state += init_state;

            Next();
        }

        t_b8 IsSeeded() const {
            return m_seeded;
        }

        // Generates a uniformly distributed random U32.
        t_u32 Next() {
            ZF_ASSERT(m_seeded);

            const t_u64 oldstate = m_state;
            m_state = (oldstate * 6364136223846793005ull) + m_inc;
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

            while (true) {
                const t_u32 r = Next();

                if (r >= threshold) {
                    return r % bound;
                }
            }
        }

    private:
        t_b8 m_seeded = false;

        t_u64 m_state = 0; // RNG state. All values are possible.
        t_u64 m_inc = 0;   // Controls which RNG sequence (stream) is selected. Must ALWAYS be odd.
    };

    struct s_rng {
        s_pcg32 pcg32 = {};
    };

    static struct {
        t_b8 initted = false;
        t_u64 seed_cnt = 0;
    } g_state;

    void InitRNGModule() {
        ZF_ASSERT(!g_state.initted);
        g_state.initted = true;
    }

    void ShutdownRNGModule() {
        ZF_ASSERT(g_state.initted);
        g_state = {};
    }

    s_rng &CreateRNG(const t_u64 seed, s_mem_arena &mem_arena) {
        ZF_ASSERT(g_state.initted);

        s_rng &rng = Alloc<s_rng>(mem_arena);
        rng.pcg32.Seed(seed, g_state.seed_cnt);
        g_state.seed_cnt++;
        return rng;
    }

    void ReseedRNG(s_rng &rng, const t_u64 seed) {
        ZF_ASSERT(g_state.initted);
        ZF_ASSERT(rng.pcg32.IsSeeded());

        rng.pcg32.Seed(seed, g_state.seed_cnt);
        g_state.seed_cnt++;
    }

    t_u32 RandU32(s_rng &rng) {
        ZF_ASSERT(g_state.initted);
        return rng.pcg32.Next();
    }

    t_u32 RandU32InRange(s_rng &rng, const t_u32 min_incl, const t_u32 max_excl) {
        ZF_ASSERT(g_state.initted);
        ZF_ASSERT(min_incl < max_excl);

        return min_incl + rng.pcg32.NextBounded(max_excl - min_incl);
    }

    t_i32 RandI32InRange(s_rng &rng, const t_i32 min_incl, const t_i32 max_excl) {
        ZF_ASSERT(g_state.initted);
        ZF_ASSERT(min_incl < max_excl);

        const auto min_incl_u = static_cast<t_u32>(min_incl);
        const auto max_excl_u = static_cast<t_u32>(max_excl);
        return static_cast<t_i32>(min_incl_u + rng.pcg32.NextBounded(max_excl_u - min_incl_u));
    }

    t_f32 RandPerc(s_rng &rng) {
        ZF_ASSERT(g_state.initted);
        return static_cast<t_f32>(rng.pcg32.Next()) / 4294967296.0f;
    }
}
