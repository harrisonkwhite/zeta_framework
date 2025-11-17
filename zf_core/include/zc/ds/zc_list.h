#pragma once

#include <zc/ds/zc_array.h>

namespace zf {
    template<typename tp_type>
    struct s_list {
        s_array<tp_type> backing_arr;
        t_size len;

        constexpr s_list() = default;
        constexpr s_list(const s_array<tp_type> backing_arr, const t_size len)
            : backing_arr(backing_arr), len(len) {}

        tp_type& operator[](const t_size index) const {
            ZF_ASSERT(index < len);
            return backing_arr[index];
        }
    };

    template<typename tp_type, t_size tp_len>
    struct s_static_list {
        s_static_array<tp_type, tp_len> backing_arr;
        t_size len;

        constexpr s_static_list() = default;

        tp_type& operator[](const t_size index) {
            ZF_ASSERT(index < len);
            return backing_arr[index];
        }

        const tp_type& operator[](const t_size index) const {
            ZF_ASSERT(index < len);
            return backing_arr[index];
        }
    };

    template<typename tp_type>
    void ListAppend(const s_array<tp_type> backing_arr, t_size& len, const tp_type& val) {
        ZF_ASSERT(len >= 0 && len < backing_arr.Len());

        backing_arr[len] = val;
        len++;
    }

    template<typename tp_type>
    void ListAppend(s_list<tp_type>& list, const tp_type& val) {
        return ListAppend(list.backing_arr, list.len, val);
    }

    template<typename tp_type, t_size tp_len>
    void ListAppend(s_static_list<tp_type, tp_len>& list, const tp_type& val) {
        return ListAppend(list.backing_arr.ToNonstatic(), list.len, val);
    }

    template<typename tp_type>
    void ListInsert(const s_array<tp_type> backing_arr, t_size& len, const t_size index, const tp_type& val) {
        ZF_ASSERT(len >= 0 && len < backing_arr.Len());
        ZF_ASSERT(index >= 0 && index <= len);

        CopyReverse(Slice(backing_arr, index + 1, len + 1), Slice(backing_arr, index, len));

        len++;
        backing_arr[index] = val;
    }

    template<typename tp_type>
    void ListInsert(s_list<tp_type>& list, const t_size index, const tp_type& val) {
        return ListInsert(list.backing_arr, list.len, index, val);
    }

    template<typename tp_type, t_size tp_len>
    void ListInsert(s_static_list<tp_type, tp_len>& list, const t_size index, const tp_type& val) {
        return ListInsert(list.backing_arr.ToNonstatic(), list.len, index, val);
    }

    template<typename tp_type>
    tp_type ListRemoveLast(const s_array<tp_type> backing_arr, t_size& len) {
        ZF_ASSERT(len > 0 && len <= backing_arr.Len());

        len--;
        return backing_arr[len];
    }

    template<typename tp_type>
    tp_type ListRemoveLast(s_list<tp_type>& list) {
        return ListRemoveLast(list.backing_arr, list.len);
    }

    template<typename tp_type, t_size tp_len>
    tp_type ListRemoveLast(s_static_list<tp_type, tp_len>& list) {
        return ListRemoveLast(list.backing_arr.ToNonstatic(), list.len);
    }

    template<typename tp_type>
    void ListRemoveSwapback(const s_array<tp_type> backing_arr, t_size& len, const t_size index) {
        ZF_ASSERT(len > 0 && len <= backing_arr.Len());
        ZF_ASSERT(index >= 0 && index < len);

        backing_arr[index] = backing_arr[len - 1];
        len--;
    }

    template<typename tp_type>
    void ListRemoveSwapback(s_list<tp_type>& list, const t_size index) {
        return ListRemoveSwapback(list.backing_arr, list.len, index);
    }

    template<typename tp_type, t_size tp_len>
    void ListRemoveSwapback(s_static_list<tp_type, tp_len>& list, const t_size index) {
        return ListRemoveSwapback(list.backing_arr.ToNonstatic(), list.len, index);
    }

    template<typename tp_type>
    void ListRemove(const s_array<tp_type> backing_arr, t_size& len, const t_size index) {
        ZF_ASSERT(len > 0 && len <= backing_arr.Len());
        ZF_ASSERT(index >= 0 && index < len);

        Copy(Slice(backing_arr, index, len - 1), Slice(backing_arr, index + 1, len));
        len--;
    }

    template<typename tp_type>
    void ListRemove(s_list<tp_type>& list, const t_size index) {
        return ListRemove(list.backing_arr, list.len, index);
    }

    template<typename tp_type, t_size tp_len>
    void ListRemove(s_static_list<tp_type, tp_len>& list, const t_size index) {
        return ListRemove(list.backing_arr.ToNonstatic(), list.len, index);
    }

    template<typename tp_type>
    t_b8 MakeList(c_mem_arena& mem_arena, const t_size cap, s_list<tp_type>& o_list, const t_size len = 1) {
        ZF_ASSERT(cap > 0 && len >= 0 && len <= cap);

        s_array<tp_type> backing_arr;

        if (!MakeArray(mem_arena, cap, backing_arr)) {
            return false;
        }

        o_list = {backing_arr};

        return true;
    }
}
