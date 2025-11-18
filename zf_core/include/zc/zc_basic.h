#pragma once

#include <climits>

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

#if defined(__cpp_lib_is_constant_evaluated)
    #define ZF_IS_CONSTEXPR() (std::is_constant_evaluated())
#elif defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
    #define ZF_IS_CONSTEXPR() (__builtin_is_constant_evaluated())
#else
    #define ZF_IS_CONSTEXPR() (false)
#endif

#define ZF_SIZE_OF(x) static_cast<zf::t_size>(sizeof(x))
#define ZF_SIZE_IN_BITS(x) (8 * ZF_SIZE_OF(x))

namespace zf {
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

    // Why signed for this?
    // 1. Mixing signed and unsigned can lead to strange overflow bugs that cannot always be caught by warnings. Better to be consistent and have predictability.
    // 2. The signed 64-bit range is more than sufficient for realistic use cases.
    // 3. If you want a value to be 0 or greater, ASSERT that it is!
    using t_size = t_s64;

    template<typename tp_type> struct s_is_integral { static constexpr t_b8 g_value = false; };
    template<> struct s_is_integral<t_s8> { static constexpr t_b8 g_value = true; };
    template<> struct s_is_integral<t_u8> { static constexpr t_b8 g_value = true; };
    template<> struct s_is_integral<t_s16> { static constexpr t_b8 g_value = true; };
    template<> struct s_is_integral<t_u16> { static constexpr t_b8 g_value = true; };
    template<> struct s_is_integral<t_s32> { static constexpr t_b8 g_value = true; };
    template<> struct s_is_integral<t_u32> { static constexpr t_b8 g_value = true; };
    template<> struct s_is_integral<t_s64> { static constexpr t_b8 g_value = true; };
    template<> struct s_is_integral<t_u64> { static constexpr t_b8 g_value = true; };

    template<typename tp_type> struct s_is_signed_integral { static constexpr t_b8 g_value = false; };
    template<> struct s_is_signed_integral<t_s8> { static constexpr t_b8 g_value = true; };
    template<> struct s_is_signed_integral<t_s16> { static constexpr t_b8 g_value = true; };
    template<> struct s_is_signed_integral<t_s32> { static constexpr t_b8 g_value = true; };
    template<> struct s_is_signed_integral<t_s64> { static constexpr t_b8 g_value = true; };

    template<typename tp_type> struct s_is_unsigned_integral { static constexpr t_b8 g_value = false; };
    template<> struct s_is_unsigned_integral<t_u8> { static constexpr t_b8 g_value = true; };
    template<> struct s_is_unsigned_integral<t_u16> { static constexpr t_b8 g_value = true; };
    template<> struct s_is_unsigned_integral<t_u32> { static constexpr t_b8 g_value = true; };
    template<> struct s_is_unsigned_integral<t_u64> { static constexpr t_b8 g_value = true; };

    template<typename tp_type> struct s_is_floating_point { static constexpr t_b8 g_value = false; };
    template<> struct s_is_floating_point<t_f32> { static constexpr t_b8 g_value = true; };
    template<> struct s_is_floating_point<t_f64> { static constexpr t_b8 g_value = true; };

    template<typename tp_type>
    concept co_integral = s_is_integral<tp_type>::g_value;

    template<typename tp_type>
    concept co_signed_integral = s_is_signed_integral<tp_type>::g_value;

    template<typename tp_type>
    concept co_unsigned_integral = s_is_unsigned_integral<tp_type>::g_value;

    template<typename tp_type>
    concept co_floating_point = s_is_floating_point<tp_type>::g_value;

    template<typename tp_type>
    concept co_numeric = s_is_integral<tp_type>::g_value || s_is_floating_point<tp_type>::g_value;

    template<typename tp_type> struct s_is_const { static constexpr t_b8 g_value = false; };
    template<typename tp_type> struct s_is_const<tp_type const> { static constexpr t_b8 g_value = true; };

    // If a < b, return a negative result, if a == b, return 0, and if a > b, return a positive result.
    template<typename tp_type>
    using t_comparator = t_s32 (*)(const tp_type& a, const tp_type& b);

    template<typename tp_type>
    t_s32 DefaultComparator(const tp_type& a, const tp_type& b) {
        if (a == b) {
            return 0;
        } else if (a < b) {
            return -1;
        } else {
            return 1;
        }
    }

    template<typename tp_type>
    constexpr void Swap(tp_type& a, tp_type& b) {
        const tp_type temp = a;
        a = b;
        b = temp;
    }
}
