#pragma once

#include <type_traits>
#include <limits>
#include <concepts>

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

#define ZF_CONCAT_IMPL(a, b) a##b
#define ZF_CONCAT(a, b) ZF_CONCAT_IMPL(a, b)

    namespace detail {
        template <typename tp_func>
        struct s_defer {
            tp_func func;

            s_defer(const tp_func f) : func(f) {}

            ~s_defer() {
                func();
            }
        };
    }

#define ZF_DEFER(x) const auto ZF_CONCAT(defer_, ZF_CONCAT(l, __LINE__)) = zf::detail::s_defer([&]() x)

    static_assert(CHAR_BIT == 8);

    using t_u8 = unsigned char;
    static_assert(sizeof(t_u8) == 1);
    constexpr t_u8 g_u8_max = std::numeric_limits<t_u8>::max();

    using t_i8 = signed char;
    static_assert(sizeof(t_i8) == 1);
    constexpr t_i8 g_i8_max = std::numeric_limits<t_i8>::max();

    using t_u16 = unsigned short;
    static_assert(sizeof(t_u16) == 2);
    constexpr t_u16 g_u16_max = std::numeric_limits<t_u16>::max();

    using t_i16 = signed short;
    static_assert(sizeof(t_i16) == 2);
    constexpr t_i16 g_i16_max = std::numeric_limits<t_i16>::max();

    using t_u32 = unsigned int;
    static_assert(sizeof(t_u32) == 4);
    constexpr t_u32 g_u32_max = std::numeric_limits<t_u32>::max();

    using t_i32 = signed int;
    static_assert(sizeof(t_i32) == 4);
    constexpr t_i32 g_i32_max = std::numeric_limits<t_i32>::max();

    using t_u64 = unsigned long long;
    static_assert(sizeof(t_u64) == 8);
    constexpr t_u64 g_u64_max = std::numeric_limits<t_u64>::max();

    using t_i64 = signed long long;
    static_assert(sizeof(t_i64) == 8);
    constexpr t_i64 g_i64_max = std::numeric_limits<t_i64>::max();

    using t_f32 = float;
    static_assert(sizeof(t_f32) == 4);
    constexpr t_f32 g_f32_max = std::numeric_limits<t_f32>::max();

    using t_f64 = double;
    static_assert(sizeof(t_f64) == 8);
    constexpr t_f64 g_f64_max = std::numeric_limits<t_f64>::max();

    using t_b8 = bool;
    static_assert(sizeof(t_b8) == 1);

    using t_uintptr = uintptr_t;
    static_assert(sizeof(t_uintptr) == 8);

    template <typename tp_type>
    using t_cvref_removed = std::remove_cvref_t<tp_type>;

#define ZF_SIZE_OF(x) static_cast<zf::t_i32>(sizeof(x))
#define ZF_SIZE_IN_BITS(x) (8 * ZF_SIZE_OF(x))

#define ZF_ALIGN_OF(x) static_cast<zf::t_i32>(alignof(x))

