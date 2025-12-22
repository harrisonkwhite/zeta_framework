#pragma once

#include <climits>

namespace zf {
#ifdef _WIN32
    #define ZF_PLATFORM_WINDOWS
#endif

#if defined(__APPLE__) && defined(__MACH__)
    #define ZF_PLATFORM_MACOS
#endif

#ifdef __linux__
    #define ZF_PLATFORM_LINUX
#endif

#ifndef NDEBUG
    #define ZF_DEBUG
#endif

#if defined(__clang__) || defined(__GNUC__)
    #define ZF_IN_CONSTEXPR() __builtin_is_constant_evaluated()
#elif defined(_MSC_VER)
    #define ZF_IN_CONSTEXPR() __builtin_is_constant_evaluated()
#else
    #define ZF_IN_CONSTEXPR() 0
#endif

#define ZF_SIZE_OF(x) static_cast<zf::t_i32>(sizeof(x))
#define ZF_SIZE_IN_BITS(x) (8 * ZF_SIZE_OF(x))

#define ZF_ALIGN_OF(x) static_cast<zf::t_i32>(alignof(x))

#define ZF_CONCAT_IMPL(a, b) a##b
#define ZF_CONCAT(a, b) ZF_CONCAT_IMPL(a, b)

    namespace internal {
        template <typename tp_func>
        struct s_defer {
            tp_func func;

            s_defer(const tp_func f) : func(f) {}

            ~s_defer() {
                func();
            }
        };
    }

#define ZF_DEFER(x) const auto ZF_CONCAT(defer_, ZF_CONCAT(l, __LINE__)) = zf::internal::s_defer([&]() x)

    // ============================================================
    // @section: Types
    // ============================================================
    static_assert(CHAR_BIT == 8);

    using t_i8 = signed char;
    static_assert(sizeof(t_i8) == 1);

    using t_u8 = unsigned char;
    static_assert(sizeof(t_u8) == 1);

    using t_i16 = signed short;
    static_assert(sizeof(t_i16) == 2);

    using t_u16 = unsigned short;
    static_assert(sizeof(t_u16) == 2);

    using t_i32 = signed int;
    static_assert(sizeof(t_i32) == 4);

    using t_u32 = unsigned int;
    static_assert(sizeof(t_u32) == 4);

    using t_i64 = signed long long;
    static_assert(sizeof(t_i64) == 8);

    using t_u64 = unsigned long long;
    static_assert(sizeof(t_u64) == 8);

    using t_f32 = float;
    static_assert(sizeof(t_f32) == 4);

    using t_f64 = double;
    static_assert(sizeof(t_f64) == 8);

    using t_b8 = bool;
    static_assert(sizeof(t_b8) == 1);

    using t_uintptr = uintptr_t;
    static_assert(sizeof(t_uintptr) == 8);

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
    struct s_is_integral<t_i8> {
        static constexpr t_b8 g_val = true;
    };

    template <>
    struct s_is_integral<t_u8> {
        static constexpr t_b8 g_val = true;
    };

    template <>
    struct s_is_integral<t_i16> {
        static constexpr t_b8 g_val = true;
    };

    template <>
    struct s_is_integral<t_u16> {
        static constexpr t_b8 g_val = true;
    };

    template <>
    struct s_is_integral<t_i32> {
        static constexpr t_b8 g_val = true;
    };

    template <>
    struct s_is_integral<t_u32> {
        static constexpr t_b8 g_val = true;
    };

    template <>
    struct s_is_integral<t_i64> {
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
    struct s_is_signed_integral<t_i8> {
        static constexpr t_b8 g_val = true;
    };

    template <>
    struct s_is_signed_integral<t_i16> {
        static constexpr t_b8 g_val = true;
    };

    template <>
    struct s_is_signed_integral<t_i32> {
        static constexpr t_b8 g_val = true;
    };

    template <>
    struct s_is_signed_integral<t_i64> {
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

    // If a < b, return a negative result, if a == b, return 0, and if a > b, return a positive result.
    template <typename tp_type>
    using t_ord_comparator = t_i32 (*)(const tp_type &a, const tp_type &b);

    template <typename tp_type>
    t_i32 DefaultOrdComparator(const tp_type &a, const tp_type &b) {
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
    namespace internal {
        [[noreturn]] void AssertError(const char *const cond, const char *const func_name, const char *const file_name, const t_i32 line);

#ifdef ZF_DEBUG
    #define ZF_ASSERT(cond)                                                         \
        do {                                                                        \
            if (!ZF_IN_CONSTEXPR() && !(cond)) {                                    \
                zf::internal::AssertError(#cond, __FUNCTION__, __FILE__, __LINE__); \
            }                                                                       \
        } while (0)
#else
    #define ZF_ASSERT(cond) static_cast<void>(0)
#endif

        [[noreturn]] void FatalError(const char *const func_name, const char *const file_name, const t_i32 line, const char *const cond = nullptr);

#define ZF_FATAL() zf::internal::FatalError(__FUNCTION__, __FILE__, __LINE__)

#define ZF_REQUIRE(cond)                                                       \
    do {                                                                       \
        if (!(cond)) {                                                         \
            zf::internal::FatalError(__FUNCTION__, __FILE__, __LINE__, #cond); \
        }                                                                      \
    } while (0)
    }

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

        if (n < min) {
            return min;
        }

        if (n > max) {
            return max;
        }

        return n;
    }

    template <c_numeric tp_type>
    constexpr t_i32 Sign(const tp_type n) {
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
