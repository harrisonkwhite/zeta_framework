#pragma once

#include <zcl/zcl_io.h>

namespace zf {
    // ============================================================
    // @section: Types and Globals

    template <typename tp_key_type, typename tp_val_type>
    struct s_kv_pair_block {
        s_array_mut<tp_key_type> keys;
        s_array_mut<tp_val_type> vals;
        s_array_mut<t_i32> next_indexes;
        s_bit_vec_mut usage;

        s_kv_pair_block *next;
    };

    enum e_kv_pair_block_seq_put_result : t_i32 {
        ek_kv_pair_block_seq_put_result_updated,
        ek_kv_pair_block_seq_put_result_added
    };

    template <typename tp_key_type, typename tp_val_type>
    struct s_kv_pair_block_seq {
        t_bin_comparator<tp_key_type> key_comparator;

        s_kv_pair_block<tp_key_type, tp_val_type> *blocks_head;
        s_arena *blocks_arena;
        t_i32 block_cnt;
        t_i32 block_cap;

        t_i32 pair_cnt;
    };

    template <typename tp_type>
    using t_hash_func = t_i32 (*)(const tp_type &key);

    // This is an FNV-1a implementation.
    constexpr t_hash_func<s_str_rdonly> g_str_hash_func =
        [](const s_str_rdonly &key) constexpr {
            const t_u32 offs_basis = 2166136261u;
            const t_u32 prime = 16777619u;

            t_u32 hash = offs_basis;

            for (t_i32 i = 0; i < key.bytes.len; i++) {
                hash ^= static_cast<t_u8>(key.bytes[i]);
                hash *= prime;
            }

            return static_cast<t_i32>(hash & 0x7FFFFFFFull);
        };

    enum e_hash_map_put_result : t_i32 {
        ek_hash_map_put_result_added,
        ek_hash_map_put_result_updated
    };

    constexpr t_i32 g_hash_map_cap_default = 32;

    template <typename tp_key_type, typename tp_val_type>
    struct s_hash_map {
        t_hash_func<tp_key_type> hash_func;

        s_array_mut<t_i32> immediate_indexes;
        s_kv_pair_block_seq<tp_key_type, tp_val_type> kv_pair_block_seq;
    };

    // ============================================================


    // ============================================================
    // @section: Functions

    template <typename tp_key_type, typename tp_val_type>
    s_kv_pair_block<tp_key_type, tp_val_type> *CreateKVPairBlock(const t_i32 cap, s_arena *const arena) {
        const auto block = PushItem<s_kv_pair_block<tp_key_type, tp_val_type>>(arena);

        block->keys = PushArray<tp_key_type>(arena, cap);

        block->vals = PushArray<tp_val_type>(arena, cap);

        block->next_indexes = PushArray<t_i32>(arena, cap);
        SetAllTo(block->next_indexes, -1);

        block->usage = CreateBitVector(cap, arena);

        block->next = nullptr;

        return block;
    }

    // @todo: Optimise by having this move to a relative block, instead of always from the start.
    template <typename tp_key_type, typename tp_val_type>
    s_kv_pair_block<tp_key_type, tp_val_type> *FindBlockOfIndex(const s_kv_pair_block_seq<tp_key_type, tp_val_type> *const block_seq, t_i32 index) {
        ZF_ASSERT(index >= -1 && index < block_seq->block_cap * block_seq->block_cnt);

        auto result = block_seq->blocks_head;

        while (index >= block_seq->block_cap) {
            result = result->next;
            index -= block_seq->block_cap;
        }

        return result;
    }

    template <typename tp_key_type, typename tp_val_type>
    [[nodiscard]] t_b8 FindInChain(const s_kv_pair_block_seq<tp_key_type, tp_val_type> *const block_seq, const t_i32 chain_begin_index, const tp_key_type &key, tp_val_type **const o_val) {
        ZF_ASSERT(chain_begin_index >= -1 && chain_begin_index < block_seq->block_cap * block_seq->block_cnt);

        t_i32 index = chain_begin_index;

        while (index != -1) {
            const auto block = FindBlockOfIndex(block_seq, index);

            const t_i32 rel_index = index % block_seq->block_cap;

            if (block_seq->key_comparator(block->keys[rel_index], key)) {
                *o_val = &block->vals[rel_index];
                return true;
            }

            index = block->next_indexes[rel_index];
        }

        return false;
    }

    template <typename tp_key_type, typename tp_val_type>
    t_b8 ExistsInChain(const s_kv_pair_block_seq<tp_key_type, tp_val_type> *const block_seq, const t_i32 chain_begin_index, const tp_key_type &key) {
        tp_val_type *val_throwaway;
        return FindInChain(chain_begin_index, key, &val_throwaway);
    }

