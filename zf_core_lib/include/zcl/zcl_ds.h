#pragma once

#include <zcl/zcl_io.h>

namespace zf {
    template <typename tp_type>
    struct s_list {
        static_assert(!s_is_const<tp_type>::g_val);

    public:
        using t_elem = tp_type;

        s_list() = default;

        s_list(const s_array<tp_type> backing_arr, const t_i32 len = 0) : m_backing_arr(backing_arr), m_len(len) {
            ZF_ASSERT(len >= 0 && len <= backing_arr.Len());
        }

        s_list(const s_list &) = delete;

        s_array<tp_type> BackingArray() const {
            return m_backing_arr;
        }

        t_i32 Len() const {
            return m_len;
        }

        void SetLen(const t_i32 len) {
            ZF_ASSERT(len >= 0 && len <= m_backing_arr.Len());
            m_len = len;
        }

        t_i32 Cap() const {
            return m_backing_arr.Len();
        }

        tp_type &operator[](const t_i32 index) const {
            ZF_ASSERT(index >= 0 && index < m_len);
            return m_backing_arr[index];
        }

        tp_type &Last() const {
            return operator[](m_len - 1);
        }

    private:
        s_array<tp_type> m_backing_arr = {};
        t_i32 m_len = 0;
    };

    template <typename tp_type, t_i32 tp_cap>
    struct s_static_list {
        static_assert(!s_is_const<tp_type>::g_val);

    public:
        using t_elem = tp_type;

        s_static_list() = default;

        s_static_list(const s_array<tp_type> backing_arr, const t_i32 len = 0) : m_backing_arr(backing_arr), m_len(len) {
            ZF_ASSERT(len >= 0 && len <= backing_arr.Len());
        }

        s_static_list(const s_static_list &) = delete;

        s_array<tp_type> BackingArray() const {
            return m_backing_arr.ToNonstatic();
        }

        t_i32 Len() const {
            return m_len;
        }

        void SetLen(const t_i32 len) {
            ZF_ASSERT(len >= 0 && len <= m_backing_arr.Len());
            m_len = len;
        }

        t_i32 Cap() const {
            return m_backing_arr.Len();
        }

        tp_type &operator[](const t_i32 index) const {
            ZF_ASSERT(index >= 0 && index < m_len);
            return m_backing_arr[index];
        }

        tp_type &Last() const {
            return operator[](m_len - 1);
        }

    private:
        s_static_array<tp_type, tp_cap> m_backing_arr = {};
        t_i32 m_len = 0;
    };

    template <typename tp_type>
    struct s_is_nonstatic_list {
        static constexpr t_b8 g_val = false;
    };

    template <typename tp_type>
    struct s_is_nonstatic_list<s_list<tp_type>> {
        static constexpr t_b8 g_val = true;
    };

    template <typename tp_type>
    struct s_is_static_list {
        static constexpr t_b8 g_val = false;
    };

    template <typename tp_type, t_i32 tp_cap>
    struct s_is_static_list<s_static_list<tp_type, tp_cap>> {
        static constexpr t_b8 g_val = true;
    };

    template <typename tp_type>
    concept c_nonstatic_list = s_is_nonstatic_list<tp_type>::g_val;

    template <typename tp_type>
    concept c_static_list = s_is_static_list<tp_type>::g_val;

    template <typename tp_type>
    concept c_list = c_nonstatic_list<tp_type> || c_static_list<tp_type>;

    template <typename tp_type>
    s_list<tp_type> ListCreate(const t_i32 cap, s_mem_arena &mem_arena, const t_i32 len = 0) {
        ZF_ASSERT(cap > 0 && len >= 0 && len <= cap);
        return {AllocArray<tp_type>(cap, mem_arena), len};
    }

    using t_list_extension_cap_calculator = t_i32 (*)(const t_i32 cap_current);

