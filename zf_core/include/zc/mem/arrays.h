#pragma once

#include <cassert>

namespace zf {
    template<typename tp_type>
    class c_array {
    public:
        c_array() = default;

        c_array(tp_type* const buf, const int len) : m_buf(buf), m_len(len) {
            assert((!buf && len == 0) || (buf && len >= 0));
        }

        tp_type* Raw() const {
            return m_buf;
        }

        int Len() const {
            return m_len;
        }

        size_t SizeInBytes() const {
            return sizeof(tp_type) * Len();
        }

        bool IsEmpty() const {
            return m_len == 0;
        }

        tp_type& operator[](const int index) const {
            assert(index >= 0 && index < m_len);
            return m_buf[index];
        }

        c_array<const tp_type> View() const {
            return {m_buf, m_len};
        }

        operator c_array<const tp_type>() const {
            return View();
        }

        c_array Slice(const int beg, const int end) const {
            assert(beg >= 0 && beg <= m_len);
            assert(end >= 0 && end <= m_len);
            assert(beg <= end);
            return {m_buf + beg, end - beg};
        }

    private:
        tp_type* m_buf = nullptr;
        int m_len = 0;
    };

    template<typename tp_type, int tp_len>
    struct s_static_array {
        static_assert(tp_len > 0, "Invalid static array length!");

        tp_type buf_raw[tp_len] = {};

        constexpr s_static_array() = default;

        constexpr s_static_array(const tp_type (&buf)[tp_len]) {
            for (int i = 0; i < tp_len; i++) {
                buf_raw[i] = buf[i];
            }
        }

        int Len() const {
            return tp_len;
        }

        tp_type& operator[](const int index) {
            assert(index >= 0 && index < tp_len);
            return buf_raw[index];
        }

        const tp_type& operator[](const int index) const {
            assert(index >= 0 && index < tp_len);
            return buf_raw[index];
        }

        c_array<tp_type> Nonstatic() {
            return {buf_raw, tp_len};
        }

        c_array<const tp_type> Nonstatic() const {
            return {buf_raw, tp_len};
        }
    };

    template<typename tp_type>
    class c_stack {
    public:
        c_stack() = default;
        c_stack(const c_array<tp_type> backing_arr, const int init_height = 0) : m_backing_arr(backing_arr), m_height(init_height) {
            assert(init_height >= 0 && init_height <= backing_arr.Len());
        }

        int Height() const;
        int Cap() const;
        tp_type& operator[](const int index) const;
        void Push(const tp_type& val);
        tp_type Pop();

    private:
        c_array<tp_type> m_backing_arr;
        int m_height = 0;
    };
}
