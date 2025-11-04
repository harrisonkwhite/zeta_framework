#pragma once

#include <zc/mem/mem.h>
#include <zc/mem/bit_vector.h>

namespace zf {
    template<typename tp_key_type, typename tp_value_type>
    class c_unordered_map {
    public:
        bool Init(c_mem_arena& mem_arena, int (* const hash_code_generator)(const tp_key_type& key)) {
            assert(hash_code_generator);

            // @todo: Add ability to customise capacities.

            *this = {};

            m_hash_code_generator = hash_code_generator;

            if (!m_backing_store.Init(mem_arena, 1 << 16)) {
                return false;
            }

            m_backing_store_indexes = mem_arena.PushArray<int>(1024);

            if (m_backing_store_indexes.IsEmpty()) {
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

        bool Remove(const tp_key_type& key, const tp_value_type& val) {
            const int hash_index = KeyToHashIndex(key);
            return m_backing_store.Remove(m_backing_store_indexes[hash_index], key, val);
        }

    private:
        class c_backing_store {
        public:
            bool Init(c_mem_arena& mem_arena, const int cap) {
                m_keys = mem_arena.PushArray<tp_key_type>(cap);

                if (m_keys.IsEmpty()) {
                    return false;
                }

                m_vals = mem_arena.PushArray<tp_value_type>(cap);

                if (m_vals.IsEmpty()) {
                    return false;
                }

                m_next_indexes = mem_arena.PushArray<int>(cap);

                if (m_next_indexes.IsEmpty()) {
                    return false;
                }

                for (int i = 0; i < m_next_indexes.Len(); i++) {
                    m_next_indexes[i] = -1; // Probably not needed!
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
                        // @todo: Actually reallocate and expand!
                        return false;
                    }

                    index = prospective_index;

                    m_keys[index] = key;
                    m_vals[index] = val;
                    m_next_indexes[index] = -1;
                    m_usage.SetBit(prospective_index);

                    return true;
                }

                if (m_keys[index] == key) {
                    m_vals[index] = val;
                    return true;
                }

                return Put(m_next_indexes[index], key, val);
            }

            bool Remove(int& index, const tp_key_type& key, const tp_value_type& val) {
                assert(index >= -1 && index < Len());

                if (index == -1) {
                    return false;
                }

                if (m_keys[index] == key) {
                    m_usage.UnsetBit(index);
                    index = m_next_indexes[index];
                    return true;
                }

                return Remove(m_next_indexes[index], key, val);
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

        int (*m_hash_code_generator)(const tp_key_type& key) = nullptr;

        int KeyToHashIndex(const tp_key_type& key) const {
            const int val = m_hash_code_generator(key);
            assert(val >= 0);

            return val % m_backing_store_indexes.Len();
        }
    };
}
