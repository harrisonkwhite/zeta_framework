#pragma once

#include <zcl/zcl_io.h>

namespace zf::ds {
    // ============================================================
    // @section: Lists

    template <typename tp_type>
    concept c_list_elem = c_simple<tp_type> && !c_const<tp_type>;

    template <c_list_elem tp_elem_type>
    struct t_list_mut {
        using t_elem = tp_elem_type;

        t_array_mut<tp_elem_type> backing_arr;
        t_i32 len;

        tp_elem_type &operator[](const t_i32 index) const {
            ZF_ASSERT(index >= 0 && index < len);
            return backing_arr[index];
        }
    };

    template <c_list_elem tp_elem_type>
    struct t_list_rdonly {
        using t_elem = tp_elem_type;

        t_array_rdonly<tp_elem_type> backing_arr;
        t_i32 len;

        const tp_elem_type &operator[](const t_i32 index) const {
            ZF_ASSERT(index >= 0 && index < len);
            return backing_arr[index];
        }
    };

    using t_list_extension_cap_calculator = t_i32 (*)(const t_i32 cap_current);

    inline const t_list_extension_cap_calculator g_list_extension_cap_calculator_default =
        [](const t_i32 cap_current) {
            ZF_ASSERT(cap_current >= 0);
            return cap_current == 0 ? 1 : cap_current * 2;
        };

    template <typename tp_type>
    concept c_list_mut = requires { typename tp_type::t_elem; } && c_same<t_cvref_removed<tp_type>, t_list_mut<typename tp_type::t_elem>>;

    template <typename tp_type>
    concept c_list_rdonly = requires { typename tp_type::t_elem; } && c_same<t_cvref_removed<tp_type>, t_list_rdonly<typename tp_type::t_elem>>;

    template <typename tp_type>
    concept c_list = c_list_mut<tp_type> || c_list_rdonly<tp_type>;

    template <c_list_elem tp_elem_type>
    t_list_mut<tp_elem_type> list_create(const t_i32 cap, mem::t_arena *const arena, const t_i32 len = 0) {
        ZF_ASSERT(cap > 0 && len >= 0 && len <= cap);
        return {mem::arena_push_array<tp_elem_type>(arena, cap), len};
    }

    template <c_list tp_list_type>
    t_i32 list_get_cap(const tp_list_type *const list) {
        return list->backing_arr.len;
    }

    template <c_list_elem tp_elem_type>
    t_array_mut<tp_elem_type> list_get_as_array(const t_list_mut<tp_elem_type> *const list) {
        return array_slice(list->backing_arr, 0, list->len);
    }

    template <c_list_elem tp_elem_type>
    t_array_rdonly<tp_elem_type> list_get_as_array(const t_list_rdonly<tp_elem_type> *const list) {
        return array_slice(list->backing_arr, 0, list->len);
    }

    template <c_list_mut tp_list_type>
    void list_extend(tp_list_type *const list, mem::t_arena *const arena, const t_list_extension_cap_calculator cap_calculator = g_list_extension_cap_calculator_default) {
        ZF_ASSERT(cap_calculator);

        const t_i32 new_cap = cap_calculator(list_get_cap(list));
        ZF_ASSERT(new_cap > list_get_cap(list));

        const auto new_backing_arr = mem::arena_push_array<tp_list_type>(arena, new_cap);
        array_copy(list->backing_arr, new_backing_arr);

        *list = {new_backing_arr, list->len};
    }

