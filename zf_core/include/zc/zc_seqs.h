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

        constexpr c_array<tp_type> Nonstatic() {
            return {buf_raw, tp_len};
        }

        constexpr operator c_array<tp_type>() {
            return Nonstatic();
        }

        constexpr c_array<const tp_type> Nonstatic() const {
            return {buf_raw, tp_len};
        }

        constexpr operator c_array<const tp_type>() const {
            return Nonstatic();
        }
    };

    template<typename tp_type>
    struct s_bounded_list_ro {
        c_array<const tp_type> backing_arr;
        t_size len;

        s_bounded_list_ro() = delete;
        s_bounded_list_ro(const c_array<const tp_type> backing_arr, const t_size& len)
            : backing_arr(backing_arr), len(len) {}
    };

    template<typename tp_type>
    struct s_bounded_list_mut {
        c_array<tp_type> backing_arr;
        t_size& len;

        s_bounded_list_mut() = delete;
        s_bounded_list_mut(c_array<tp_type> backing_arr, t_size& len)
            : backing_arr(backing_arr), len(len) {}

        s_bounded_list_ro<tp_type> ToReadonly() const {
            return {backing_arr, len};
        }

        operator s_bounded_list_ro<tp_type>() const {
            return ToReadonly();
        }
    };

    template<typename tp_type, t_size tp_cap>
    struct s_static_bounded_list {
        s_static_array<tp_type, tp_cap> backing_arr;
        t_size len = 0;

        s_bounded_list_mut<tp_type> ToNonstatic() {
            return {backing_arr, len};
        }

        operator s_bounded_list_mut<tp_type>() {
            return ToNonstatic();
        }

        s_bounded_list_ro<tp_type> ToNonstatic() const {
            return {backing_arr, len};
        }

        operator s_bounded_list_ro<tp_type>() const {
            return ToNonstatic();
        }
    };

    template<typename tp_type>
    t_b8 IsEmpty(const s_bounded_list_ro<tp_type>& list) {
        return list.len == 0;
    }

    template<typename tp_type>
    t_b8 IsFull(const s_bounded_list_ro<tp_type>& list) {
        return list.len == list.backing_arr.Len();
    }

    template<typename tp_type>
    void Append(const s_bounded_list_mut<tp_type>& list, const tp_type& val) {
        ZF_ASSERT(!IsFull<tp_type>(list));

        list.backing_arr[list.len] = val;
        list.len++;
    }

    template<typename tp_type>
    void Insert(const s_bounded_list_mut<tp_type>& list, const t_size index, const tp_type& val) {
        ZF_ASSERT(!IsFull<tp_type>(list));
        ZF_ASSERT(index >= 0 && index < list.len);

        CopyReverse(list.backing_arr.Slice(index + 1, list.len + 1), list.backing_arr.Slice(index, list.len));
        list.len++;
    }

    template<typename tp_type>
    void RemoveLast(const s_bounded_list_mut<tp_type>& list) {
        ZF_ASSERT(!IsEmpty<tp_type>(list));

        list.len--;
        return list.backing_arr[list.len];
    }

    template<typename tp_type>
    void RemoveSwapback(const s_bounded_list_mut<tp_type>& list, const t_size index) {
        ZF_ASSERT(!IsEmpty<tp_type>(list));
        ZF_ASSERT(index >= 0 && index < list.len);

        list.backing_arr[index] = list.backing_arr[list.len - 1];
        list.len--;
    }

    template<typename tp_type>
    void Remove(const s_bounded_list_mut<tp_type>& list, const t_size index) {
        ZF_ASSERT(!IsFull<tp_type>(list));
        ZF_ASSERT(index >= 0 && index < list.len);

        Copy(list.backing_arr.Slice(index, list.len - 1), list.backing_arr.Slice(index + 1, list.len));
        list.len--;
    }
}