    template <typename tp_key_type, typename tp_val_type>
    e_kv_pair_block_seq_put_result PutInChain(s_kv_pair_block_seq<tp_key_type, tp_val_type> *const block_seq, t_i32 *const chain_begin_index, const tp_key_type &key, const tp_val_type &val) {
        t_i32 *index = chain_begin_index;

        while (*index != -1) {
            const auto block = FindBlockOfIndex(block_seq, *index);

            const t_i32 rel_index = *index % block_seq->block_cap;

            if (block_seq->key_comparator(block->keys[rel_index], key)) {
                block->vals[rel_index] = val;
                return ek_kv_pair_block_seq_put_result_updated;
            }

            index = &block->next_indexes[rel_index];
        }

        // Insert and get the absolute index of the new pair.
        *index = [block_seq, &key, &val]() {
            s_kv_pair_block<tp_key_type, tp_val_type> *block = block_seq->blocks_head;
            s_kv_pair_block<tp_key_type, tp_val_type> *block_previous = nullptr;
            t_i32 block_index = 0;

            block_seq->pair_cnt++;

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

                return (block_index * block_seq->block_cap) + possible_rel_index_to_use;
            }

            // All blocks are full - create a new one.
            const auto new_block = CreateKVPairBlock<tp_key_type, tp_val_type>(block_seq->block_cap, block_seq->blocks_arena);
            block_seq->block_cnt++;

            if (block_previous) {
                block_previous->next = new_block;
            } else {
                block_seq->blocks_head = new_block;
            }

            new_block->keys[0] = key;
            new_block->vals[0] = val;
            SetBit(new_block->usage, 0);

            return block_index * block_seq->block_cap;
        }();

