#include <zcl/zcl_rand.h>

namespace zf {
    struct t_pcg32 {
        t_u64 state; // RNG state. All values are possible.
        t_u64 inc;   // Controls which RNG sequence (stream) is selected. Must ALWAYS be odd.
    };

    struct t_rng {
        t_pcg32 pcg32;
    };

    // Generates a uniformly distributed random U32.
    static t_u32 f_rand_calc_next_pcg32(t_pcg32 *const pcg32) {
        const t_u64 oldstate = pcg32->state;
        pcg32->state = (oldstate * 6364136223846793005ull) + pcg32->inc;
        const auto xorshifted = static_cast<t_u32>(((oldstate >> 18u) ^ oldstate) >> 27u);
        const auto rot = static_cast<t_u32>(oldstate >> 59u);
        return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
    }

    // Generates a uniformly distributed U32 strictly less than the bound.
    // The bound must be greater than 0.
    static t_u32 f_rand_calc_next_pcg32_bounded(t_pcg32 *const pcg32, const t_u32 bound) {
        ZF_ASSERT(bound > 0);

        const t_u32 threshold = -bound % bound;

        while (true) {
            const t_u32 r = f_rand_calc_next_pcg32(pcg32);

            if (r >= threshold) {
                return r % bound;
            }
        }
    }

    static void f_rand_seed_pcg32(t_pcg32 *const pcg32, const t_u64 init_state, const t_u64 seq) {
        pcg32->state = 0;
        pcg32->inc = (seq << 1u) | 1u;

        f_rand_calc_next_pcg32(pcg32);

        pcg32->state += init_state;

        f_rand_calc_next_pcg32(pcg32);
    }

    t_rng *f_rand_create_rng(const t_u64 seed, mem::t_arena *const arena) {
        const auto rng = mem::f_arena_push_item<t_rng>(arena);
        f_rand_seed_pcg32(&rng->pcg32, seed, 0); // @todo: Infer sequence from seed with mixing function!
        return rng;
    }

    t_u32 f_rand_gen_u32(t_rng *const rng) {
        return f_rand_calc_next_pcg32(&rng->pcg32);
    }

    t_u32 f_rand_gen_u32_in_range(t_rng *const rng, const t_u32 min_incl, const t_u32 max_excl) {
        ZF_ASSERT(min_incl < max_excl);
        return min_incl + f_rand_calc_next_pcg32_bounded(&rng->pcg32, max_excl - min_incl);
    }

    t_i32 f_rand_gen_i32_in_range(t_rng *const rng, const t_i32 min_incl, const t_i32 max_excl) {
        ZF_ASSERT(min_incl < max_excl);

        const auto min_incl_u = static_cast<t_u32>(min_incl);
        const auto max_excl_u = static_cast<t_u32>(max_excl);
        return static_cast<t_i32>(min_incl_u + f_rand_calc_next_pcg32_bounded(&rng->pcg32, max_excl_u - min_incl_u));
    }

    t_f32 f_rand_gen_perc(t_rng *const rng) {
        return static_cast<t_f32>(f_rand_calc_next_pcg32(&rng->pcg32)) / 4294967296.0f;
    }
}
