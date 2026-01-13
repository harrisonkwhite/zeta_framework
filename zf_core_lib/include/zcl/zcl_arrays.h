#pragma once

#include <zcl/zcl_basic.h>
#include <zcl/zcl_errors.h>

namespace zcl {
    template <typename tp_type>
    concept c_array_elem = c_simple<tp_type> && c_same<tp_type, t_cvref_removed<tp_type>>;

    template <c_array_elem tp_elem_type>
    struct t_array_rdonly {
        using t_elem = tp_elem_type;

        const tp_elem_type *raw;
        t_i32 len;

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
    constexpr t_array_mut<tp_elem_type> array_to_nonstatic(t_static_array<tp_elem_type, tp_len> *const arr) {
        return {arr->raw, arr->k_len};
    }

    template <c_array_elem tp_elem_type, t_i32 tp_len>
    constexpr t_array_rdonly<tp_elem_type> array_to_nonstatic(const t_static_array<tp_elem_type, tp_len> *const arr) {
        return {arr->raw, arr->k_len};
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
            const auto min_len = calc_min(src.len, dest.len);

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

    template <c_array_elem tp_elem_type>
    t_array_mut<t_u8> array_to_byte_array(const t_array_mut<tp_elem_type> arr) {
        return {reinterpret_cast<t_u8 *>(arr.raw), array_get_size_in_bytes(arr)};
    }

    template <c_array_elem tp_elem_type>
    t_array_rdonly<t_u8> array_to_byte_array(const t_array_rdonly<tp_elem_type> arr) {
        return {reinterpret_cast<const t_u8 *>(arr.raw), array_get_size_in_bytes(arr)};
    }

    template <c_simple tp_type>
    t_array_mut<t_u8> to_bytes(tp_type *const val) {
        return {reinterpret_cast<t_u8 *>(val), ZF_SIZE_OF(*val)};
    }

    template <c_simple tp_type>
    t_array_rdonly<t_u8> to_bytes(const tp_type *const val) {
        return {reinterpret_cast<const t_u8 *>(val), ZF_SIZE_OF(*val)};
    }
}