    constexpr t_list_extension_cap_calculator g_default_list_extension_cap_calculator =
        [](const t_i32 cap_current) {
            ZF_ASSERT(cap_current >= 0);
            return cap_current == 0 ? 1 : cap_current * 2;
        };

    template <c_nonstatic_list tp_list_type>
    void ListExtend(tp_list_type &list, s_mem_arena &mem_arena, const t_list_extension_cap_calculator cap_calculator = g_default_list_extension_cap_calculator) {
        ZF_ASSERT(cap_calculator);

        const t_i32 new_cap = cap_calculator(list.Cap());
        ZF_ASSERT(new_cap > list.Cap());

        const auto new_backing_arr = AllocArray<typename tp_list_type::t_elem>(new_cap, mem_arena);
        Copy(new_backing_arr, list.BackingArray());

        list = {new_backing_arr, list.Len()};
    }

    template <c_nonstatic_list tp_list_type>
    void ListExtendToFit(tp_list_type &list, const t_i32 min_cap, s_mem_arena &mem_arena, const t_list_extension_cap_calculator cap_calculator = g_default_list_extension_cap_calculator) {
        ZF_ASSERT(min_cap > list.Cap());
        ZF_ASSERT(cap_calculator);

        const t_i32 new_cap = [cap = list.Cap(), min_cap, cap_calculator]() {
            t_i32 res = cap;

            do {
                const auto res_last = res;
                res = cap_calculator(res);
                ZF_ASSERT(res > res_last);
            } while (res < min_cap);

            return res;
        }();

        const auto new_backing_arr = AllocArray<typename tp_list_type::t_elem>(new_cap, mem_arena);
        Copy(new_backing_arr, list.BackingArray());

        list = {new_backing_arr, list.Len()};
    }

    template <c_list tp_list_type>
    typename tp_list_type::t_elem &ListAppend(tp_list_type &list, const typename tp_list_type::t_elem &val) {
        ZF_ASSERT(list.Len() < list.Cap());

        list.SetLen(list.Len() + 1);
        list.Last() = val;
        return list.Last();
    }

    template <c_nonstatic_list tp_list_type>
    typename tp_list_type::t_elem &ListAppendDynamic(tp_list_type &list, const typename tp_list_type::t_elem &val, s_mem_arena &extension_mem_arena, const t_list_extension_cap_calculator extension_cap_calculator = g_default_list_extension_cap_calculator) {
        if (list.Len() == list.Cap()) {
            ListExtend(list, extension_mem_arena, extension_cap_calculator);
        }

        return ListAppend(list, val);
    }

    template <c_list tp_list_type>
    s_array<typename tp_list_type::t_elem> ListAppendMany(tp_list_type &list, const s_array_rdonly<typename tp_list_type::t_elem> vals) {
        ZF_ASSERT(list.Len() + vals.Len() <= list.Cap());

        Copy(list.BackingArray().SliceFrom(list.Len()), vals);
        list.SetLen(list.Len() + vals.Len());
        return list.BackingArray().Slice(list.Len() - vals.Len(), list.Len());
    }

    template <c_nonstatic_list tp_list_type>
    s_array<typename tp_list_type::t_elem> ListAppendManyDynamic(tp_list_type &list, const s_array_rdonly<typename tp_list_type::t_elem> vals, s_mem_arena &extension_mem_arena, const t_list_extension_cap_calculator extension_cap_calculator = g_default_list_extension_cap_calculator) {
        const auto min_cap_needed = list.Len() + vals.Len();

        if (min_cap_needed > list.Cap()) {
            ListExtendToFit(list, min_cap_needed, extension_mem_arena, extension_cap_calculator);
        }

        return ListAppendMany(list, vals);
    }

    template <c_list tp_list_type>
    typename tp_list_type::t_elem &ListInsert(tp_list_type &list, const t_i32 index, const typename tp_list_type::t_elem &val) {
        ZF_ASSERT(list.Len() < list.Cap());
        ZF_ASSERT(index >= 0 && index <= list.Len());

        list.SetLen(list.Len() + 1);

        for (t_i32 i = list.Len() - 1; i > index; i--) {
            list[i] = list[i - 1];
        }

        list[index] = val;

        return list[index];
    }

