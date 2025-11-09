#pragma once

#include <climits>

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

    template<typename tp_type> struct s_is_integral { static constexpr t_b8 sm_value = false; };
    template<> struct s_is_integral<t_s8> { static constexpr t_b8 sm_value = true; };
    template<> struct s_is_integral<t_u8> { static constexpr t_b8 sm_value = true; };
    template<> struct s_is_integral<t_s16> { static constexpr t_b8 sm_value = true; };
    template<> struct s_is_integral<t_u16> { static constexpr t_b8 sm_value = true; };
    template<> struct s_is_integral<t_s32> { static constexpr t_b8 sm_value = true; };
    template<> struct s_is_integral<t_u32> { static constexpr t_b8 sm_value = true; };
    template<> struct s_is_integral<t_s64> { static constexpr t_b8 sm_value = true; };
    template<> struct s_is_integral<t_u64> { static constexpr t_b8 sm_value = true; };

    template<typename tp_type> struct s_is_floating_point { static constexpr t_b8 sm_value = false; };
    template<> struct s_is_floating_point<t_f32> { static constexpr t_b8 sm_value = true; };
    template<> struct s_is_floating_point<t_f64> { static constexpr t_b8 sm_value = true; };

    template<typename tp_type>
    concept co_integral = s_is_integral<tp_type>::sm_value;

    template<typename tp_type>
    concept co_floating_point = s_is_floating_point<tp_type>::sm_value;

    template<typename tp_type>
    concept co_numeric = s_is_integral<tp_type>::sm_value || s_is_floating_point<tp_type>::sm_value;
}
