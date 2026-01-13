#pragma once

#include <zcl/ds/zcl_kv_stores.h>

namespace zcl::ds {
    template <typename tp_type> concept c_hash_map_key = c_simple<tp_type> && c_same<tp_type, t_cvref_removed<tp_type>>;
    template <typename tp_type> concept c_hash_map_value = c_simple<tp_type> && c_same<tp_type, t_cvref_removed<tp_type>>;

    template <c_hash_map_key tp_type>
    using t_hash_func = t_i32 (*)(const tp_type &key);

#if 0
    // This is an FNV-1a implementation.
    constexpr t_hash_func<t_str_rdonly> k_str_hash_func =
        [](const t_str_rdonly &key) {
            const t_u32 offs_basis = 2166136261u;
            const t_u32 prime = 16777619u;

            t_u32 hash = offs_basis;

            for (t_i32 i = 0; i < key.bytes.len; i++) {
                hash ^= static_cast<t_u8>(key.bytes[i]);
                hash *= prime;
            }

            return static_cast<t_i32>(hash & 0x7FFFFFFFull);
        };
#endif

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
    t_hash_map<tp_key_type, tp_value_type> hash_map_create(const t_hash_func<tp_key_type> hash_func, t_arena *const arena, const t_i32 cap = k_hash_map_cap_default, const t_comparator_bin<tp_key_type> key_comparator = k_comparator_bin_default<tp_key_type>) {
        const auto immediate_indexes = arena_push_array<t_i32>(arena, cap);
        array_set_all_to(immediate_indexes, -1);

        return {
            .hash_func = hash_func,
            .immediate_indexes = immediate_indexes,
            .kv_store = {.key_comparator = key_comparator, .blocks_arena = arena, .block_cap = static_cast<t_i32>(align_forward(cap, 8))},
        };
    }

    template <c_hash_map tp_hash_map_type>
    t_i32 hash_map_get_cap(const tp_hash_map_type *const hash_map) {
        return hash_map->immediate_indexes.len;
    }

    template <c_hash_map tp_hash_map_type>
    t_i32 hash_map_get_entry_count(const tp_hash_map_type *const hash_map) {
        return hash_map->kv_store.pair_cnt;
    }

    template <c_hash_map_key tp_key_type>
    t_i32 hash_map_key_to_hash_index(const tp_key_type &key, const t_hash_func<tp_key_type> hash_func, const t_i32 cap) {
        ZF_ASSERT(cap > 0);

        const t_i32 value = hash_func(key);
        ZF_ASSERT(value >= 0);

        return value % cap;
    }

    template <c_hash_map tp_hash_map_type>
    [[nodiscard]] t_b8 hash_map_find(const tp_hash_map_type *const hash_map, const typename tp_hash_map_type::t_key &key, typename tp_hash_map_type::t_value **const o_val) {
        const t_i32 hash_index = hash_map_key_to_hash_index(key, hash_map->hash_func, hash_map_get_cap(hash_map));
        return kv_store_find_in_chain(&hash_map->kv_store, hash_map->immediate_indexes[hash_index], key, o_val);
    }

    enum t_hash_map_put_result : t_i32 {
        ek_hash_map_put_result_added,
        ek_hash_map_put_result_updated
    };

    // Try adding the key-value pair to the hash map or just updating the value if the key is already present.
    template <c_hash_map tp_hash_map_type>
    t_hash_map_put_result hash_map_put(tp_hash_map_type *const hash_map, const typename tp_hash_map_type::t_key &key, const typename tp_hash_map_type::t_value &value) {
        const t_i32 hash_index = hash_map_key_to_hash_index(key, hash_map->hash_func, hash_map_get_cap(hash_map));
        return kv_store_put_in_chain(&hash_map->kv_store, &hash_map->immediate_indexes[hash_index], key, value) == ek_kv_store_put_result_updated ? ek_hash_map_put_result_updated : ek_hash_map_put_result_added;
    }

