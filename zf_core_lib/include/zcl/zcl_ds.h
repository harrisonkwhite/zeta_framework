#pragma once

#include <zcl/zcl_mem.h>

namespace zf {
    template <typename tp_type>
    struct s_list {
        static_assert(!s_is_const<tp_type>::g_val);

    public:
        s_list() = default;

        s_list(const s_array<tp_type> backing_arr, const t_len len) : m_backing_arr(backing_arr), m_len(len) {
            ZF_ASSERT(len >= 0 && len <= backing_arr.len);
        }

        s_array<tp_type> BackingArray() {
            return m_backing_arr;
        }

        t_len Len() const {
            return m_len;
        }

        t_len Cap() const {
            return m_backing_arr.len;
        }

        t_b8 IsEmpty() const {
            return m_len == 0;
        }

        t_b8 IsFull() const {
            return m_len == m_backing_arr.len;
        }

        tp_type &operator[](const t_len index) const {
            ZF_ASSERT(index >= 0 && index < m_len);
            return m_backing_arr[index];
        }

        tp_type &Last() const {
            return operator[](m_len - 1);
        }

        tp_type *Append(const tp_type &val) {
            ZF_ASSERT(!IsFull());

            m_backing_arr[m_len] = val;
            m_len++;
            return &m_backing_arr[m_len - 1];
        }

        void Insert(const t_len index, const tp_type &val) {
            ZF_ASSERT(!IsFull());
            ZF_ASSERT(index >= 0 && index <= m_len);

            for (t_len i = m_len; i > index; i--) {
                m_backing_arr[m_len] = m_backing_arr[m_len - 1];
            }

            m_len++;
            m_backing_arr[index] = val;
        }

        void Remove(const t_len index) {
            ZF_ASSERT(!IsEmpty());
            ZF_ASSERT(index >= 0 && index < m_len);

            m_backing_arr.Slice(index + 1, m_len).CopyTo(m_backing_arr.Slice(index, m_len - 1));
            m_len--;
        }

        void RemoveSwapback(const t_len index) {
            ZF_ASSERT(m_len > 0 && m_len <= m_backing_arr.len);
            ZF_ASSERT(index >= 0 && index < m_len);

            m_backing_arr[index] = m_backing_arr[m_len - 1];
            m_len--;
        }

        tp_type RemoveLast() {
            ZF_ASSERT(m_len > 0 && m_len <= m_backing_arr.len);

            m_len--;
            return m_backing_arr[m_len];
        }

    private:
        s_array<tp_type> m_backing_arr = {};
        t_len m_len = 0;
    };

    template <typename tp_type>
    [[nodiscard]] t_b8 CreateList(const t_len cap, s_mem_arena *const mem_arena, s_list<tp_type> *const o_list, const t_len len = 0) {
        ZF_ASSERT(cap > 0 && len >= 0 && len <= cap);

        s_array<tp_type> backing_arr;

        if (!AllocArray(cap, mem_arena, &backing_arr)) {
            return false;
        }

        *o_list = {backing_arr, len};

        return true;
    }
}
