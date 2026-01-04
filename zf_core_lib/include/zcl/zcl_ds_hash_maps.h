#pragma once

#include <zcl/zcl_io.h>

namespace zf {
    // ============================================================
    // @section: Types and Globals

    template <co_simple tp_key_type, co_simple tp_val_type>
    struct s_kv_store_block {
        s_array_mut<tp_key_type> keys;
        s_array_mut<tp_val_type> vals;
        s_array_mut<t_i32> next_indexes;
        s_bit_vec_mut usage;

        s_kv_store_block *next;
    };

    template <co_simple tp_key_type, co_simple tp_val_type>
    struct s_kv_store {
        t_comparator_bin<tp_key_type> key_comparator;

        s_kv_store_block<tp_key_type, tp_val_type> *blocks_head;
        s_arena *blocks_arena;
        t_i32 block_cnt;
        t_i32 block_cap;

        t_i32 pair_cnt;
    };

    enum e_kv_store_put_result : t_i32 {
        ek_kv_store_put_result_updated,
        ek_kv_store_put_result_added
    };

    template <co_simple tp_type>
    using t_hash_func = t_i32 (*)(const tp_type &key);

    // This is an FNV-1a implementation.
    inline const t_hash_func<strs::StrRdonly> g_str_hash_func =
        [](const strs::StrRdonly &key) {
            const t_u32 offs_basis = 2166136261u;
            const t_u32 prime = 16777619u;

            t_u32 hash = offs_basis;

            for (t_i32 i = 0; i < key.bytes.len; i++) {
                hash ^= static_cast<t_u8>(key.bytes[i]);
                hash *= prime;
            }

            return static_cast<t_i32>(hash & 0x7FFFFFFFull);
        };

    template <co_simple tp_key_type, co_simple tp_val_type>
    struct s_hash_map {
        t_hash_func<tp_key_type> hash_func;

        s_array_mut<t_i32> immediate_indexes;
        s_kv_store<tp_key_type, tp_val_type> kv_store;
    };

    enum e_hash_map_put_result : t_i32 {
        ek_hash_map_put_result_added,
        ek_hash_map_put_result_updated
    };

    constexpr t_i32 g_hash_map_cap_default = 32;

    // ============================================================


    // ============================================================
    // @section: Functions

    template <co_simple tp_key_type, co_simple tp_val_type>
    s_kv_store_block<tp_key_type, tp_val_type> *CreateKVStoreBlock(const t_i32 cap, s_arena *const arena) {
        const auto block = ArenaPushItem<s_kv_store_block<tp_key_type, tp_val_type>>(arena);

        block->keys = ArenaPushArray<tp_key_type>(arena, cap);

        block->vals = ArenaPushArray<tp_val_type>(arena, cap);

        block->next_indexes = ArenaPushArray<t_i32>(arena, cap);
        SetAllTo(block->next_indexes, -1);

        block->usage = CreateBitVector(cap, arena);

        block->next = nullptr;

        return block;
    }

    // @todo: Optimise by having this move to a relative block, instead of always from the start.
    template <co_simple tp_key_type, co_simple tp_val_type>
    s_kv_store_block<tp_key_type, tp_val_type> *FindBlockOfIndex(const s_kv_store<tp_key_type, tp_val_type> *const kv_store, t_i32 index) {
        ZF_ASSERT(index >= -1 && index < kv_store->block_cap * kv_store->block_cnt);

        auto result = kv_store->blocks_head;

        while (index >= kv_store->block_cap) {
            result = result->next;
            index -= kv_store->block_cap;
        }

        return result;
    }

    template <co_simple tp_key_type, co_simple tp_val_type>
    [[nodiscard]] B8 FindInChain(const s_kv_store<tp_key_type, tp_val_type> *const kv_store, const t_i32 chain_begin_index, const tp_key_type &key, tp_val_type **const o_val) {
        ZF_ASSERT(chain_begin_index >= -1 && chain_begin_index < kv_store->block_cap * kv_store->block_cnt);

        t_i32 index = chain_begin_index;

        while (index != -1) {
            const auto block = FindBlockOfIndex(kv_store, index);

            const t_i32 rel_index = index % kv_store->block_cap;

            if (kv_store->key_comparator(block->keys[rel_index], key)) {
                *o_val = &block->vals[rel_index];
                return true;
            }

            index = block->next_indexes[rel_index];
        }

        return false;
    }

