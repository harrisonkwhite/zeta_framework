#pragma once

#include <zc/mem/arrays.h>

namespace zf {
    template<typename tp_type>
    class c_min_heap {
    public:
        [[nodiscard]]
        t_b8 Init(c_mem_arena& mem_arena, const t_s32 cap) {
            ZF_ASSERT(cap > 0);

            c_array<tp_type> nodes;

            if (!nodes.Init(mem_arena, cap)) {
                return false;
            }

            m_nodes = nodes;
            m_len = 0;

            return true;
        }

        void Insert(const tp_type& val) {
            ZF_ASSERT(!IsFull());

            m_len++;
            m_nodes[m_len - 1] = val;
            BubbleUp(m_len - 1);
        }

        tp_type RemoveMin() {
            ZF_ASSERT(!IsEmpty());

            const tp_type ret = m_nodes[0];

            m_len--;

            if (m_len > 0) {
                m_nodes[0] = m_nodes[m_len];
                BubbleDown(0);
            }

            return ret;
        }

        t_s32 Len() const {
            return m_len;
        }

        t_s32 Cap() const {
            return m_nodes.Len();
        }

        t_b8 IsEmpty() const {
            return m_len == 0;
        }

        t_b8 IsFull() const {
            return m_len == Cap();
        }

        const tp_type& Min() const {
            ZF_ASSERT(!IsEmpty());
            return m_nodes[0];
        }

    private:
        c_array<tp_type> m_nodes;
        t_s32 m_len = 0;

        static t_s32 IndexOfLeft(const t_s32 index) {
            return (2 * index) + 1;
        }

        static t_s32 IndexOfRight(const t_s32 index) {
            return (2 * index) + 2;
        }

        static t_s32 IndexOfParent(const t_s32 index) {
            return (index - 1) / 2;
        }

        t_b8 Contains(const t_s32 index) const {
            return index >= 0 && index < m_len;
        }

        void BubbleUp(const t_s32 index) {
            ZF_ASSERT(Contains(index));

            if (index == 0) {
                return;
            }

            const t_s32 par_index = IndexOfParent(index);

            if (m_nodes[par_index] <= m_nodes[index]) {
                return;
            }

            Swap(m_nodes[index], m_nodes[par_index]);
            BubbleUp(par_index);
        }

        void BubbleDown(const t_s32 index) {
            ZF_ASSERT(Contains(index));

            const t_s32 left_index = IndexOfLeft(index);
            const t_b8 left_exists = Contains(left_index);

            const t_s32 right_index = IndexOfRight(index);
            const t_b8 right_exists = Contains(right_index);

            if (!left_exists && !right_exists) {
                return;
            }

            const t_s32 index_of_smaller = !right_exists || m_nodes[left_index] <= m_nodes[right_index] ? left_index : right_index;

            if (m_nodes[index_of_smaller] < m_nodes[index]) {
                Swap(m_nodes[index], m_nodes[index_of_smaller]);
                BubbleDown(index_of_smaller);
            }
        }
    };
}