    template <c_list tp_list_type>
    typename tp_list_type::t_elem &ListInsertDynamic(tp_list_type &list, const t_i32 index, const typename tp_list_type::t_elem &val, s_mem_arena &extension_mem_arena, const t_list_extension_cap_calculator extension_cap_calculator = g_default_list_extension_cap_calculator) {
        if (list.Len() == list.Cap()) {
            ListExtend(list, extension_mem_arena, extension_cap_calculator);
        }

        return ListInsert(list, index, val);
    }

    template <c_list tp_list_type>
    void ListRemove(tp_list_type &list, const t_i32 index) {
        ZF_ASSERT(list.Len() > 0);
        ZF_ASSERT(index >= 0 && index < list.Len());

        Copy(list.BackingArray().Slice(index, list.Len() - 1), list.BackingArray().Slice(index + 1, list.Len()));
        list.SetLen(list.Len() - 1);
    }

    template <c_list tp_list_type>
    void ListRemoveSwapback(tp_list_type &list, const t_i32 index) {
        ZF_ASSERT(list.Len() > 0);
        ZF_ASSERT(index >= 0 && index < list.Len());

        list[index] = list.Last();
        list.SetLen(list.Len() - 1);
    }

    template <c_list tp_list_type>
    void ListRemoveLast(tp_list_type &list) {
        ZF_ASSERT(list.Len() > 0);
        list.SetLen(list.Len() - 1);
    }

    // ============================================================
    // @section: Hash Map
    // ============================================================
    enum e_kv_pair_block_seq_put_result {
        ek_kv_pair_block_seq_put_result_updated,
        ek_kv_pair_block_seq_put_result_added
    };

    template <typename tp_key_type, typename tp_val_type>
    struct s_kv_pair_block_seq {
    public:
        s_kv_pair_block_seq() = default;

        s_kv_pair_block_seq(const t_i32 block_cap, s_mem_arena &blocks_mem_arena, const t_bin_comparator<tp_key_type> key_comparator = DefaultBinComparator)
            : m_active(true), m_block_cap(block_cap), m_blocks_mem_arena(&blocks_mem_arena), m_key_comparator(key_comparator) {
            ZF_ASSERT(key_comparator);
        };

        s_kv_pair_block_seq(const s_kv_pair_block_seq &) = delete;

        t_i32 BlockCount() const {
            ZF_ASSERT(m_active);
            return m_block_cnt;
        }

        t_i32 PairCount() const {
            ZF_ASSERT(m_active);
            return m_pair_cnt;
        }

        [[nodiscard]] t_b8 FindInChain(const t_i32 chain_begin_index, const tp_key_type &key, s_ptr<tp_val_type> &o_val) const {
            ZF_ASSERT(m_active);
            ZF_ASSERT(chain_begin_index >= -1 && chain_begin_index < m_block_cap * m_block_cnt);

            t_i32 index = chain_begin_index;

            while (index != -1) {
                const s_block &block = FindBlockOfIndex(index);

                const t_i32 rel_index = index % m_block_cap;

                if (m_key_comparator(block.keys[rel_index], key)) {
                    o_val = &block.vals[rel_index];
                    return true;
                }

                index = block.next_indexes[rel_index];
            }

            return false;
        }

        t_b8 ExistsInChain(const t_i32 chain_begin_index, const tp_key_type &key) const {
            s_ptr<tp_val_type> val_throwaway;
            return FindInChain(chain_begin_index, key, val_throwaway);
        }

