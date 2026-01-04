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
        template <typename TPFunc>
        struct Defer {
            TPFunc func;

            Defer(const TPFunc f) : func(f) {}

            ~Defer() {
                func();
            }
        };
    }

#define ZF_DEFER(x) const auto ZF_CONCAT(defer_, ZF_CONCAT(l, __LINE__)) = zf::detail::Defer([&]() x)

    static_assert(CHAR_BIT == 8);

    using U8 = unsigned char;
    static_assert(sizeof(U8) == 1);
    constexpr U8 g_u8_max = std::numeric_limits<U8>::max();

    using I8 = signed char;
    static_assert(sizeof(I8) == 1);
    constexpr I8 g_i8_max = std::numeric_limits<I8>::max();

    using U16 = unsigned short;
    static_assert(sizeof(U16) == 2);
    constexpr U16 g_u16_max = std::numeric_limits<U16>::max();

    using I16 = signed short;
    static_assert(sizeof(I16) == 2);
    constexpr I16 g_i16_max = std::numeric_limits<I16>::max();

    using U32 = unsigned int;
    static_assert(sizeof(U32) == 4);
    constexpr U32 g_u32_max = std::numeric_limits<U32>::max();

    using I32 = signed int;
    static_assert(sizeof(I32) == 4);
    constexpr I32 g_i32_max = std::numeric_limits<I32>::max();

    using U64 = unsigned long long;
    static_assert(sizeof(U64) == 8);
    constexpr U64 g_u64_max = std::numeric_limits<U64>::max();

    using I64 = signed long long;
    static_assert(sizeof(I64) == 8);
    constexpr I64 g_i64_max = std::numeric_limits<I64>::max();

    using F32 = float;
    static_assert(sizeof(F32) == 4);
    constexpr F32 g_f32_max = std::numeric_limits<F32>::max();

    using F64 = double;
    static_assert(sizeof(F64) == 8);
    constexpr F64 g_f64_max = std::numeric_limits<F64>::max();

    using B8 = bool;
    static_assert(sizeof(B8) == 1);

    using UIntPtr = uintptr_t;
    static_assert(sizeof(UIntPtr) == 8);

    template <typename TPType>
    using CVRefRemoved = std::remove_cvref<TPType>::type;

#define ZF_SIZE_OF(x) static_cast<zf::I32>(sizeof(x))
#define ZF_SIZE_IN_BITS(x) (8 * ZF_SIZE_OF(x))

#define ZF_ALIGN_OF(x) static_cast<zf::I32>(alignof(x))

