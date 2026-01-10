#pragma once

#include <type_traits>
#include <limits>
#include <concepts>

namespace zcl {
#define ZF_SIZE_OF(x) static_cast<zcl::t_i32>(sizeof(x))
#define ZF_SIZE_IN_BITS(x) (8 * ZF_SIZE_OF(x))

#define ZF_ALIGN_OF(x) static_cast<zcl::t_i32>(alignof(x))

#define ZF_IN_CONSTEXPR() std::is_constant_evaluated()

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
        template <typename tp_type>
        struct t_defer {
            tp_type func;

            t_defer(const tp_type f) : func(f) {}

            ~t_defer() {
                func();
            }
        };
    }

#define ZF_DEFER(x) const auto ZF_CONCAT(defer_, ZF_CONCAT(l, __LINE__)) = zcl::detail::t_defer([&]() x)

    namespace detail {
        void try_breaking_into_debugger_if(const bool cond);

        [[noreturn]] void handle_assert_error(const char *const cond_cstr, const char *const func_name_cstr, const char *const file_name_cstr, const int line);

#ifdef ZF_DEBUG
    #define ZF_DEBUG_BREAK() detail::try_breaking_into_debugger_if(true)
    #define ZF_DEBUG_BREAK_IF(cond) detail::try_breaking_into_debugger_if(cond)

    #define ZF_ASSERT(cond)                                                                    \
        do {                                                                                   \
            if (!ZF_IN_CONSTEXPR()) {                                                          \
                if (!(cond)) {                                                                 \
                    zcl::detail::handle_assert_error(#cond, __FUNCTION__, __FILE__, __LINE__); \
                }                                                                              \
            }                                                                                  \
        } while (0)
#else
    #define ZF_DEBUG_BREAK() static_cast<void>(0)
    #define ZF_DEBUG_BREAK_IF(cond) static_cast<void>(0)
    #define ZF_ASSERT(cond) static_cast<void>(0)
#endif

        [[noreturn]] void handle_fatal_error(const char *const func_name_cstr, const char *const file_name_cstr, const int line, const char *const cond_cstr = nullptr);

#define ZF_FATAL() zcl::detail::handle_fatal_error(__FUNCTION__, __FILE__, __LINE__)
#define ZF_UNREACHABLE() ZF_FATAL() // @todo: This should probably have some helper message to differentiate it from normal fatal errors.

#define ZF_REQUIRE(cond)                                                                  \
    do {                                                                                  \
        if (!ZF_IN_CONSTEXPR()) {                                                         \
            if (!(cond)) {                                                                \
                zcl::detail::handle_fatal_error(__FUNCTION__, __FILE__, __LINE__, #cond); \
            }                                                                             \
        }                                                                                 \
    } while (0)
    }

    static_assert(CHAR_BIT == 8);

    using t_u8 = unsigned char;
    static_assert(sizeof(t_u8) == 1);
    constexpr t_u8 k_u8_max = std::numeric_limits<t_u8>::max();

    using t_i8 = signed char;
    static_assert(sizeof(t_i8) == 1);
    constexpr t_i8 k_i8_max = std::numeric_limits<t_i8>::max();

    using t_u16 = unsigned short;
    static_assert(sizeof(t_u16) == 2);
    constexpr t_u16 k_u16_max = std::numeric_limits<t_u16>::max();

    using t_i16 = signed short;
    static_assert(sizeof(t_i16) == 2);
    constexpr t_i16 k_i16_max = std::numeric_limits<t_i16>::max();

    using t_u32 = unsigned int;
    static_assert(sizeof(t_u32) == 4);
    constexpr t_u32 k_u32_max = std::numeric_limits<t_u32>::max();

    using t_i32 = signed int;
    static_assert(sizeof(t_i32) == 4);
    constexpr t_i32 k_i32_max = std::numeric_limits<t_i32>::max();

    using t_u64 = unsigned long long;
    static_assert(sizeof(t_u64) == 8);
    constexpr t_u64 k_u64_max = std::numeric_limits<t_u64>::max();

    using t_i64 = signed long long;
    static_assert(sizeof(t_i64) == 8);
    constexpr t_i64 k_i64_max = std::numeric_limits<t_i64>::max();

    using t_f32 = float;
    static_assert(sizeof(t_f32) == 4);
    constexpr t_f32 k_f32_max = std::numeric_limits<t_f32>::max();

    using t_f64 = double;
    static_assert(sizeof(t_f64) == 8);
    constexpr t_f64 k_f64_max = std::numeric_limits<t_f64>::max();

    using t_b8 = bool;
    static_assert(sizeof(t_b8) == 1);

    using t_uintptr = uintptr_t;
    static_assert(sizeof(t_uintptr) == 8);

    template <typename tp_type> using t_const_removed = std::remove_const<tp_type>::type;
    template <typename tp_type> using t_volatile_removed = std::remove_volatile<tp_type>::type;
    template <typename tp_type> using t_ref_removed = std::remove_reference<tp_type>::type;
    template <typename tp_type> using t_cvref_removed = std::remove_cvref<tp_type>::type;

    // "Simple" meaning that it's safe to use with arenas and C-style memory operations.
    template <typename tp_type>
    concept c_simple = std::is_trivially_default_constructible_v<tp_type> && std::is_trivially_destructible_v<tp_type> && std::is_trivially_copyable_v<tp_type> && std::is_standard_layout_v<tp_type>;

    template <typename tp_type>
    concept c_integral_unsigned = std::is_same_v<tp_type, t_u8> || std::is_same_v<tp_type, t_u16> || std::is_same_v<tp_type, t_u32> || std::is_same_v<tp_type, t_u64>;

    template <typename tp_type>
    concept c_integral_signed = std::is_same_v<tp_type, t_i8> || std::is_same_v<tp_type, t_i16> || std::is_same_v<tp_type, t_i32> || std::is_same_v<tp_type, t_i64>;

    template <typename tp_type>
    concept c_integral = c_integral_unsigned<tp_type> || c_integral_signed<tp_type>;

    template <typename tp_type>
    concept c_floating_point = std::is_floating_point_v<tp_type>;

    template <typename tp_type>
    concept c_numeric = c_integral<tp_type> || c_floating_point<tp_type>;

    template <typename tp_type> concept c_ptr = std::is_pointer_v<tp_type>;
    template <typename tp_type> concept c_const = std::is_const_v<tp_type>;
    template <typename tp_type> concept c_union = std::is_union_v<tp_type>;
    template <typename tp_type> concept c_enum = std::is_enum_v<tp_type>;
    template <typename tp_type> concept c_scalar = std::is_scalar_v<tp_type>;

    template <typename tp_type_a, typename tp_type_b>
    concept c_same = std::same_as<tp_type_a, tp_type_b>;

    template <typename tp_type>
    concept c_cstr = c_same<std::remove_cv_t<std::remove_pointer_t<std::remove_extent_t<std::remove_reference_t<tp_type>>>>, char>;

    // Return true iff a and b are equal.
    template <c_simple tp_type>
    using t_comparator_bin = t_b8 (*)(const tp_type &a, const tp_type &b);

    template <c_simple tp_type>
    constexpr t_comparator_bin<tp_type> k_comparator_bin_default =
        [](const tp_type &a, const tp_type &b) {
            return a == b;
        };

    // Return a negative result if a < b, 0 if a == b, and a positive result if a > b.
    template <c_simple tp_type>
    using t_comparator_ord = t_i32 (*)(const tp_type &a, const tp_type &b);

    template <c_simple tp_type>
    constexpr t_comparator_ord<tp_type> k_comparator_ord_default =
        [](const tp_type &a, const tp_type &b) {
            if (a == b) {
                return 0;
            } else if (a < b) {
                return -1;
            } else {
                return 1;
            }
        };

    template <c_numeric tp_type>
    constexpr tp_type min(const tp_type a, const tp_type b) {
        return a <= b ? a : b;
    }

    template <c_numeric tp_type>
    constexpr tp_type max(const tp_type a, const tp_type b) {
        return a >= b ? a : b;
    }

    template <c_simple tp_type>
    constexpr void swap(tp_type *const a, tp_type *const b) {
        const tp_type temp = *a;
        *a = *b;
        *b = temp;
    }

    template <c_numeric tp_type>
    constexpr tp_type abs(const tp_type n) {
        return n < 0 ? -n : n;
    }

    template <c_numeric tp_type>
    constexpr tp_type clamp(const tp_type n, const tp_type min, const tp_type max) {
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
    constexpr t_i32 sign(const tp_type n) {
        if (n > 0) {
            return 1;
        } else if (n < 0) {
            return -1;
        }

        return 0;
    }

    template <c_integral tp_type>
    constexpr tp_type wrap(const tp_type val, const tp_type max_excl) {
        return ((val % max_excl) + max_excl) % max_excl;
    }

    template <c_integral tp_type>
    constexpr tp_type wrap(const tp_type val, const tp_type min, const tp_type max_excl) {
        return min + wrap(val - min, max_excl - min);
    }


    // ============================================================
    // @section: Arrays

    template <typename tp_type>
    concept c_array_elem = c_simple<tp_type> && c_same<tp_type, t_cvref_removed<tp_type>>;

    template <c_array_elem tp_elem_type>
    struct t_array_rdonly {
        using t_elem = tp_elem_type;

        const tp_elem_type *raw;
        t_i32 len;

        // @todo: Consider replacing with explicit function for safety.
        constexpr const tp_elem_type &operator[](const t_i32 index) const {
            ZF_REQUIRE(index >= 0 && index < len);
            return raw[index];
        }
    };

    template <c_array_elem tp_elem_type>
    struct t_array_mut {
        using t_elem = tp_elem_type;

        tp_elem_type *raw;
        t_i32 len;

        constexpr tp_elem_type &operator[](const t_i32 index) const {
            ZF_REQUIRE(index >= 0 && index < len);
            return raw[index];
        }

        constexpr operator t_array_rdonly<tp_elem_type>() const {
            return {raw, len};
        }
    };

    template <c_array_elem tp_elem_type, t_i32 tp_len>
    struct t_static_array {
        using t_elem = tp_elem_type;

        static constexpr t_i32 k_len = tp_len;

        tp_elem_type raw[tp_len];

        constexpr tp_elem_type &operator[](const t_i32 index) {
            ZF_REQUIRE(index >= 0 && index < tp_len);
            return raw[index];
        }

        constexpr const tp_elem_type &operator[](const t_i32 index) const {
            ZF_REQUIRE(index >= 0 && index < tp_len);
            return raw[index];
        }

        constexpr operator t_array_mut<tp_elem_type>() {
            return {raw, k_len};
        }

        constexpr operator t_array_rdonly<tp_elem_type>() const {
            return {raw, k_len};
        }
    };

    template <typename tp_type>
    concept c_array_mut = requires { typename tp_type::t_elem; } && c_same<t_cvref_removed<tp_type>, t_array_mut<typename tp_type::t_elem>>;

    template <typename tp_type>
    concept c_array_rdonly = requires { typename tp_type::t_elem; } && c_same<t_cvref_removed<tp_type>, t_array_rdonly<typename tp_type::t_elem>>;

    template <typename tp_type>
    concept c_array = c_array_mut<tp_type> || c_array_rdonly<tp_type>;

    template <c_array tp_arr_type>
    constexpr t_comparator_bin<tp_arr_type> k_array_comparator_bin =
        [](const tp_arr_type &a, const tp_arr_type &b) {
            if (a.len != b.len) {
                return false;
            }

            for (t_i32 i = 0; i < a.len; i++) {
                if (a[i] != b[i]) {
                    return false;
                }
            }

            return true;
        };

    template <c_array_elem tp_elem_type, t_i32 tp_len>
    constexpr t_array_mut<tp_elem_type> array_to_nonstatic(t_static_array<tp_elem_type, tp_len> &arr) {
        return {arr.raw, arr.k_len};
    }

    template <c_array_elem tp_elem_type, t_i32 tp_len>
    constexpr t_array_rdonly<tp_elem_type> array_to_nonstatic(const t_static_array<tp_elem_type, tp_len> &arr) {
        return {arr.raw, arr.k_len};
    }

    template <c_array_elem tp_elem_type>
    constexpr t_array_mut<tp_elem_type> array_slice(const t_array_mut<tp_elem_type> arr, const t_i32 beg, const t_i32 end) {
        ZF_ASSERT(beg >= 0 && beg <= arr.len);
        ZF_ASSERT(end >= beg && end <= arr.len);

        return {arr.raw + beg, end - beg};
    }

    template <c_array_elem tp_elem_type>
    constexpr t_array_rdonly<tp_elem_type> array_slice(const t_array_rdonly<tp_elem_type> arr, const t_i32 beg, const t_i32 end) {
        ZF_ASSERT(beg >= 0 && beg <= arr.len);
        ZF_ASSERT(end >= beg && end <= arr.len);

        return {arr.raw + beg, end - beg};
    }

    template <c_array_elem tp_elem_type>
    constexpr t_array_mut<tp_elem_type> array_slice_from(const t_array_mut<tp_elem_type> arr, const t_i32 beg) {
        ZF_ASSERT(beg >= 0 && beg <= arr.len);
        return {arr.raw + beg, arr.len - beg};
    }

    template <c_array_elem tp_elem_type>
    constexpr t_array_rdonly<tp_elem_type> array_slice_from(const t_array_rdonly<tp_elem_type> arr, const t_i32 beg) {
        ZF_ASSERT(beg >= 0 && beg <= arr.len);
        return {arr.raw + beg, arr.len - beg};
    }

    template <c_array tp_arr_type>
    constexpr t_i32 array_get_size_in_bytes(const tp_arr_type arr) {
        return ZF_SIZE_OF(typename tp_arr_type::t_elem) * arr.len;
    }

    template <c_array tp_src_arr_type, c_array_mut tp_dest_arr_type>
        requires c_same<typename tp_src_arr_type::t_elem, typename tp_dest_arr_type::t_elem>
    constexpr void array_copy(const tp_src_arr_type src, const tp_dest_arr_type dest, const t_b8 allow_truncation = false) {
        if (!allow_truncation) {
            ZF_ASSERT(dest.len >= src.len);

            for (t_i32 i = 0; i < src.len; i++) {
                dest[i] = src[i];
            }
        } else {
            const auto min_len = min(src.len, dest.len);

            for (t_i32 i = 0; i < min_len; i++) {
                dest[i] = src[i];
            }
        }
    }

    template <c_array tp_arr_a_type, c_array tp_arr_b_type>
        requires c_same<typename tp_arr_a_type::t_elem, typename tp_arr_b_type::t_elem>
    t_b8 arrays_check_equal(const tp_arr_a_type a, const tp_arr_b_type b, const t_comparator_bin<typename tp_arr_a_type::t_elem> comparator = k_comparator_ord_default<typename tp_arr_a_type::t_elem>) {
        if (a.len != b.len) {
            return false;
        }

        for (t_i32 i = 0; i < a.len; i++) {
            if (!comparator(a[i], b[i])) {
                return false;
            }
        }

        return true;
    }

    template <c_array tp_arr_a_type, c_array tp_arr_b_type>
        requires c_same<typename tp_arr_a_type::t_elem, typename tp_arr_b_type::t_elem>
    t_i32 array_compare(const tp_arr_a_type a, const tp_arr_b_type b, const t_comparator_ord<typename tp_arr_a_type::t_elem> comparator = k_comparator_ord_default<typename tp_arr_a_type::t_elem>) {
        if (a.len != b.len) {
            return a.len < b.len ? -1 : 1;
        }

        for (t_i32 i = 0; i < a.len; i++) {
            const t_i32 comp = comparator(a[i], b[i]);

            if (comp != 0) {
                return comp;
            }
        }

        return 0;
    }

    template <c_array tp_arr_type>
    t_b8 array_check_all_equal(const tp_arr_type arr, const typename tp_arr_type::t_elem &val, const t_comparator_bin<typename tp_arr_type::t_elem> comparator = k_comparator_bin_default<typename tp_arr_type::t_elem>) {
        if (arr.len == 0) {
            return false;
        }

        for (t_i32 i = 0; i < arr.len; i++) {
            if (!comparator(arr[i], val)) {
                return false;
            }
        }

        return true;
    }

    template <c_array tp_arr_type>
    t_b8 array_check_any_equal(const tp_arr_type arr, const typename tp_arr_type::t_elem &val, const t_comparator_bin<typename tp_arr_type::t_elem> comparator = k_comparator_bin_default<typename tp_arr_type::t_elem>) {
        for (t_i32 i = 0; i < arr.len; i++) {
            if (comparator(arr[i], val)) {
                return true;
            }
        }

        return false;
    }

    template <c_array_mut tp_arr_type>
    constexpr void array_set_all_to(const tp_arr_type arr, const typename tp_arr_type::t_elem &val) {
        for (t_i32 i = 0; i < arr.len; i++) {
            arr[i] = val;
        }
    }

    template <c_array_mut tp_arr_type>
    constexpr void array_reverse(const tp_arr_type arr) {
        for (t_i32 i = 0; i < arr.len / 2; i++) {
            swap(&arr[i], &arr[arr.len - 1 - i]);
        }
    }

    // ============================================================
}