        e_kv_pair_block_seq_put_result PutInChain(t_i32 &chain_begin_index, const tp_key_type &key, const tp_val_type &val) {
            ZF_ASSERT(m_active);

            s_ptr<t_i32> index = &chain_begin_index;

            while (*index != -1) {
                const s_block &block = FindBlockOfIndex(*index);

                const t_i32 rel_index = *index % m_block_cap;

                if (m_key_comparator(block.keys[rel_index], key)) {
                    block.vals[rel_index] = val;
                    return ek_kv_pair_block_seq_put_result_updated;
                }

                index = &block.next_indexes[rel_index];
            }

            *index = Insert(key, val);

            return ek_kv_pair_block_seq_put_result_added;
        }

        t_b8 RemoveInChain(t_i32 &chain_begin_index, const tp_key_type &key) {
            ZF_ASSERT(m_active);
            ZF_ASSERT(chain_begin_index >= -1 && chain_begin_index < m_block_cap * m_block_cnt);

            s_ptr<t_i32> index = &chain_begin_index;

            while (*index != -1) {
                s_block &block = FindBlockOfIndex(*index);

                const t_i32 rel_index = *index % m_block_cap;

                if (m_key_comparator(block.keys[rel_index], key)) {
                    *index = block.next_indexes[rel_index];
                    m_pair_cnt--;
                    return true;
                }

                index = &block.next_indexes[rel_index];
            }

            return false;
        }

        // Loads keys and values of the chain into the given PRE-ALLOCATED arrays.
        t_i32 LoadChain(const t_i32 begin_index, const s_array<tp_key_type> keys, const s_array<tp_val_type> vals) const {
            ZF_ASSERT(m_active);
            ZF_ASSERT(begin_index >= -1 && begin_index < m_block_cap * m_block_cnt);

            t_i32 loaded_cnt = 0;

            t_i32 index = begin_index;

            while (index != -1) {
                s_block &block = FindBlockOfIndex(index);

                const t_i32 rel_index = index % m_block_cap;

                keys[loaded_cnt] = block.keys[rel_index];
                vals[loaded_cnt] = block.vals[rel_index];

                loaded_cnt++;

                index = block.next_indexes[rel_index];
            }

            return loaded_cnt;
        }

    private:
        struct s_block {
            s_array<tp_key_type> keys = {};
            s_array<tp_val_type> vals = {};
            s_array<t_i32> next_indexes = {}; // -1 means no "next" (i.e. it is the last in the chain).
            s_bit_vec usage = {};

            s_ptr<s_block> next = nullptr;
        };

        static s_block &CreateBlock(const t_i32 cap, s_mem_arena &mem_arena) {
            auto &block = zf::Alloc<s_block>(mem_arena);

            block.keys = AllocArray<tp_key_type>(cap, mem_arena);

            block.vals = AllocArray<tp_val_type>(cap, mem_arena);

            block.next_indexes = AllocArray<t_i32>(cap, mem_arena);
            SetAllTo(block.next_indexes, -1);

            block.usage = CreateBitVec(cap, mem_arena);

            return block;
        }

        // @todo: Optimise by having this move to a relative block, instead of always from the start.
        s_block &FindBlockOfIndex(t_i32 index) const {
            ZF_ASSERT(index >= -1 && index < m_block_cap * m_block_cnt);

            s_ptr<s_block> res = m_blocks_head;

            while (index >= m_block_cap) {
                res = res->next;
                index -= m_block_cap;
            }

            return *res;
        }

        // Returns the absolute index of the inserted pair.
        t_i32 Insert(const tp_key_type &key, const tp_val_type &val) {
            s_ptr<s_block> block = m_blocks_head;
            s_ptr<s_block> block_previous = nullptr;
            t_i32 block_index = 0;

            m_pair_cnt++;

            while (block) {
                const auto possible_rel_index_to_use = IndexOfFirstUnsetBit(block->usage);

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

                return (block_index * m_block_cap) + possible_rel_index_to_use;
            }

            // All blocks are full - create a new one.
            auto &new_block = CreateBlock(m_block_cap, *m_blocks_mem_arena);
            m_block_cnt++;

            if (block_previous) {
                block_previous->next = &new_block;
            } else {
                m_blocks_head = &new_block;
            }

            new_block.keys[0] = key;
            new_block.vals[0] = val;
            SetBit(new_block.usage, 0);

            return block_index * m_block_cap;
        }

