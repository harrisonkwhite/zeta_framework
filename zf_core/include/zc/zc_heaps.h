#pragma once

#include <zc/zc_allocators.h>

namespace zf {
    template<typename tp_key_type, typename tp_value_type>
    class c_min_heap {
    public:
        struct s_node {
            tp_key_type key = {};
            tp_value_type val = {};
        };

        [[nodiscard]]
        t_b8 Init(c_mem_arena& mem_arena, const t_size cap, const t_comparator<tp_key_type> comparator = DefaultComparator) {
            ZF_ASSERT(cap > 0);

            m_len = 0;

            if (!m_nodes.Init(mem_arena, cap)) {
                return false;
            }

            return true;
        }

        void Insert(const tp_key_type& key, const tp_value_type& val) {
            ZF_ASSERT(!IsFull());

            m_len++;
            m_nodes[m_len - 1].key = key;
            m_nodes[m_len - 1].val = val;
            BubbleUp(m_len - 1);
        }

        s_node RemoveMin() {
            ZF_ASSERT(!IsEmpty());

            const s_node ret = m_nodes[0];

            m_len--;

            if (m_len > 0) {
                m_nodes[0] = m_nodes[m_len];
                BubbleDown(0);
            }

            return ret;
        }

        t_size Len() const {
            return m_len;
        }

        t_size Cap() const {
            return m_nodes.Len();
        }

        t_b8 IsEmpty() const {
            return m_len == 0;
        }

        t_b8 IsFull() const {
            return m_len == Cap();
        }

        const s_node& Min() const {
            ZF_ASSERT(!IsEmpty());
            return m_nodes[0];
        }

    private:
        c_array<s_node> m_nodes;
        t_size m_len = 0;
        t_comparator<tp_key_type> m_comparator = nullptr;

        static t_size IndexOfLeft(const t_size index) {
            return (2 * index) + 1;
        }

        static t_size IndexOfRight(const t_size index) {
            return (2 * index) + 2;
        }

        static t_size IndexOfParent(const t_size index) {
            return (index - 1) / 2;
        }

        t_b8 Contains(const t_size index) const {
            return index >= 0 && index < m_len;
        }

        void BubbleUp(const t_size index) {
            ZF_ASSERT(Contains(index));

            if (index == 0) {
                return;
            }

            const t_size par_index = IndexOfParent(index);

            if (m_comparator(m_nodes[par_index].key, m_nodes[index].key) <= 0) {
                return;
            }

            Swap(m_nodes[index], m_nodes[par_index]);
            BubbleUp(par_index);
        }

        void BubbleDown(const t_size index) {
            ZF_ASSERT(Contains(index));

            const t_size left_index = IndexOfLeft(index);
            const t_b8 left_exists = Contains(left_index);

            const t_size right_index = IndexOfRight(index);
            const t_b8 right_exists = Contains(right_index);

            if (!left_exists && !right_exists) {
                return;
            }

            const t_size index_of_smaller = (!right_exists || m_comparator(m_nodes[left_index].key, m_nodes[right_index].key) <= 0) ? left_index : right_index;

            if (m_comparator(m_nodes[index_of_smaller].key, m_nodes[index].key)) {
                Swap(m_nodes[index], m_nodes[index_of_smaller]);
                BubbleDown(index_of_smaller);
            }
        }
    };
}
