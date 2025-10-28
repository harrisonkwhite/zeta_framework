#pragma once

#include "zc_mem.h"
#include "zc_dynamic_array.h"

namespace zf {
    template<typename tp_key_type, typename tp_value_type>
    class c_binary_tree {
    public:
        struct s_node {
            // @todo: Probably better off putting these into distinct arrays.
            tp_key_type key = {};
            tp_value_type val = {};
            s_node* left = nullptr; // @todo: Use index instead?
            s_node* right = nullptr;

            int CalcHeight() {
                return 1 + Min(left ? left->CalcHeight() : 0, right ? right->CalcHeight() : 0);
            }
        };

        [[nodiscard]]
        bool Init(c_mem_arena& mem_arena, const int cap) {
            assert(cap > 0);

            *this = {};

            return m_nodes.Init(mem_arena, cap);
        }

        [[nodiscard]]
        bool Find(const tp_key_type key, tp_value_type* const val = nullptr) {
            const s_node* node = m_root;

            while (node) {
                if (key == node->key) {
                    if (val) {
                        *val = node->val;
                    }

                    return true;
                } else if (key > node->key) {
                    node = node->right;
                } else {
                    node = node->left;
                }
            }

            return false;
        }

        [[nodiscard]]
        bool Insert(const tp_key_type& key, const tp_value_type& val) {
            s_node** node = &m_root;

            while (*node) {
                if (key >= (*node)->key) {
                    node = &(*node)->right;
                } else {
                    node = &(*node)->left;
                }
            }

            s_node* const new_node = m_nodes.Append();

            if (!new_node) {
                return false;
            }

            new_node->key = key;
            new_node->val = val;

            *node = new_node;

            return true;
        }

    private:
        c_dynamic_array<s_node> m_nodes;
        s_node* m_root = nullptr;
    };
}
