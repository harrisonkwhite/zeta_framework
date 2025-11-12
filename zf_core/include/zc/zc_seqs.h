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

        constexpr c_array<const tp_type> Nonstatic() const {
            return {buf_raw, tp_len};
        }

        constexpr operator c_array<tp_type>() {
            return Nonstatic();
        }

        constexpr operator c_array<const tp_type>() const {
            return Nonstatic();
        }
    };

    template<typename tp_type>
    struct s_bounded_list_view {
        const c_array<const tp_type> backing_arr;
        const t_size& len;

        s_bounded_list_view() = delete;
        s_bounded_list_view(const c_array<const tp_type> backing_arr, const t_size& len)
            : backing_arr(backing_arr), len(len) {}
    };

    template<typename tp_type>
    struct s_bounded_list {
        const c_array<tp_type> backing_arr;
        t_size& len;

        s_bounded_list() = delete;
        s_bounded_list(c_array<tp_type> backing_arr, t_size& len)
            : backing_arr(backing_arr), len(len) {}

        operator s_bounded_list_view<tp_type>() const {
            return {backing_arr, len};
        }
    };

    template<typename tp_type, t_size tp_cap>
    struct s_static_bounded_list {
        s_static_array<tp_type, tp_cap> backing_arr;
        t_size len = 0;

        operator s_bounded_list<tp_type>() {
            return {backing_arr, len};
        }

        operator s_bounded_list_view<tp_type>() const {
            return {backing_arr, len};
        }
    };

    template<typename tp_type>
    t_b8 IsEmpty(const s_bounded_list_view<tp_type>& list) {
        return list.len == 0;
    }

    template<typename tp_type>
    t_b8 IsFull(const s_bounded_list_view<tp_type>& list) {
        return list.len == list.backing_arr.Len();
    }

    template<typename tp_type>
    void Append(const s_bounded_list<tp_type>& list, const tp_type& val) {
        ZF_ASSERT(!IsFull<tp_type>(list));

        list.backing_arr[list.len] = val;
        list.len++;
    }

    template<typename tp_type>
    void Insert(const s_bounded_list<tp_type>& list, const t_size index, const tp_type& val) {
        ZF_ASSERT(!IsFull<tp_type>(list));
        ZF_ASSERT(index >= 0 && index < list.len);

        CopyReverse(list.backing_arr.Slice(index + 1, list.len + 1), list.backing_arr.Slice(index, list.len));
        list.len++;
    }

    template<typename tp_type>
    void RemoveLast(const s_bounded_list<tp_type>& list) {
        ZF_ASSERT(!IsEmpty<tp_type>(list));

        list.len--;
        return list.backing_arr[list.len];
    }

    template<typename tp_type>
    void RemoveSwapback(const s_bounded_list<tp_type>& list, const t_size index) {
        ZF_ASSERT(!IsEmpty<tp_type>(list));
        ZF_ASSERT(index >= 0 && index < list.len);

        list.backing_arr[index] = list.backing_arr[list.len - 1];
        list.len--;
    }

    template<typename tp_type>
    void Remove(const s_bounded_list<tp_type>& list, const t_size index) {
        ZF_ASSERT(!IsFull<tp_type>(list));
        ZF_ASSERT(index >= 0 && index < list.len);

        Copy(list.backing_arr.Slice(index, list.len - 1), list.backing_arr.Slice(index + 1, list.len));
        list.len--;
    }

#if 0
    template<typename tp_type>
    class c_bounded_list {
    public:
        c_bounded_list() = default;

        c_bounded_list(const c_array<tp_type> backing_arr, t_size& len) : m_backing_arr(backing_arr), m_len(len) {
            ZF_ASSERT(len >= 0 && len <= backing_arr.Len());
        }

        t_size Len() const {
            return m_len;
        }

        t_size Cap() const {
            return m_backing_arr.Len();
        }

        t_b8 IsEmpty() const {
            return m_len == 0;
        }

        t_b8 IsFull() const {
            return m_len == Cap();
        }

        tp_type& operator[](const t_size index) const {
            ZF_ASSERT(index < m_len);
            return m_backing_arr[index];
        }

        void Append(const tp_type& val) {
            ZF_ASSERT(!IsFull());

            m_backing_arr[m_len] = val;
            m_len++;
        }

        void Insert(const t_size index, const tp_type& val) {
            ZF_ASSERT(!IsFull());

            CopyReverse(m_backing_arr.Slice(index + 1, m_len + 1), m_backing_arr.Slice(index, m_len));
            m_len++;
        }

        void RemoveLast() {
            ZF_ASSERT(!IsEmpty());

            m_len--;
            return m_backing_arr[m_len];
        }

        void RemoveSwapback(const t_size index) {
            ZF_ASSERT(index >= 0 && index < m_len);

            m_backing_arr[index] = m_backing_arr[m_len - 1];
            m_len--;
        }

        void Remove(const t_size index) {
            ZF_ASSERT(!IsFull());

            Copy(m_backing_arr.Slice(index, m_len - 1), m_backing_arr.Slice(index + 1, m_len));
            m_len--;
        }

        void Clear() {
            m_len = 0;
        }

    private:
        c_array<tp_type> m_backing_arr;
        t_size& m_len;
    };

    template<typename tp_type, t_size tp_cap>
    struct s_static_bounded_list {
        s_static_array<tp_type, tp_cap> backing_arr;
        t_size len = 0;

        operator c_bounded_list<tp_type>
    };
