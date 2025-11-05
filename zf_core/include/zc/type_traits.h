#pragma once

namespace zf {
    template<typename tp_type> struct s_is_integral { static constexpr bool sm_value = false; };
    template<> struct s_is_integral<char> { static constexpr bool sm_value = true; };
    template<> struct s_is_integral<signed char> { static constexpr bool sm_value = true; };
    template<> struct s_is_integral<unsigned char> { static constexpr bool sm_value = true; };
    template<> struct s_is_integral<short> { static constexpr bool sm_value = true; };
    template<> struct s_is_integral<unsigned short> { static constexpr bool sm_value = true; };
    template<> struct s_is_integral<int> { static constexpr bool sm_value = true; };
    template<> struct s_is_integral<unsigned int> { static constexpr bool sm_value = true; };
    template<> struct s_is_integral<long> { static constexpr bool sm_value = true; };
    template<> struct s_is_integral<unsigned long> { static constexpr bool sm_value = true; };
    template<> struct s_is_integral<long long> { static constexpr bool sm_value = true; };
    template<> struct s_is_integral<unsigned long long> { static constexpr bool sm_value = true; };

    template<typename tp_type> struct s_is_floating_point { static constexpr bool sm_value = false; };
    template<> struct s_is_floating_point<float> { static constexpr bool sm_value = true; };
    template<> struct s_is_floating_point<double> { static constexpr bool sm_value = true; };
    template<> struct s_is_floating_point<long double> { static constexpr bool sm_value = true; };

    template<typename tp_type>
    concept co_integral = s_is_integral<tp_type>::sm_value;

    template<typename tp_type>
    concept co_floating_point = s_is_floating_point<tp_type>::sm_value;

    template<typename tp_type>
    concept co_numeric = s_is_integral<tp_type>::sm_value || s_is_floating_point<tp_type>::sm_value;
}