    template <co_simple tp_key_type, co_simple tp_val_type>
    e_kv_store_put_result PutInChain(s_kv_store<tp_key_type, tp_val_type> *const kv_store, t_i32 *const chain_begin_index, const tp_key_type &key, const tp_val_type &val) {
        t_i32 *index = chain_begin_index;

        while (*index != -1) {
            const auto block = FindBlockOfIndex(kv_store, *index);

            const t_i32 rel_index = *index % kv_store->block_cap;

            if (kv_store->key_comparator(block->keys[rel_index], key)) {
                block->vals[rel_index] = val;
                return ek_kv_store_put_result_updated;
            }

            index = &block->next_indexes[rel_index];
        }

        // Insert and get the absolute index of the new pair.
        *index = [kv_store, &key, &val]() {
            s_kv_store_block<tp_key_type, tp_val_type> *block = kv_store->blocks_head;
            s_kv_store_block<tp_key_type, tp_val_type> *block_previous = nullptr;
            t_i32 block_index = 0;

            kv_store->pair_cnt++;

            while (block) {
                const auto possible_rel_index_to_use = FindIndexOfFirstUnsetBit(block->usage);

                if (possible_rel_index_to_use == -1) {
                    block_previous = block;
                    block = block->next;
                    block_index++;
                    continue;
                }

                block->keys[possible_rel_index_to_use] = key;
                block->vals[possible_rel_index_to_use] = val;
                SetBit(block->usage, possible_rel_index_to_use);
                ZF_ASSERT(block->next_indexes[possible_rel_index_to_use] == -1);

                return (block_index * kv_store->block_cap) + possible_rel_index_to_use;
            }

            // All blocks are full - create a new one.
            const auto new_block = CreateKVStoreBlock<tp_key_type, tp_val_type>(kv_store->block_cap, kv_store->blocks_arena);
            kv_store->block_cnt++;

            if (block_previous) {
                block_previous->next = new_block;
            } else {
                kv_store->blocks_head = new_block;
            }

            new_block->keys[0] = key;
            new_block->vals[0] = val;
            SetBit(new_block->usage, 0);

            return block_index * kv_store->block_cap;
        }();

        return ek_kv_store_put_result_added;
    }

    template <co_simple tp_key_type, co_simple tp_val_type>
    B8 RemoveInChain(s_kv_store<tp_key_type, tp_val_type> *const kv_store, t_i32 *const chain_begin_index, const tp_key_type &key) {
        ZF_ASSERT(*chain_begin_index >= -1 && *chain_begin_index < kv_store->block_cap * kv_store->block_cnt);

        t_i32 *index = chain_begin_index;

        while (*index != -1) {
            const auto block = FindBlockOfIndex(kv_store, *index);

            const t_i32 rel_index = *index % kv_store->block_cap;

            if (kv_store->key_comparator(block->keys[rel_index], key)) {
                *index = block->next_indexes[rel_index];
                kv_store->pair_cnt--;
                return true;
            }

            index = &block->next_indexes[rel_index];
        }

        return false;
    }

    // Loads keys and values of the chain into the given PRE-ALLOCATED arrays.
    template <co_simple tp_key_type, co_simple tp_val_type>
    t_i32 LoadChain(const s_kv_store<tp_key_type, tp_val_type> *const kv_store, const t_i32 begin_index, const s_array_mut<tp_key_type> keys, const s_array_mut<tp_val_type> vals) {
        ZF_ASSERT(begin_index >= -1 && begin_index < kv_store->block_cap * kv_store->block_cnt);

        t_i32 loaded_cnt = 0;

        t_i32 index = begin_index;

        while (index != -1) {
            const auto block = FindBlockOfIndex(kv_store, index);

            const t_i32 rel_index = index % kv_store->block_cap;

            keys[loaded_cnt] = block->keys[rel_index];
            vals[loaded_cnt] = block->vals[rel_index];

            loaded_cnt++;

            index = block->next_indexes[rel_index];
        }

        return loaded_cnt;
    }

    template <co_simple tp_type>
    t_i32 KeyToHashIndex(const tp_type &key, const t_hash_func<tp_type> hash_func, const t_i32 cap) {
        ZF_ASSERT(cap > 0);

        const t_i32 val = hash_func(key);
        ZF_ASSERT(val >= 0);

        return val % cap;
    }

    // The provided hash function has to map a key to an integer 0 or higher. The given memory arena will be saved and used for allocating new memory for entries when needed.
    template <co_simple tp_key_type, co_simple tp_val_type>
    s_hash_map<tp_key_type, tp_val_type> HashMapCreate(const t_hash_func<tp_key_type> hash_func, s_arena *const arena, const t_i32 cap = g_hash_map_cap_default, const t_comparator_bin<tp_key_type> key_comparator = g_comparator_bin_default<tp_key_type>) {
        const auto immediate_indexes = ArenaPushArray<t_i32>(arena, cap);
        SetAllTo(immediate_indexes, -1);

        return {
            .hash_func = hash_func,
            .immediate_indexes = immediate_indexes,
            .kv_store = {.key_comparator = key_comparator, .blocks_arena = arena, .block_cap = static_cast<t_i32>(AlignForward(cap, 8))},
        };
    }

    template <co_simple tp_key_type, co_simple tp_val_type>
    t_i32 HashMapCap(const s_hash_map<tp_key_type, tp_val_type> *const hash_map) {
        return hash_map->immediate_indexes.len;
    }

    template <co_simple tp_key_type, co_simple tp_val_type>
    t_i32 HashMapEntryCount(const s_hash_map<tp_key_type, tp_val_type> *const hash_map) {
        return hash_map->kv_store.pair_cnt;
    }

