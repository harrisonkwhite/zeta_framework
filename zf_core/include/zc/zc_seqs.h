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
