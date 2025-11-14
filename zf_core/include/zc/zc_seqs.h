#pragma once

#include <zc/zc_allocators.h>

namespace zf {
    template<typename tp_type, t_size tp_len>
    struct s_static_array {
        static_assert(tp_len > 0, "Invalid static array length!");

        tp_type buf[tp_len] = {};

        constexpr s_static_array() = default;

        constexpr s_static_array(const tp_type (&buf)[tp_len]) {
            for (t_size i = 0; i < tp_len; i++) {
                buf[i] = buf[i];
            }
        }

        constexpr t_size Len() const {
            return tp_len;
        }

        tp_type& operator[](const t_size index) {
            ZF_ASSERT(index >= 0 && index < tp_len);
            return buf[index];
        }

        constexpr const tp_type& operator[](const t_size index) const {
            ZF_ASSERT(index >= 0 && index < tp_len);
            return buf[index];
        }

        constexpr c_array<tp_type> ToNonstatic() {
            return {buf, tp_len};
        }

        constexpr operator c_array<tp_type>() {
            return ToNonstatic();
        }

        constexpr c_array<const tp_type> ToNonstatic() const {
            return {buf, tp_len};
        }

        constexpr operator c_array<const tp_type>() const {
            return ToNonstatic();
        }
    };

    template<typename tp_type>
    void ListAppend(const c_array<tp_type> backing_arr, t_size& len, const tp_type& val) {
        ZF_ASSERT(len >= 0 && len < backing_arr.Len());

        backing_arr[len] = val;
        len++;
    }

    template<typename tp_type>
    void ListInsert(const c_array<tp_type> backing_arr, t_size& len, const t_size index, const tp_type& val) {
        ZF_ASSERT(len >= 0 && len < backing_arr.Len());
        ZF_ASSERT(index >= 0 && index <= len);

        CopyReverse(backing_arr.Slice(index + 1, len + 1), backing_arr.Slice(index, len));

        len++;
        backing_arr[index] = val;
    }

    template<typename tp_type>
    tp_type ListRemoveLast(const c_array<tp_type> backing_arr, t_size& len) {
        ZF_ASSERT(len > 0 && len <= backing_arr.Len());

        len--;
        return backing_arr[len];
    }

    template<typename tp_type>
    void ListRemoveSwapback(const c_array<tp_type> backing_arr, t_size& len, const t_size index) {
        ZF_ASSERT(len > 0 && len <= backing_arr.Len());
        ZF_ASSERT(index >= 0 && index < len);

        backing_arr[index] = backing_arr[len - 1];
        len--;
    }

    template<typename tp_type>
    void ListRemove(const c_array<tp_type> backing_arr, t_size& len, const t_size index) {
        ZF_ASSERT(len > 0 && len <= backing_arr.Len());
        ZF_ASSERT(index >= 0 && index < len);

        Copy(backing_arr.Slice(index, len - 1), backing_arr.Slice(index + 1, len));
        len--;
    }

    template<typename tp_type>
    class c_list {
    public:
        constexpr c_list() = default;

        constexpr c_list(const c_array<tp_type> backing_arr, const t_size len = 0)
            : m_backing_arr(backing_arr), m_len(len) {
            ZF_ASSERT(len >= 0 && len <= backing_arr.Len());
        }

        tp_type& operator[](const t_size index) const {
            ZF_ASSERT(index < m_len);
            return m_backing_arr[index];
        }

        constexpr t_size Len() const {
            return m_len;
        }

        constexpr t_size Cap() const {
            return m_backing_arr.Len();
        }

        t_b8 IsEmpty() const {
            return Len() == 0;
        }

        t_b8 IsFull() const {
            return Len() == Cap();
        }

        void Append(const tp_type& val) {
            ListAppend(m_backing_arr, m_len);
        }

        void Insert(const t_size index, const tp_type& val) {
            ListInsert(m_backing_arr, m_len, index, val);
        }

        tp_type RemoveLast() {
            ListRemoveLast(m_backing_arr, m_len);
        }

        void RemoveSwapback(const t_size index) {
            ListRemoveSwapback(m_backing_arr, m_len, index);
        }

        void Remove(const t_size index) {
            ListRemove(m_backing_arr, m_len, index);
        }

    private:
        c_array<tp_type> m_backing_arr;
        t_size m_len = 0;
    };

    template<typename tp_type, t_size tp_cap>
    class c_static_list {
        static_assert(tp_cap > 0, "Invalid capacity for static list!");

    public:
        constexpr c_static_list() = default;

        constexpr c_static_list(const tp_type (&backing_buf)[tp_cap], const t_size len = 0) : m_len(len) {
            ZF_ASSERT(len >= 0 && len <= tp_cap);

            for (t_size i = 0; i < tp_cap; i++) {
                m_backing_arr[i] = backing_buf[i];
            }
        }

        tp_type& operator[](const t_size index) {
            ZF_ASSERT(index < m_len);
            return m_backing_arr[index];
        }

        const tp_type& operator[](const t_size index) const {
            ZF_ASSERT(index < m_len);
            return m_backing_arr[index];
        }

        constexpr t_size Len() const {
            return m_len;
        }

        constexpr t_size Cap() const {
            return tp_cap;
        }

        t_b8 IsEmpty() const {
            return Len() == 0;
        }

