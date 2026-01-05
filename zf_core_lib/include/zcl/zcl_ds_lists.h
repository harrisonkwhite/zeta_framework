#pragma once

#include <zcl/zcl_io.h>

namespace zf {
    // ============================================================
    // @section: Types and Globals

    template <typename tp_type>
    concept co_list_elem = c_simple<tp_type> && !c_const<tp_type>;

    template <co_list_elem tp_type>
    struct s_list_mut {
        using t_elem = tp_type;
        using t_mut_tag = tp_type;

        t_array_mut<tp_type> backing_arr;
        t_i32 len;

        t_i32 Cap() const {
            return backing_arr.len;
        }

        tp_type &operator[](const t_i32 index) const {
            ZF_ASSERT(index >= 0 && index < len);
            return backing_arr[index];
        }

        t_array_mut<tp_type> AsArray() const {
            return f_mem_slice_array(backing_arr, 0, len);
        }
    };

    template <co_list_elem tp_type>
    struct s_list_rdonly {
        using t_elem = tp_type;

        t_array_rdonly<tp_type> backing_arr;
        t_i32 len;

        t_i32 Cap() const {
            return backing_arr.len;
        }

        const tp_type &operator[](const t_i32 index) const {
            ZF_ASSERT(index >= 0 && index < len);
            return backing_arr[index];
        }

        t_array_rdonly<tp_type> AsArray() const {
            return f_mem_slice_array(backing_arr, 0, len);
        }
    };

    template <co_list_elem tp_type, t_i32 tp_cap>
    struct s_static_list {
        using t_elem = tp_type;

        static constexpr t_i32 g_cap = tp_cap;

        t_static_array<tp_type, tp_cap> backing_arr;
        t_i32 len;

        t_i32 Cap() const {
            return backing_arr.g_len;
        }

        tp_type &operator[](const t_i32 index) {
            ZF_ASSERT(index >= 0 && index < len);
            return backing_arr[index];
        }

        const tp_type &operator[](const t_i32 index) const {
            ZF_ASSERT(index >= 0 && index < len);
            return backing_arr[index];
        }

        t_array_mut<tp_type> AsArray() { return f_mem_slice_array(backing_arr.AsNonstatic(), 0, len); }
        t_array_rdonly<tp_type> AsArray() const { return f_mem_slice_array(backing_arr.AsNonstatic(), 0, len); }
    };

    using t_list_extension_cap_calculator = t_i32 (*)(const t_i32 cap_current);

    inline const t_list_extension_cap_calculator g_list_extension_cap_calculator_default =
        [](const t_i32 cap_current) {
            ZF_ASSERT(cap_current >= 0);
            return cap_current == 0 ? 1 : cap_current * 2;
        };

    // ============================================================


    // ============================================================
    // @section: Functions

    template <typename tp_type>
    concept co_list_nonstatic_mut = requires { typename tp_type::t_elem; } && c_same<t_cvref_removed<tp_type>, s_list_mut<typename tp_type::t_elem>>;

    template <typename tp_type>
    struct s_is_mut_list {
        static constexpr t_b8 g_val = false;
    };

    template <typename tp_type>
    struct s_is_mut_list<s_list_mut<tp_type>> {
        static constexpr t_b8 g_val = true;
    };

    template <typename tp_type, t_i32 tp_cap>
    struct s_is_mut_list<s_static_list<tp_type, tp_cap>> {
        static constexpr t_b8 g_val = true;
    };

    template <typename tp_type>
    concept co_list_mut = s_is_mut_list<t_cvref_removed<tp_type>>::g_val;

    template <typename tp_type>
    s_list_mut<tp_type> CreateList(const t_i32 cap, t_arena *const arena, const t_i32 len = 0) {
        ZF_ASSERT(cap > 0 && len >= 0 && len <= cap);
        return {f_mem_push_array<tp_type>(arena, cap), len};
    }

    template <co_list_nonstatic_mut tp_list_type>
    void ListExtend(tp_list_type *const list, t_arena *const arena, const t_list_extension_cap_calculator cap_calculator = g_list_extension_cap_calculator_default) {
        ZF_ASSERT(cap_calculator);

        const t_i32 new_cap = cap_calculator(list->Cap());
        ZF_ASSERT(new_cap > list->Cap());

        const auto new_backing_arr = f_mem_push_array<tp_list_type>(arena, new_cap);
        CopyAll(list->backing_arr, new_backing_arr);

        *list = {new_backing_arr, list->len};
    }

