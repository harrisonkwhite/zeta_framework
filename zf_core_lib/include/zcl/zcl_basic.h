#pragma once

#include <climits>
#include <type_traits> // The only permitted use of STL.

namespace zf {
#ifdef _WIN32
    #define ZF_PLATFORM_WINDOWS
#endif

#ifdef __linux__
    #define ZF_PLATFORM_LINUX
#endif

#if defined(__APPLE__) && defined(__MACH__)
    #define ZF_PLATFORM_MACOS
#endif

#ifndef NDEBUG
    #define ZF_DEBUG
#endif

#define ZF_IN_CONSTEXPR() std::is_constant_evaluated()

#define ZF_SIZE_OF(x) static_cast<zf::t_size>(sizeof(x))
#define ZF_SIZE_IN_BITS(x) (8 * ZF_SIZE_OF(x))

#define ZF_ALIGN_OF(x) static_cast<zf::t_size>(alignof(x))

    template <typename tp_func>
    struct i_s_defer {
        tp_func func;

        i_s_defer(const tp_func f) : func(f) {}

        ~i_s_defer() {
            func();
        }
    };

#define ZF_CONCAT_IMPL(a, b) a##b
#define ZF_CONCAT(a, b) ZF_CONCAT_IMPL(a, b)

#define ZF_DEFER(x) auto ZF_CONCAT(defer_, ZF_CONCAT(l, __LINE__)) = zf::i_s_defer([&]() x)

    // ============================================================
    // @section: Types
    // ============================================================
    static_assert(CHAR_BIT == 8);

    using t_s8 = signed char;
    static_assert(sizeof(t_s8) == 1);

    using t_u8 = unsigned char;
    static_assert(sizeof(t_u8) == 1);

    using t_s16 = signed short;
    static_assert(sizeof(t_s16) == 2);

    using t_u16 = unsigned short;
    static_assert(sizeof(t_u16) == 2);

    using t_s32 = signed int;
    static_assert(sizeof(t_s32) == 4);

    using t_u32 = unsigned int;
    static_assert(sizeof(t_u32) == 4);

    using t_s64 = signed long long;
    static_assert(sizeof(t_s64) == 8);

    using t_u64 = unsigned long long;
    static_assert(sizeof(t_u64) == 8);

    using t_f32 = float;
    static_assert(sizeof(t_f32) == 4);

    using t_f64 = double;
    static_assert(sizeof(t_f64) == 8);

    using t_b8 = bool;
    static_assert(sizeof(t_b8) == 1);

    using t_uintptr = uintptr_t; // Generally 64 bits EXCEPT for WASM.

    // Why signed for this?
    // 1. Mixing signed and unsigned can lead to strange overflow bugs that cannot always be
    // caught by warnings. Better to be consistent and have predictability.
    // 2. The signed 64-bit range is more than sufficient for realistic use cases.
    // 3. If you want a value to be 0 or greater, ASSERT that it is!
    // 4. -1 is far more useful as a sentinel than the positive upper bound is since it is more
    // commonly an outlier.
    using t_size = intptr_t; // Generally 64 bits EXCEPT for WASM.

    template <typename tp_type_a, typename tp_type_b>
    struct s_is_same {
        static constexpr t_b8 g_val = false;
    };

    template <typename tp_type>
    struct s_is_same<tp_type, tp_type> {
        static constexpr t_b8 g_val = true;
    };

    template <t_b8 tp_cond, typename tp_then, typename tp_else>
    struct s_conditional {
        using t_type = tp_then;
    };

    template <typename tp_then, typename tp_else>
    struct s_conditional<false, tp_then, tp_else> {
        using t_type = tp_else;
    };

    template <typename tp_type>
    struct s_is_integral {
        static constexpr t_b8 g_val = false;
    };

    template <>
    struct s_is_integral<t_s8> {
        static constexpr t_b8 g_val = true;
    };

    template <>
    struct s_is_integral<t_u8> {
        static constexpr t_b8 g_val = true;
    };

    template <>
    struct s_is_integral<t_s16> {
        static constexpr t_b8 g_val = true;
    };

    template <>
    struct s_is_integral<t_u16> {
        static constexpr t_b8 g_val = true;
    };

    template <>
    struct s_is_integral<t_s32> {
        static constexpr t_b8 g_val = true;
    };

    template <>
    struct s_is_integral<t_u32> {
        static constexpr t_b8 g_val = true;
    };

    template <>
    struct s_is_integral<t_s64> {
        static constexpr t_b8 g_val = true;
    };

    template <>
    struct s_is_integral<t_u64> {
        static constexpr t_b8 g_val = true;
    };

    template <typename tp_type>
    struct s_is_signed_integral {
        static constexpr t_b8 g_val = false;
    };

    template <>
    struct s_is_signed_integral<t_s8> {
        static constexpr t_b8 g_val = true;
    };

    template <>
    struct s_is_signed_integral<t_s16> {
        static constexpr t_b8 g_val = true;
    };

    template <>
    struct s_is_signed_integral<t_s32> {
        static constexpr t_b8 g_val = true;
    };

