#include <zcl/zcl_rand.h>

#ifdef ZCL_PLATFORM_WINDOWS
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif

    #ifndef NOMINMAX
        #define NOMINMAX
    #endif

    #include <windows.h>

    #include <bcrypt.h>
    #pragma comment(lib, "bcrypt.lib")
#endif

namespace zcl {
    struct t_pcg32 {
        t_u64 state; // RNG state. All values are possible.
        t_u64 inc;   // Controls which RNG sequence (stream) is selected. Must ALWAYS be odd.
    };

    struct t_rng {
        t_pcg32 pcg32;
    };

    // Generates a uniformly distributed random U32.
    static t_u32 PCG32CalcNext(t_pcg32 *const pcg32) {
        const t_u64 oldstate = pcg32->state;
        pcg32->state = (oldstate * 6364136223846793005ull) + pcg32->inc;
        const auto xorshifted = static_cast<t_u32>(((oldstate >> 18u) ^ oldstate) >> 27u);
        const auto rot = static_cast<t_u32>(oldstate >> 59u);
        return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
    }

    // Generates a uniformly distributed U32 strictly less than the bound.
    // The bound must be greater than 0.
    static t_u32 PCG32CalcNextBounded(t_pcg32 *const pcg32, const t_u32 bound) {
        ZCL_ASSERT(bound > 0);

        const t_u32 threshold = -bound % bound;

        while (true) {
            const t_u32 r = PCG32CalcNext(pcg32);

            if (r >= threshold) {
                return r % bound;
            }
        }
    }

    static void PCG32Seed(t_pcg32 *const pcg32, const t_u64 init_state, const t_u64 seq) {
        pcg32->state = 0;
        pcg32->inc = (seq << 1u) | 1u;

        PCG32CalcNext(pcg32);

        pcg32->state += init_state;

        PCG32CalcNext(pcg32);
    }

    static void RNGSeed(t_rng *const rng, const t_u64 seed) {
        t_u64 x = seed;
        const t_u64 init_state = Scramble(&x);
        const t_u64 seq = Scramble(&x);

        PCG32Seed(&rng->pcg32, init_state, seq);
    }

    t_rng *RNGCreate(const t_u64 seed, t_arena *const arena) {
        const auto rng = ArenaPushItem<t_rng>(arena);
        RNGSeed(rng, seed);
        return rng;
    }

    void RNGReseed(t_rng *const rng, const t_u64 seed) {
        RNGSeed(rng, seed);
    }

    t_u32 RandGenU32(t_rng *const rng) {
        return PCG32CalcNext(&rng->pcg32);
    }

    t_u32 RandGenU32InRange(t_rng *const rng, const t_u32 min_incl, const t_u32 max_excl) {
        ZCL_ASSERT(min_incl < max_excl);
        return min_incl + PCG32CalcNextBounded(&rng->pcg32, max_excl - min_incl);
    }

    t_i32 RandGenI32InRange(t_rng *const rng, const t_i16 min_incl, const t_i16 max_excl) {
        ZCL_ASSERT(min_incl < max_excl);

        const auto diff = static_cast<t_u32>(max_excl - min_incl);
        return min_incl + static_cast<t_i32>(PCG32CalcNextBounded(&rng->pcg32, diff));
    }

    t_f32 RandGenPerc(t_rng *const rng) {
        return static_cast<t_f32>(PCG32CalcNext(&rng->pcg32)) / 4294967296.0f;
    }

    static t_u64 GetOSEntropy() {
#ifdef ZCL_PLATFORM_WINDOWS
        zcl::t_u64 result = 0;

        const NTSTATUS status = BCryptGenRandom(nullptr, reinterpret_cast<UCHAR *>(&result), static_cast<ULONG>(ZCL_SIZE_OF(result)), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
        ZCL_REQUIRE(status >= 0);

        return result;
#else
    #error "Platform support not complete!" // @todo
#endif
    }

    t_u64 RandGenSeed() {
        return GetOSEntropy();
    }

    static t_u64 SplitMix64Next(t_u64 *const x) {
        t_u64 z = (*x += 0x9E3779B97F4A7C15ull);
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
        return z ^ (z >> 31);
    }

    t_u64 Scramble(t_u64 *const x) {
        return SplitMix64Next(x);
    }
}
