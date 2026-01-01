#include <zcl/zcl_rng.h>

namespace zf {
    struct s_pcg32 {
        t_u64 state; // RNG state. All values are possible.
        t_u64 inc;   // Controls which RNG sequence (stream) is selected. Must ALWAYS be odd.
    };

    struct s_rng {
        s_pcg32 pcg32;
    };

    // Generates a uniformly distributed random U32.
    static t_u32 CalcNext(s_pcg32 *const pcg32) {
        const t_u64 oldstate = pcg32->state;
        pcg32->state = (oldstate * 6364136223846793005ull) + pcg32->inc;
        const auto xorshifted = static_cast<t_u32>(((oldstate >> 18u) ^ oldstate) >> 27u);
        const auto rot = static_cast<t_u32>(oldstate >> 59u);
        return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
    }

    // Generates a uniformly distributed U32 strictly less than the bound.
    // The bound must be greater than 0.
    static t_u32 CalcNextBounded(s_pcg32 *const pcg32, const t_u32 bound) {
        ZF_ASSERT(bound > 0);

        const t_u32 threshold = -bound % bound;

        while (true) {
            const t_u32 r = CalcNext(pcg32);

            if (r >= threshold) {
                return r % bound;
            }
        }
    }

    static void Seed(s_pcg32 *const pcg32, const t_u64 init_state, const t_u64 seq) {
        pcg32->state = 0;
        pcg32->inc = (seq << 1u) | 1u;

        CalcNext(pcg32);

        pcg32->state += init_state;

        CalcNext(pcg32);
    }

    s_rng *CreateRNG(const t_u64 seed, s_arena *const arena) {
        const auto rng = PushItem<s_rng>(arena);
        Seed(&rng->pcg32, seed, 0); // @todo: Infer sequence from seed with mixing function!
        return rng;
    }

    t_u32 GenU32(s_rng *const rng) {
        return CalcNext(&rng->pcg32);
    }

    t_u32 GenU32InRange(s_rng *const rng, const t_u32 min_incl, const t_u32 max_excl) {
        ZF_ASSERT(min_incl < max_excl);
        return min_incl + CalcNextBounded(&rng->pcg32, max_excl - min_incl);
    }

    t_i32 GenI32InRange(s_rng *const rng, const t_i32 min_incl, const t_i32 max_excl) {
        ZF_ASSERT(min_incl < max_excl);

        const auto min_incl_u = static_cast<t_u32>(min_incl);
        const auto max_excl_u = static_cast<t_u32>(max_excl);
        return static_cast<t_i32>(min_incl_u + CalcNextBounded(&rng->pcg32, max_excl_u - min_incl_u));
    }

    t_f32 GenPerc(s_rng *const rng) {
        return static_cast<t_f32>(CalcNext(&rng->pcg32)) / 4294967296.0f;
    }
}
