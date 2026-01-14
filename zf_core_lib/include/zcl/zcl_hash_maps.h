#pragma once

#include <zcl/zcl_basic.h>
#include <zcl/zcl_bits.h>

namespace zcl {
    // ============================================================
    // @section: Key-Value Stores

    template <typename tp_type> concept c_kv_store_key = c_simple<tp_type> && c_same<tp_type, t_cvref_removed<tp_type>>;
    template <typename tp_type> concept c_kv_store_value = c_simple<tp_type> && c_same<tp_type, t_cvref_removed<tp_type>>;

    template <c_kv_store_key tp_key_type, c_kv_store_value tp_value_type>
    struct t_kv_store_block {
        using t_key = tp_key_type;
        using t_value = tp_value_type;

        t_array_mut<tp_key_type> keys;
        t_array_mut<tp_value_type> values;
        t_array_mut<t_i32> next_indexes;
        t_bitset_mut usage;

        t_kv_store_block *next;
    };

    template <typename tp_type>
    concept c_kv_store_block = c_same<t_cvref_removed<tp_type>, t_kv_store_block<typename tp_type::t_key, typename tp_type::t_value>>;

    template <c_kv_store_key tp_key_type, c_kv_store_value tp_value_type>
    struct t_kv_store {
        using t_key = tp_key_type;
        using t_value = tp_value_type;

        t_comparator_bin<tp_key_type> key_comparator;

        t_kv_store_block<tp_key_type, tp_value_type> *blocks_head;
        t_arena *blocks_arena;
        t_i32 block_cnt;
        t_i32 block_cap;

        t_i32 pair_cnt;
    };

    template <typename tp_type>
    concept c_kv_store = c_same<t_cvref_removed<tp_type>, t_kv_store<typename tp_type::t_key, typename tp_type::t_value>>;

    enum t_kv_store_put_result : t_i32 {
        ek_kv_store_put_result_updated,
        ek_kv_store_put_result_added
    };

    template <c_kv_store_key tp_key_type, c_kv_store_value tp_value_type>
    t_kv_store_block<tp_key_type, tp_value_type> *KVStoreCreateBlock(const t_i32 cap, t_arena *const arena) {
        const auto block = ArenaPushItem<t_kv_store_block<tp_key_type, tp_value_type>>(arena);

        block->keys = ArenaPushArray<tp_key_type>(arena, cap);

        block->values = ArenaPushArray<tp_value_type>(arena, cap);

        block->next_indexes = ArenaPushArray<t_i32>(arena, cap);
        SetAllTo(block->next_indexes, -1);

        block->usage = BitsetCreate(cap, arena);

        return block;
    }

    // @speed: Optimise by having this move to a relative block, instead of always from the start.
    template <c_kv_store tp_kv_store_type>
    t_kv_store_block<typename tp_kv_store_type::t_key, typename tp_kv_store_type::t_value> *KVStoreFindBlockOfIndex(const tp_kv_store_type *const kv_store, t_i32 index) {
        ZCL_ASSERT(index >= -1 && index < kv_store->block_cap * kv_store->block_cnt);

        auto result = kv_store->blocks_head;

        while (index >= kv_store->block_cap) {
            result = result->next;
            index -= kv_store->block_cap;
        }

        return result;
    }

    template <c_kv_store tp_kv_store_type>
    [[nodiscard]] t_b8 KVStoreFindInChain(const tp_kv_store_type *const kv_store, const t_i32 chain_begin_index, const typename tp_kv_store_type::t_key &key, typename tp_kv_store_type::t_value **const o_val) {
        ZCL_ASSERT(chain_begin_index >= -1 && chain_begin_index < kv_store->block_cap * kv_store->block_cnt);

        t_i32 index = chain_begin_index;

        while (index != -1) {
            const auto block = KVStoreFindBlockOfIndex(kv_store, index);

            const t_i32 rel_index = index % kv_store->block_cap;

            if (kv_store->key_comparator(block->keys[rel_index], key)) {
                *o_val = &block->values[rel_index];
                return true;
            }

            index = block->next_indexes[rel_index];
        }

        return false;
    }

