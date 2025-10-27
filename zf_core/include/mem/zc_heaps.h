#pragma once

#include "zc_mem.h"

namespace zf {
    template<typename tp_type>
    class c_min_heap {
    public:
        bool Init(c_mem_arena& mem_arena, const int cap) {
            assert(cap > 0);

            *this = {};
            m_elems = mem_arena.PushArray<tp_type>(cap);
            return !m_elems.IsEmpty();
        }

        void Insert(const tp_type& val) {
            assert(m_elems_used_cnt < m_elems.Len());

            m_elems_used_cnt++;
            m_elems[m_elems_used_cnt - 1] = val;
            BubbleUp(m_elems_used_cnt - 1);
        }

        const tp_type& GetMin() const {
            assert(m_elems_used_cnt > 0);
            return m_elems[0];
        }

        void RemoveMin() {
            m_elems_used_cnt--;

            if (m_elems_used_cnt > 0) {
                m_elems[0] = m_elems[m_elems_used_cnt];
                BubbleDown(0);
            }
        }

        int GetElemCnt() {
            return m_elems_used_cnt;
        }

    private:
        c_array<tp_type> m_elems;
        int m_elems_used_cnt = 0;

        static int IndexOfLeft(const int index) {
            return (2 * index) + 1;
        }

        static int IndexOfRight(const int index) {
            return (2 * index) + 2;
        }

        static int IndexOfParent(const int index) {
            return (index - 1) / 2;
        }

        bool Contains(const int index) const {
            return index >= 0 && index < m_elems_used_cnt;
        }

        void BubbleUp(const int index) {
            assert(Contains(index));

            if (index == 0) {
                return;
            }

            const int par_index = IndexOfParent(index);

            if (m_elems[par_index] <= m_elems[index]) {
                return;
            }

            Swap(m_elems[index], m_elems[par_index]);
            BubbleUp(par_index);
        }

        void BubbleDown(const int index) {
            assert(Contains(index));

            const int left_index = IndexOfLeft(index);
            const bool left_exists = Contains(left_index);

            const int right_index = IndexOfRight(index);
            const bool right_exists = Contains(right_index);

            if (!left_exists && !right_exists) {
                return;
            }

            const int index_of_smaller = !right_exists || m_elems[left_index] <= m_elems[right_index] ? left_index : right_index;

            if (m_elems[index_of_smaller] < m_elems[index]) {
                Swap(m_elems[index], m_elems[index_of_smaller]);
                BubbleDown(index_of_smaller);
            }
        }
    };
}
