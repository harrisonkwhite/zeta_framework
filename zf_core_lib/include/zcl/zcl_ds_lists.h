#pragma once

#include <zcl/zcl_io.h>

namespace zf {
    // ============================================================
    // @section: Types and Globals

    template <typename tp_type>
    struct s_list_mut {
        static_assert(!s_is_const<tp_type>::g_val);

        using t_elem = tp_type;

        s_array_mut<tp_type> backing_arr;
        t_i32 len;

        t_i32 Cap() const {
            return backing_arr.len;
        }

        tp_type &operator[](const t_i32 index) const {
            ZF_ASSERT(index >= 0 && index < len);
            return backing_arr[index];
        }

        s_array_mut<tp_type> AsArray() const {
            return Slice(backing_arr, 0, len);
        }
    };

    template <typename tp_type>
    struct s_list_rdonly {
        static_assert(!s_is_const<tp_type>::g_val);

        using t_elem = tp_type;

        s_array_rdonly<tp_type> backing_arr;
        t_i32 len;

        t_i32 Cap() const {
            return backing_arr.len;
        }

        const tp_type &operator[](const t_i32 index) const {
            ZF_ASSERT(index >= 0 && index < len);
            return backing_arr[index];
        }

        s_array_rdonly<tp_type> AsArray() const {
            return Slice(backing_arr, 0, len);
        }
    };

    template <typename tp_type, t_i32 tp_cap>
    struct s_static_list {
        static_assert(!s_is_const<tp_type>::g_val);

        using t_elem = tp_type;

        s_static_array<tp_type, tp_cap> backing_arr;
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

        s_array_mut<tp_type> AsArray() { return Slice(backing_arr.AsNonstatic(), 0, len); }
        s_array_rdonly<tp_type> AsArray() const { return Slice(backing_arr.AsNonstatic(), 0, len); }
    };

    template <typename tp_type>
    struct s_is_nonstatic_mut_list {
        static constexpr t_b8 g_val = false;
    };

    template <typename tp_type>
    struct s_is_nonstatic_mut_list<s_list_mut<tp_type>> {
        static constexpr t_b8 g_val = true;
    };

    template <typename tp_type>
    struct s_is_nonstatic_rdonly_list {
        static constexpr t_b8 g_val = false;
    };

    template <typename tp_type>
    struct s_is_nonstatic_rdonly_list<s_list_rdonly<tp_type>> {
        static constexpr t_b8 g_val = true;
    };

    template <typename tp_type>
    struct s_is_static_list {
        static constexpr t_b8 g_val = false;
    };

    template <typename tp_type, t_i32 tp_cap>
    struct s_is_static_list<s_static_list<tp_type, tp_cap>> {
        static constexpr t_b8 g_val = true;
    };

    template <typename tp_type> concept co_list_nonstatic_mut = s_is_nonstatic_mut_list<tp_type>::g_val;
    template <typename tp_type> concept co_list_nonstatic_rdonly = s_is_nonstatic_rdonly_list<tp_type>::g_val;
    template <typename tp_type> concept co_list_nonstatic = co_list_nonstatic_mut<tp_type> || co_list_nonstatic_rdonly<tp_type>;
    template <typename tp_type> concept co_list_static = s_is_static_list<tp_type>::g_val;
    template <typename tp_type> concept co_list = co_list_nonstatic<tp_type> || co_list_static<tp_type>;

    using t_list_extension_cap_calculator = t_i32 (*)(const t_i32 cap_current);

    constexpr t_list_extension_cap_calculator g_default_list_extension_cap_calculator =
        [](const t_i32 cap_current) {
            ZF_ASSERT(cap_current >= 0);
            return cap_current == 0 ? 1 : cap_current * 2;
        };

    // ============================================================


    // ============================================================
    // @section: Functions

    template <typename tp_type>
    s_list_mut<tp_type> ListCreate(const t_i32 cap, s_arena *const arena, const t_i32 len = 0) {
        ZF_ASSERT(cap > 0 && len >= 0 && len <= cap);
        return {ArenaPushArray<tp_type>(arena, cap), len};
    }

