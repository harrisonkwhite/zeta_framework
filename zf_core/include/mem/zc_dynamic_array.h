#pragma once

#include "zc_mem.h"
#include <cassert>

namespace zf {
    constexpr float g_dyn_arr_resize_scalar = 1.5f;

    template<typename tp_type>
    class c_dynamic_array {
    public:
        [[nodiscard]]
        bool Init(c_mem_arena& mem_arena, const int cap) {
            assert(cap > 0);

            *this = {};
            m_elems = mem_arena.PushArray<tp_type>(cap);
            m_mem_arena = &mem_arena;
            return !m_elems.IsEmpty();
        }

        int Len() const {
            assert(IsInitted());
            return m_len;
        }

        int Capacity() const {
            assert(IsInitted());
            return m_elems.Len();
        }

        tp_type& operator[](const int index) {
            assert(IsInitted());
            assert(index >= 0 && index < m_len);
            return m_elems[index];
        }

        const tp_type& operator[](const int index) const {
            assert(IsInitted());
            assert(index >= 0 && index < m_len);
            return m_elems[index];
        }

        [[nodiscard]]
        tp_type* Append() {
            assert(IsInitted());

            if (m_len == Capacity()) {
                // Resize!
                const int cap_next = m_elems.Len() * g_dyn_arr_resize_scalar;
                const auto elems_next = m_mem_arena->PushArray<tp_type>(cap_next);

                if (elems_next.IsEmpty()) {
                    return nullptr;
                }

                MemCopy(elems_next, m_elems);

                m_elems = elems_next;
            }

            m_len++;

            return &m_elems[m_len - 1];
        }

        [[nodiscard]]
        bool Append(const tp_type& elem) {
            tp_type* const elem_in_arr = Append();

            if (!elem_in_arr) {
                return false;
            }

            *elem_in_arr = elem;

            return true;
        }

    private:
        bool IsInitted() const {
            return m_mem_arena && !m_elems.IsEmpty();
        }

        c_mem_arena* m_mem_arena = nullptr;
        c_array<tp_type> m_elems;
        int m_len = 0;
    };
}