    // Returns true iff an entry with the key was found and removed.
    template <c_hash_map tp_hash_map_type>
    t_b8 hash_map_remove(tp_hash_map_type *const hash_map, const typename tp_hash_map_type::t_key &key) {
        const t_i32 hash_index = hash_map_key_to_hash_index(key, hash_map->hash_func, hash_map_get_cap(hash_map));
        return kv_store_remove_in_chain(&hash_map->kv_store, hash_map->immediate_indexes[hash_index], key);
    }

    // Loads all key-value pairs into the given PRE-ALLOCATED arrays.
    template <c_hash_map tp_hash_map_type>
    void hash_map_load_entries(const tp_hash_map_type *const hash_map, const t_array_mut<typename tp_hash_map_type::t_key> keys, const t_array_mut<typename tp_hash_map_type::t_value> values) {
        ZF_ASSERT(keys.len >= hash_map_get_entry_count(hash_map) && values.len >= hash_map_get_entry_count(hash_map));

        t_i32 loaded_cnt = 0;

        for (t_i32 i = 0; i < hash_map->immediate_indexes.len; i++) {
            loaded_cnt += kv_store_load_chain(&hash_map->kv_store, hash_map->immediate_indexes[i], array_slice_from(keys, loaded_cnt), array_slice_from(values, loaded_cnt));
        }
    }

    // Allocates the given arrays with the arena and loads key-value pairs into them.
    template <c_hash_map tp_hash_map_type>
    void hash_map_load_entries(const tp_hash_map_type *const hash_map, t_arena *const arena, t_array_mut<typename tp_hash_map_type::t_key> *const o_keys, t_array_mut<typename tp_hash_map_type::t_value> *const o_values) {
        *o_keys = arena_push_array<typename tp_hash_map_type::t_key>(arena, hash_map_get_entry_count(hash_map));
        *o_values = arena_push_array<typename tp_hash_map_type::t_value>(arena, hash_map_get_entry_count(hash_map));
        return hash_map_load_entries(hash_map, *o_keys, *o_values);
    }

    template <c_hash_map tp_hash_map_type>
    [[nodiscard]] t_b8 hash_map_serialize(const tp_hash_map_type *const hm, const t_stream stream, t_arena *const temp_arena) {
        if (!stream_write_item(stream, hash_map_get_cap(hm))) {
            return false;
        }

        if (!stream_write_item(stream, hash_map_get_entry_count(hm))) {
            return false;
        }

        t_array_mut<typename tp_hash_map_type::t_key> keys;
        t_array_mut<typename tp_hash_map_type::t_value> values;
        hash_map_load_entries(hm, temp_arena, &keys, &values);

        if (!stream_write_items_of_array(stream, keys)) {
            return false;
        }

        if (!stream_write_items_of_array(stream, values)) {
            return false;
        }

        return true;
    }

    template <c_hash_map tp_hash_map_type>
    [[nodiscard]] t_b8 hash_map_deserialize(const t_stream stream, t_arena *const hm_arena, const t_hash_func<typename tp_hash_map_type::t_key> hm_hash_func, t_arena *const temp_arena, tp_hash_map_type *const o_hm, const t_comparator_bin<typename tp_hash_map_type::t_key> hm_key_comparator = k_comparator_bin_default<typename tp_hash_map_type::t_key>) {
        t_i32 cap;

        if (!stream_read_item(stream, &cap)) {
            return false;
        }

        t_i32 entry_cnt;

        if (!stream_read_item(stream, &entry_cnt)) {
            return false;
        }

        *o_hm = hash_map_create<typename tp_hash_map_type::t_key, typename tp_hash_map_type::t_value>(hm_hash_func, hm_arena, cap, hm_key_comparator);

        const auto keys = arena_push_array<typename tp_hash_map_type::t_key>(temp_arena, entry_cnt);

        if (!stream_read_items_into_array(stream, keys, entry_cnt)) {
            return false;
        }

        const auto values = arena_push_array<typename tp_hash_map_type::t_value>(temp_arena, entry_cnt);

        if (!stream_read_items_into_array(stream, values, entry_cnt)) {
            return false;
        }

        for (t_i32 i = 0; i < entry_cnt; i++) {
            hash_map_put(o_hm, keys[i], values[i]);
        }

        return true;
    }
}