    template <c_kv_store tp_kv_store_type>
    t_kv_store_put_result KVStorePutInChain(tp_kv_store_type *const kv_store, t_i32 *const chain_begin_index, const typename tp_kv_store_type::t_key &key, const typename tp_kv_store_type::t_value &value) {
        t_i32 *index = chain_begin_index;

        while (*index != -1) {
            const auto block = KVStoreFindBlockOfIndex(kv_store, *index);

            const t_i32 rel_index = *index % kv_store->block_cap;

            if (kv_store->key_comparator(block->keys[rel_index], key)) {
                block->values[rel_index] = value;
                return ek_kv_store_put_result_updated;
            }

            index = &block->next_indexes[rel_index];
        }

        // Insert and get the absolute index of the new pair.
        *index = [kv_store, &key, &value]() {
            t_kv_store_block<typename tp_kv_store_type::t_key, typename tp_kv_store_type::t_value> *block = kv_store->blocks_head;
            t_kv_store_block<typename tp_kv_store_type::t_key, typename tp_kv_store_type::t_value> *block_previous = nullptr;
            t_i32 block_index = 0;

            kv_store->pair_cnt++;

            while (block) {
                const auto possible_rel_index_to_use = BitsetFindFirstUnsetBit(block->usage);

                if (possible_rel_index_to_use == -1) {
                    block_previous = block;
                    block = block->next;
                    block_index++;
                    continue;
                }

                block->keys[possible_rel_index_to_use] = key;
                block->values[possible_rel_index_to_use] = value;
                BitsetSet(block->usage, possible_rel_index_to_use);
                ZCL_ASSERT(block->next_indexes[possible_rel_index_to_use] == -1);

                return (block_index * kv_store->block_cap) + possible_rel_index_to_use;
            }

            // All blocks are full - create a new one.
            const auto new_block = KVStoreCreateBlock<typename tp_kv_store_type::t_key, typename tp_kv_store_type::t_value>(kv_store->block_cap, kv_store->blocks_arena);
            kv_store->block_cnt++;

            if (block_previous) {
                block_previous->next = new_block;
            } else {
                kv_store->blocks_head = new_block;
            }

            new_block->keys[0] = key;
            new_block->values[0] = value;
            BitsetSet(new_block->usage, 0);

            return block_index * kv_store->block_cap;
        }();

        return ek_kv_store_put_result_added;
    }

