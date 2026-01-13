#pragma once

#include <zcl/zcl_basic.h>

namespace zcl {
    template <typename tp_type>
    concept c_list_elem = c_simple<tp_type> && c_same<tp_type, t_cvref_removed<tp_type>>;

    template <c_list_elem tp_elem_type>
    struct t_list {
        using t_elem = tp_elem_type;

        t_array_mut<tp_elem_type> backing_arr;
        t_i32 len;

        constexpr tp_elem_type &operator[](const t_i32 index) const {
            ZF_ASSERT(index >= 0 && index < len);
            return backing_arr[index];
        }
    };

    template <c_list_elem tp_elem_type, t_i32 tp_cap>
    struct t_static_list {
        using t_elem = tp_elem_type;

        t_static_array<tp_elem_type, tp_cap> backing_arr;
        t_i32 len;

        constexpr tp_elem_type &operator[](const t_i32 index) {
            ZF_ASSERT(index >= 0 && index < len);
            return backing_arr[index];
        }

        constexpr const tp_elem_type &operator[](const t_i32 index) const {
            ZF_ASSERT(index >= 0 && index < len);
            return backing_arr[index];
        }
    };

    using t_list_extension_cap_calculator = t_i32 (*)(const t_i32 cap_current);

    constexpr t_list_extension_cap_calculator k_list_extension_cap_calculator_default =
        [](const t_i32 cap_current) {
            ZF_ASSERT(cap_current >= 0);
            return cap_current == 0 ? 1 : cap_current * 2;
        };

    template <typename tp_type>
    concept c_list_nonstatic = c_same<t_cvref_removed<tp_type>, t_list<typename tp_type::t_elem>>;

    namespace detail {
        template <typename tp_type>
        struct t_is_list_static {
            static constexpr t_b8 k_value = false;
        };

        template <c_list_elem tp_elem_type, t_i32 tp_cap>
        struct t_is_list_static<t_static_list<tp_elem_type, tp_cap>> {
            static constexpr t_b8 k_value = true;
        };
    }

    template <typename tp_type>
    concept c_list_static = detail::t_is_list_static<t_cvref_removed<tp_type>>::k_value;

    template <typename tp_type>
    concept c_list = c_list_nonstatic<tp_type> || c_list_static<tp_type>;

    template <c_list_elem tp_elem_type>
    t_list<tp_elem_type> list_create(const t_i32 cap, t_arena *const arena, const t_i32 len = 0) {
        ZF_ASSERT(cap > 0 && len >= 0 && len <= cap);
        return {arena_push_array<tp_elem_type>(arena, cap), len};
    }

    template <c_list_nonstatic tp_list_type>
    constexpr t_i32 list_get_cap(const tp_list_type *const list) {
        return list->backing_arr.len;
    }

    template <c_list_static tp_list_type>
    constexpr t_i32 list_get_cap(const tp_list_type *const list) {
        return list->backing_arr.k_len;
    }

    template <c_list_nonstatic tp_list_type>
    constexpr t_array_mut<typename tp_list_type::t_elem> list_to_array(const tp_list_type *const list) {
        return array_slice(list->backing_arr, 0, list->len);
    }

    template <c_list_static tp_list_type>
    constexpr t_array_mut<typename tp_list_type::t_elem> list_to_array(tp_list_type *const list) {
        return array_slice(array_to_nonstatic(&list->backing_arr), 0, list->len);
    }

    template <c_list_static tp_list_type>
    constexpr t_array_rdonly<typename tp_list_type::t_elem> list_to_array(const tp_list_type *const list) {
        return array_slice(array_to_nonstatic(&list->backing_arr), 0, list->len);
    }

    template <c_list_nonstatic tp_list_type>
    void list_extend(tp_list_type *const list, t_arena *const arena, const t_list_extension_cap_calculator cap_calculator = k_list_extension_cap_calculator_default) {
        ZF_ASSERT(cap_calculator);

        const t_i32 new_cap = cap_calculator(list_get_cap(list));
        ZF_ASSERT(new_cap > list_get_cap(list));

        const auto new_backing_arr = arena_push_array<tp_list_type>(arena, new_cap);
        array_copy(list->backing_arr, new_backing_arr);

        *list = {new_backing_arr, list->len};
    }