        t_b8 m_active = false;

        t_bin_comparator<tp_key_type> m_key_comparator = nullptr;

        s_ptr<s_block> m_blocks_head = nullptr;
        s_ptr<s_mem_arena> m_blocks_mem_arena = nullptr;
        t_i32 m_block_cnt = 0;
        t_i32 m_block_cap = 0;

        t_i32 m_pair_cnt = 0;
    };

    template <typename tp_type>
    using t_hash_func = t_i32 (*)(const tp_type &key);

    // This is an FNV-1a implementation.
    constexpr t_hash_func<s_str_rdonly> g_str_hash_func =
        [](const s_str_rdonly &key) constexpr {
            const t_u32 offs_basis = 2166136261u;
            const t_u32 prime = 16777619u;

            t_u32 hash = offs_basis;

            for (t_i32 i = 0; i < key.bytes.Len(); i++) {
                hash ^= static_cast<t_u8>(key.bytes[i]);
                hash *= prime;
            }

            return static_cast<t_i32>(hash & 0x7FFFFFFFull);
        };

    template <typename tp_type>
    t_i32 KeyToHashIndex(const tp_type &key, const t_hash_func<tp_type> hash_func, const t_i32 cap) {
        ZF_ASSERT(hash_func);
        ZF_ASSERT(cap > 0);

        const t_i32 val = hash_func(key);
        ZF_ASSERT(val >= 0);

        return val % cap;
    }

    enum e_hash_map_put_result : t_i32 {
        ek_hash_map_put_result_added,
        ek_hash_map_put_result_updated
    };

    constexpr t_i32 g_hash_map_cap_default = 32;

    template <typename tp_key_type, typename tp_val_type>
    struct s_hash_map {
        t_hash_func<tp_key_type> hash_func = nullptr;

        s_array<t_i32> immediate_indexes = {};
        s_kv_pair_block_seq<tp_key_type, tp_val_type> kv_pair_block_seq = {};

        t_i32 Cap() const {
            return immediate_indexes.Len();
        }

        t_i32 EntryCount() const {
            return kv_pair_block_seq.PairCount();
        }

        [[nodiscard]] t_b8 Find(const tp_key_type &key, s_ptr<tp_val_type> &o_val) const {
            const t_i32 hash_index = KeyToHashIndex(key, hash_func, Cap());
            return kv_pair_block_seq.FindInChain(immediate_indexes[hash_index], key, o_val);
        }

        t_b8 Exists(const tp_key_type &key) const {
            const t_i32 hash_index = KeyToHashIndex(key, hash_func, Cap());
            return kv_pair_block_seq.ExistsInChain(immediate_indexes[hash_index], key);
        }

        // Try adding the key-value pair to the hash map or just updating the value if the key is already present.
        e_hash_map_put_result Put(const tp_key_type &key, const tp_val_type &val) {
            const t_i32 hash_index = KeyToHashIndex(key, hash_func, Cap());
            return kv_pair_block_seq.PutInChain(immediate_indexes[hash_index], key, val) == ek_kv_pair_block_seq_put_result_updated ? ek_hash_map_put_result_updated : ek_hash_map_put_result_added;
        }

        // Returns true iff an entry with the key was found and removed.
        t_b8 Remove(const tp_key_type &key) {
            const t_i32 hash_index = KeyToHashIndex(key, hash_func, Cap());
            return kv_pair_block_seq.RemoveInChain(immediate_indexes[hash_index], key);
        }

