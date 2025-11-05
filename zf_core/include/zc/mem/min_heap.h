#pragma once

#include <zc/mem/arrays.h>

namespace zf {
    template<typename tp_type>
    class c_min_heap {
    public:
        [[nodiscard]] bool Init(c_mem_arena& mem_arena, const int cap);
        void Insert(const tp_type& val);
        tp_type RemoveMin();

        int Len() const {
            return m_len;
        }

        int Cap() const {
            return m_nodes.Len();
        }

        bool IsEmpty() const {
            return m_len == 0;
        }

        bool IsFull() const {
            return m_len == Cap();
        }

        const tp_type& Min() const {
            assert(!IsEmpty());
            return m_nodes[0];
        }

    private:
        c_array<tp_type> m_nodes;
        int m_len = 0;

        static int IndexOfLeft(const int index) {
            return (2 * index) + 1;
        }

        static int IndexOfRight(const int index) {
            return (2 * index) + 2;
        }

        static int IndexOfParent(const int index) {
            return (index - 1) / 2;
        }

        void BubbleUp(const int index);
        void BubbleDown(const int index);

        bool Contains(const int index) const {
            return index >= 0 && index < m_len;
        }
    };

    template<typename tp_type>
    bool c_min_heap<tp_type>::Init(c_mem_arena& mem_arena, const int cap) {
        assert(cap > 0);

        c_array<tp_type> nodes;

        if (!nodes.Init(mem_arena, cap)) {
            return false;
        }

        m_nodes = nodes;
        m_len = 0;

        return true;
    }

    template<typename tp_type>
    void c_min_heap<tp_type>::Insert(const tp_type& val) {
        assert(!IsFull());

        m_len++;
        m_nodes[m_len - 1] = val;
        BubbleUp(m_len - 1);
    }

    template<typename tp_type>
    tp_type c_min_heap<tp_type>::RemoveMin() {
        assert(!IsEmpty());

        const tp_type ret = m_nodes[0];

        m_len--;

        if (m_len > 0) {
            m_nodes[0] = m_nodes[m_len];
            BubbleDown(0);
        }

        return ret;
    }

    template<typename tp_type>
    void c_min_heap<tp_type>::BubbleUp(const int index) {
        assert(Contains(index));

        if (index == 0) {
            return;
        }

        const int par_index = IndexOfParent(index);

        if (m_nodes[par_index] <= m_nodes[index]) {
            return;
        }

        Swap(m_nodes[index], m_nodes[par_index]);
        BubbleUp(par_index);
    }

    template<typename tp_type>
    void c_min_heap<tp_type>::BubbleDown(const int index) {
        assert(Contains(index));

        const int left_index = IndexOfLeft(index);
        const bool left_exists = Contains(left_index);

        const int right_index = IndexOfRight(index);
        const bool right_exists = Contains(right_index);

        if (!left_exists && !right_exists) {
            return;
        }

        const int index_of_smaller = !right_exists || m_nodes[left_index] <= m_nodes[right_index] ? left_index : right_index;

        if (m_nodes[index_of_smaller] < m_nodes[index]) {
            Swap(m_nodes[index], m_nodes[index_of_smaller]);
            BubbleDown(index_of_smaller);
        }
    }
}