    template <c_list_mut tp_list_type>
    void list_extend_to_fit(tp_list_type *const list, const t_i32 min_cap, mem::t_arena *const arena, const t_list_extension_cap_calculator cap_calculator = g_list_extension_cap_calculator_default) {
        ZF_ASSERT(min_cap > list_get_cap(list));
        ZF_ASSERT(cap_calculator);

        const t_i32 new_cap = [cap = list_get_cap(list), min_cap, cap_calculator]() {
            t_i32 result = cap;

            do {
                const auto res_last = result;
                result = cap_calculator(result);
                ZF_ASSERT(result > res_last);
            } while (result < min_cap);

            return result;
        }();

        const auto new_backing_arr = mem::arena_push_array<typename tp_list_type::t_elem>(arena, new_cap);
        array_copy(list->backing_arr, new_backing_arr);

        *list = {new_backing_arr, list->len};
    }

    template <c_list_mut tp_list_type>
    typename tp_list_type::t_elem *list_append(tp_list_type *const list, const typename tp_list_type::t_elem &val) {
        ZF_ASSERT(list->len < list_get_cap(list));

        list->len++;
        (*list)[list->len - 1] = val;
        return &(*list)[list->len - 1];
    }

    template <c_list_mut tp_list_type>
    typename tp_list_type::t_elem *list_append_dynamic(tp_list_type *const list, const typename tp_list_type::t_elem &val, mem::t_arena *const extension_arena, const t_list_extension_cap_calculator extension_cap_calculator = g_list_extension_cap_calculator_default) {
        if (list->len == list_get_cap(list)) {
            list_extend(list, extension_arena, extension_cap_calculator);
        }

        return list_append(list, val);
    }

    template <c_list_mut tp_list_type>
    t_array_mut<typename tp_list_type::t_elem> list_append_many(tp_list_type *const list, const t_array_rdonly<typename tp_list_type::t_elem> vals) {
        ZF_ASSERT(list->len + vals.len <= list_get_cap(list));

        array_copy(vals, array_slice_from(list->backing_arr, list->len));
        list->len += vals.len;
        return array_slice(list->backing_arr, list->len - vals.len, list->len);
    }

    template <c_list_mut tp_list_type>
    t_array_mut<typename tp_list_type::t_elem> list_append_many_dynamic(tp_list_type *const list, const t_array_rdonly<typename tp_list_type::t_elem> vals, mem::t_arena *const extension_arena, const t_list_extension_cap_calculator extension_cap_calculator = g_list_extension_cap_calculator_default) {
        const auto min_cap_needed = list->len + vals.len;

        if (min_cap_needed > list_get_cap(list)) {
            list_extend_to_fit(list, min_cap_needed, extension_arena, extension_cap_calculator);
        }

        return list_append_many(list, vals);
    }

    template <c_list_mut tp_list_type>
    typename tp_list_type::t_elem *list_insert_at(tp_list_type *const list, const t_i32 index, const typename tp_list_type::t_elem &val) {
        ZF_ASSERT(list->len < list_get_cap(list));
        ZF_ASSERT(index >= 0 && index <= list->len);

        list->len++;

        for (t_i32 i = list->len - 1; i > index; i--) {
            (*list)[i] = (*list)[i - 1];
        }

        (*list)[index] = val;

        return &(*list)[index];
    }

    template <c_list_mut tp_list_type>
    typename tp_list_type::t_elem *list_insert_at_dynamic(tp_list_type *const list, const t_i32 index, const typename tp_list_type::t_elem &val, mem::t_arena *const extension_arena, const t_list_extension_cap_calculator extension_cap_calculator = g_list_extension_cap_calculator_default) {
        if (list->len == list_get_cap(list)) {
            list_extend(list, extension_arena, extension_cap_calculator);
        }

        return list_insert_at(list, index, val);
    }

    template <c_list_mut tp_list_type>
    void list_remove_at_shift(tp_list_type *const list, const t_i32 index) {
        ZF_ASSERT(list->len > 0);
        ZF_ASSERT(index >= 0 && index < list->len);

        array_copy(array_slice(list->backing_arr, index + 1, list->len), array_slice(list->backing_arr, index, list->len - 1));
        list->len--;
    }