        return ek_kv_pair_block_seq_put_result_added;
    }

    template <typename tp_key_type, typename tp_val_type>
    t_b8 RemoveInChain(s_kv_pair_block_seq<tp_key_type, tp_val_type> *const block_seq, t_i32 *const chain_begin_index, const tp_key_type &key) {
        ZF_ASSERT(*chain_begin_index >= -1 && *chain_begin_index < block_seq->block_cap * block_seq->block_cnt);

        t_i32 *index = chain_begin_index;

        while (*index != -1) {
            const auto block = FindBlockOfIndex(block_seq, *index);

            const t_i32 rel_index = *index % block_seq->block_cap;

            if (block_seq->key_comparator(block->keys[rel_index], key)) {
                *index = block->next_indexes[rel_index];
                block_seq->pair_cnt--;
                return true;
            }

            index = &block->next_indexes[rel_index];
        }

        return false;
    }

    // Loads keys and values of the chain into the given PRE-ALLOCATED arrays.
    template <typename tp_key_type, typename tp_val_type>
    t_i32 LoadChain(const s_kv_pair_block_seq<tp_key_type, tp_val_type> *const block_seq, const t_i32 begin_index, const s_array_mut<tp_key_type> keys, const s_array_mut<tp_val_type> vals) {
        ZF_ASSERT(begin_index >= -1 && begin_index < block_seq->block_cap * block_seq->block_cnt);

        t_i32 loaded_cnt = 0;

        t_i32 index = begin_index;

        while (index != -1) {
            const auto block = FindBlockOfIndex(block_seq, index);

            const t_i32 rel_index = index % block_seq->block_cap;

            keys[loaded_cnt] = block->keys[rel_index];
            vals[loaded_cnt] = block->vals[rel_index];

            loaded_cnt++;

            index = block->next_indexes[rel_index];
        }

        return loaded_cnt;
    }

    // The provided hash function has to map a key to an integer 0 or higher. The given memory arena will be saved and used for allocating new memory for entries when needed.
    template <typename tp_key_type, typename tp_val_type>
    s_hash_map<tp_key_type, tp_val_type> CreateHashMap(const t_hash_func<tp_key_type> hash_func, s_arena *const arena, const t_i32 cap = g_hash_map_cap_default, const t_bin_comparator<tp_key_type> key_comparator = DefaultBinComparator) {
        const auto immediate_indexes = PushArray<t_i32>(arena, cap);
        SetAllTo(immediate_indexes, -1);

        return {
            .hash_func = hash_func,
            .immediate_indexes = immediate_indexes,
            .kv_pair_block_seq = {.key_comparator = key_comparator, .blocks_arena = arena, .block_cap = static_cast<t_i32>(AlignForward(cap, 8))},
        };
    }

    template <typename tp_type>
    t_i32 KeyToHashIndex(const tp_type &key, const t_hash_func<tp_type> hash_func, const t_i32 cap) {
        ZF_ASSERT(cap > 0);

        const t_i32 val = hash_func(key);
        ZF_ASSERT(val >= 0);

        return val % cap;
    }

    template <typename tp_key_type, typename tp_val_type>
    t_i32 Cap(const s_hash_map<tp_key_type, tp_val_type> *const hm) {
        return hm->immediate_indexes.len;
    }

    template <typename tp_key_type, typename tp_val_type>
    t_i32 EntryCount(const s_hash_map<tp_key_type, tp_val_type> *const hm) {
        return hm->kv_pair_block_seq.pair_cnt;
    }

    template <typename tp_key_type, typename tp_val_type>
    [[nodiscard]] t_b8 Find(const s_hash_map<tp_key_type, tp_val_type> *const hm, const tp_key_type &key, tp_val_type **const o_val) {
        const t_i32 hash_index = KeyToHashIndex(key, hm->hash_func, Cap(hm));
        return FindInChain(&hm->kv_pair_block_seq, hm->immediate_indexes[hash_index], key, o_val);
    }

    template <typename tp_key_type, typename tp_val_type>
    t_b8 Exists(const s_hash_map<tp_key_type, tp_val_type> *const hm, const tp_key_type &key) {
        const t_i32 hash_index = KeyToHashIndex(key, hm->hash_func, Cap(hm));
        return ExistsInChain(&hm->kv_pair_block_seq, hm->immediate_indexes[hash_index], key);
    }

    // Try adding the key-value pair to the hash map or just updating the value if the key is already present.
    template <typename tp_key_type, typename tp_val_type>
    e_hash_map_put_result Put(s_hash_map<tp_key_type, tp_val_type> *const hm, const tp_key_type &key, const tp_val_type &val) {
        const t_i32 hash_index = KeyToHashIndex(key, hm->hash_func, Cap(hm));
        return PutInChain(&hm->kv_pair_block_seq, &hm->immediate_indexes[hash_index], key, val) == ek_kv_pair_block_seq_put_result_updated ? ek_hash_map_put_result_updated : ek_hash_map_put_result_added;
    }

    // Returns true iff an entry with the key was found and removed.
    template <typename tp_key_type, typename tp_val_type>
    t_b8 Remove(s_hash_map<tp_key_type, tp_val_type> *const hm, const tp_key_type &key) {
        const t_i32 hash_index = KeyToHashIndex(key, hm->hash_func, Cap(hm));
        return RemoveInChain(&hm->kv_pair_block_seq, hm->immediate_indexes[hash_index], key);
    }

    // Loads all key-value pairs into the given PRE-ALLOCATED arrays.
    template <typename tp_key_type, typename tp_val_type>
    void LoadEntries(const s_hash_map<tp_key_type, tp_val_type> *const hm, const s_array_mut<tp_key_type> keys, const s_array_mut<tp_val_type> vals) {
        ZF_ASSERT(keys.len >= EntryCount(hm) && vals.len >= EntryCount(hm));

        t_i32 loaded_cnt = 0;

        for (t_i32 i = 0; i < hm->immediate_indexes.len; i++) {
            loaded_cnt += LoadChain(&hm->kv_pair_block_seq, hm->immediate_indexes[i], SliceFrom(keys, loaded_cnt), SliceFrom(vals, loaded_cnt));
        }
    }

    // Allocates the given arrays with the memory arena and loads key-value pairs into them.
    template <typename tp_key_type, typename tp_val_type>
    void LoadEntries(const s_hash_map<tp_key_type, tp_val_type> *const hm, s_arena *const arena, s_array_mut<tp_key_type> *const o_keys, s_array_mut<tp_val_type> *const o_vals) {
        *o_keys = PushArray<tp_key_type>(arena, EntryCount(hm));
        *o_vals = PushArray<tp_val_type>(arena, EntryCount(hm));
        return LoadEntries(hm, *o_keys, *o_vals);
    }

    template <typename tp_key_type, typename tp_val_type>
    [[nodiscard]] t_b8 SerializeHashMap(c_stream *const stream, const s_hash_map<tp_key_type, tp_val_type> *const hm, s_arena *const temp_arena) {
        if (!stream->WriteItem(Cap(hm))) {
            return false;
        }

        if (!stream->WriteItem(EntryCount(hm))) {
            return false;
        }

        s_array_mut<tp_key_type> keys;
        s_array_mut<tp_val_type> vals;
        LoadEntries(hm, temp_arena, &keys, &vals);

        if (!stream->WriteItemsOfArray(keys)) {
            return false;
        }

        if (!stream->WriteItemsOfArray(vals)) {
            return false;
        }

        return true;
    }

    template <typename tp_key_type, typename tp_val_type>
    [[nodiscard]] t_b8 DeserializeHashMap(c_stream *const stream, s_arena *const hm_arena, const t_hash_func<tp_key_type> hm_hash_func, s_arena *const temp_arena, s_hash_map<tp_key_type, tp_val_type> *const o_hm, const t_bin_comparator<tp_key_type> hm_key_comparator = DefaultBinComparator) {
        t_i32 cap;

        if (!stream->ReadItem(&cap)) {
            return false;
        }

        t_i32 entry_cnt;

        if (!stream->ReadItem(&entry_cnt)) {
            return false;
        }

        *o_hm = CreateHashMap<tp_key_type, tp_val_type>(hm_hash_func, hm_arena, cap, hm_key_comparator);

        const auto keys = PushArray<tp_key_type>(temp_arena, entry_cnt);

        if (!stream->ReadItemsIntoArray(keys, entry_cnt)) {
            return false;
        }

        const auto vals = PushArray<tp_val_type>(temp_arena, entry_cnt);

        if (!stream->ReadItemsIntoArray(vals, entry_cnt)) {
            return false;
        }

        for (t_i32 i = 0; i < entry_cnt; i++) {
            Put(o_hm, keys[i], vals[i]);
        }

        return true;
    }

    // ============================================================
}