#define ZF_IN_CONSTEXPR() std::is_constant_evaluated()

    namespace detail {
        void try_breaking_into_debugger_if(const B8 cond);

        [[noreturn]] void handle_assert_error(const char *const cond_cstr, const char *const func_name_cstr, const char *const file_name_cstr, const I32 line);

#ifdef ZF_DEBUG
    #define ZF_DEBUG_BREAK() detail::try_breaking_into_debugger_if(true)
    #define ZF_DEBUG_BREAK_IF(cond) detail::try_breaking_into_debugger_if(cond)

    #define ZF_ASSERT(cond)                                                                   \
        do {                                                                                  \
            if (!ZF_IN_CONSTEXPR()) {                                                         \
                if (!(cond)) {                                                                \
                    zf::detail::handle_assert_error(#cond, __FUNCTION__, __FILE__, __LINE__); \
                }                                                                             \
            }                                                                                 \
        } while (0)
#else
    #define ZF_DEBUG_BREAK() static_cast<void>(0)
    #define ZF_DEBUG_BREAK_IF(cond) static_cast<void>(0)
    #define ZF_ASSERT(cond) static_cast<void>(0)
#endif

        [[noreturn]] void handle_fatal_error(const char *const func_name_cstr, const char *const file_name_cstr, const I32 line, const char *const cond_cstr = nullptr);

#define ZF_FATAL() zf::detail::handle_fatal_error(__FUNCTION__, __FILE__, __LINE__)
#define ZF_UNREACHABLE() ZF_FATAL()

#define ZF_REQUIRE(cond)                                                                 \
    do {                                                                                 \
        if (!ZF_IN_CONSTEXPR()) {                                                        \
            if (!(cond)) {                                                               \
                zf::detail::handle_fatal_error(__FUNCTION__, __FILE__, __LINE__, #cond); \
            }                                                                            \
        }                                                                                \
    } while (0)
    }

    // "Simple" meaning that it's safe to use with arenas and C-style memory operations.
    template <typename T>
    concept Simple = std::is_trivially_default_constructible_v<T> && std::is_trivially_destructible_v<T> && std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>;

    template <typename T>
    concept Integral = std::is_same_v<T, I8> || std::is_same_v<T, U8> || std::is_same_v<T, I16> || std::is_same_v<T, U16> || std::is_same_v<T, I32> || std::is_same_v<T, U32> || std::is_same_v<T, I64> || std::is_same_v<T, U64>;

    template <typename T> concept IntegralUnsigned = Integral<T> && std::is_unsigned_v<T>;
    template <typename T> concept IntegralSigned = Integral<T> && std::is_signed_v<T>;
    template <typename T> concept FloatingPoint = std::is_floating_point_v<T>;
    template <typename T> concept Numeric = Integral<T> || FloatingPoint<T>;

    template <typename T> concept Pointer = std::is_pointer_v<T>;
    template <typename T> concept Const = std::is_const_v<T>;
    template <typename T> concept Union = std::is_union_v<T>;
    template <typename T> concept Enum = std::is_enum_v<T>;
    template <typename T> concept Scalar = std::is_scalar_v<T>;

    template <typename T_a, typename T_b>
    concept co_same = std::same_as<T_a, T_b>;

    template <typename T>
    concept co_cstr = co_same<std::remove_cv_t<std::remove_pointer_t<std::remove_extent_t<std::remove_reference_t<T>>>>, char>;

    // Return true iff a and b are equal.
    template <Simple T>
    using t_comparator_bin = B8 (*)(const T &a, const T &b);

    template <Simple T>
    inline const t_comparator_bin<T> g_comparator_bin_default =
        [](const T &a, const T &b) {
            return a == b;
        };

    // Return a negative result if a < b, 0 if a == b, and a positive result if a > b.
    template <Simple T>
    using t_comparator_ord = I32 (*)(const T &a, const T &b);

    template <Simple T>
    inline const t_comparator_ord<T> g_comparator_ord_default =
        [](const T &a, const T &b) {
            if (a == b) {
                return 0;
            } else if (a < b) {
                return -1;
            } else {
                return 1;
            }
        };

    constexpr I32 get_kilobytes_to_bytes(const I32 n) { return (1 << 10) * n; }
    constexpr I32 get_megabytes_to_bytes(const I32 n) { return (1 << 20) * n; }
    constexpr I32 get_gigabytes_to_bytes(const I32 n) { return (1 << 30) * n; }
    constexpr I32 get_bits_to_bytes(const I32 n) { return (n + 7) / 8; }
    constexpr I32 get_bytes_to_bits(const I32 n) { return n * 8; }

    // Is n a power of 2?
    constexpr B8 get_is_alignment_valid(const I32 n) {
        return n > 0 && (n & (n - 1)) == 0;
    }

    // Take n up to the next multiple of the alignment.
    constexpr I32 get_aligned_forward(const I32 n, const I32 alignment) {
        ZF_ASSERT(get_is_alignment_valid(alignment));
        return (n + alignment - 1) & ~(alignment - 1);
    }

#define ZF_MIN(a, b) ((a) <= (b) ? (a) : (b))
#define ZF_MAX(a, b) ((a) >= (b) ? (a) : (b))

    template <Simple T>
    void swap(T *const a, T *const b) {
        const T temp = *a;
        *a = *b;
        *b = temp;
    }

    template <Numeric T>
    T get_abs(const T n) {
        return n < 0 ? -n : n;
    }

    template <Numeric T>
    T clamp(const T n, const T min, const T max) {
        ZF_ASSERT(min <= max);

        if (n < min) {
            return min;
        }

        if (n > max) {
            return max;
        }

        return n;
    }

    template <Numeric T>
    I32 get_sign(const T n) {
        if (n > 0) {
            return 1;
        } else if (n < 0) {
            return -1;
        }

        return 0;
    }

    template <Integral T>
    T WrapUpper(const T val, const T max_excl) {
        return ((val % max_excl) + max_excl) % max_excl;
    }

    template <Integral T>
    T get_wrapped(const T val, const T min, const T max_excl) {
        return min + WrapUpper(val - min, max_excl - min);
    }
}
