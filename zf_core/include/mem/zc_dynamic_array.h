#pragma once

#include "zc_mem.h"

namespace zf {
    template<typename tp_type>
    class c_dynamic_array {
    public:
        bool Init(c_mem_arena& mem_arena, const int cap) {
            assert(cap > 0);
            *this = {};
            m_elems = mem_arena.PushArray<tp_type>(cap);
            m_mem_arena = &mem_arena;
            return !m_elems.IsEmpty();
        }

        int Len() const {
            return m_len;
        }

        int Capacity() const {
            return m_elems.Len();
        }

        tp_type& operator[](const int index) {
            assert(index >= 0 && index < m_len);
            return m_elems[index];
        }

        const tp_type& operator[](const int index) const {
            assert(index >= 0 && index < m_len);
            return m_elems[index];
        }

        bool Append(const tp_type& elem) {
            if (m_len == Capacity()) {
                const int cap_next = m_elems.Len() + (m_elems.Len() / 2);
                const c_array<tp_type> elems_next = m_mem_arena->PushArray<tp_type>(cap_next);

                if (elems_next.IsEmpty()) {
                    return false;
                }

                MemCopy(elems_next, m_elems);

                m_elems = elems_next;
            }

            m_elems[m_len] = elem;

            m_len++;

            return true;
        }

    private:
        c_mem_arena* m_mem_arena = nullptr;
        c_array<tp_type> m_elems;
        int m_len = 0;
    };
}