#endif

    template<typename tp_type>
    class c_stack {
    public:
        c_stack() = default;

        c_stack(const c_array<tp_type> backing_arr, const t_size init_height = 0) : m_backing_arr(backing_arr), m_height(init_height) {
            ZF_ASSERT(init_height >= 0 && init_height <= backing_arr.Len());
        }

        [[nodiscard]]
        t_b8 Init(c_mem_arena& mem_arena, const t_size cap) {
            ZF_ASSERT(cap > 0);

            c_array<tp_type> arr;

            if (!arr.Init(mem_arena, cap)) {
                return false;
            }

            m_backing_arr = arr;
            m_height = 0;

            return true;
        }

        t_size Height() const {
            return m_height;
        }

        t_size Cap() const {
            return m_backing_arr.Len();
        }

        t_b8 IsEmpty() const {
            return m_height == 0;
        }

        t_b8 IsFull() const {
            return m_height == Cap();
        }

        tp_type& operator[](const t_size index) const {
            ZF_ASSERT(index < m_height);
            return m_backing_arr[index];
        }

        void Push(const tp_type& val) {
            ZF_ASSERT(!IsFull());

            m_backing_arr[m_height] = val;
            m_height++;
        }

        tp_type Pop() {
            ZF_ASSERT(!IsEmpty());

            m_height--;
            return m_backing_arr[m_height];
        }

        void Clear() {
            m_height = 0;
        }

    private:
        c_array<tp_type> m_backing_arr;
        t_size m_height = 0;
    };

    template<typename tp_type>
    class c_queue {
    public:
        c_queue() = default;

        c_queue(const c_array<tp_type> backing_arr, const t_size init_len = 0, const t_size init_begin_index = 0) : m_backing_arr(backing_arr), m_len(init_len), m_begin_index(init_begin_index) {
            ZF_ASSERT(init_len >= 0 && init_len <= backing_arr.Len());
            ZF_ASSERT(init_begin_index >= 0 && init_begin_index < backing_arr.Len());
        }

        [[nodiscard]]
        t_b8 Init(c_mem_arena& mem_arena, const t_size cap) {
            ZF_ASSERT(cap > 0);

            c_array<tp_type> arr;

            if (!arr.Init(mem_arena, cap)) {
                return false;
            }

            m_backing_arr = arr;
            m_len = 0;
            m_begin_index = 0;

            return true;
        }

        t_size Len() const {
            return m_len;
        }

        t_size Cap() const {
            return m_backing_arr.Len();
        }

        t_b8 IsEmpty() const {
            return m_len == 0;
        }

        t_b8 IsFull() const {
            return m_len == Cap();
        }

        tp_type& operator[](const t_size index) const {
            ZF_ASSERT(index >= 0 && index < m_len);
            return m_backing_arr[Wrap(m_begin_index + index, 0, m_backing_arr.Len())];
        }

        void Enqueue(const tp_type& val) {
            ZF_ASSERT(!IsFull());

            m_backing_arr[Wrap(m_begin_index + m_len, 0, m_backing_arr.Len())] = val;
            m_len++;
        }

        tp_type Dequeue() {
            ZF_ASSERT(!IsEmpty());

            const t_size bi_old = m_begin_index;
            m_begin_index = Wrap(m_begin_index + 1, 0, m_backing_arr.Len());
            m_len--;
            return m_backing_arr[bi_old];
        }

        void Clear() {
            m_len = 0;
            m_begin_index = 0;
        }

    private:
        c_array<tp_type> m_backing_arr;
        t_size m_len = 0;
        t_size m_begin_index = 0;
    };
}
