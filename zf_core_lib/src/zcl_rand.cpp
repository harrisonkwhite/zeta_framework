#include <zcl/zcl_rand.h>

namespace zf::rand {
    struct PCG32 {
        U64 state; // RNG state. All values are possible.
        U64 inc;   // Controls which RNG sequence (stream) is selected. Must ALWAYS be odd.
    };

    struct RNG {
        PCG32 pcg32;
    };

    // Generates a uniformly distributed random U32.
    static U32 calc_next_pcg32(PCG32 *const pcg32) {
        const U64 oldstate = pcg32->state;
        pcg32->state = (oldstate * 6364136223846793005ull) + pcg32->inc;
        const auto xorshifted = static_cast<U32>(((oldstate >> 18u) ^ oldstate) >> 27u);
        const auto rot = static_cast<U32>(oldstate >> 59u);
        return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
    }

    // Generates a uniformly distributed U32 strictly less than the bound.
    // The bound must be greater than 0.
    static U32 calc_next_pcg32_bounded(PCG32 *const pcg32, const U32 bound) {
        ZF_ASSERT(bound > 0);

        const U32 threshold = -bound % bound;

        while (true) {
            const U32 r = calc_next_pcg32(pcg32);

            if (r >= threshold) {
                return r % bound;
            }
        }
    }

    static void seed_pcg32(PCG32 *const pcg32, const U64 init_state, const U64 seq) {
        pcg32->state = 0;
        pcg32->inc = (seq << 1u) | 1u;

        calc_next_pcg32(pcg32);

        pcg32->state += init_state;

        calc_next_pcg32(pcg32);
    }

    RNG *create_rng(const U64 seed, s_arena *const arena) {
        const auto rng = ArenaPushItem<RNG>(arena);
        seed_pcg32(&rng->pcg32, seed, 0); // @todo: Infer sequence from seed with mixing function!
        return rng;
    }

    U32 gen_u32(RNG *const rng) {
        return calc_next_pcg32(&rng->pcg32);
    }

    U32 gen_u32_in_range(RNG *const rng, const U32 min_incl, const U32 max_excl) {
        ZF_ASSERT(min_incl < max_excl);
        return min_incl + calc_next_pcg32_bounded(&rng->pcg32, max_excl - min_incl);
    }

    I32 gen_i32_in_range(RNG *const rng, const I32 min_incl, const I32 max_excl) {
        ZF_ASSERT(min_incl < max_excl);

        const auto min_incl_u = static_cast<U32>(min_incl);
        const auto max_excl_u = static_cast<U32>(max_excl);
        return static_cast<I32>(min_incl_u + calc_next_pcg32_bounded(&rng->pcg32, max_excl_u - min_incl_u));
    }

    F32 gen_perc(RNG *const rng) {
        return static_cast<F32>(calc_next_pcg32(&rng->pcg32)) / 4294967296.0f;
    }
}