    template <c_list_mut tp_list_type>
    void list_remove_at_swapback(tp_list_type *const list, const t_i32 index) {
        ZF_ASSERT(list->len > 0);
        ZF_ASSERT(index >= 0 && index < list->len);

        (*list)[index] = (*list)[list->len - 1];
        list->len--;
    }

    template <c_list_mut tp_list_type>
    void list_remove_end(tp_list_type *const list) {
        ZF_ASSERT(list->len > 0);
        list->len--;
    }

    // @todo: Serialisation function!

    // ============================================================


    // ============================================================
    // @section: Key-Value Pair Stores

    template <c_simple tp_key_type, c_simple tp_val_type>
    struct t_kv_store_block {
        t_array_mut<tp_key_type> keys;
        t_array_mut<tp_val_type> vals;
        t_array_mut<t_i32> next_indexes;
        mem::t_bitset_mut usage;

        t_kv_store_block *next;
    };

    template <c_simple tp_key_type, c_simple tp_val_type>
    struct t_kv_store {
        t_comparator_bin<tp_key_type> key_comparator;

        t_kv_store_block<tp_key_type, tp_val_type> *blocks_head;
        mem::t_arena *blocks_arena;
        t_i32 block_cnt;
        t_i32 block_cap;

        t_i32 pair_cnt;
    };

    template <c_simple tp_key_type, c_simple tp_val_type>
    t_kv_store_block<tp_key_type, tp_val_type> *kv_store_create_block(const t_i32 cap, mem::t_arena *const arena) {
        const auto block = mem::arena_push_item<t_kv_store_block<tp_key_type, tp_val_type>>(arena);

        block->keys = mem::arena_push_array<tp_key_type>(arena, cap);

        block->vals = mem::arena_push_array<tp_val_type>(arena, cap);

        block->next_indexes = mem::arena_push_array<t_i32>(arena, cap);
        array_set_all_to(block->next_indexes, -1);

        block->usage = mem::bitset_create(cap, arena);

        block->next = nullptr;

        return block;
    }

    // @todo: Optimise by having this move to a relative block, instead of always from the start.
    template <c_simple tp_key_type, c_simple tp_val_type>
    t_kv_store_block<tp_key_type, tp_val_type> *kv_store_find_block_of_index(const t_kv_store<tp_key_type, tp_val_type> *const kv_store, t_i32 index) {
        ZF_ASSERT(index >= -1 && index < kv_store->block_cap * kv_store->block_cnt);

        auto result = kv_store->blocks_head;

        while (index >= kv_store->block_cap) {
            result = result->next;
            index -= kv_store->block_cap;
        }

        return result;
    }

    template <c_simple tp_key_type, c_simple tp_val_type>
    [[nodiscard]] t_b8 kv_store_find_in_chain(const t_kv_store<tp_key_type, tp_val_type> *const kv_store, const t_i32 chain_begin_index, const tp_key_type &key, tp_val_type **const o_val) {
        ZF_ASSERT(chain_begin_index >= -1 && chain_begin_index < kv_store->block_cap * kv_store->block_cnt);

        t_i32 index = chain_begin_index;

        while (index != -1) {
            const auto block = kv_store_find_block_of_index(kv_store, index);

            const t_i32 rel_index = index % kv_store->block_cap;

            if (kv_store->key_comparator(block->keys[rel_index], key)) {
                *o_val = &block->vals[rel_index];
                return true;
            }

            index = block->next_indexes[rel_index];
        }

        return false;
    }

    enum t_kv_store_put_result : t_i32 {
        ec_kv_store_put_result_updated,
        ec_kv_store_put_result_added
    };

