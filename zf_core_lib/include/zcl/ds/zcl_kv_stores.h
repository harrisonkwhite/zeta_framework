#pragma once

#include <zcl/zcl_mem.h>

namespace zcl::ds {
    template <typename tp_type> concept c_kv_store_key = c_simple<tp_type> && c_same<tp_type, t_cvref_removed<tp_type>>;
    template <typename tp_type> concept c_kv_store_value = c_simple<tp_type> && c_same<tp_type, t_cvref_removed<tp_type>>;

    template <c_kv_store_key tp_key_type, c_kv_store_value tp_value_type>
    struct t_kv_store_block {
        using t_key = tp_key_type;
        using t_value = tp_value_type;

        t_array_mut<tp_key_type> keys;
        t_array_mut<tp_value_type> values;
        t_array_mut<t_i32> next_indexes;
        mem::t_bitset_mut usage;

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
    t_kv_store_block<tp_key_type, tp_value_type> *kv_store_create_block(const t_i32 cap, t_arena *const arena) {
        const auto block = arena_push_item<t_kv_store_block<tp_key_type, tp_value_type>>(arena);

        block->keys = arena_push_array<tp_key_type>(arena, cap);

        block->values = arena_push_array<tp_value_type>(arena, cap);

        block->next_indexes = arena_push_array<t_i32>(arena, cap);
        array_set_all_to(block->next_indexes, -1);

        block->usage = mem::bitset_create(cap, arena);

        return block;
    }

    // @todo: Optimise by having this move to a relative block, instead of always from the start.
    template <c_kv_store tp_kv_store_type>
    t_kv_store_block<typename tp_kv_store_type::t_key, typename tp_kv_store_type::t_value> *kv_store_find_block_of_index(const tp_kv_store_type *const kv_store, t_i32 index) {
        ZF_ASSERT(index >= -1 && index < kv_store->block_cap * kv_store->block_cnt);

        auto result = kv_store->blocks_head;

        while (index >= kv_store->block_cap) {
            result = result->next;
            index -= kv_store->block_cap;
        }

        return result;
    }

    template <c_kv_store tp_kv_store_type>
    [[nodiscard]] t_b8 kv_store_find_in_chain(const tp_kv_store_type *const kv_store, const t_i32 chain_begin_index, const typename tp_kv_store_type::t_key &key, typename tp_kv_store_type::t_value **const o_val) {
        ZF_ASSERT(chain_begin_index >= -1 && chain_begin_index < kv_store->block_cap * kv_store->block_cnt);

        t_i32 index = chain_begin_index;

        while (index != -1) {
            const auto block = kv_store_find_block_of_index(kv_store, index);

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
    t_kv_store_put_result kv_store_put_in_chain(tp_kv_store_type *const kv_store, t_i32 *const chain_begin_index, const typename tp_kv_store_type::t_key &key, const typename tp_kv_store_type::t_value &value) {
        t_i32 *index = chain_begin_index;

        while (*index != -1) {
            const auto block = kv_store_find_block_of_index(kv_store, *index);

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
                const auto possible_rel_index_to_use = mem::find_first_unset_bit(block->usage);

                if (possible_rel_index_to_use == -1) {
                    block_previous = block;
                    block = block->next;
                    block_index++;
                    continue;
                }

                block->keys[possible_rel_index_to_use] = key;
                block->values[possible_rel_index_to_use] = value;
                mem::set(block->usage, possible_rel_index_to_use);
                ZF_ASSERT(block->next_indexes[possible_rel_index_to_use] == -1);

                return (block_index * kv_store->block_cap) + possible_rel_index_to_use;
            }

            // All blocks are full - create a new one.
            const auto new_block = kv_store_create_block<typename tp_kv_store_type::t_key, typename tp_kv_store_type::t_value>(kv_store->block_cap, kv_store->blocks_arena);
            kv_store->block_cnt++;

            if (block_previous) {
                block_previous->next = new_block;
            } else {
                kv_store->blocks_head = new_block;
            }

            new_block->keys[0] = key;
            new_block->values[0] = value;
            mem::set(new_block->usage, 0);

            return block_index * kv_store->block_cap;
        }();

        return ek_kv_store_put_result_added;
    }

    template <c_kv_store tp_kv_store_type>
    t_b8 kv_store_remove_in_chain(tp_kv_store_type *const kv_store, t_i32 *const chain_begin_index, const typename tp_kv_store_type::t_key &key) {
        ZF_ASSERT(*chain_begin_index >= -1 && *chain_begin_index < kv_store->block_cap * kv_store->block_cnt);

        t_i32 *index = chain_begin_index;

        while (*index != -1) {
            const auto block = kv_store_find_block_of_index(kv_store, *index);

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
    t_i32 kv_store_load_chain(tp_kv_store_type *const kv_store, const t_i32 begin_index, const t_array_mut<typename tp_kv_store_type::t_key> keys, const t_array_mut<typename tp_kv_store_type::t_value> values) {
        ZF_ASSERT(begin_index >= -1 && begin_index < kv_store->block_cap * kv_store->block_cnt);

        t_i32 loaded_cnt = 0;

        t_i32 index = begin_index;

        while (index != -1) {
            const auto block = kv_store_find_block_of_index(kv_store, index);

            const t_i32 rel_index = index % kv_store->block_cap;

            keys[loaded_cnt] = block->keys[rel_index];
            values[loaded_cnt] = block->values[rel_index];

            loaded_cnt++;

            index = block->next_indexes[rel_index];
        }

        return loaded_cnt;
    }
}
