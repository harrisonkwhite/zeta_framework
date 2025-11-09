#pragma once

#include <zc/mem/mem.h>

namespace zf {
    template<typename tp_type>
    class c_array {
    public:
        constexpr c_array() = default;

        constexpr c_array(tp_type* const buf, const t_s32 len) : m_buf(buf), m_len(len) {
            ZF_ASSERT((!buf && len == 0) || (buf && len >= 0));
        }

        [[nodiscard]]
        t_b8 Init(c_mem_arena& mem_arena, const t_s32 len) {
            ZF_ASSERT(len > 0);

            m_buf = mem_arena.PushType<tp_type>(len);

            if (!m_buf) {
                return false;
            }

            m_len = len;

            return true;
        }

        constexpr tp_type* Raw() const {
            return m_buf;
        }

        constexpr t_s32 Len() const {
            return m_len;
        }

        constexpr t_u64 SizeInBytes() const {
            return sizeof(tp_type) * Len();
        }

        constexpr t_b8 IsEmpty() const {
            return m_len == 0;
        }

        tp_type& operator[](const t_s32 index) const {
            ZF_ASSERT(index >= 0 && index < m_len);
            return m_buf[index];
        }

        constexpr c_array<const tp_type> View() const {
            return {m_buf, m_len};
        }

        constexpr operator c_array<const tp_type>() const {
            return View();
        }

        constexpr c_array Slice(const t_s32 beg, const t_s32 end) const {
            ZF_ASSERT(beg >= 0 && beg <= m_len);
            ZF_ASSERT(end >= 0 && end <= m_len);
            ZF_ASSERT(beg <= end);
            return {m_buf + beg, end - beg};
        }

    private:
        tp_type* m_buf = nullptr;
        t_s32 m_len = 0;
    };

    template<typename tp_type>
    void CopyArray(const c_array<tp_type> dest, const c_array<const tp_type> src) {
        ZF_ASSERT(dest.Len() >= src.Len());

        for (t_s32 i = 0; i < src.Len(); i++) {
            dest[i] = src[i];
        }
    }

    template<typename tp_type>
    t_b8 CloneArray(c_array<tp_type>& out, c_mem_arena& out_mem_arena, const c_array<const tp_type> src) {
        ZF_ASSERT(src.Len() > 0);

        if (!out.Init(out_mem_arena, src.Len())) {
            return false;
        }

        CopyArray(out, src);

        return true;
    }

    template<typename tp_type, t_s32 tp_len>
    struct s_static_array {
        static_assert(tp_len > 0, "Invalid static array length!");

        tp_type buf_raw[tp_len] = {};

        constexpr s_static_array() = default;

        constexpr s_static_array(const tp_type (&buf)[tp_len]) {
            for (t_s32 i = 0; i < tp_len; i++) {
                buf_raw[i] = buf[i];
            }
        }

        constexpr t_s32 Len() const {
            return tp_len;
        }

        tp_type& operator[](const t_s32 index) {
            ZF_ASSERT(index >= 0 && index < tp_len);
            return buf_raw[index];
        }

        constexpr const tp_type& operator[](const t_s32 index) const {
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

        c_stack(const c_array<tp_type> backing_arr, const t_s32 init_height = 0) : m_backing_arr(backing_arr), m_height(init_height) {
            ZF_ASSERT(init_height >= 0 && init_height <= backing_arr.Len());
        }

        [[nodiscard]]
        t_b8 Init(c_mem_arena& mem_arena, const t_s32 cap) {
            ZF_ASSERT(cap > 0);

            c_array<tp_type> arr;

            if (!arr.Init(mem_arena, cap)) {
                return false;
            }

            m_backing_arr = arr;
            m_height = 0;

            return true;
        }

        t_s32 Height() const {
            return m_height;
        }

        t_s32 Cap() const {
            return m_backing_arr.Len();
        }

        t_b8 IsEmpty() const {
            return m_height == 0;
        }

        t_b8 IsFull() const {
            return m_height == Cap();
        }

        tp_type& operator[](const t_s32 index) const {
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

    private:
        c_array<tp_type> m_backing_arr;
        t_s32 m_height = 0;
    };

    template<typename tp_type>
    class c_queue {
    public:
        c_queue() = default;

        c_queue(const c_array<tp_type> backing_arr, const t_s32 init_len = 0, const t_s32 init_begin_index = 0) : m_backing_arr(backing_arr), m_len(init_len), m_begin_index(init_begin_index) {
            ZF_ASSERT(init_len >= 0 && init_len <= backing_arr.Len());
            ZF_ASSERT(init_begin_index >= 0 && init_begin_index < backing_arr.Len());
        }

        [[nodiscard]]
        t_b8 Init(c_mem_arena& mem_arena, const t_s32 cap) {
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

        t_s32 Len() const {
            return m_len;
        }

        t_s32 Cap() const {
            return m_backing_arr.Len();
        }

        t_b8 IsEmpty() const {
            return m_len == 0;
        }

        t_b8 IsFull() const {
            return m_len == Cap();
        }

        tp_type& operator[](const t_s32 index) const {
            ZF_ASSERT(index < m_len);
            return m_backing_arr[Wrap(m_begin_index + index, 0, m_backing_arr.Len())];
        }

        void Enqueue(const tp_type& val) {
            ZF_ASSERT(!IsFull());

            m_backing_arr[Wrap(m_begin_index + m_len, 0, m_backing_arr.Len())] = val;
            m_len++;
        }

        tp_type Dequeue() {
            ZF_ASSERT(!IsEmpty());

            const t_s32 bi_old = m_begin_index;
            m_begin_index = Wrap(m_begin_index + 1, 0, m_backing_arr.Len());
            m_len--;
            return m_backing_arr[bi_old];
        }

    private:
        c_array<tp_type> m_backing_arr;
        t_s32 m_len = 0;
        t_s32 m_begin_index = 0;
    };

    template<typename tp_type>
    t_b8 BinarySearch(const c_array<const tp_type> arr, const tp_type& elem) {
        ZF_ASSERT(IsSorted(arr));

        if (arr.Len() == 0) {
            return false;
        }

        const tp_type& mid = elem[arr.Len() / 2];

        if (mid == elem) {
            return true;
        }

        return elem < mid ? BinarySearch(arr.Slice(0, arr.Len() / 2), elem) : BinarySearch(arr.Slice((arr.Len() / 2) + 1, arr.Len()), elem);
    }
}
