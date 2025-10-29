#pragma once

#include "zc_mem.h"
#include "zc_dynamic_array.h"

namespace zf {
    template<typename tp_key_type, typename tp_value_type>
    class c_binary_tree {
    public:
        struct s_node {
            // @todo: Probably better off putting these into distinct arrays.
            // @todo: The pointers could all be put into the 
            tp_key_type key = {};
            tp_value_type val = {};
            s_node* left = nullptr; // @todo: Use index instead?
            s_node* right = nullptr;
            s_node* par = nullptr;
        };

#if 0
        void LogState() {
            LogStateFrom(m_root, 0);
        }

        static void LogStateFrom(const s_node* const node, const int indent) {
            for (int i = 0; i < indent; i++) {
                printf("    ");
            }

            if (node) {
                ZF_LOG("KEY: %d, CUR: %p, LEFT: %p, RIGHT: %p, PAR: %p", node->key, node, node->left, node->right, node->par); // @todo: Can't assume the value is an integer!

                LogStateFrom(node->left, indent + 1);
                LogStateFrom(node->right, indent + 1);
            } else {
                ZF_LOG("NONE");
            }
        }
#endif

        static bool IsNodeBalanced(const s_node* node) {
            if (!node) {
                return true;
            }

            return abs(CalcHeight(node->right) - CalcHeight(node->left)) <= 1
                && IsNodeBalanced(node->left)
                && IsNodeBalanced(node->right);
        }

        bool IsBalanced() {
            return IsNodeBalanced(m_root);
        }

        [[nodiscard]]
        bool Init(c_mem_arena& mem_arena, const int cap) {
            assert(cap > 0);

            *this = {};

            return m_nodes.Init(mem_arena, cap);
        }

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

    private:
        static int CalcHeight(const s_node* const node) {
            if (!node) {
                return 0;
            }

            return 1 + Max(CalcHeight(node->left), CalcHeight(node->right));
        }

        static void TrinodeRestructuringLL(s_node& a) {
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

        static void TrinodeRestructuringRR(s_node& c) {
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

        static void TrinodeRestructuringRL(s_node& a) {
            s_node* const c = a.right;
            assert(c);

            s_node* const b = c->left;
            assert(b);

            b->par = c;
            c->par = &a;
            a.right = b;
            c->left = b->right;
            b->right = c;

            TrinodeRestructuringLL(a);
        }

        static void TrinodeRestructuringLR(s_node& c) {
            s_node* const a = c.left;
            assert(a);

            s_node* const b = a->right;
            assert(b);

            b->par = a;
            a->par = &c;
            a->right = b->left;
            b->left = a;
            c.left = b;

            TrinodeRestructuringRR(c);
        }

        c_dynamic_array<s_node> m_nodes;
        s_node* m_root = nullptr;

        void HandleImbalanceFrom(s_node& src_node) {
            const auto is_imbalanced = [](const s_node& node) {
                return abs(CalcHeight(node.right) - CalcHeight(node.left)) > 1;
            };

            class c_trace {
            public:
                void Update(const bool right) {
                    ZF_LOG("RESTRUCTURE: updating trace with %s", right ? "right" : "left");

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
                            TrinodeRestructuringLL(node);
                            break;

                        case 1:
                            TrinodeRestructuringRL(node);
                            break;

                        case 2:
                            TrinodeRestructuringLR(node);
                            break;

                        case 3:
                            TrinodeRestructuringRR(node);
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
    };
}