        t_b8 IsFull() const {
            return Len() == Cap();
        }

        void Append(const tp_type& val) {
            ListAppend(m_backing_arr, m_len);
        }

        void Insert(const t_size index, const tp_type& val) {
            ListInsert(m_backing_arr, m_len, index, val);
        }

        tp_type RemoveLast() {
            ListRemoveLast(m_backing_arr, m_len);
        }

        void RemoveSwapback(const t_size index) {
            ListRemoveSwapback(m_backing_arr, m_len, index);
        }

        void Remove(const t_size index) {
            ListRemove(m_backing_arr, m_len, index);
        }

    private:
        s_static_array<tp_type, tp_cap> m_backing_arr;
        t_size m_len = 0;
    };

#if 0
    template<typename tp_type, typename tp_self_type>
    struct s_list_ops {
        tp_type& operator[](const t_size index) const {
            const auto& self = static_cast<const tp_self_type&>(*this);
            ZF_ASSERT(index < self.m_len);
            return m_backing_arr[index];
        }

        t_size Len() const {
            const auto& self = static_cast<const tp_self_type&>(*this);
            return self.m_len;
        }

        t_size Cap() const {
            const auto& self = static_cast<const tp_self_type&>(*this);
            return self.m_backing_arr.Len();
        }

        t_b8 IsEmpty() const {
            return Len() == 0;
        }

        t_b8 IsFull() const {
            return Len() == Cap();
        }

        void Append(const tp_type& val) {
            static_assert(!s_is_const<tp_type>::sm_value);
            ZF_ASSERT(!IsFull());

            auto& self = static_cast<tp_self_type&>(*this);
            self.m_backing_arr[self.m_len] = val;
            self.m_len++;
        }

        void Insert(const t_size index, const tp_type& val) {
            static_assert(!s_is_const<tp_type>::sm_value);

            auto& self = static_cast<tp_self_type&>(*this);

            ZF_ASSERT(!IsFull());
            ZF_ASSERT(index >= 0 && index < self.m_len);

            CopyReverse(self.m_backing_arr.Slice(index + 1, self.m_len + 1), self.m_backing_arr.Slice(index, self.m_len));

            self.m_len++;
            self.m_backing_arr[index] = val;
        }

        tp_type RemoveLast() {
            static_assert(!s_is_const<tp_type>::sm_value);
            ZF_ASSERT(!IsEmpty());

            auto& self = static_cast<tp_self_type&>(*this);
            self.m_len--;
            return self.m_backing_arr[self.m_len];
        }

        void RemoveSwapback(const t_size index) {
            static_assert(!s_is_const<tp_type>::sm_value);

            auto& self = static_cast<tp_self_type&>(*this);

            ZF_ASSERT(!IsEmpty());
            ZF_ASSERT(index >= 0 && index < self.m_len);

            self.m_backing_arr[index] = self.m_backing_arr[self.m_len - 1];
            self.m_len--;
        }

        void Remove(const t_size index) {
            static_assert(!s_is_const<tp_type>::sm_value);

            auto& self = static_cast<tp_self_type&>(*this);

            ZF_ASSERT(!IsEmpty());
            ZF_ASSERT(index >= 0 && index < self.m_len);

            Copy(self.m_backing_arr.Slice(index, self.m_len - 1), self.m_backing_arr.Slice(index + 1, self.m_len));

            self.m_len--;
        }
    };

    template<typename tp_type>
    class c_list : public s_list_ops<tp_type, c_list<tp_type>> {
    public:
        c_list() = default;

        c_list(const s_array<tp_type> backing_arr, const t_size len = 0)
            : m_backing_arr(backing_arr), m_len(len) {
            ZF_ASSERT(len >= 0 && len <= backing_arr.Len());
        }

        tp_type& operator[](const t_size index) const {
            ZF_ASSERT(index < m_len);
            return m_backing_arr[index];
        }

    private:
        friend struct s_list_ops<tp_type, c_list<tp_type>>;

        s_array<tp_type> m_backing_arr;
        t_size m_len = 0;
    };

    template<typename tp_type, t_size tp_cap>
    class c_static_list : public s_list_ops<tp_type, c_static_list<tp_type, tp_cap>> {
    public:
        c_static_list() = default;

        c_static_list(const tp_type (&backing_buf)[tp_cap]) {
            for (t_size i = 0; i < tp_cap; i++) {
                m_backing_arr.buf_raw[i] = backing_buf[i];
            }
        }

        tp_type& operator[](const t_size index) {
            ZF_ASSERT(index < m_len);
            return m_backing_arr[index];
        }

        const tp_type& operator[](const t_size index) const {
            ZF_ASSERT(index < m_len);
            return m_backing_arr[index];
        }

    private:
        friend struct s_list_ops<tp_type, c_static_list<tp_type, tp_cap>>;

        s_static_array<tp_type, tp_cap> m_backing_arr;
        t_size m_len = 0;
    };
#endif

    template<typename tp_type>
    t_b8 MakeList(c_mem_arena& mem_arena, const t_size cap, c_list<tp_type>& o_list) {
        c_array<tp_type> backing_arr;

        if (!mem_arena.PushArray(cap, backing_arr)) {
            return false;
        }

        o_list = {backing_arr};

        return true;
    }
}
