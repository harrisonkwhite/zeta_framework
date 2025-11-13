#pragma once

#include <zc/zc_allocators.h>

namespace zf {
    template<typename tp_type, t_size tp_len>
    struct s_static_array {
        static_assert(tp_len > 0, "Invalid static array length!");

        tp_type buf_raw[tp_len] = {};

        constexpr s_static_array() = default;

        constexpr s_static_array(const tp_type (&buf)[tp_len]) {
            for (t_size i = 0; i < tp_len; i++) {
                buf_raw[i] = buf[i];
            }
        }

        constexpr t_size Len() const {
            return tp_len;
        }

        tp_type& operator[](const t_size index) {
            ZF_ASSERT(index >= 0 && index < tp_len);
            return buf_raw[index];
        }

        constexpr const tp_type& operator[](const t_size index) const {
            ZF_ASSERT(index >= 0 && index < tp_len);
            return buf_raw[index];
        }

        constexpr s_array<tp_type> ToNonstatic() {
            return {buf_raw, tp_len};
        }

        constexpr operator s_array<tp_type>() {
            return ToNonstatic();
        }

        constexpr s_array<const tp_type> ToNonstatic() const {
            return {buf_raw, tp_len};
        }

        constexpr operator s_array<const tp_type>() const {
            return ToNonstatic();
        }
    };

    template<typename tp_type>
    struct s_list_state {
        s_array<tp_type> backing_arr;
        t_size& len;
    };

    template<typename tp_type>
    struct s_list {
        using elem_type = tp_type;

        s_array<tp_type> backing_arr;
        t_size len = 0;

        s_list() = default;
        s_list(const s_array<tp_type> backing_arr, const t_size len) : backing_arr(backing_arr), len(len) {}

        tp_type& operator[](const t_size index) const {
            ZF_ASSERT(index < len);
            return backing_arr[index];
        }
    };

    template<typename tp_type, t_size tp_cap>
    struct s_static_list {
        using elem_type = tp_type;

        s_static_array<tp_type, tp_cap> backing_arr;
        t_size len = 0;

        constexpr s_static_list() = default;
        constexpr s_static_list(const s_static_array<tp_type, tp_cap> backing_arr, const t_size len)
            : backing_arr(backing_arr), len(len) {}

        tp_type& operator[](const t_size index) const {
            ZF_ASSERT(index < len);
            return backing_arr[index];
        }

        constexpr s_list_state<tp_type> State() {
            return {backing_arr, len};
        }

        constexpr operator s_list_state<tp_type>() {
            return State();
        }
    };

    template<typename tp_type>
    t_b8 MakeList(c_mem_arena& mem_arena, const t_size cap, s_list<tp_type>& o_list) {
        if (!mem_arena.PushArray(cap, o_list.backing_arr)) {
            return false;
        }

        o_list.len = 0;

        return true;
    }

    template<typename tp_type>
    t_b8 IsListEmpty(const s_list_state<const tp_type> list) {
        return list.len == 0;
    }

    template<typename tp_type>
    t_b8 IsListFull(const s_list_state<const tp_type>& list) {
        return list.len == list.backing_arr.Len();
    }

    template<typename tp_type>
    void ListAppend(tp_type& list, const typename tp_type::elem_type& val) {
        const auto& ls = list.State();

        //ZF_ASSERT(!IsListFull(ls));

        ls.backing_arr[ls.len] = val;
        ls.len++;
    }

    template<typename tp_type>
    void ListInsert(const s_list_state<tp_type> list, const t_size index, const tp_type& val) {
        ZF_ASSERT(!IsListFull<tp_type>(list));
        ZF_ASSERT(index >= 0 && index < list.len);

        CopyReverse(list.backing_arr.Slice(index + 1, list.len + 1), list.backing_arr.Slice(index, list.len));
        list.len++;
    }

    template<typename tp_type>
    void ListRemoveLast(const s_list_state<tp_type> list) {
        ZF_ASSERT(!IsListEmpty<tp_type>(list));

        list.len--;
        return list.backing_arr[list.len];
    }

    template<typename tp_type>
    void ListRemoveSwapback(const s_list_state<tp_type> list, const t_size index) {
        ZF_ASSERT(!IsListEmpty<tp_type>(list));
        ZF_ASSERT(index >= 0 && index < list.len);

        list.backing_arr[index] = list.backing_arr[list.len - 1];
        list.len--;
    }

    template<typename tp_type>
    void ListRemove(const s_list_state<tp_type> list, const t_size index) {
        ZF_ASSERT(!IsListFull<tp_type>(list));
        ZF_ASSERT(index >= 0 && index < list.len);

        Copy(list.backing_arr.Slice(index, list.len - 1), list.backing_arr.Slice(index + 1, list.len));
        list.len--;
    }

#if 0
    template<typename tp_type>
    t_b8 IsEmpty(const s_list_state_ro<tp_type>& list) {
        return list.len == 0;
    }

    template<typename tp_type>
    t_b8 IsFull(const s_list_state_ro<tp_type>& list) {
        return list.len == list.backing_arr.Len();
    }

    template<typename tp_type>
    void Append(const s_list_state_mut<tp_type>& list, const tp_type& val) {
        ZF_ASSERT(!IsFull<tp_type>(list));

        list.backing_arr[list.len] = val;
        list.len++;
    }

    template<typename tp_type>
    void Insert(const s_list_state_mut<tp_type>& list, const t_size index, const tp_type& val) {
        ZF_ASSERT(!IsFull<tp_type>(list));
        ZF_ASSERT(index >= 0 && index < list.len);

        CopyReverse(list.backing_arr.Slice(index + 1, list.len + 1), list.backing_arr.Slice(index, list.len));
        list.len++;
    }

    template<typename tp_type>
    void RemoveLast(const s_list_state_mut<tp_type>& list) {
        ZF_ASSERT(!IsEmpty<tp_type>(list));

        list.len--;
        return list.backing_arr[list.len];
    }

    template<typename tp_type>
    void RemoveSwapback(const s_list_state_mut<tp_type>& list, const t_size index) {
        ZF_ASSERT(!IsEmpty<tp_type>(list));
        ZF_ASSERT(index >= 0 && index < list.len);

        list.backing_arr[index] = list.backing_arr[list.len - 1];
        list.len--;
    }

    template<typename tp_type>
    void Remove(const s_list_state_mut<tp_type>& list, const t_size index) {
        ZF_ASSERT(!IsFull<tp_type>(list));
        ZF_ASSERT(index >= 0 && index < list.len);

        Copy(list.backing_arr.Slice(index, list.len - 1), list.backing_arr.Slice(index + 1, list.len));
        list.len--;
    }
#endif
}
