#pragma once

#include <zc/ds/zc_array.h>

namespace zf {
    template<typename tp_type>
    struct s_list {
        s_array<tp_type> backing_arr;
        t_size len;

        tp_type& operator[](const t_size index) const {
            ZF_ASSERT(index < len);
            return backing_arr[index];
        }
    };

    template<typename tp_type, t_size tp_cap>
    struct s_static_list {
        s_static_array<tp_type, tp_cap> backing_arr;
        t_size len;

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
    s_array<tp_type> ListAsArray(const s_list<tp_type> list) {
        return Slice(list.backing_arr, 0, list.len);
    }

    template<typename tp_type>
    t_b8 IsListEmpty(const s_list<tp_type> arr) {
        return arr.len == 0;
    }

    template<typename tp_type, t_size tp_cap>
    t_b8 IsListEmpty(const s_static_list<tp_type, tp_cap>& arr) {
        return arr.len == 0;
    }

    template<typename tp_type>
    t_b8 IsListFull(const s_list<tp_type> arr) {
        return arr.len == arr.backing_arr.len;
    }

    template<typename tp_type, t_size tp_cap>
    t_b8 IsListFull(const s_static_list<tp_type, tp_cap>& arr) {
        return arr.len == tp_cap;
    }

    template<typename tp_type>
    tp_type& ListAppend(const s_array<tp_type> backing_arr, t_size& len, const tp_type& val) {
        ZF_ASSERT(len >= 0 && len < backing_arr.len);

        backing_arr[len] = val;
        len++;
        return backing_arr[len - 1];
    }

    template<typename tp_type>
    tp_type& ListAppend(s_list<tp_type>& list, const tp_type& val) {
        return ListAppend(list.backing_arr, list.len, val);
    }

    template<typename tp_type, t_size tp_cap>
    tp_type& ListAppend(s_static_list<tp_type, tp_cap>& list, const tp_type& val) {
        return ListAppend(static_cast<s_array<tp_type>>(list.backing_arr), list.len, val);
    }

    template<typename tp_type>
    void ListInsert(const s_array<tp_type> backing_arr, t_size& len, const t_size index, const tp_type& val) {
        ZF_ASSERT(len >= 0 && len < backing_arr.len);
        ZF_ASSERT(index >= 0 && index <= len);

        CopyReverse(Slice(backing_arr, index + 1, len + 1), Slice(backing_arr, index, len));

        len++;
        backing_arr[index] = val;
    }

    template<typename tp_type>
    void ListInsert(s_list<tp_type>& list, const t_size index, const tp_type& val) {
        ListInsert(list.backing_arr, list.len, index, val);
    }

    template<typename tp_type, t_size tp_cap>
    void ListInsert(s_static_list<tp_type, tp_cap>& list, const t_size index, const tp_type& val) {
        ListInsert(static_cast<s_array<tp_type>>(list.backing_arr), list.len, index, val);
    }

    template<typename tp_type>
    tp_type ListRemoveLast(const s_array<tp_type> backing_arr, t_size& len) {
        ZF_ASSERT(len > 0 && len <= backing_arr.len);

        len--;
        return backing_arr[len];
    }

    template<typename tp_type>
    tp_type ListRemoveLast(s_list<tp_type>& list) {
        return ListRemoveLast(list.backing_arr, list.len);
    }

    template<typename tp_type, t_size tp_cap>
    tp_type ListRemoveLast(s_static_list<tp_type, tp_cap>& list) {
        return ListRemoveLast(static_cast<s_array<tp_type>>(list.backing_arr), list.len);
    }

    template<typename tp_type>
    void ListRemoveSwapback(const s_array<tp_type> backing_arr, t_size& len, const t_size index) {
        ZF_ASSERT(len > 0 && len <= backing_arr.len);
        ZF_ASSERT(index >= 0 && index < len);

        backing_arr[index] = backing_arr[len - 1];
        len--;
    }

    template<typename tp_type>
    void ListRemoveSwapback(s_list<tp_type>& list, const t_size index) {
        ListRemoveSwapback(list.backing_arr, list.len, index);
    }

    template<typename tp_type, t_size tp_cap>
    void ListRemoveSwapback(s_static_list<tp_type, tp_cap>& list, const t_size index) {
        ListRemoveSwapback(static_cast<s_array<tp_type>>(list.backing_arr), list.len, index);
    }

    template<typename tp_type>
    void ListRemove(const s_array<tp_type> backing_arr, t_size& len, const t_size index) {
        ZF_ASSERT(len > 0 && len <= backing_arr.len);
        ZF_ASSERT(index >= 0 && index < len);

        Copy(Slice(backing_arr, index, len - 1), Slice(backing_arr, index + 1, len));
        len--;
    }

    template<typename tp_type>
    void ListRemove(s_list<tp_type>& list, const t_size index) {
        ListRemove(list.backing_arr, list.len, index);
    }

    template<typename tp_type, t_size tp_cap>
    void ListRemove(s_static_list<tp_type, tp_cap>& list, const t_size index) {
        ListRemove(static_cast<s_array<tp_type>>(list.backing_arr), list.len, index);
    }

    template<typename tp_type>
    t_b8 MakeList(s_mem_arena& mem_arena, const t_size cap, s_list<tp_type>& o_list, const t_size len = 0) {
        ZF_ASSERT(cap > 0 && len >= 0 && len <= cap);

        s_array<tp_type> backing_arr;

        if (!MakeArray(mem_arena, cap, backing_arr)) {
            return false;
        }

        o_list = {backing_arr, len};

        return true;
    }
}