    template <c_kv_store tp_kv_store_type>
    t_b8 KVStoreRemoveInChain(tp_kv_store_type *const kv_store, t_i32 *const chain_begin_index, const typename tp_kv_store_type::t_key &key) {
        ZCL_ASSERT(*chain_begin_index >= -1 && *chain_begin_index < kv_store->block_cap * kv_store->block_cnt);

        t_i32 *index = chain_begin_index;

        while (*index != -1) {
            const auto block = KVStoreFindBlockOfIndex(kv_store, *index);

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
    template <c_kv_store tp_kv_store_type>
    t_i32 KVStoreLoadChain(tp_kv_store_type *const kv_store, const t_i32 begin_index, const t_array_mut<typename tp_kv_store_type::t_key> keys, const t_array_mut<typename tp_kv_store_type::t_value> values) {
        ZCL_ASSERT(begin_index >= -1 && begin_index < kv_store->block_cap * kv_store->block_cnt);

        t_i32 loaded_cnt = 0;

        t_i32 index = begin_index;

        while (index != -1) {
            const auto block = KVStoreFindBlockOfIndex(kv_store, index);

            const t_i32 rel_index = index % kv_store->block_cap;

            keys[loaded_cnt] = block->keys[rel_index];
            values[loaded_cnt] = block->values[rel_index];

            loaded_cnt++;

            index = block->next_indexes[rel_index];
        }

        return loaded_cnt;
    }

    // ============================================================


    // ============================================================
    // @section: Hash Maps

    template <typename tp_type> concept c_hash_map_key = c_simple<tp_type> && c_same<tp_type, t_cvref_removed<tp_type>>;
    template <typename tp_type> concept c_hash_map_value = c_simple<tp_type> && c_same<tp_type, t_cvref_removed<tp_type>>;

    template <c_hash_map_key tp_type>
    using t_hash_func = t_i32 (*)(const tp_type &key);

    template <c_hash_map_key tp_key_type, c_hash_map_value tp_value_type>
    struct t_hash_map {
        using t_key = tp_key_type;
        using t_value = tp_value_type;

        t_hash_func<tp_key_type> hash_func;

        t_array_mut<t_i32> immediate_indexes;
        t_kv_store<tp_key_type, tp_value_type> kv_store;
    };

    constexpr t_i32 k_hash_map_cap_default = 32;

    template <typename tp_type>
    concept c_hash_map = c_same<t_cvref_removed<tp_type>, t_hash_map<typename tp_type::t_key, typename tp_type::t_value>>;

    // The provided hash function has to map a key to an integer 0 or higher. The given memory arena will be saved and used for allocating new memory for entries when needed.
    template <c_hash_map_key tp_key_type, c_hash_map_value tp_value_type>
    t_hash_map<tp_key_type, tp_value_type> HashMapCreate(const t_hash_func<tp_key_type> hash_func, t_arena *const arena, const t_i32 cap = k_hash_map_cap_default, const t_comparator_bin<tp_key_type> key_comparator = k_comparator_bin_default<tp_key_type>) {
        const auto immediate_indexes = ArenaPushArray<t_i32>(arena, cap);
        SetAllTo(immediate_indexes, -1);

        return {
            .hash_func = hash_func,
            .immediate_indexes = immediate_indexes,
            .kv_store = {.key_comparator = key_comparator, .blocks_arena = arena, .block_cap = static_cast<t_i32>(AlignForward(cap, 8))},
        };
    }

    template <c_hash_map tp_hash_map_type>
    t_i32 HashMapGetCap(const tp_hash_map_type *const hash_map) {
        return hash_map->immediate_indexes.len;
    }

    template <c_hash_map tp_hash_map_type>
    t_i32 HashMapGetEntryCount(const tp_hash_map_type *const hash_map) {
        return hash_map->kv_store.pair_cnt;
    }

    template <c_hash_map_key tp_key_type>
    t_i32 HashMapKeyToHashIndex(const tp_key_type &key, const t_hash_func<tp_key_type> hash_func, const t_i32 cap) {
        ZCL_ASSERT(cap > 0);

        const t_i32 value = hash_func(key);
        ZCL_ASSERT(value >= 0);

        return value % cap;
    }

    template <c_hash_map tp_hash_map_type>
    [[nodiscard]] t_b8 HashMapFind(const tp_hash_map_type *const hash_map, const typename tp_hash_map_type::t_key &key, typename tp_hash_map_type::t_value **const o_val) {
        const t_i32 hash_index = HashMapKeyToHashIndex(key, hash_map->hash_func, HashMapGetCap(hash_map));
        return KVStoreFindInChain(&hash_map->kv_store, hash_map->immediate_indexes[hash_index], key, o_val);
    }

    enum t_hash_map_put_result : t_i32 {
        ek_hash_map_put_result_added,
        ek_hash_map_put_result_updated
    };

    // Try adding the key-value pair to the hash map or just updating the value if the key is already present.
    template <c_hash_map tp_hash_map_type>
    t_hash_map_put_result HashMapPut(tp_hash_map_type *const hash_map, const typename tp_hash_map_type::t_key &key, const typename tp_hash_map_type::t_value &value) {
        const t_i32 hash_index = HashMapKeyToHashIndex(key, hash_map->hash_func, HashMapGetCap(hash_map));
        return KVStorePutInChain(&hash_map->kv_store, &hash_map->immediate_indexes[hash_index], key, value) == ek_kv_store_put_result_updated ? ek_hash_map_put_result_updated : ek_hash_map_put_result_added;
    }

    // Returns true iff an entry with the key was found and removed.
    template <c_hash_map tp_hash_map_type>
    t_b8 HashMapRemove(tp_hash_map_type *const hash_map, const typename tp_hash_map_type::t_key &key) {
        const t_i32 hash_index = HashMapKeyToHashIndex(key, hash_map->hash_func, HashMapGetCap(hash_map));
        return KVStoreRemoveInChain(&hash_map->kv_store, hash_map->immediate_indexes[hash_index], key);
    }

    // Loads all key-value pairs into the given PRE-ALLOCATED arrays.
    template <c_hash_map tp_hash_map_type>
    void HashMapLoadEntries(const tp_hash_map_type *const hash_map, const t_array_mut<typename tp_hash_map_type::t_key> keys, const t_array_mut<typename tp_hash_map_type::t_value> values) {
        ZCL_ASSERT(keys.len >= HashMapGetEntryCount(hash_map) && values.len >= HashMapGetEntryCount(hash_map));

        t_i32 loaded_cnt = 0;

        for (t_i32 i = 0; i < hash_map->immediate_indexes.len; i++) {
            loaded_cnt += KVStoreLoadChain(&hash_map->kv_store, hash_map->immediate_indexes[i], ArraySliceFrom(keys, loaded_cnt), ArraySliceFrom(values, loaded_cnt));
        }
    }

    // Allocates the given arrays with the arena and loads key-value pairs into them.
    template <c_hash_map tp_hash_map_type>
    void HashMapLoadEntries(const tp_hash_map_type *const hash_map, t_arena *const arena, t_array_mut<typename tp_hash_map_type::t_key> *const o_keys, t_array_mut<typename tp_hash_map_type::t_value> *const o_values) {
        *o_keys = ArenaPushArray<typename tp_hash_map_type::t_key>(arena, HashMapGetEntryCount(hash_map));
        *o_values = ArenaPushArray<typename tp_hash_map_type::t_value>(arena, HashMapGetEntryCount(hash_map));
        return HashMapLoadEntries(hash_map, *o_keys, *o_values);
    }

    // ============================================================
}