    template <c_simple tp_key_type, c_simple tp_val_type>
    t_kv_store_put_result kv_store_put_in_chain(t_kv_store<tp_key_type, tp_val_type> *const kv_store, t_i32 *const chain_begin_index, const tp_key_type &key, const tp_val_type &val) {
        t_i32 *index = chain_begin_index;

        while (*index != -1) {
            const auto block = kv_store_find_block_of_index(kv_store, *index);

            const t_i32 rel_index = *index % kv_store->block_cap;

            if (kv_store->key_comparator(block->keys[rel_index], key)) {
                block->vals[rel_index] = val;
                return ec_kv_store_put_result_updated;
            }

            index = &block->next_indexes[rel_index];
        }

        // Insert and get the absolute index of the new pair.
        *index = [kv_store, &key, &val]() {
            t_kv_store_block<tp_key_type, tp_val_type> *block = kv_store->blocks_head;
            t_kv_store_block<tp_key_type, tp_val_type> *block_previous = nullptr;
            t_i32 block_index = 0;

            kv_store->pair_cnt++;

            while (block) {
                const auto possible_rel_index_to_use = mem::get_index_of_first_unset_bit(block->usage);

                if (possible_rel_index_to_use == -1) {
                    block_previous = block;
                    block = block->next;
                    block_index++;
                    continue;
                }

                block->keys[possible_rel_index_to_use] = key;
                block->vals[possible_rel_index_to_use] = val;
                mem::set_bit(block->usage, possible_rel_index_to_use);
                ZF_ASSERT(block->next_indexes[possible_rel_index_to_use] == -1);

                return (block_index * kv_store->block_cap) + possible_rel_index_to_use;
            }

            // All blocks are full - create a new one.
            const auto new_block = kv_store_create_block<tp_key_type, tp_val_type>(kv_store->block_cap, kv_store->blocks_arena);
            kv_store->block_cnt++;

            if (block_previous) {
                block_previous->next = new_block;
            } else {
                kv_store->blocks_head = new_block;
            }

            new_block->keys[0] = key;
            new_block->vals[0] = val;
            mem::set_bit(new_block->usage, 0);

            return block_index * kv_store->block_cap;
        }();

        return ec_kv_store_put_result_added;
    }

