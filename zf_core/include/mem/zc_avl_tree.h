#pragma once

#include "zc_mem.h"
#include "zc_dynamic_array.h"

namespace zf {
    template<typename tp_type>
    class c_binary_tree {
    public:
        struct s_node {
            tp_type val;

            // @todo: Probably better off putting these into 2 distinct arrays.
            s_node* left;
            s_node* right;
        };

        bool Init(c_mem_arena& mem_arena, const int cap) {
            assert(cap > 0);

            *this = {};

            if (!m_nodes.Init(mem_arena, cap)) {
                return false;
            }

            m_root = &m_nodes[0];

            return true;
        }

        bool Insert(const tp_type& val) {
            s_node*& n = m_root;

            while (n) {
                if (val == n->val) {
                    // @todo: Probably increment a counter or something?
                    return true;
                } else if (val < n->val) {
                    n = n->left;
                } else {
                    n = n->right;
                }
            }

            s_node* const new_node = m_nodes.Append();

            if (!new_node) {
                return false;
            }

            n = new_node;

            return true;
        }

    private:
        c_dynamic_array<s_node> m_nodes;
        s_node* m_root;
    };
}
