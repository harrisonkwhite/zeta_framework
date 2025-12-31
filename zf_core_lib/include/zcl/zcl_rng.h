#pragma once

#include <zcl/zcl_mem.h>

namespace zf {
    class c_rng {
    public:
        // Reseeds are allowed.
        void Seed(const t_u64 seed) {
            m_seeded = true;
            m_pcg32.Seed(seed, 0); // @todo: Infer sequence from seed!
        }

        t_u32 GenU32() {
            ZF_ASSERT(m_seeded);
            return m_pcg32.CalcNext();
        }

        t_i32 GenI32() {
            ZF_ASSERT(m_seeded);
            return static_cast<t_i32>(GenU32());
        }

        t_u32 GenU32InRange(const t_u32 min_incl, const t_u32 max_excl) {
            ZF_ASSERT(m_seeded);
            ZF_ASSERT(min_incl < max_excl);

            return min_incl + m_pcg32.CalcNextBounded(max_excl - min_incl);
        }

        t_i32 GenI32InRange(const t_i32 min_incl, const t_i32 max_excl) {
            ZF_ASSERT(m_seeded);
            ZF_ASSERT(min_incl < max_excl);

            const auto min_incl_u = static_cast<t_u32>(min_incl);
            const auto max_excl_u = static_cast<t_u32>(max_excl);
            return static_cast<t_i32>(min_incl_u + m_pcg32.CalcNextBounded(max_excl_u - min_incl_u));
        }

        // Generates a random F32 in the range [0, 1).
        t_f32 GenPerc() {
            ZF_ASSERT(m_seeded);
            return static_cast<t_f32>(m_pcg32.CalcNext()) / 4294967296.0f;
        }

        t_f32 GenF32InRange(const t_f32 min_incl, const t_f32 max_excl) {
            ZF_ASSERT(m_seeded);
            return min_incl + (GenPerc() * (max_excl - min_incl));
        }

    private:
        class c_pcg32 {
        public:
            void Seed(const t_u64 init_state, const t_u64 seq);

            // Generates a uniformly distributed random U32.
            t_u32 CalcNext();

            // Generates a uniformly distributed U32 strictly less than the bound.
            // The bound must be greater than 0.
            t_u32 CalcNextBounded(const t_u32 bound);

        private:
            t_u64 m_state = 0; // RNG state. All values are possible.
            t_u64 m_inc = 0;   // Controls which RNG sequence (stream) is selected. Must ALWAYS be odd.
        };

        t_b8 m_seeded = false;
        c_pcg32 m_pcg32;
    };
}
