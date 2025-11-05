#pragma once

#include <zc/mem/mem.h>
#include <zc/mem/bit_vector.h>

namespace zf {
    template<typename tp_type>
    using t_hash_func = int (*)(const tp_type& key);

    template<typename tp_key_type, typename tp_value_type>
    class c_unordered_map {
    public:
        // The provided hash function has to map a key to an integer 0 or higher.
        // The immediate capacity is the total number of upfront slots (i.e. the maximum possible number of slots for which an O(1) access of a value from a key can happen).
        // The key-value pair capacity is the overall limit of how many key-value pairs this map can ever hold. It obviously has to be equal to or greater than the immediate capacity.
        bool Init(c_mem_arena& mem_arena, const t_hash_func<tp_key_type> hash_func, const int immediate_cap = 1024, const int kv_pair_cap = 1 << 16) {
            assert(hash_func);
            assert(kv_pair_cap >= immediate_cap);

            *this = {};

            m_hash_func = hash_func;

            if (!m_backing_store.Init(mem_arena, kv_pair_cap)) {
                return false;
            }

            if (!m_backing_store_indexes.Init(mem_arena, immediate_cap)) {
                return false;
            }

            for (int i = 0; i < m_backing_store_indexes.Len(); i++) {
                m_backing_store_indexes[i] = -1;
            }

            return true;
        }

        bool Get(const tp_key_type& key, tp_value_type* const val = nullptr) const {
            const int hash_index = KeyToHashIndex(key);
            return m_backing_store.Get(m_backing_store_indexes[hash_index], key, val);
        }

        [[nodiscard]]
        bool Put(const tp_key_type& key, const tp_value_type& val) {
            const int hash_index = KeyToHashIndex(key);
            return m_backing_store.Put(m_backing_store_indexes[hash_index], key, val);
        }

        bool Remove(const tp_key_type& key) {
            const int hash_index = KeyToHashIndex(key);
            return m_backing_store.Remove(m_backing_store_indexes[hash_index], key);
        }

    private:
        // This is where all the key-value pairs are actually stored, it's basically a single buffer containing a bunch of linked lists.
        class c_backing_store {
        public:
            bool Init(c_mem_arena& mem_arena, const int cap) {
                if (!m_keys.Init(mem_arena, cap)) {
                    return false;
                }

                if (!m_vals.Init(mem_arena, cap)) {
                    return false;
                }

                if (!m_next_indexes.Init(mem_arena, cap)) {
                    return false;
                }

                if (!m_usage.Init(mem_arena, cap)) {
                    return false;
                }

                return true;
            }

            bool Get(const int index, const tp_key_type& key, tp_value_type* const val) const {
                assert(index >= -1 && index < Len());

                if (index == -1) {
                    return false;
                }

                if (m_keys[index] == key) {
                    if (val) {
                        *val = m_vals[index];
                    }

                    return true;
                }

                return Get(m_next_indexes[index], key, val);
            }

            [[nodiscard]]
            bool Put(int& index, const tp_key_type& key, const tp_value_type& val) {
                assert(index >= -1 && index < Len());

                if (index == -1) {
                    const int prospective_index = m_usage.IndexOfFirstUnsetBit();

                    if (prospective_index == -1) {
                        // We're out of room!
                        return false;
                    }

                    index = prospective_index;

                    m_keys[index] = key;
                    m_vals[index] = val;
                    m_next_indexes[index] = -1;
                    m_usage.SetBit(index);

                    return true;
                }

                if (m_keys[index] == key) {
                    m_vals[index] = val;
                    return true;
                }

                return Put(m_next_indexes[index], key, val);
            }

            bool Remove(int& index, const tp_key_type& key) {
                assert(index >= -1 && index < Len());

                if (index == -1) {
                    return false;
                }

                if (m_keys[index] == key) {
                    m_usage.UnsetBit(index);
                    index = m_next_indexes[index];
                    return true;
                }

                return Remove(m_next_indexes[index], key);
            }

        private:
            int Len() const {
                return m_keys.Len();
            }

            c_array<tp_key_type> m_keys;
            c_array<tp_value_type> m_vals;
            c_array<int> m_next_indexes;
            c_bit_vector m_usage;
        };

        c_backing_store m_backing_store;
        c_array<int> m_backing_store_indexes;

        t_hash_func<tp_key_type> m_hash_func;

        int KeyToHashIndex(const tp_key_type& key) const {
            const int val = m_hash_func(key);
            assert(val >= 0);

            return val % m_backing_store_indexes.Len();
        }
    };
}