    template <co_list_nonstatic_mut tp_list_type>
    void ListExtend(tp_list_type *const list, s_arena *const arena, const t_list_extension_cap_calculator cap_calculator = g_default_list_extension_cap_calculator) {
        ZF_ASSERT(cap_calculator);

        const t_i32 new_cap = cap_calculator(list->Cap());
        ZF_ASSERT(new_cap > list->Cap());

        const auto new_backing_arr = ArenaPushArray<typename tp_list_type::t_elem>(arena, new_cap);
        CopyAll(list->backing_arr, new_backing_arr);

        *list = {new_backing_arr, list->len};
    }

    template <co_list_nonstatic_mut tp_list_type>
    void ListExtendToFit(tp_list_type *const list, const t_i32 min_cap, s_arena *const arena, const t_list_extension_cap_calculator cap_calculator = g_default_list_extension_cap_calculator) {
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

        const auto new_backing_arr = ArenaPushArray<typename tp_list_type::t_elem>(arena, new_cap);
        CopyAll(list->backing_arr, new_backing_arr);

        *list = {new_backing_arr, list->len};
    }

    template <co_list tp_list_type>
    typename tp_list_type::t_elem *ListAppend(tp_list_type *const list, const typename tp_list_type::t_elem &val) {
        ZF_ASSERT(list->len < list->Cap());

        list->len++;
        (*list)[list->len - 1] = val;
        return &(*list)[list->len - 1];
    }

    template <co_list_nonstatic tp_list_type>
    typename tp_list_type::t_elem *ListAppendDynamic(tp_list_type *const list, const typename tp_list_type::t_elem &val, s_arena *const extension_arena, const t_list_extension_cap_calculator extension_cap_calculator = g_default_list_extension_cap_calculator) {
        if (list->len == list->Cap()) {
            ListExtend(list, extension_arena, extension_cap_calculator);
        }

        return ListAppend(list, val);
    }

    template <co_list tp_list_type>
    s_array_mut<typename tp_list_type::t_elem> ListAppendMany(tp_list_type *const list, const s_array_rdonly<typename tp_list_type::t_elem> vals) {
        ZF_ASSERT(list->len + vals.len <= list->Cap());

        CopyAll(vals, SliceFrom(list->backing_arr, list->len));
        list->len += vals.len;
        return Slice(list->backing_arr, list->len - vals.len, list->len);
    }

    template <co_list_nonstatic tp_list_type>
    s_array_mut<typename tp_list_type::t_elem> ListAppendManyDynamic(tp_list_type *const list, const s_array_rdonly<typename tp_list_type::t_elem> vals, s_arena *const extension_arena, const t_list_extension_cap_calculator extension_cap_calculator = g_default_list_extension_cap_calculator) {
        const auto min_cap_needed = list->len + vals.len;

        if (min_cap_needed > list->Cap()) {
            ListExtendToFit(list, min_cap_needed, extension_arena, extension_cap_calculator);
        }

        return ListAppendMany(list, vals);
    }

    template <co_list tp_list_type>
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

    template <co_list_nonstatic tp_list_type>
    typename tp_list_type::t_elem *ListInsertAtDynamic(tp_list_type *const list, const t_i32 index, const typename tp_list_type::t_elem &val, s_arena *const extension_arena, const t_list_extension_cap_calculator extension_cap_calculator = g_default_list_extension_cap_calculator) {
        if (list->len == list->Cap()) {
            ListExtend(list, extension_arena, extension_cap_calculator);
        }

        return ListInsertAt(list, index, val);
    }

    template <co_list tp_list_type>
    void ListRemoveAt(tp_list_type *const list, const t_i32 index) {
        ZF_ASSERT(list->len > 0);
        ZF_ASSERT(index >= 0 && index < list->len);

        CopyAll(Slice(list->backing_arr, index + 1, list->len), Slice(list->backing_arr, index, list->len - 1));
        list->len--;
    }

    template <co_list tp_list_type>
    void ListRemoveAtSwapback(tp_list_type *const list, const t_i32 index) {
        ZF_ASSERT(list->len > 0);
        ZF_ASSERT(index >= 0 && index < list->len);

        (*list)[index] = (*list)[list->len - 1];
        list->len--;
    }

    template <co_list tp_list_type>
    void ListRemoveEnd(tp_list_type *const list) {
        ZF_ASSERT(list->len > 0);
        list->len--;
    }

    // ============================================================
}