        // Loads all key-value pairs into the given PRE-ALLOCATED arrays.
        void LoadEntries(const s_array<tp_key_type> keys, const s_array<tp_val_type> vals) const {
            ZF_ASSERT(keys.Len() >= EntryCount() && vals.Len() >= EntryCount());

            t_i32 loaded_cnt = 0;

            for (t_i32 i = 0; i < immediate_indexes.Len(); i++) {
                loaded_cnt += kv_pair_block_seq.LoadChain(immediate_indexes[i], keys.SliceFrom(loaded_cnt), vals.SliceFrom(loaded_cnt));
            }
        }

        // Allocates the given arrays with the memory arena and loads key-value pairs into them.
        void LoadEntries(s_mem_arena &mem_arena, s_array<tp_key_type> &o_keys, s_array<tp_val_type> &o_vals) const {
            o_keys = AllocArray<tp_key_type>(EntryCount(), mem_arena);
            o_vals = AllocArray<tp_val_type>(EntryCount(), mem_arena);
            return LoadEntries(o_keys, o_vals);
        }
    };

    // The provided hash function has to map a key to an integer 0 or higher. The given memory arena will be saved and used for allocating new memory for entries when needed.
    template <typename tp_key_type, typename tp_val_type>
    s_hash_map<tp_key_type, tp_val_type> HashMapCreate(const t_hash_func<tp_key_type> hash_func, s_mem_arena &mem_arena, const t_i32 cap = g_hash_map_cap_default, const t_bin_comparator<tp_key_type> key_comparator = DefaultBinComparator) {
        const auto immediate_indexes = AllocArray<t_i32>(cap, mem_arena);
        SetAllTo(immediate_indexes, -1);

        return {
            .hash_func = hash_func,
            .immediate_indexes = immediate_indexes,
            .kv_pair_block_seq = {static_cast<t_i32>(AlignForward(cap, 8)), mem_arena, key_comparator},
        };
    }

    template <typename tp_key_type, typename tp_val_type>
    [[nodiscard]] t_b8 SerializeHashMap(s_stream &stream, const s_hash_map<tp_key_type, tp_val_type> &hm, s_mem_arena &temp_mem_arena) {
        const t_i32 cap = hm.Cap();

        if (!stream.WriteItem(cap)) {
            return false;
        }

        const t_i32 entry_cnt = hm.EntryCount();

        if (!stream.WriteItem(entry_cnt)) {
            return false;
        }

        s_array<tp_key_type> keys;
        s_array<tp_val_type> vals;
        hm.LoadEntries(temp_mem_arena, keys, vals);

        if (!stream.WriteItemsOfArray(keys)) {
            return false;
        }

        if (!stream.WriteItemsOfArray(vals)) {
            return false;
        }

        return true;
    }

    template <typename tp_key_type, typename tp_val_type>
    [[nodiscard]] t_b8 DeserializeHashMap(s_stream &stream, s_mem_arena &hm_mem_arena, const t_hash_func<tp_key_type> hm_hash_func, s_mem_arena &temp_mem_arena, s_hash_map<tp_key_type, tp_val_type> &o_hm, const t_bin_comparator<tp_key_type> hm_key_comparator = DefaultBinComparator) {
        ZF_ASSERT(hm_hash_func);
        ZF_ASSERT(hm_key_comparator);

        t_i32 cap;

        if (!stream.ReadItem(cap)) {
            return false;
        }

        t_i32 entry_cnt;

        if (!stream.ReadItem(entry_cnt)) {
            return false;
        }

        o_hm = HashMapCreate<tp_key_type, tp_val_type>(hm_hash_func, hm_mem_arena, cap, hm_key_comparator);

        const auto keys = AllocArray<tp_key_type>(entry_cnt, temp_mem_arena);

        if (!stream.ReadItemsIntoArray(keys, entry_cnt)) {
            return false;
        }

        const auto vals = AllocArray<tp_val_type>(entry_cnt, temp_mem_arena);

        if (!stream.ReadItemsIntoArray(vals, entry_cnt)) {
            return false;
        }

        for (t_i32 i = 0; i < entry_cnt; i++) {
            o_hm.Put(keys[i], vals[i]);
        }

        return true;
    }
}
