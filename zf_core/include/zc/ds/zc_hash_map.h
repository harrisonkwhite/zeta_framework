#pragma once

#include <zc/zc_strs.h>
#include <zc/ds/zc_bit_vector.h>
#include <zc/ds/zc_list.h>

namespace zf {
    template<typename tp_type>
    using t_hash_func = t_size (*)(const tp_type& key);

    inline const t_hash_func<s_str_ascii_rdonly> g_str_hash_func = [](const s_str_ascii_rdonly& key) {
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

    template<typename tp_key_type, typename tp_val_type>
    struct s_hash_map {
        t_hash_func<tp_key_type> hash_func;
        t_bin_comparator<tp_key_type> key_comparator;

        t_size kv_pair_cnt;

        s_array<t_size> backing_store_indexes; // These are what the hash function initially maps to after modulo. They are indexes into "slots" (linked-list nodes) in the backing store below.

        // This is where all the key-value pairs are actually stored.
        struct {
            // Keeping all of these in distinct arrays to not waste space with padding had they been put in a struct.
            s_array<tp_key_type> keys;
            s_array<tp_val_type> vals;
            s_array<t_size> next_indexes; // Like the standard "next" pointer of a linked list node, but for an index specific to this backing store.

            // Indicates what slots or "nodes" are in use.
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
    template<typename tp_key_type, typename tp_val_type>
    [[nodiscard]] t_b8 HashMapGet(const s_hash_map<tp_key_type, tp_val_type>& hm, const tp_key_type& key, tp_val_type* const o_val = nullptr) {
        const t_size hash_index = KeyToHashIndex(key, hm.hash_func, hm.backing_store_indexes.len);

        const auto bs_get = [&hm, &key, o_val](const auto self, const t_size index) {
            if (index == -1) {
                return false;
            }

            if (hm.key_comparator(hm.backing_store.keys[index], key)) {
                if (o_val) {
                    *o_val = hm.backing_store.vals[index];
                }

                return true;
            }

            return self(self, hm.backing_store.next_indexes[index]);
        };

        return bs_get(bs_get, hm.backing_store_indexes[hash_index]);
    }

    enum e_hash_map_put_result : t_s32 {
        ek_hash_map_put_result_added,
        ek_hash_map_put_result_updated,
        ek_hash_map_put_result_error
    };

    template<typename tp_key_type, typename tp_val_type>
    [[nodiscard]] e_hash_map_put_result HashMapPut(s_hash_map<tp_key_type, tp_val_type>& hm, const tp_key_type& key, const tp_val_type& val) {
        const t_size hash_index = KeyToHashIndex(key, hm.hash_func, hm.backing_store_indexes.len);

        const auto bs_put = [&hm, &key, &val](const auto self, t_size& index) {
            if (index == -1) {
                const t_size prospective_index = IndexOfFirstUnsetBit(hm.backing_store.usage);

                if (prospective_index == -1) {
                    // We're out of room!
                    return ek_hash_map_put_result_error;
                }

                index = prospective_index;

                hm.backing_store.keys[index] = key;
                hm.backing_store.vals[index] = val;
                hm.backing_store.next_indexes[index] = -1;
                SetBit(hm.backing_store.usage, index);

                hm.kv_pair_cnt++;

                return ek_hash_map_put_result_added;
            }

            if (hm.key_comparator(hm.backing_store.keys[index], key)) {
                hm.backing_store.vals[index] = val;
                return ek_hash_map_put_result_updated;
            }

            return self(self, hm.backing_store.next_indexes[index]);
        };

        return bs_put(bs_put, hm.backing_store_indexes[hash_index]);
    }

    // Returns true iff an entry with the given key was found and removed.
    template<typename tp_key_type, typename tp_val_type>
    t_b8 HashMapRemove(const s_hash_map<tp_key_type, tp_val_type>& hm, const tp_key_type& key) {
        const t_size hash_index = KeyToHashIndex(key, hm.hash_func, hm.backing_store_indexes.len);

        const auto bs_remove = [&hm, &key](const auto self, t_size& index) {
            if (index == -1) {
                return false;
            }

            if (hm.key_comparator(hm.backing_store.keys[index], key)) {
                UnsetBit(hm.backing_store.usage, index);
                index = hm.backing_store.next_indexes[index];
                hm.kv_pair_cnt--;
                return true;
            }

            return self(self, hm.backing_store.next_indexes[index]);
        };

        return bs_remove(bs_remove, hm.backing_store_indexes[hash_index]);
    }

    // Returns true iff the destination array had enough room to fit all keys.
    template<typename tp_key_type, typename tp_val_type, c_array tp_arr_type>
    [[nodiscard]] t_b8 LoadHashMapKeys(const s_hash_map<tp_key_type, tp_val_type>& hm, tp_arr_type& dest_arr) {
        static_assert(s_is_same<tp_key_type, typename tp_arr_type::t_elem>::g_val);

        s_list<tp_key_type> dest_list = {ToNonstatic(dest_arr)};

        const auto add_from_bs = [&hm, &dest_list](const auto self, const t_size index) {
            if (index == -1) {
                return true;
            }
            
            if (IsListFull(dest_list)) {
                return false;
            }

            ListAppend(dest_list, hm.backing_store.keys[index]);
            self(self, hm.backing_store.next_indexes[index]);
        };

        for (t_size i = 0; i < hm.backing_store_indexes.len; i++) {
            if (!add_from_bs(add_from_bs, hm.backing_store_indexes[i])) {
                return false;
            }
        }

        return true;
    }

    template<typename tp_key_type, typename tp_val_type>
    [[nodiscard]] t_b8 LoadHashMapKeys(s_mem_arena& mem_arena, const s_hash_map<tp_key_type, tp_val_type>& hm, s_array<tp_key_type>& o_keys) {
        if (!MakeArray(mem_arena, hm.kv_pair_cnt, o_keys)) {
            return false;
        }

        const t_b8 success = LoadHashMapKeys(hm, o_keys);
        ZF_ASSERT(success);

        return true;
    }

    // The provided hash function has to map a key to an integer 0 or higher.
    // The immediate capacity is the total number of upfront slots (i.e. the maximum possible number of slots for which an O(1) access of a value from a key can happen).
    // The key-value pair capacity is the overall limit of how many key-value pairs this map can ever hold. It obviously has to be equal to or greater than the immediate capacity.
    template<typename tp_key_type, typename tp_val_type>
    [[nodiscard]] t_b8 MakeHashMap(s_mem_arena& mem_arena, const t_hash_func<tp_key_type> hash_func, s_hash_map<tp_key_type, tp_val_type>& o_um, const t_bin_comparator<tp_key_type> key_comparator = DefaultBinComparator, const t_size immediate_cap = 1024, const t_size kv_pair_cap = 1 << 16) {
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

        return true;
    }

    // This DOES NOT serialize the hash function pointer and binary comparator function pointer!
    template<typename tp_key_type, typename tp_val_type>
    [[nodiscard]] t_b8 SerializeHashMap(s_stream& stream, const s_hash_map<tp_key_type, tp_val_type>& hm) {
        if (!StreamWriteItem(stream, hm.kv_pair_cnt)) {
            return false;
        }

        if (!SerializeArray(stream, hm.backing_store_indexes)) {
            return false;
        }

        if (!SerializeArray(stream, hm.backing_store.keys)) {
            return false;
        }

        if (!SerializeArray(stream, hm.backing_store.vals)) {
            return false;
        }

        if (!SerializeArray(stream, hm.backing_store.next_indexes)) {
            return false;
        }

        if (!SerializeBitVector(stream, hm.backing_store.usage)) {
            return false;
        }

        return true;
    }

    template<typename tp_key_type, typename tp_val_type>
    [[nodiscard]] t_b8 DeserializeHashMap(s_mem_arena& mem_arena, s_stream& stream, const t_hash_func<tp_key_type> hash_func, const t_bin_comparator<tp_key_type> key_comparator, s_hash_map<tp_key_type, tp_val_type>& o_hm) {
        o_hm = {
            .hash_func = hash_func,
            .key_comparator = key_comparator
        };

        if (!StreamReadItem(stream, o_hm.kv_pair_cnt)) {
            return false;
        }

        if (!DeserializeArray(stream, mem_arena, o_hm.backing_store_indexes)) {
            return false;
        }

        if (!DeserializeArray(stream, mem_arena, o_hm.backing_store.keys)) {
            return false;
        }

        if (!DeserializeArray(stream, mem_arena, o_hm.backing_store.vals)) {
            return false;
        }

        if (!DeserializeArray(stream, mem_arena, o_hm.backing_store.next_indexes)) {
            return false;
        }

        if (!DeserializeBitVector(stream, mem_arena, o_hm.backing_store.usage)) {
            return false;
        }

        return true;
    }
}
