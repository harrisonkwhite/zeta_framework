#pragma once

#include <type_traits>
#include <limits>
#include <concepts>

namespace zcl {
    static_assert(CHAR_BIT == 8);

    using t_u8 = unsigned char;
    static_assert(sizeof(t_u8) == 1);
    constexpr t_u8 k_u8_min = std::numeric_limits<t_u8>::min();
    constexpr t_u8 k_u8_max = std::numeric_limits<t_u8>::max();

    using t_i8 = signed char;
    static_assert(sizeof(t_i8) == 1);
    constexpr t_i8 k_i8_min = std::numeric_limits<t_i8>::min();
    constexpr t_i8 k_i8_max = std::numeric_limits<t_i8>::max();

    using t_u16 = unsigned short;
    static_assert(sizeof(t_u16) == 2);
    constexpr t_u16 k_u16_min = std::numeric_limits<t_u16>::min();
    constexpr t_u16 k_u16_max = std::numeric_limits<t_u16>::max();

    using t_i16 = signed short;
    static_assert(sizeof(t_i16) == 2);
    constexpr t_i16 k_i16_min = std::numeric_limits<t_i16>::min();
    constexpr t_i16 k_i16_max = std::numeric_limits<t_i16>::max();

    using t_u32 = unsigned int;
    static_assert(sizeof(t_u32) == 4);
    constexpr t_u32 k_u32_min = std::numeric_limits<t_u32>::min();
    constexpr t_u32 k_u32_max = std::numeric_limits<t_u32>::max();

    using t_i32 = signed int;
    static_assert(sizeof(t_i32) == 4);
    constexpr t_i32 k_i32_min = std::numeric_limits<t_i32>::min();
    constexpr t_i32 k_i32_max = std::numeric_limits<t_i32>::max();

    using t_u64 = unsigned long long;
    static_assert(sizeof(t_u64) == 8);
    constexpr t_u64 k_u64_min = std::numeric_limits<t_u64>::min();
    constexpr t_u64 k_u64_max = std::numeric_limits<t_u64>::max();

    using t_i64 = signed long long;
    static_assert(sizeof(t_i64) == 8);
    constexpr t_i64 k_i64_min = std::numeric_limits<t_i64>::min();
    constexpr t_i64 k_i64_max = std::numeric_limits<t_i64>::max();

    using t_f32 = float;
    static_assert(sizeof(t_f32) == 4);
    constexpr t_f32 k_f32_min = std::numeric_limits<t_f32>::min();
    constexpr t_f32 k_f32_max = std::numeric_limits<t_f32>::max();

    using t_f64 = double;
    static_assert(sizeof(t_f64) == 8);
    constexpr t_f64 k_f64_min = std::numeric_limits<t_f64>::min();
    constexpr t_f64 k_f64_max = std::numeric_limits<t_f64>::max();

    using t_b8 = bool;
    static_assert(sizeof(t_b8) == 1);

    using t_uintptr = uintptr_t;
    static_assert(sizeof(t_uintptr) == 8);

    template <typename tp_type> using t_const_removed = std::remove_const<tp_type>::type;
    template <typename tp_type> using t_volatile_removed = std::remove_volatile<tp_type>::type;
    template <typename tp_type> using t_ref_removed = std::remove_reference<tp_type>::type;
    template <typename tp_type> using t_cvref_removed = std::remove_cvref<tp_type>::type;
}