    template <c_list_nonstatic tp_list_type>
    void list_extend_to_fit(tp_list_type *const list, const t_i32 min_cap, t_arena *const arena, const t_list_extension_cap_calculator cap_calculator = k_list_extension_cap_calculator_default) {
        ZF_ASSERT(min_cap > list_get_cap(list));
        ZF_ASSERT(cap_calculator);

        const t_i32 new_cap = [cap = list_get_cap(list), min_cap, cap_calculator]() {
            t_i32 result = cap;

            do {
                const auto res_last = result;
                result = cap_calculator(result);
                ZF_ASSERT(result > res_last);
            } while (result < min_cap);

            return result;
        }();

        const auto new_backing_arr = arena_push_array<typename tp_list_type::t_elem>(arena, new_cap);
        array_copy(list->backing_arr, new_backing_arr);

        *list = {new_backing_arr, list->len};
    }

    template <c_list tp_list_type>
    constexpr typename tp_list_type::t_elem *list_append(tp_list_type *const list, const typename tp_list_type::t_elem &value) {
        ZF_ASSERT(list->len < list_get_cap(list));

        list->len++;
        (*list)[list->len - 1] = value;
        return &(*list)[list->len - 1];
    }

    template <c_list_nonstatic tp_list_type>
    typename tp_list_type::t_elem *list_append_dynamic(tp_list_type *const list, const typename tp_list_type::t_elem &value, t_arena *const extension_arena, const t_list_extension_cap_calculator extension_cap_calculator = k_list_extension_cap_calculator_default) {
        if (list->len == list_get_cap(list)) {
            list_extend(list, extension_arena, extension_cap_calculator);
        }

        return list_append(list, value);
    }

    template <c_list tp_list_type>
    constexpr t_array_mut<typename tp_list_type::t_elem> list_append_many(tp_list_type *const list, const t_array_rdonly<typename tp_list_type::t_elem> values) {
        ZF_ASSERT(list->len + values.len <= list_get_cap(list));

        array_copy(values, array_slice_from(list->backing_arr, list->len));
        list->len += values.len;
        return array_slice(list->backing_arr, list->len - values.len, list->len);
    }

    template <c_list_nonstatic tp_list_type>
    t_array_mut<typename tp_list_type::t_elem> list_append_many_dynamic(tp_list_type *const list, const t_array_rdonly<typename tp_list_type::t_elem> values, t_arena *const extension_arena, const t_list_extension_cap_calculator extension_cap_calculator = k_list_extension_cap_calculator_default) {
        const auto min_cap_needed = list->len + values.len;

        if (min_cap_needed > list_get_cap(list)) {
            list_extend_to_fit(list, min_cap_needed, extension_arena, extension_cap_calculator);
        }

        return list_append_many(list, values);
    }

    template <c_list tp_list_type>
    constexpr typename tp_list_type::t_elem *list_insert_at(tp_list_type *const list, const t_i32 index, const typename tp_list_type::t_elem &value) {
        ZF_ASSERT(list->len < list_get_cap(list));
        ZF_ASSERT(index >= 0 && index <= list->len);

        list->len++;

        for (t_i32 i = list->len - 1; i > index; i--) {
            (*list)[i] = (*list)[i - 1];
        }

        (*list)[index] = value;

        return &(*list)[index];
    }

    template <c_list_nonstatic tp_list_type>
    typename tp_list_type::t_elem *list_insert_at_dynamic(tp_list_type *const list, const t_i32 index, const typename tp_list_type::t_elem &value, t_arena *const extension_arena, const t_list_extension_cap_calculator extension_cap_calculator = k_list_extension_cap_calculator_default) {
        if (list->len == list_get_cap(list)) {
            list_extend(list, extension_arena, extension_cap_calculator);
        }

        return list_insert_at(list, index, value);
    }

    template <c_list tp_list_type>
    constexpr void list_remove_at_shift(tp_list_type *const list, const t_i32 index) {
        ZF_ASSERT(list->len > 0);
        ZF_ASSERT(index >= 0 && index < list->len);

        array_copy(array_slice(list->backing_arr, index + 1, list->len), array_slice(list->backing_arr, index, list->len - 1));
        list->len--;
    }

    template <c_list tp_list_type>
    constexpr void list_remove_at_swapback(tp_list_type *const list, const t_i32 index) {
        ZF_ASSERT(list->len > 0);
        ZF_ASSERT(index >= 0 && index < list->len);

        (*list)[index] = (*list)[list->len - 1];
        list->len--;
    }

    template <c_list tp_list_type>
    constexpr void list_remove_end(tp_list_type *const list) {
        ZF_ASSERT(list->len > 0);
        list->len--;
    }
}