    template <>
    struct s_is_signed_integral<t_s64> {
        static constexpr t_b8 g_val = true;
    };

    template <typename tp_type>
    struct s_is_unsigned_integral {
        static constexpr t_b8 g_val = false;
    };

    template <>
    struct s_is_unsigned_integral<t_u8> {
        static constexpr t_b8 g_val = true;
    };

    template <>
    struct s_is_unsigned_integral<t_u16> {
        static constexpr t_b8 g_val = true;
    };

    template <>
    struct s_is_unsigned_integral<t_u32> {
        static constexpr t_b8 g_val = true;
    };

    template <>
    struct s_is_unsigned_integral<t_u64> {
        static constexpr t_b8 g_val = true;
    };

    template <typename tp_type>
    struct s_is_floating_point {
        static constexpr t_b8 g_val = false;
    };

    template <>
    struct s_is_floating_point<t_f32> {
        static constexpr t_b8 g_val = true;
    };

    template <>
    struct s_is_floating_point<t_f64> {
        static constexpr t_b8 g_val = true;
    };

    template <typename tp_type>
    concept c_integral = s_is_integral<tp_type>::g_val;

    template <typename tp_type>
    concept c_signed_integral = s_is_signed_integral<tp_type>::g_val;

    template <typename tp_type>
    concept c_unsigned_integral = s_is_unsigned_integral<tp_type>::g_val;

    template <typename tp_type>
    concept c_floating_point = s_is_floating_point<tp_type>::g_val;

    template <typename tp_type>
    concept c_numeric = s_is_integral<tp_type>::g_val || s_is_floating_point<tp_type>::g_val;

    template <typename tp_type>
    struct s_is_const {
        static constexpr t_b8 g_val = false;
    };

    template <typename tp_type>
    struct s_is_const<tp_type const> {
        static constexpr t_b8 g_val = true;
    };

    template <typename tp_type>
    struct s_is_ptr {
        static constexpr t_b8 g_val = false;
    };

    template <typename tp_type>
    struct s_is_ptr<tp_type *> {
        static constexpr t_b8 g_val = true;
    };

    // Return true iff a and b are equal.
    template <typename tp_type>
    using t_bin_comparator = t_b8 (*)(const tp_type &a, const tp_type &b);

    template <typename tp_type>
    t_b8 DefaultBinComparator(const tp_type &a, const tp_type &b) {
        return a == b;
    }

    // If a < b, return a negative result, if a == b, return 0, and if a > b, return a positive
    // result.
    template <typename tp_type>
    using t_ord_comparator = t_s32 (*)(const tp_type &a, const tp_type &b);

    template <typename tp_type>
    t_s32 DefaultOrdComparator(const tp_type &a, const tp_type &b) {
        if (a == b) {
            return 0;
        } else if (a < b) {
            return -1;
        } else {
            return 1;
        }
    }

    // ============================================================
    // @section: Key Debugging Features
    // ============================================================
    void I_ReportAssertError(const char *const cond_raw, const char *const func_name_raw,
                             const char *const file_name_raw, const t_s32 line);

#ifdef ZF_DEBUG
    #define ZF_ASSERT(cond)                                                                   \
        do {                                                                                  \
            if (!ZF_IN_CONSTEXPR() && !(cond)) {                                              \
                zf::I_ReportAssertError(#cond, __FUNCTION__, __FILE__, __LINE__);             \
            }                                                                                 \
        } while (0)
#else
    #define ZF_ASSERT(cond) static_cast<void>(0)
#endif

    void I_ReportError(const char *const func_name_raw, const char *const file_name_raw,
                       const t_s32 line);

#define ZF_REPORT_ERROR() zf::I_ReportError(__FUNCTION__, __FILE__, __LINE__)

    void ShowErrorBox(const char *const title_raw, const char *const contents_raw);

    // ============================================================
    // @section: Essential Utilities
    // ============================================================
#define ZF_MIN(a, b) ((a) <= (b) ? (a) : (b))
#define ZF_MAX(a, b) ((a) >= (b) ? (a) : (b))

    template <typename tp_type>
    constexpr void Swap(tp_type &a, tp_type &b) {
        const tp_type temp = a;
        a = b;
        b = temp;
    }

    template <c_numeric tp_type>
    constexpr tp_type Abs(const tp_type n) {
        return n < 0 ? -n : n;
    }

    template <c_numeric tp_type>
    constexpr tp_type Clamp(const tp_type n, const tp_type min, const tp_type max) {
        ZF_ASSERT(min <= max);
        return n < 0 ? -n : n;
    }

    template <c_numeric tp_type>
    constexpr t_s32 Sign(const tp_type n) {
        if (n > 0) {
            return 1;
        } else if (n < 0) {
            return -1;
        }

        return 0;
    }

    template <c_integral tp_type>
    constexpr tp_type WrapUpper(const tp_type val, const tp_type max_excl) {
        return ((val % max_excl) + max_excl) % max_excl;
    }

    template <c_integral tp_type>
    constexpr tp_type Wrap(const tp_type val, const tp_type min, const tp_type max_excl) {
        return min + WrapUpper(val - min, max_excl - min);
    }
}
