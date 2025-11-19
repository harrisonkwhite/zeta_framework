#pragma once

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
    struct s_hash_map_backing_store {
        s_array<tp_key_type> keys;
        s_array<tp_value_type> vals;
        s_array<t_size> next_indexes;
        s_bit_vector usage;

        t_key_cmp_func<tp_key_type> key_cmp_func;
    };

    template<typename tp_key_type, typename tp_value_type>
    struct s_hash_map {
        s_hash_map_backing_store<tp_key_type, tp_value_type> backing_store;
        s_array<t_size> backing_store_indexes;

        t_hash_func<tp_key_type> hash_func;
    };

    template<typename tp_key_type, typename tp_value_type>
    t_size HashMapBackingStoreLen(const s_hash_map_backing_store<tp_key_type, tp_value_type>& bs) {
        return bs.keys.Len();
    }

    template<typename tp_key_type, typename tp_value_type>
    t_b8 HashMapBackingStoreGet(s_hash_map_backing_store<tp_key_type, tp_value_type>& bs, const t_size index, const tp_key_type& key, tp_value_type* const o_val = nullptr) {
        ZF_ASSERT(index >= -1 && index < HashMapBackingStoreLen(bs));

        if (index == -1) {
            return false;
        }

        if (m_key_cmp_func(bs.keys[index], key)) {
            if (o_val) {
                *o_val = bs.vals[index];
            }

            return true;
        }

        return Get(bs.next_indexes[index], key, o_val);
    }

    template<typename tp_key_type, typename tp_value_type>
    [[nodiscard]] t_b8 HashMapBackingStorePut(s_hash_map_backing_store<tp_key_type, tp_value_type>& bs, t_size& index, const tp_key_type& key, const tp_value_type& val) {
        ZF_ASSERT(index >= -1 && index < HashMapBackingStoreLen(bs));

        if (index == -1) {
            const t_size prospective_index = IndexFirstUnsetBit(bs.usage);

            if (prospective_index == -1) {
                // We're out of room!
                return false;
            }

            index = prospective_index;

            bs.keys[index] = key;
            bs.vals[index] = val;
            bs.next_indexes[index] = -1;
            SetBit(bs.usage, index);

            return true;
        }

        if (bs.key_cmp_func(bs.keys[index], key)) {
            bs.vals[index] = val;
            return true;
        }

        return HashMapBackingStorePut(bs.next_indexes[index], key, val);
    }

    template<typename tp_key_type, typename tp_value_type>
    t_b8 HashMapBackingStoreRemove(s_hash_map_backing_store<tp_key_type, tp_value_type>& bs, t_size& index, const tp_key_type& key) {
        ZF_ASSERT(index >= -1 && index < HashMapBackingStoreLen(bs));

        if (index == -1) {
            return false;
        }

        if (bs.key_cmp_func(bs.keys[index], key)) {
            UnsetBit(bs.usage, index);
            index = bs.next_indexes[index];
            return true;
        }

        return HashMapBackingStoreRemove(bs.next_indexes[index], key);
    }

    template<typename tp_type>
    t_size KeyToHashIndex(const tp_type& key, const t_hash_func<tp_type> hash_func, const t_size table_size) {
        ZF_ASSERT(hash_func);
        ZF_ASSERT(table_size > 0);

        const t_size val = hash_func(key);
        ZF_ASSERT(val >= 0);

        return val % table_size;
    }

    // If you don't care about getting a reference to the value and are solely looking for whether it exists, pass nullptr in for val.
    template<typename tp_key_type, typename tp_value_type>
    t_b8 HashMapGet(s_hash_map<tp_key_type, tp_value_type>& hm, const tp_key_type& key, tp_value_type* const val = nullptr) {
        const t_size hash_index = KeyToHashIndex(key, hm.hash_func, hm.backing_store_indexes.Len());
        return hm.backing_store.Get(hm.backing_store_indexes[hash_index], key, val);
    }

    template<typename tp_key_type, typename tp_value_type>
    [[nodiscard]] t_b8 HashMapPut(s_hash_map<tp_key_type, tp_value_type>& hm, const tp_key_type& key, const tp_value_type& val) {
        const t_size hash_index = KeyToHashIndex(key);
        return hm.backing_store.Put(hm.backing_store_indexes[hash_index], key, val);
    }

    template<typename tp_key_type, typename tp_value_type>
    t_b8 HashMapRemove(s_hash_map<tp_key_type, tp_value_type>& hm, const tp_key_type& key) {
        const t_size hash_index = KeyToHashIndex(key);
        return hm.backing_store.Remove(hm.backing_store_indexes[hash_index], key);
    }

    template<typename tp_key_type, typename tp_value_type>
    [[nodiscard]] t_b8 MakeHashMapBackingStore(s_mem_arena& mem_arena, const t_size cap, const t_key_cmp_func<tp_key_type> key_cmp_func, s_hash_map_backing_store<tp_key_type, tp_value_type>& o_umbs) {
        ZF_ASSERT(cap > 0);
        ZF_ASSERT(key_cmp_func);

        s_array<tp_key_type> keys;
        s_array<tp_value_type> vals;
        s_array<t_size> next_indexes;
        s_bit_vector usage;

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
    [[nodiscard]] t_b8 MakeHashMap(s_mem_arena& mem_arena, const t_hash_func<tp_key_type> hash_func, s_hash_map<tp_key_type, tp_value_type>& o_um, const t_key_cmp_func<tp_key_type> key_cmp_func = DefaultComparator, const t_size immediate_cap = 1024, const t_size kv_pair_cap = 1 << 16) {
        ZF_ASSERT(hash_func);
        ZF_ASSERT(key_cmp_func);
        ZF_ASSERT(immediate_cap > 0 && kv_pair_cap >= immediate_cap);

        s_hash_map_backing_store<tp_key_type, tp_value_type> backing_store;

        if (!MakeHashMapBackingStore(mem_arena, kv_pair_cap, key_cmp_func, backing_store)) {
            return false;
        }

        s_array<t_size> backing_store_indexes;

        if (!MakeArray(mem_arena, immediate_cap, backing_store_indexes)) {
            return false;
        }

        SetAllTo(backing_store_indexes, static_cast<t_size>(-1));

        o_um = {backing_store, backing_store_indexes, hash_func};

        return true;
    }
}
