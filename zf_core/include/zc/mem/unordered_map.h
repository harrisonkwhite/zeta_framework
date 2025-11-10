#pragma once

#include <zc/mem/mem.h>
#include <zc/mem/bit_vector.h>
#include <zc/mem/strs.h>

namespace zf {
    template<typename tp_type>
    using t_hash_func = t_size (*)(const tp_type& key);

    inline const t_hash_func<t_s32> g_int_hash_func = [](const t_s32& key) {
        return static_cast<t_size>(key & 0x7FFFFFFF); // Mask out the sign bit.
    };

    inline const t_hash_func<s_str_view> g_str_hash_func = [](const s_str_view& key) {
        ZF_ASSERT(key.IsTerminated());

        // This is an FNV-1a implementation.
        const t_u32 offs_basis = 2166136261u;
        const t_u32 prime = 16777619u;

        t_u32 hash = offs_basis;

        for (t_size i = 0; key.chrs[i]; i++) {
            hash ^= static_cast<unsigned char>(key.chrs[i]);
            hash *= prime;
        }

        return static_cast<t_size>(hash & 0x7FFFFFFF);
    };

    template<typename tp_type>
    using t_key_cmp_func = t_b8 (*)(const tp_type& key_a, const tp_type& key_b); // Returns true if the two keys are equal.

    template<typename tp_key_type, typename tp_value_type>
    class c_unordered_map {
    public:
        // The provided hash function has to map a key to an integer 0 or higher.
        // The immediate capacity is the total number of upfront slots (i.e. the maximum possible number of slots for which an O(1) access of a value from a key can happen).
        // The key-value pair capacity is the overall limit of how many key-value pairs this map can ever hold. It obviously has to be equal to or greater than the immediate capacity.
        t_b8 Init(c_mem_arena& mem_arena, const t_hash_func<tp_key_type> hash_func, const t_key_cmp_func<tp_key_type> key_cmp_func, const t_size immediate_cap = 1024, const t_size kv_pair_cap = 1 << 16) {
            ZF_ASSERT(hash_func);
            ZF_ASSERT(key_cmp_func);
            ZF_ASSERT(kv_pair_cap >= immediate_cap);

            *this = {};

            if (!m_backing_store.Init(mem_arena, kv_pair_cap, key_cmp_func)) {
                return false;
            }

            if (!m_backing_store_indexes.Init(mem_arena, immediate_cap)) {
                return false;
            }

            for (t_size i = 0; i < m_backing_store_indexes.Len(); i++) {
                m_backing_store_indexes[i] = -1;
            }

            m_hash_func = hash_func;

            return true;
        }

        t_b8 Get(const tp_key_type& key, tp_value_type* const val = nullptr) const {
            const t_size hash_index = KeyToHashIndex(key);
            return m_backing_store.Get(m_backing_store_indexes[hash_index], key, val);
        }

        [[nodiscard]]
        t_b8 Put(const tp_key_type& key, const tp_value_type& val) const {
            const t_size hash_index = KeyToHashIndex(key);
            return m_backing_store.Put(m_backing_store_indexes[hash_index], key, val);
        }

        t_b8 Remove(const tp_key_type& key) const {
            const t_size hash_index = KeyToHashIndex(key);
            return m_backing_store.Remove(m_backing_store_indexes[hash_index], key);
        }

    private:
        // This is where all the key-value pairs are actually stored, it's basically a single buffer containing a bunch of linked lists.
        class c_backing_store {
        public:
            t_b8 Init(c_mem_arena& mem_arena, const t_size cap, const t_key_cmp_func<tp_key_type> key_cmp_func) {
                ZF_ASSERT(cap > 0);
                ZF_ASSERT(key_cmp_func);

                *this = {};

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

                m_key_cmp_func = key_cmp_func;

                return true;
            }

            t_b8 Get(const t_size index, const tp_key_type& key, tp_value_type* const val) const {
                ZF_ASSERT(index >= -1 && index < Len());

                if (index == -1) {
                    return false;
                }

                if (m_key_cmp_func(m_keys[index], key)) {
                    if (val) {
                        *val = m_vals[index];
                    }

                    return true;
                }

                return Get(m_next_indexes[index], key, val);
            }

            [[nodiscard]]
            t_b8 Put(t_size& index, const tp_key_type& key, const tp_value_type& val) const {
                ZF_ASSERT(index >= -1 && index < Len());

                if (index == -1) {
                    const t_size prospective_index = m_usage.FindFirstUnsetBit();

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

                if (m_key_cmp_func(m_keys[index], key)) {
                    m_vals[index] = val;
                    return true;
                }

                return Put(m_next_indexes[index], key, val);
            }

            t_b8 Remove(t_size& index, const tp_key_type& key) const {
                ZF_ASSERT(index >= -1 && index < Len());

                if (index == -1) {
                    return false;
                }

                if (m_key_cmp_func(m_keys[index], key)) {
                    m_usage.UnsetBit(index);
                    index = m_next_indexes[index];
                    return true;
                }

                return Remove(m_next_indexes[index], key);
            }

        private:
            t_size Len() const {
                return m_keys.Len();
            }

            c_array<tp_key_type> m_keys;
            c_array<tp_value_type> m_vals;
            c_array<t_size> m_next_indexes;
            c_bit_vector m_usage;

            t_key_cmp_func<tp_key_type> m_key_cmp_func = nullptr;
        };

        c_backing_store m_backing_store;
        c_array<t_size> m_backing_store_indexes;

        t_hash_func<tp_key_type> m_hash_func = nullptr;

        t_size KeyToHashIndex(const tp_key_type& key) const {
            const t_size val = m_hash_func(key);
            ZF_ASSERT(val >= 0);

            return val % m_backing_store_indexes.Len();
        }
    };
}