    template <c_simple tp_key_type, c_simple tp_val_type>
    t_b8 kv_store_remove_in_chain(t_kv_store<tp_key_type, tp_val_type> *const kv_store, t_i32 *const chain_begin_index, const tp_key_type &key) {
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
    template <c_simple tp_key_type, c_simple tp_val_type>
    t_i32 kv_store_load_chain(const t_kv_store<tp_key_type, tp_val_type> *const kv_store, const t_i32 begin_index, const t_array_mut<tp_key_type> keys, const t_array_mut<tp_val_type> vals) {
        ZF_ASSERT(begin_index >= -1 && begin_index < kv_store->block_cap * kv_store->block_cnt);

        t_i32 loaded_cnt = 0;

        t_i32 index = begin_index;

        while (index != -1) {
            const auto block = kv_store_find_block_of_index(kv_store, index);

            const t_i32 rel_index = index % kv_store->block_cap;

            keys[loaded_cnt] = block->keys[rel_index];
            vals[loaded_cnt] = block->vals[rel_index];

            loaded_cnt++;

            index = block->next_indexes[rel_index];
        }

        return loaded_cnt;
    }

    // ============================================================


    // ============================================================
    // @section: Hash Maps

    template <c_simple tp_type>
    using t_hash_func = t_i32 (*)(const tp_type &key);

    // This is an FNV-1a implementation.
    inline const t_hash_func<strs::t_str_rdonly> g_str_hash_func =
        [](const strs::t_str_rdonly &key) {
            const t_u32 offs_basis = 2166136261u;
            const t_u32 prime = 16777619u;

            t_u32 hash = offs_basis;

            for (t_i32 i = 0; i < key.bytes.len; i++) {
                hash ^= static_cast<t_u8>(key.bytes[i]);
                hash *= prime;
            }

            return static_cast<t_i32>(hash & 0x7FFFFFFFull);
        };

    template <c_simple tp_key_type, c_simple tp_val_type>
    struct t_hash_map {
        t_hash_func<tp_key_type> hash_func;

        t_array_mut<t_i32> immediate_indexes;
        t_kv_store<tp_key_type, tp_val_type> kv_store;
    };

    constexpr t_i32 g_hash_map_cap_default = 32;

    // The provided hash function has to map a key to an integer 0 or higher. The given memory arena will be saved and used for allocating new memory for entries when needed.
    template <c_simple tp_key_type, c_simple tp_val_type>
    t_hash_map<tp_key_type, tp_val_type> hash_map_create(const t_hash_func<tp_key_type> hash_func, mem::t_arena *const arena, const t_i32 cap = g_hash_map_cap_default, const t_comparator_bin<tp_key_type> key_comparator = g_comparator_bin_default<tp_key_type>) {
        const auto immediate_indexes = mem::arena_push_array<t_i32>(arena, cap);
        array_set_all_to(immediate_indexes, -1);

        return {
            .hash_func = hash_func,
            .immediate_indexes = immediate_indexes,
            .kv_store = {.key_comparator = key_comparator, .blocks_arena = arena, .block_cap = static_cast<t_i32>(mem::align_forward(cap, 8))},
        };
    }

    template <c_simple tp_key_type, c_simple tp_val_type>
    t_i32 hash_map_get_cap(const t_hash_map<tp_key_type, tp_val_type> *const hash_map) {
        return hash_map->immediate_indexes.len;
    }

    template <c_simple tp_key_type, c_simple tp_val_type>
    t_i32 hash_map_get_entry_cnt(const t_hash_map<tp_key_type, tp_val_type> *const hash_map) {
        return hash_map->kv_store.pair_cnt;
    }

    template <c_simple tp_type>
    t_i32 hash_map_convert_key_to_hash_index(const tp_type &key, const t_hash_func<tp_type> hash_func, const t_i32 cap) {
        ZF_ASSERT(cap > 0);

        const t_i32 val = hash_func(key);
        ZF_ASSERT(val >= 0);

        return val % cap;
    }

    template <c_simple tp_key_type, c_simple tp_val_type>
    [[nodiscard]] t_b8 hash_map_find(const t_hash_map<tp_key_type, tp_val_type> *const hash_map, const tp_key_type &key, tp_val_type **const o_val) {
        const t_i32 hash_index = hash_map_convert_key_to_hash_index(key, hash_map->hash_func, hash_map_get_cap(hash_map));
        return kv_store_find_in_chain(&hash_map->kv_store, hash_map->immediate_indexes[hash_index], key, o_val);
    }

    enum t_hash_map_put_result : t_i32 {
        ec_hash_map_put_result_added,
        ec_hash_map_put_result_updated
    };

    // Try adding the key-value pair to the hash map or just updating the value if the key is already present.
    template <c_simple tp_key_type, c_simple tp_val_type>
    t_hash_map_put_result hash_map_put(t_hash_map<tp_key_type, tp_val_type> *const hash_map, const tp_key_type &key, const tp_val_type &val) {
        const t_i32 hash_index = hash_map_convert_key_to_hash_index(key, hash_map->hash_func, hash_map_get_cap(hash_map));
        return kv_store_put_in_chain(&hash_map->kv_store, &hash_map->immediate_indexes[hash_index], key, val) == ec_kv_store_put_result_updated ? ec_hash_map_put_result_updated : ec_hash_map_put_result_added;
    }

    // Returns true iff an entry with the key was found and removed.
    template <c_simple tp_key_type, c_simple tp_val_type>
    t_b8 hash_map_remove(t_hash_map<tp_key_type, tp_val_type> *const hash_map, const tp_key_type &key) {
        const t_i32 hash_index = hash_map_convert_key_to_hash_index(key, hash_map->hash_func, hash_map_get_cap(hash_map));
        return kv_store_remove_in_chain(&hash_map->kv_store, hash_map->immediate_indexes[hash_index], key);
    }

    // Loads all key-value pairs into the given PRE-ALLOCATED arrays.
    template <c_simple tp_key_type, c_simple tp_val_type>
    void hash_map_load_entries(const t_hash_map<tp_key_type, tp_val_type> *const hash_map, const t_array_mut<tp_key_type> keys, const t_array_mut<tp_val_type> vals) {
        ZF_ASSERT(keys.len >= hash_map_get_entry_cnt(hash_map) && vals.len >= hash_map_get_entry_cnt(hash_map));

        t_i32 loaded_cnt = 0;

        for (t_i32 i = 0; i < hash_map->immediate_indexes.len; i++) {
            loaded_cnt += kv_store_load_chain(&hash_map->kv_store, hash_map->immediate_indexes[i], array_slice_from(keys, loaded_cnt), array_slice_from(vals, loaded_cnt));
        }
    }

    // Allocates the given arrays with the memory arena and loads key-value pairs into them.
    template <c_simple tp_key_type, c_simple tp_val_type>
    void hash_map_load_entries(const t_hash_map<tp_key_type, tp_val_type> *const hash_map, mem::t_arena *const arena, t_array_mut<tp_key_type> *const o_keys, t_array_mut<tp_val_type> *const o_vals) {
        *o_keys = mem::arena_push_array<tp_key_type>(arena, hash_map_get_entry_cnt(hash_map));
        *o_vals = mem::arena_push_array<tp_val_type>(arena, hash_map_get_entry_cnt(hash_map));
        return hash_map_load_entries(hash_map, *o_keys, *o_vals);
    }

    template <c_simple tp_key_type, c_simple tp_val_type>
    [[nodiscard]] t_b8 hash_map_serialize(const t_hash_map<tp_key_type, tp_val_type> *const hm, io::t_stream *const stream, mem::t_arena *const temp_arena) {
        if (!io::f_write_item(stream, hash_map_get_cap(hm))) {
            return false;
        }

        if (!io::f_write_item(stream, hash_map_get_entry_cnt(hm))) {
            return false;
        }

        t_array_mut<tp_key_type> keys;
        t_array_mut<tp_val_type> vals;
        hash_map_load_entries(hm, temp_arena, &keys, &vals);

        if (!io::f_write_items_of_array(stream, keys)) {
            return false;
        }

        if (!io::f_write_items_of_array(stream, vals)) {
            return false;
        }

        return true;
    }

    template <c_simple tp_key_type, c_simple tp_val_type>
    [[nodiscard]] t_b8 hash_map_deserialize(io::t_stream *const stream, mem::t_arena *const hm_arena, const t_hash_func<tp_key_type> hm_hash_func, mem::t_arena *const temp_arena, t_hash_map<tp_key_type, tp_val_type> *const o_hm, const t_comparator_bin<tp_key_type> hm_key_comparator = g_comparator_bin_default<tp_key_type>) {
        t_i32 cap;

        if (!io::f_read_item(stream, &cap)) {
            return false;
        }

        t_i32 entry_cnt;

        if (!io::f_read_item(stream, &entry_cnt)) {
            return false;
        }

        *o_hm = hash_map_create<tp_key_type, tp_val_type>(hm_hash_func, hm_arena, cap, hm_key_comparator);

        const auto keys = mem::arena_push_array<tp_key_type>(temp_arena, entry_cnt);

        if (!io::f_read_items_into_array(stream, keys, entry_cnt)) {
            return false;
        }

        const auto vals = mem::arena_push_array<tp_val_type>(temp_arena, entry_cnt);

        if (!io::f_read_items_into_array(stream, vals, entry_cnt)) {
            return false;
        }

        for (t_i32 i = 0; i < entry_cnt; i++) {
            hash_map_put(o_hm, keys[i], vals[i]);
        }

        return true;
    }

    // ============================================================
}