#define ZF_IN_CONSTEXPR() std::is_constant_evaluated()

    namespace detail {
        void TryBreakingIntoDebuggerIf(const t_b8 cond);

        [[noreturn]] void AssertError(const char *const cond_cstr, const char *const func_name_cstr, const char *const file_name_cstr, const t_i32 line);

#ifdef ZF_DEBUG
    #define ZF_DEBUG_BREAK() detail::TryBreakingIntoDebuggerIf(true)
    #define ZF_DEBUG_BREAK_IF(cond) detail::TryBreakingIntoDebuggerIf(cond)

    #define ZF_ASSERT(cond)                                                       \
        do {                                                                      \
            if (!ZF_IN_CONSTEXPR() && !(cond)) {                                  \
                zf::detail::AssertError(#cond, __FUNCTION__, __FILE__, __LINE__); \
            }                                                                     \
        } while (0)
#else
    #define ZF_DEBUG_BREAK() static_cast<void>(0)
    #define ZF_DEBUG_BREAK_IF(cond) static_cast<void>(0)
    #define ZF_ASSERT(cond) static_cast<void>(0)
#endif

        [[noreturn]] void FatalError(const char *const func_name_cstr, const char *const file_name_cstr, const t_i32 line, const char *const cond_cstr = nullptr);

#define ZF_FATAL() zf::detail::FatalError(__FUNCTION__, __FILE__, __LINE__)
#define ZF_UNREACHABLE() ZF_FATAL()

#define ZF_REQUIRE(cond)                                                     \
    do {                                                                     \
        if (!(cond)) {                                                       \
            zf::detail::FatalError(__FUNCTION__, __FILE__, __LINE__, #cond); \
        }                                                                    \
    } while (0)
    }

    // "Simple" meaning that it's safe to use with arenas and C-style memory operations.
    template <typename tp_type>
    concept co_simple = std::is_trivially_default_constructible_v<tp_type> && std::is_trivially_destructible_v<tp_type> && std::is_trivially_copyable_v<tp_type> && std::is_standard_layout_v<tp_type>;

    template <typename tp_type>
    concept co_integral = std::is_same_v<tp_type, t_i8> || std::is_same_v<tp_type, t_u8> || std::is_same_v<tp_type, t_i16> || std::is_same_v<tp_type, t_u16> || std::is_same_v<tp_type, t_i32> || std::is_same_v<tp_type, t_u32> || std::is_same_v<tp_type, t_i64> || std::is_same_v<tp_type, t_u64>;

    template <typename tp_type> concept co_integral_unsigned = co_integral<tp_type> && std::is_unsigned_v<tp_type>;
    template <typename tp_type> concept co_integral_signed = co_integral<tp_type> && std::is_signed_v<tp_type>;
    template <typename tp_type> concept co_floating_point = std::is_floating_point_v<tp_type>;
    template <typename tp_type> concept co_numeric = co_integral<tp_type> || co_floating_point<tp_type>;

    template <typename tp_type> concept co_ptr = std::is_pointer_v<tp_type>;
    template <typename tp_type> concept co_const = std::is_const_v<tp_type>;
    template <typename tp_type> concept co_union = std::is_union_v<tp_type>;
    template <typename tp_type> concept co_enum = std::is_enum_v<tp_type>;
    template <typename tp_type> concept co_scalar = std::is_scalar_v<tp_type>;

    template <typename tp_type_a, typename tp_type_b>
    concept co_same = std::same_as<tp_type_a, tp_type_b>;

    template <typename tp_type>
    concept co_cstr = co_same<std::remove_cv_t<std::remove_pointer_t<std::remove_extent_t<std::remove_reference_t<tp_type>>>>, char>;

    // Return true iff a and b are equal.
    template <co_simple tp_type>
    using t_comparator_bin = t_b8 (*)(const tp_type &a, const tp_type &b);

    template <co_simple tp_type>
    constexpr t_comparator_bin<tp_type> g_comparator_bin_default =
        [](const tp_type &a, const tp_type &b) {
            return a == b;
        };

    // Return a negative result if a < b, 0 if a == b, and a positive result if a > b.
    template <co_simple tp_type>
    using t_comparator_ord = t_i32 (*)(const tp_type &a, const tp_type &b);

    template <co_simple tp_type>
    constexpr t_comparator_ord<tp_type> g_comparator_ord_default =
        [](const tp_type &a, const tp_type &b) {
            if (a == b) {
                return 0;
            } else if (a < b) {
                return -1;
            } else {
                return 1;
            }
        };

    constexpr t_i32 KilobytesToBytes(const t_i32 n) {
        return (1 << 10) * n;
    }

    constexpr t_i32 MegabytesToBytes(const t_i32 n) {
        return (1 << 20) * n;
    }

    constexpr t_i32 GigabytesToBytes(const t_i32 n) {
        return (1 << 30) * n;
    }

    constexpr t_i32 BitsToBytes(const t_i32 n) {
        return (n + 7) / 8;
    }

    constexpr t_i32 BytesToBits(const t_i32 n) {
        return n * 8;
    }

    // Is n a power of 2?
    constexpr t_b8 IsAlignmentValid(const t_i32 n) {
        return n > 0 && (n & (n - 1)) == 0;
    }

    // Take n up to the next multiple of the alignment.
    constexpr t_i32 AlignForward(const t_i32 n, const t_i32 alignment) {
        ZF_ASSERT(IsAlignmentValid(alignment));
        return (n + alignment - 1) & ~(alignment - 1);
    }

#define ZF_MIN(a, b) ((a) <= (b) ? (a) : (b))
#define ZF_MAX(a, b) ((a) >= (b) ? (a) : (b))

    template <co_simple tp_type>
    constexpr void Swap(tp_type *const a, tp_type *const b) {
        const tp_type temp = *a;
        *a = *b;
        *b = temp;
    }

    template <co_numeric tp_type>
    constexpr tp_type Abs(const tp_type n) {
        return n < 0 ? -n : n;
    }

    template <co_numeric tp_type>
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

    template <co_numeric tp_type>
    constexpr t_i32 Sign(const tp_type n) {
        if (n > 0) {
            return 1;
        } else if (n < 0) {
            return -1;
        }

        return 0;
    }

    template <co_integral tp_type>
    constexpr tp_type WrapUpper(const tp_type val, const tp_type max_excl) {
        return ((val % max_excl) + max_excl) % max_excl;
    }

    template <co_integral tp_type>
    constexpr tp_type Wrap(const tp_type val, const tp_type min, const tp_type max_excl) {
        return min + WrapUpper(val - min, max_excl - min);
    }

    // ============================================================
}
