#pragma once

#include "zc_mem.h"

namespace zf {
    template<typename tp_type>
    class c_array_list {
    public:
        tp_type& operator[](const int index) const {
            assert(index < m_len);
            return m_arr[index];
        }

        int Len() const {
            return m_len;
        }

        int Capacity() const {
            return m_arr.Len();
        }

        void Append(const tp_type& elem) {
            m_arr[m_len] = elem;
            m_len++;
        }

    private:
        c_array<tp_type> m_arr;
        int m_len;
    };
}
