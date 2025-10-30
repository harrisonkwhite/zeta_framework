#pragma once

#include "zc_mem.h"
#include "zc_dynamic_array.h"

namespace zf {
    struct s_avl_node_mapping {
        int left = -1;
        int right = -1;
        int par = -1;
    };

#if 0
    int CalcAVLNodeHeight(const c_array<const s_avl_node_mapping> node_mappings, const int node_index) {
        if (node_index == -1) {
            return 0;
        }

        const auto& map = node_mappings[node_index];
        return 1 + Max(CalcAVLNodeHeight(node_mappings, map.left), CalcAVLNodeHeight(node_mappings, map.right));
    }

    bool IsAVLNodeBalanced(const c_array<const s_avl_node_mapping> node_mappings, const int node_index) {
        if (node_index == -1) {
            return true;
        }

        const auto& map = node_mappings[node_index];
        return abs(CalcAVLNodeHeight(node_mappings, map.right) - CalcAVLNodeHeight(node_mappings, map.left)) <= 1
            && IsAVLNodeBalanced(node_mappings, map.left)
            && IsAVLNodeBalanced(node_mappings, map.right);
    }

    void AVLTrinodeRestructuringLL(const c_array<const s_avl_node_mapping> ptrs, const int index) {
        s_node* const b = a.right;
        assert(b);

        s_node* const c = b->right;
        assert(c);

        a.right = b->left;
        b->par = a.par;
        a.par = b;
        b->left = &a;
        b->right = c;
    }

    void AVLTrinodeRestructuringRR(s_node& c) {
        s_node* const b = c.left;
        assert(b);

        s_node* const a = b->left;
        assert(a);

        a->par = b;
        b->par = c.par;
        c.par = b;
        c.left = b->right;
        b->right = &c;
    }

    void TrinodeRestructuringRL(s_node& a) {
        s_node* const c = a.right;
        assert(c);

        s_node* const b = c->left;
        assert(b);

        b->par = c;
        c->par = &a;
        a.right = b;
        c->left = b->right;
        b->right = c;

        AVLTrinodeRestructuringLL(a);
    }

    void TrinodeRestructuringLR(s_node& c) {
        s_node* const a = c.left;
        assert(a);

        s_node* const b = a->right;
        assert(b);

        b->par = a;
        a->par = &c;
        a->right = b->left;
        b->left = a;
        c.left = b;

        AVLTrinodeRestructuringRR(c);
    }

    void HandleImbalanceFrom(s_node& src_node) {
        class c_trace {
        public:
            void Update(const bool right) {
                // Uses FIFO logic.
                m_raw >>= 1;

                if (right) {
                    m_raw |= 2;
                } else {
                    m_raw &= ~2;
                }
            }

            void RunAppropriateTrinodeRestructuringFunc(s_node& node) {
                switch (m_raw) {
                    case 0:
                        AVLTrinodeRestructuringLL(node);
                        break;

                    case 1:
                        TrinodeRestructuringRL(node);
                        break;

                    case 2:
                        TrinodeRestructuringLR(node);
                        break;

                    case 3:
                        AVLTrinodeRestructuringRR(node);
                        break;
                }
            }

        private:
            t_u8 m_raw = 0; // Encodes 4 relevant traces with 2 bits: left-left (00), left-right (01), right-left (10), and right-right (11).
        };

        c_trace trace;

        s_node* node = &src_node;

        while (true) {
            if (CalcHeight(node) > 2 && is_imbalanced(*node)) {
                trace.RunAppropriateTrinodeRestructuringFunc(*node);
                node = &src_node; // @todo: This is inefficient but is easiest to reason about for now.
                continue;
            }

            if (!node->par) {
                break;
            }

            trace.Update(node->par->key > node->key);

            node = node->par;
        }

        // @todo: Currently updating the root in case it changed during restructuring. There is probably a better way to do this.
        while (m_root->par) {
            m_root = m_root->par;
        }
    }
#endif

    template<typename tp_key_type, typename tp_value_type>
    class c_avl_tree {
    public:
        [[nodiscard]] bool Init(c_mem_arena& mem_arena, const int cap);
        bool Find(const tp_key_type& key, tp_value_type* const val = nullptr) const;
        [[nodiscard]] bool Insert(const tp_key_type& key, const tp_value_type& val);
        bool Remove(const tp_key_type& key);

    private:
        c_dynamic_array<tp_key_type> m_node_keys;
        c_dynamic_array<tp_value_type> m_node_vals;
        c_dynamic_array<s_avl_node_mapping> m_node_mappings;
        int m_root_node_index = -1;
    };

    template<typename tp_key_type, typename tp_value_type>
    bool c_avl_tree<tp_key_type, tp_value_type>::Init(c_mem_arena& mem_arena, const int cap) {
        assert(cap > 0);

        *this = {};

        return m_node_keys.Init(mem_arena, cap)
            || m_node_vals.Init(mem_arena, cap)
            || m_node_mappings.Init(mem_arena, cap);
    }

    template<typename tp_key_type, typename tp_value_type>
    bool c_avl_tree<tp_key_type, tp_value_type>::Find(const tp_key_type& key, tp_value_type* const val) const {
        int node_index = m_root_node_index;

        while (node_index != -1) {
            if (key == m_node_keys[node_index]) {
                if (val) {
                    *val = m_node_vals[node_index];
                }

                return true;
            } else if (key > m_node_keys[node_index]) {
                node_index = m_node_mappings[node_index].right;
            } else {
                node_index = m_node_mappings[node_index].left;
            }
        }

        return false;
    }

#if 0
    template<typename tp_key_type, typename tp_value_type>
    bool c_avl_tree<tp_key_type, tp_value_type>::Insert(const tp_key_type& key, const tp_value_type& val) {
        s_node** node = &m_root;
        s_node* node_par = nullptr;

        while (*node) {
            node_par = *node;

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
        new_node->par = node_par;

        *node = new_node;

        HandleImbalanceFrom(**node);

        return true;
    }

    template<typename tp_key_type, typename tp_value_type>
    bool c_avl_tree<tp_key_type, tp_value_type>::Remove(const tp_key_type& key) {
        s_node* node = m_root;

        while (node) {
            if (key == node->key) {
                if (!node->right && !node->left) {
                    if (node->par) {
                        if (node->par.key > node->key) {
                            node->par.left = nullptr;
                        } else {
                            node->par.right = nullptr;
                        }

                        node->par = nullptr;
                    }
                }

            } else if (key > node->key) {
                node = node->right;
            } else {
                node = node->left;
            }
        }
    }
#endif
}
