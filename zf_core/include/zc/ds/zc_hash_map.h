#pragma once

#include <zc/zc_strs.h>
#include <zc/ds/zc_bit_vector.h>

namespace zf {
    template<typename tp_type>
    using t_hash_func = t_size (*)(const tp_type& key);

    inline const t_hash_func<t_s32> g_s32_hash_func = [](const t_s32& key) {
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

    template<typename tp_key_type, typename tp_value_type>
    struct s_hash_map {
        t_hash_func<tp_key_type> hash_func;
        t_comparator<tp_key_type> key_comparator;

        s_array<t_size> backing_store_indexes; // These are what the hash function initially maps to after modulo. They are indexes into "slots" (linked-list nodes) in the backing store below.

        // This is where all the key-value pairs are actually stored, it's basically a single buffer containing a bunch of linked lists.
        struct {
            s_array<tp_key_type> keys;
            s_array<tp_value_type> vals;
            s_array<t_size> next_indexes;
            s_bit_vector usage;
        } backing_store;
    };

    template<typename tp_type>
    t_size KeyToHashIndex(const tp_type& key, const t_hash_func<tp_type> hash_func, const t_size table_size) {
        ZF_ASSERT(hash_func);
        ZF_ASSERT(table_size > 0);

        const t_size val = hash_func(key);
        ZF_ASSERT(val >= 0);

        return val % table_size;
    }

    // Returns true iff the key was found.
    // If you don't care about getting a reference to the value and are solely looking for whether it exists, pass nullptr in for o_val.
    template<typename tp_key_type, typename tp_value_type>
    [[nodiscard]] t_b8 HashMapGet(s_hash_map<tp_key_type, tp_value_type>& hm, const tp_key_type& key, tp_value_type* const o_val = nullptr) {
        const t_size hash_index = KeyToHashIndex(key, hm.hash_func, hm.backing_store_indexes.len);

        const auto bs_get = [&hm, &key, o_val](const auto self, const t_size index) {
            if (index == -1) {
                return false;
            }

            if (hm.key_comparator(hm.backing_store.keys[index], key) == 0) {
                if (o_val) {
                    *o_val = hm.backing_store.vals[index];
                }

                return true;
            }

            return self(self, hm.backing_store.next_indexes[index]);
        };

        return bs_get(bs_get, hm.backing_store_indexes[hash_index]);
    }

    // Returns true iff the operation was successful (in which failure occurs when there isn't enough room).
    template<typename tp_key_type, typename tp_value_type>
    [[nodiscard]] t_b8 HashMapPut(s_hash_map<tp_key_type, tp_value_type>& hm, const tp_key_type& key, const tp_value_type& val) {
        const t_size hash_index = KeyToHashIndex(key, hm.hash_func, hm.backing_store_indexes.len);

        const auto bs_put = [&hm, &key, &val](const auto self, t_size& index) {
            if (index == -1) {
                const t_size prospective_index = IndexOfFirstUnsetBit(hm.backing_store.usage);

                if (prospective_index == -1) {
                    // We're out of room!
                    return false;
                }

                index = prospective_index;

                hm.backing_store.keys[index] = key;
                hm.backing_store.vals[index] = val;
                hm.backing_store.next_indexes[index] = -1;
                SetBit(hm.backing_store.usage, index);

                return true;
            }

            if (hm.key_comparator(hm.backing_store.keys[index], key) == 0) {
                hm.backing_store.vals[index] = val;
                return true;
            }

            return self(self, hm.backing_store.next_indexes[index]);
        };

        return bs_put(bs_put, hm.backing_store_indexes[hash_index]);
    }

    // Returns true iff an entry with the given key was found and removed.
    template<typename tp_key_type, typename tp_value_type>
    t_b8 HashMapRemove(s_hash_map<tp_key_type, tp_value_type>& hm, const tp_key_type& key) {
        const t_size hash_index = KeyToHashIndex(key, hm.hash_func, hm.backing_store_indexes.len);

        const auto bs_remove = [&hm, &key](const auto self, t_size& index) {
            if (index == -1) {
                return false;
            }

            if (hm.key_comparator(hm.backing_store.keys[index], key) == 0) {
                UnsetBit(hm.backing_store.usage, index);
                index = hm.backing_store.next_indexes[index];
                return true;
            }

            return self(self, hm.backing_store.next_indexes[index]);
        };

        return bs_remove(bs_remove, hm.backing_store_indexes[hash_index]);
    }

    // The provided hash function has to map a key to an integer 0 or higher.
    // The immediate capacity is the total number of upfront slots (i.e. the maximum possible number of slots for which an O(1) access of a value from a key can happen).
    // The key-value pair capacity is the overall limit of how many key-value pairs this map can ever hold. It obviously has to be equal to or greater than the immediate capacity.
    template<typename tp_key_type, typename tp_value_type>
    [[nodiscard]] t_b8 MakeHashMap(s_mem_arena& mem_arena, const t_hash_func<tp_key_type> hash_func, s_hash_map<tp_key_type, tp_value_type>& o_um, const t_comparator<tp_key_type> key_comparator = DefaultComparator, const t_size immediate_cap = 1024, const t_size kv_pair_cap = 1 << 16) {
        ZF_ASSERT(hash_func);
        ZF_ASSERT(key_comparator);
        ZF_ASSERT(immediate_cap > 0 && kv_pair_cap >= immediate_cap);

        o_um = {};

        o_um.hash_func = hash_func;
        o_um.key_comparator = key_comparator;

        if (!MakeArray(mem_arena, immediate_cap, o_um.backing_store_indexes)) {
            return false;
        }

        SetAllTo(o_um.backing_store_indexes, -1);

        if (![&o_um, &mem_arena, kv_pair_cap]() {
            if (!MakeArray(mem_arena, kv_pair_cap, o_um.backing_store.keys)) {
                return false;
            }

            if (!MakeArray(mem_arena, kv_pair_cap, o_um.backing_store.vals)) {
                return false;
            }

            if (!MakeArray(mem_arena, kv_pair_cap, o_um.backing_store.next_indexes)) {
                return false;
            }

            if (!MakeBitVector(mem_arena, kv_pair_cap, o_um.backing_store.usage)) {
                return false;
            }
        }()) {
            return false;
        }

        return true;
    }
}