    template <co_simple tp_key_type, co_simple tp_val_type>
    [[nodiscard]] B8 HashMapPut(const s_hash_map<tp_key_type, tp_val_type> *const hash_map, const tp_key_type &key, tp_val_type **const o_val) {
        const t_i32 hash_index = KeyToHashIndex(key, hash_map->hash_func, HashMapCap(hash_map));
        return FindInChain(&hash_map->kv_store, hash_map->immediate_indexes[hash_index], key, o_val);
    }

    // Try adding the key-value pair to the hash map or just updating the value if the key is already present.
    template <co_simple tp_key_type, co_simple tp_val_type>
    e_hash_map_put_result HashMapPut(s_hash_map<tp_key_type, tp_val_type> *const hash_map, const tp_key_type &key, const tp_val_type &val) {
        const t_i32 hash_index = KeyToHashIndex(key, hash_map->hash_func, HashMapCap(hash_map));
        return PutInChain(&hash_map->kv_store, &hash_map->immediate_indexes[hash_index], key, val) == ek_kv_store_put_result_updated ? ek_hash_map_put_result_updated : ek_hash_map_put_result_added;
    }

    // Returns true iff an entry with the key was found and removed.
    template <co_simple tp_key_type, co_simple tp_val_type>
    B8 HashMapRemove(s_hash_map<tp_key_type, tp_val_type> *const hash_map, const tp_key_type &key) {
        const t_i32 hash_index = KeyToHashIndex(key, hash_map->hash_func, HashMapCap(hash_map));
        return RemoveInChain(&hash_map->kv_store, hash_map->immediate_indexes[hash_index], key);
    }

    // Loads all key-value pairs into the given PRE-ALLOCATED arrays.
    template <co_simple tp_key_type, co_simple tp_val_type>
    void HashMapLoadEntries(const s_hash_map<tp_key_type, tp_val_type> *const hash_map, const s_array_mut<tp_key_type> keys, const s_array_mut<tp_val_type> vals) {
        ZF_ASSERT(keys.len >= HashMapEntryCount(hash_map) && vals.len >= HashMapEntryCount(hash_map));

        t_i32 loaded_cnt = 0;

        for (t_i32 i = 0; i < hash_map->immediate_indexes.len; i++) {
            loaded_cnt += LoadChain(&hash_map->kv_store, hash_map->immediate_indexes[i], ArraySliceFrom(keys, loaded_cnt), ArraySliceFrom(vals, loaded_cnt));
        }
    }

    // Allocates the given arrays with the memory arena and loads key-value pairs into them.
    template <co_simple tp_key_type, co_simple tp_val_type>
    void HashMapLoadEntries(const s_hash_map<tp_key_type, tp_val_type> *const hash_map, s_arena *const arena, s_array_mut<tp_key_type> *const o_keys, s_array_mut<tp_val_type> *const o_vals) {
        *o_keys = ArenaPushArray<tp_key_type>(arena, HashMapEntryCount(hash_map));
        *o_vals = ArenaPushArray<tp_val_type>(arena, HashMapEntryCount(hash_map));
        return HashMapLoadEntries(hash_map, *o_keys, *o_vals);
    }

    template <co_simple tp_key_type, co_simple tp_val_type>
    [[nodiscard]] B8 SerializeHashMap(s_stream *const stream, const s_hash_map<tp_key_type, tp_val_type> *const hm, s_arena *const temp_arena) {
        if (!WriteItem(stream, HashMapCap(hm))) {
            return false;
        }

        if (!WriteItem(stream, HashMapEntryCount(hm))) {
            return false;
        }

        s_array_mut<tp_key_type> keys;
        s_array_mut<tp_val_type> vals;
        HashMapLoadEntries(hm, temp_arena, &keys, &vals);

        if (!WriteItemsOfArray(stream, keys)) {
            return false;
        }

        if (!WriteItemsOfArray(stream, vals)) {
            return false;
        }

        return true;
    }

    template <co_simple tp_key_type, co_simple tp_val_type>
    [[nodiscard]] B8 DeserializeHashMap(s_stream *const stream, s_arena *const hm_arena, const t_hash_func<tp_key_type> hm_hash_func, s_arena *const temp_arena, s_hash_map<tp_key_type, tp_val_type> *const o_hm, const t_comparator_bin<tp_key_type> hm_key_comparator = g_comparator_bin_default<tp_key_type>) {
        t_i32 cap;

        if (!ReadItem(stream, &cap)) {
            return false;
        }

        t_i32 entry_cnt;

        if (!ReadItem(stream, &entry_cnt)) {
            return false;
        }

        *o_hm = HashMapCreate<tp_key_type, tp_val_type>(hm_hash_func, hm_arena, cap, hm_key_comparator);

        const auto keys = ArenaPushArray<tp_key_type>(temp_arena, entry_cnt);

        if (!ReadItemsIntoArray(stream, keys, entry_cnt)) {
            return false;
        }

        const auto vals = ArenaPushArray<tp_val_type>(temp_arena, entry_cnt);

        if (!ReadItemsIntoArray(stream, vals, entry_cnt)) {
            return false;
        }

        for (t_i32 i = 0; i < entry_cnt; i++) {
            HashMapPut(o_hm, keys[i], vals[i]);
        }

        return true;
    }

    // ============================================================
}
