#pragma once

#include <zc/zc_allocators.h>
#include <zc/zc_strs.h>
#include <zc/ds/zc_bit_vector.h>

namespace zf {
    template<typename tp_type>
    using t_hash_func = t_size (*)(const tp_type& key);

    inline const t_hash_func<t_s32> g_int_hash_func = [](const t_s32& key) {
        return static_cast<t_size>(key & 0x7FFFFFFF); // Mask out the sign bit.
    };

    inline const t_hash_func<s_str_rdonly> g_str_hash_func = [](const s_str_rdonly& key) {
        ZF_ASSERT(IsStrTerminated(key));

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

    // This is where all the key-value pairs are actually stored, it's basically a single buffer containing a bunch of linked lists.
    template<typename tp_key_type, typename tp_value_type>
    struct s_unordered_map_backing_store {
    public:
        s_unordered_map_backing_store() = default;

        s_unordered_map_backing_store(const c_array<tp_key_type> keys, const c_array<tp_value_type> vals, const c_array<t_size> next_indexes, const c_bit_vector usage, const t_key_cmp_func<tp_key_type> key_cmp_func)
            : m_keys(keys), m_vals(vals), m_next_indexes(next_indexes), m_usage(usage), m_key_cmp_func(m_key_cmp_func) {
            ZF_ASSERT(m_vals.Len() == m_keys.Len() && m_next_indexes.Len() == m_keys.Len() && m_usage.BitCount() == m_keys.Len());
            ZF_ASSERT(AreAllEqualTo(m_next_indexes, -1));
            ZF_ASSERT(AreAllBitsUnset(m_usage));
            ZF_ASSERT(m_key_cmp_func);
        }

        t_size Len() const {
            return m_keys.Len();
        }

        // If you don't care about getting a reference to the value and are solely looking for whether it exists, pass nullptr in for val.
        t_b8 Get(const t_size index, const tp_key_type& key, tp_value_type* const o_val = nullptr) const {
            ZF_ASSERT(index >= -1 && index < Len());

            if (index == -1) {
                return false;
            }

            if (m_key_cmp_func(m_keys[index], key)) {
                if (o_val) {
                    *o_val = m_vals[index];
                }

                return true;
            }

            return Get(m_next_indexes[index], key, o_val);
        }

        [[nodiscard]] t_b8 Put(t_size& index, const tp_key_type& key, const tp_value_type& val) const {
            ZF_ASSERT(index >= -1 && index < Len());

            if (index == -1) {
                const t_size prospective_index = FindFirstUnsetBit(m_usage);

                if (prospective_index == -1) {
                    // We're out of room!
                    return false;
                }

                index = prospective_index;

                m_keys[index] = key;
                m_vals[index] = val;
                m_next_indexes[index] = -1;
                SetBit(m_usage, index);

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
                UnsetBit(m_usage, index);
                index = m_next_indexes[index];
                return true;
            }

            return Remove(m_next_indexes[index], key);
        }

    private:
        c_array<tp_key_type> m_keys;
        c_array<tp_value_type> m_vals;
        c_array<t_size> m_next_indexes;
        c_bit_vector m_usage;

        t_key_cmp_func<tp_key_type> m_key_cmp_func;
    };

    template<typename tp_key_type, typename tp_value_type>
    struct s_unordered_map {
    public:
        s_unordered_map() = default;

        s_unordered_map(const s_unordered_map_backing_store<tp_key_type, tp_value_type> backing_store, const c_array<t_size> backing_store_indexes, const t_hash_func<tp_key_type> hash_func)
            : m_backing_store(backing_store), m_backing_store_indexes(backing_store_indexes), m_hash_func(hash_func) {
            ZF_ASSERT(backing_store.Len() >= backing_store_indexes);
            ZF_ASSERT(AreAllEqualTo(backing_store_indexes, -1));
            ZF_ASSERT(hash_func);
        }

        // If you don't care about getting a reference to the value and are solely looking for whether it exists, pass nullptr in for val.
        t_b8 Get(const tp_key_type& key, tp_value_type* const val = nullptr) const {
            const t_size hash_index = KeyToHashIndex(key);
            return m_backing_store.Get(m_backing_store_indexes[hash_index], key, val);
        }

        [[nodiscard]] t_b8 Put(const tp_key_type& key, const tp_value_type& val) const {
            const t_size hash_index = KeyToHashIndex(key);
            return m_backing_store.Put(m_backing_store_indexes[hash_index], key, val);
        }

        t_b8 Remove(const tp_key_type& key) const {
            const t_size hash_index = KeyToHashIndex(key);
            return m_backing_store.Remove(m_backing_store_indexes[hash_index], key);
        }

    private:
        s_unordered_map_backing_store<tp_key_type, tp_value_type> m_backing_store;
        c_array<t_size> m_backing_store_indexes;

        t_hash_func<tp_key_type> m_hash_func;

        t_size KeyToHashIndex(const tp_key_type& key) const {
            const t_size val = m_hash_func(key);
            ZF_ASSERT(val >= 0);

            return val % m_backing_store_indexes.Len();
        }
    };

    template<typename tp_key_type, typename tp_value_type>
    [[nodiscard]] t_b8 MakeUnorderedMapBackingStore(c_mem_arena& mem_arena, const t_size cap, const t_key_cmp_func<tp_key_type> key_cmp_func, s_unordered_map_backing_store<tp_key_type, tp_value_type>& o_umbs) {
        ZF_ASSERT(cap > 0);
        ZF_ASSERT(key_cmp_func);

        c_array<tp_key_type> keys;
        c_array<tp_value_type> vals;
        c_array<t_size> next_indexes;
        c_bit_vector usage;

        if (!MakeArray(mem_arena, cap, keys)) {
            return false;
        }

        if (!MakeArray(mem_arena, cap, vals)) {
            return false;
        }

        if (!MakeArray(mem_arena, cap, next_indexes)) {
            return false;
        }

        if (!MakeBitVector(mem_arena, cap, usage)) {
            return false;
        }

        o_umbs = {keys, vals, next_indexes, usage, key_cmp_func};

        return true;
    }

    // The provided hash function has to map a key to an integer 0 or higher.
    // The immediate capacity is the total number of upfront slots (i.e. the maximum possible number of slots for which an O(1) access of a value from a key can happen).
    // The key-value pair capacity is the overall limit of how many key-value pairs this map can ever hold. It obviously has to be equal to or greater than the immediate capacity.
    template<typename tp_key_type, typename tp_value_type>
    [[nodiscard]] t_b8 MakeUnorderedMap(c_mem_arena& mem_arena, const t_hash_func<tp_key_type> hash_func, s_unordered_map<tp_key_type, tp_value_type>& o_um, const t_key_cmp_func<tp_key_type> key_cmp_func = DefaultComparator, const t_size immediate_cap = 1024, const t_size kv_pair_cap = 1 << 16) {
        ZF_ASSERT(hash_func);
        ZF_ASSERT(key_cmp_func);
        ZF_ASSERT(immediate_cap > 0 && kv_pair_cap >= immediate_cap);

        s_unordered_map_backing_store<tp_key_type, tp_value_type> backing_store;

        if (!MakeUnorderedMapBackingStore(mem_arena, kv_pair_cap, key_cmp_func, backing_store)) {
            return false;
        }

        c_array<t_size> backing_store_indexes;

        if (!MakeArray(mem_arena, immediate_cap, backing_store_indexes)) {
            return false;
        }

        SetAllTo(backing_store_indexes, -1);

        o_um = {backing_store, backing_store_indexes, hash_func};

        return true;
    }
}