    template <co_list_nonstatic_mut tp_list_type>
    void ListExtendToFit(tp_list_type *const list, const t_i32 min_cap, t_arena *const arena, const t_list_extension_cap_calculator cap_calculator = g_list_extension_cap_calculator_default) {
        ZF_ASSERT(min_cap > list->Cap());
        ZF_ASSERT(cap_calculator);

        const t_i32 new_cap = [cap = list->Cap(), min_cap, cap_calculator]() {
            t_i32 result = cap;

            do {
                const auto res_last = result;
                result = cap_calculator(result);
                ZF_ASSERT(result > res_last);
            } while (result < min_cap);

            return result;
        }();

        const auto new_backing_arr = f_mem_push_array<typename tp_list_type::t_elem>(arena, new_cap);
        CopyAll(list->backing_arr, new_backing_arr);

        *list = {new_backing_arr, list->len};
    }

    template <co_list_mut tp_list_type>
    typename tp_list_type::t_elem *ListAppend(tp_list_type *const list, const typename tp_list_type::t_elem &val) {
        ZF_ASSERT(list->len < list->Cap());

        list->len++;
        (*list)[list->len - 1] = val;
        return &(*list)[list->len - 1];
    }

    template <co_list_mut tp_list_type>
    typename tp_list_type::t_elem *ListAppendDynamic(tp_list_type *const list, const typename tp_list_type::t_elem &val, t_arena *const extension_arena, const t_list_extension_cap_calculator extension_cap_calculator = g_list_extension_cap_calculator_default) {
        if (list->len == list->Cap()) {
            ListExtend(list, extension_arena, extension_cap_calculator);
        }

        return ListAppend(list, val);
    }

    template <co_list_mut tp_list_type>
    t_array_mut<typename tp_list_type::t_elem> ListAppendMany(tp_list_type *const list, const t_array_rdonly<typename tp_list_type::t_elem> vals) {
        ZF_ASSERT(list->len + vals.len <= list->Cap());

        CopyAll(vals, f_mem_slice_array_from(list->backing_arr, list->len));
        list->len += vals.len;
        return f_mem_slice_array(list->backing_arr, list->len - vals.len, list->len);
    }

    template <co_list_mut tp_list_type>
    t_array_mut<typename tp_list_type::t_elem> ListAppendManyDynamic(tp_list_type *const list, const t_array_rdonly<typename tp_list_type::t_elem> vals, t_arena *const extension_arena, const t_list_extension_cap_calculator extension_cap_calculator = g_list_extension_cap_calculator_default) {
        const auto min_cap_needed = list->len + vals.len;

        if (min_cap_needed > list->Cap()) {
            ListExtendToFit(list, min_cap_needed, extension_arena, extension_cap_calculator);
        }

        return ListAppendMany(list, vals);
    }

    template <co_list_mut tp_list_type>
    typename tp_list_type::t_elem *ListInsertAt(tp_list_type *const list, const t_i32 index, const typename tp_list_type::t_elem &val) {
        ZF_ASSERT(list->len < list->Cap());
        ZF_ASSERT(index >= 0 && index <= list->len);

        list->len++;

        for (t_i32 i = list->len - 1; i > index; i--) {
            (*list)[i] = (*list)[i - 1];
        }

        (*list)[index] = val;

        return &(*list)[index];
    }

    template <co_list_mut tp_list_type>
    typename tp_list_type::t_elem *ListInsertAtDynamic(tp_list_type *const list, const t_i32 index, const typename tp_list_type::t_elem &val, t_arena *const extension_arena, const t_list_extension_cap_calculator extension_cap_calculator = g_list_extension_cap_calculator_default) {
        if (list->len == list->Cap()) {
            ListExtend(list, extension_arena, extension_cap_calculator);
        }

        return ListInsertAt(list, index, val);
    }

    template <co_list_mut tp_list_type>
    void ListRemoveAtShift(tp_list_type *const list, const t_i32 index) {
        ZF_ASSERT(list->len > 0);
        ZF_ASSERT(index >= 0 && index < list->len);

        CopyAll(f_mem_slice_array(list->backing_arr, index + 1, list->len), f_mem_slice_array(list->backing_arr, index, list->len - 1));
        list->len--;
    }

    template <co_list_mut tp_list_type>
    void ListRemoveAtSwapback(tp_list_type *const list, const t_i32 index) {
        ZF_ASSERT(list->len > 0);
        ZF_ASSERT(index >= 0 && index < list->len);

        (*list)[index] = (*list)[list->len - 1];
        list->len--;
    }

    template <co_list_mut tp_list_type>
    void ListRemoveEnd(tp_list_type *const list) {
        ZF_ASSERT(list->len > 0);
        list->len--;
    }

    // @todo: Serialisation function!

    // ============================================================
}
