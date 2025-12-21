#pragma once

#include <zcl/zcl_io.h>

namespace zf {
    template <typename tp_type>
    struct s_list {
        static_assert(!s_is_const<tp_type>::g_val);

    public:
        s_list() = default;

        explicit s_list(const s_array<tp_type> backing_arr, const t_len len = 0) : m_backing_arr(backing_arr), m_len(len) {
            ZF_ASSERT(len >= 0 && len <= backing_arr.Len());
        }

        s_list(const s_list &) = delete;

        s_array<tp_type> BackingArray() {
            return m_backing_arr;
        }

        t_len Len() const {
            return m_len;
        }

        t_len Cap() const {
            return m_backing_arr.Len();
        }

        t_b8 IsEmpty() const {
            return m_len == 0;
        }

        t_b8 IsFull() const {
            return m_len == m_backing_arr.Len();
        }

        s_array<tp_type> ToArray() const {
            return m_backing_arr.Slice(0, m_len);
        }

        tp_type &operator[](const t_len index) const {
            ZF_ASSERT(index >= 0 && index < m_len);
            return m_backing_arr[index];
        }

        tp_type &Last() const {
            return operator[](m_len - 1);
        }

        void Clear() {
            m_len = 0;
        }

        tp_type &Append(const tp_type &val) {
            ZF_ASSERT(!IsFull());

            m_backing_arr[m_len] = val;
            m_len++;
            return m_backing_arr[m_len - 1];
        }

        void Insert(const t_len index, const tp_type &val) {
            ZF_ASSERT(!IsFull());
            ZF_ASSERT(index >= 0 && index <= m_len);

            for (t_len i = m_len; i > index; i--) {
                m_backing_arr[i] = m_backing_arr[i - 1];
            }

            m_len++;
            m_backing_arr[index] = val;
        }

        void Remove(const t_len index) {
            ZF_ASSERT(!IsEmpty());
            ZF_ASSERT(index >= 0 && index < m_len);

            m_backing_arr.Slice(index + 1, m_len).CopyTo(m_backing_arr.Slice(index, m_len - 1));
            m_len--;
        }

        void RemoveSwapback(const t_len index) {
            ZF_ASSERT(!IsEmpty());
            ZF_ASSERT(index >= 0 && index < m_len);

            m_backing_arr[index] = m_backing_arr[m_len - 1];
            m_len--;
        }

        tp_type RemoveLast() {
            ZF_ASSERT(!IsEmpty());

            m_len--;
            return m_backing_arr[m_len];
        }

    private:
        s_array<tp_type> m_backing_arr = {};
        t_len m_len = 0;
    };

    template <typename tp_type, t_len tp_cap>
    struct s_static_list : public s_list<tp_type> {
    public:
        s_static_list() : s_list<tp_type>(m_backing_arr) {}

    private:
        s_static_array<tp_type, tp_cap> m_backing_arr = {};
    };

    template <typename tp_type>
    [[nodiscard]] t_b8 CreateList(const t_len cap, s_mem_arena &mem_arena, s_list<tp_type> &o_list, const t_len len = 0) {
        ZF_ASSERT(cap > 0 && len >= 0 && len <= cap);
        o_list = {AllocArray<tp_type>(cap, mem_arena), len};
        return true;
    }

    // ============================================================
    // @section: Stack
    // ============================================================
    template <typename tp_type>
    struct s_stack {
        static_assert(!s_is_const<tp_type>::g_val);

    public:
        s_stack() = default;

        explicit s_stack(const s_array<tp_type> backing_arr, const t_len height = 0) : m_backing_arr(backing_arr), m_height(height) {
            ZF_ASSERT(height >= 0 && height <= backing_arr.Len());
        }

        s_stack(const s_stack &) = delete;

        s_array<tp_type> BackingArray() {
            return m_backing_arr;
        }

        t_len Height() const {
            return m_height;
        }

        t_len Cap() const {
            return m_backing_arr.Len();
        }

        t_b8 IsEmpty() const {
            return m_height == 0;
        }

        t_b8 IsFull() const {
            return m_height == m_backing_arr.Len();
        }

        s_array<tp_type> ToArray() const {
            return m_backing_arr.Slice(0, m_height);
        }

        tp_type &operator[](const t_len index) const {
            ZF_ASSERT(index >= 0 && index < m_height);
            return m_backing_arr[index];
        }

        tp_type &Peek() const {
            return operator[](m_height - 1);
        }

        void Clear() {
            m_height = 0;
        }

        tp_type &Push(const tp_type &val) {
            ZF_ASSERT(!IsFull());

            m_backing_arr[m_height] = val;
            m_height++;
            return m_backing_arr[m_height - 1];
        }

        tp_type Pop() {
            ZF_ASSERT(!IsEmpty());

            m_height--;
            return m_backing_arr[m_height];
        }

    private:
        s_array<tp_type> m_backing_arr = {};
        t_len m_height = 0;
    };

    template <typename tp_type, t_len tp_cap>
    struct s_static_stack : public s_stack<tp_type> {
    public:
        s_static_stack() : s_stack<tp_type>(m_backing_arr) {}

    private:
        s_static_array<tp_type, tp_cap> m_backing_arr = {};
    };

    template <typename tp_type>
    [[nodiscard]] t_b8 CreateStack(const t_len cap, s_mem_arena &mem_arena, s_stack<tp_type> &o_stack, const t_len height = 0) {
        ZF_ASSERT(cap > 0 && height >= 0 && height <= cap);
        o_stack = {AllocArray<tp_type>(cap, mem_arena), height};
        return true;
    }

    // ============================================================
    // @section: Key-Value Pair Block Sequences
    // ============================================================
    enum e_kv_pair_block_seq_put_result {
        ek_kv_pair_block_seq_put_result_updated,
        ek_kv_pair_block_seq_put_result_added
    };

    struct s_kv_pair_block_seq {
    public:
        [[nodiscard]] t_b8 FindInChain(const t_i32 chain_begin_index, const t_i32 &key, const s_ptr<s_ptr<t_i32>> o_val = nullptr) {
            ZF_ASSERT(chain_begin_index >= -1 && chain_begin_index < m_block_cap * m_block_cnt);

            t_i32 index = chain_begin_index;

            while (index != -1) {
                const s_block &block = FindBlockOfIndex(index);

                const t_i32 rel_index = index % m_block_cap;

                if (block.keys[rel_index] == key) {
                    if (o_val) {
                        *o_val = &block.vals[rel_index];
                    }

                    return true;
                }

                index = block.next_indexes[rel_index];
            }

            return false;
        }

        e_kv_pair_block_seq_put_result PutInChain(t_i32 &chain_begin_index, const t_i32 &key, const t_i32 &val) {
            s_ptr<t_i32> index = &chain_begin_index;

            while (*index != -1) {
                const s_block &block = FindBlockOfIndex(*index);

                const t_i32 rel_index = *index % m_block_cap;

                if (block.keys[rel_index] == key) {
                    block.vals[rel_index] = val;
                    return ek_kv_pair_block_seq_put_result_updated;
                }

                index = &block.next_indexes[rel_index];
            }

            *index = Insert(key, val);

            return ek_kv_pair_block_seq_put_result_added;
        }

        t_b8 RemoveInChain(t_i32 &begin_index, const t_i32 &key) {
            ZF_ASSERT(begin_index >= -1 && begin_index < m_block_cap * m_block_cnt);

            s_ptr<t_i32> index = &begin_index;

            while (*index != -1) {
                s_block &block = FindBlockOfIndex(index);

                const t_i32 rel_index = *index % m_block_cap;

                if (block.keys[rel_index] == key) {
                    *index = block.next_indexes[rel_index];
                    return true;
                }

                index = &block.next_indexes[rel_index];
            }

            return false;
        }

    private:
        struct s_block {
            s_array<t_i32> keys = {};
            s_array<t_i32> vals = {};
            s_array<t_i32> next_indexes = {}; // -1 means no "next" (i.e. it is the last in the chain).
            s_bit_vec usage = {};

            s_ptr<s_block> next = nullptr;
        };

        static s_block &CreateBlock(const t_i32 cap, s_mem_arena &mem_arena) {
            auto &block = zf::Alloc<s_block>(mem_arena);

            block.keys = AllocArray<t_i32>(cap, mem_arena);

            block.vals = AllocArray<t_i32>(cap, mem_arena);

            block.next_indexes = AllocArray<t_i32>(cap, mem_arena);
            block.next_indexes.SetAllTo(-1);

            block.usage = AllocBitVec(cap, mem_arena);

            return block;
        }

        // @todo: Optimise by having this move to a relative block, instead of always from the start.
        s_block &FindBlockOfIndex(t_len index) const {
            ZF_ASSERT(index >= -1 && index < m_block_cap * m_block_cnt);

            s_ptr<s_block> res = m_head;

            while (index >= m_block_cap) {
                res = m_head->next;
                index -= m_block_cap;
            }

            return *res;
        }

        // Returns the absolute index of the inserted pair.
        t_i32 Insert(const t_i32 &key, const t_i32 &val) {
            s_ptr<s_block> block = m_head;
            s_ptr<s_block> block_previous = nullptr;
            t_i32 block_index = 0;

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
            auto &new_block = CreateBlock(m_block_cap, *m_mem_arena);
            m_block_cnt++;

            if (block_previous) {
                block_previous->next = &new_block;
            }

            block->keys[0] = key;
            block->vals[0] = val;
            SetBit(block->usage, 0);

            return block_index * m_block_cap;
        }

        s_ptr<s_block> m_head = nullptr;
        t_i32 m_block_cnt = 0;
        t_i32 m_block_cap = 0;
        s_ptr<s_mem_arena> m_mem_arena = nullptr; // Used for allocating new blocks.
    };

    // ============================================================
    // @section: Hash Map
    // ============================================================
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
    public:
        t_b8 IsActive() const {
            return m_active;
        }

        t_i32 Cap() const {
            ZF_ASSERT(m_active);
            return m_immediate_indexes.Len();
        }

        t_i32 EntryCount() const {
            ZF_ASSERT(m_active);
            return m_entry_cnt;
        }

        // Returns true iff an entry with the key was found. Leave o_val as nullptr if you don't care about getting the value. o_val is untouched if the key is not found.
        [[nodiscard]] t_b8 Get(const tp_key_type &key, const s_ptr<tp_val_type> o_val = nullptr) const {
            ZF_ASSERT(m_active);

            const t_i32 hash_index = KeyToHashIndex(key, m_hash_func, Cap());
            return FindValueOfKeyInChain<tp_key_type, tp_val_type>(m_kv_pair_blocks_head, m_immediate_indexes[hash_index], key, o_val);
        }

        // Try adding the key-value pair to the hash map or just updating the value if the key is already present.
        e_hash_map_put_result Put(const tp_key_type &key, const tp_val_type &val) {
            ZF_ASSERT(m_active);

            const t_i32 hash_index = KeyToHashIndex(key, m_hash_func, Cap());

            s_ptr<tp_val_type> val_cur;

            if (FindValueOfKeyInChain(m_kv_pair_blocks_head, hash_index, key, &val_cur)) {
                *val_cur = val;
                return ek_hash_map_put_result_updated;
            }

            InsertKVPair(m_kv_pair_blocks_head, key, val);
            m_entry_cnt++;
            return ek_hash_map_put_result_added;
        }

        // Returns true iff an entry with the key was found and removed.
        t_b8 Remove(const tp_key_type &key) {
            ZF_ASSERT(m_active);

            const t_len hash_index = KeyToHashIndex(key, m_hash_func, Cap());

            if (RemoveKVPairInChain(m_kv_pair_blocks_head, m_immediate_indexes[hash_index], m_immediate_indexes[hash_index], key)) {
                m_entry_cnt--;
                return true;
            }

            return false;
        }

        // Loads all key-value pairs into the given PRE-ALLOCATED arrays.
        void LoadEntries(const s_array<tp_key_type> keys, const s_array<tp_val_type> vals) const {
            ZF_ASSERT(m_active);
            ZF_ASSERT(keys.Len() >= m_entry_cnt && vals.Len() >= m_entry_cnt);

            t_i32 loaded_cnt = 0;

            for (t_len i = 0; i < m_immediate_indexes.Len(); i++) {
                loaded_cnt += LoadKVPairsInChain(m_kv_pair_blocks_head, m_immediate_indexes[i], keys.SliceFrom(loaded_cnt), vals.SliceFrom(loaded_cnt));
            }
        }

        // Allocates the given arrays with the memory arena and loads key-value pairs into them.
        void LoadEntries(s_mem_arena &mem_arena, s_array<tp_key_type> &o_keys, s_array<tp_val_type> &o_vals) const {
            ZF_ASSERT(m_active);

            o_keys = AllocArray<tp_key_type>(m_entry_cnt, mem_arena);
            o_vals = AllocArray<tp_val_type>(m_entry_cnt, mem_arena);
            return LoadEntries(o_keys, o_vals);
        }

    private:
        t_b8 m_active = false;

        t_hash_func<tp_key_type> m_hash_func = nullptr;
        t_bin_comparator<tp_key_type> m_key_comparator = nullptr;

        t_i32 m_entry_cnt = 0;

        s_array<t_i32> m_immediate_indexes;
        s_ptr<s_kv_pair_block<tp_key_type, tp_val_type>> m_kv_pair_blocks_head;

        s_ptr<s_mem_arena> m_mem_arena;

        template <typename tp_other_key_type, typename tp_other_val_type>
        friend s_hash_map<tp_other_key_type, tp_other_val_type> CreateHashMap(const t_hash_func<tp_other_key_type> hash_func, s_mem_arena &mem_arena, const t_len cap, const t_bin_comparator<tp_other_key_type> key_comparator);
    };

    // The provided hash function has to map a key to an integer 0 or higher. The given memory arena will be saved and used for allocating new memory for entries when needed.
    template <typename tp_key_type, typename tp_val_type>
    s_hash_map<tp_key_type, tp_val_type> CreateHashMap(const t_hash_func<tp_key_type> hash_func, s_mem_arena &mem_arena, const t_i32 cap = g_hash_map_cap_default, const t_bin_comparator<tp_key_type> key_comparator = DefaultBinComparator) {
        ZF_ASSERT(hash_func);
        ZF_ASSERT(key_comparator);

        static_assert(false);
        s_hash_map<tp_key_type, tp_val_type> hm;
#if 0
        hm.m_hash_func = hash_func;
        hm.m_key_comparator = key_comparator;
        hm.m_immediate_indexes = AllocArray<t_i32>(cap, mem_arena);
        hm.m_immediate_indexes.SetAllTo(-1);
        hm.m_backing_block_cap = AlignForward(cap, 8);
        hm.m_mem_arena = &mem_arena;
        hm.m_active = true;
#endif

        return hm;
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

        t_i32 cap = 0;

        if (!stream.ReadItem(cap)) {
            return false;
        }

        t_i32 entry_cnt = 0;

        if (!stream.ReadItem(entry_cnt)) {
            return false;
        }

        o_hm = CreateHashMap<tp_key_type, tp_val_type>(hm_hash_func, hm_mem_arena, cap, hm_key_comparator);

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

#if 0
    template <typename tp_key_type, typename tp_val_type>
    struct s_hash_map {
    public:
        s_hash_map() = default;

        t_b8 IsActive() const {
            return m_active;
        }

        t_i32 Cap() const {
            ZF_ASSERT(m_active);
            return m_immediate_indexes.Len();
        }

        t_i32 EntryCount() const {
            ZF_ASSERT(m_active);
            return m_entry_cnt;
        }

        // Returns true iff an entry with the key was found. Leave o_val as nullptr if you don't care about getting the value. o_val is untouched if the key is not found.
        [[nodiscard]] t_b8 Get(const tp_key_type &key, const s_ptr<tp_val_type> o_val = nullptr) const {
            ZF_ASSERT(m_active);

            const t_i32 hash_index = KeyToHashIndex(key, m_hash_func, Cap());

            t_i32 index = m_immediate_indexes[hash_index];

            while (index != -1) {
                const auto bb = FindBackingBlockOf(index);

                if (m_key_comparator(bb->keys[index], key)) {
                    if (o_val) {
                        *o_val = bb->vals[index];
                    }

                    return true;
                }

                index = bb->next_indexes[index];
            }

            return false;
        }

        // Returns true iff an entry with the key was found. Leave o_val as nullptr if you don't care about getting the value. o_val is untouched if the key is not found.
        [[nodiscard]] t_b8 Get(const tp_key_type &key, const s_ptr<tp_val_type> o_val = nullptr) const {
            ZF_ASSERT(m_active);

            const t_len hash_index = KeyToHashIndex(key, m_hash_func, Cap());

            s_ptr<s_backing_block> bb = m_backing_blocks_head;
            t_len index = m_immediate_indexes[hash_index];

            while (bb && index != -1) {
                while (index >= m_backing_block_cap) {
                    bb = bb->next;
                    index -= m_backing_block_cap;
                }

                if (m_key_comparator(bb->keys[index], key)) {
                    if (o_val) {
                        *o_val = bb->vals[index];
                    }

                    return true;
                }

                index = bb->next_indexes[index];
            }

            return false;
        }

        // Try adding the key-value pair to the hash map or just updating the value if the key is already present.
        e_hash_map_put_result Put(const tp_key_type &key, const tp_val_type &val) {
            ZF_ASSERT(m_active);

            const t_len hash_index = KeyToHashIndex(key, m_hash_func, Cap());

            s_ptr<s_backing_block> bb = m_backing_blocks_head;
            s_ptr<s_backing_block> bb_prev = nullptr;

            t_len index = m_immediate_indexes[hash_index];
            s_ptr<t_len> index_from_ptr = &m_immediate_indexes[hash_index];

            Log(s_cstr_literal("Immediate indexes on Put() start: %"), m_immediate_indexes);

            if (m_backing_blocks_head) {
                Log(s_cstr_literal("BB head \"nexts\" on Put() start: %"), m_backing_blocks_head->next_indexes);
            }

            while (true) {
                if (!bb) {
                    bb = &s_backing_block::Alloc(m_backing_block_cap, *m_mem_arena);

                    if (bb_prev) {
                        bb_prev->next = bb;
                    } else {
                        m_backing_blocks_head = bb;
                    }
                }

                if (index == -1) {
                    // We've reached the end of the chain without finding the key, so we need to create a new pair.
                    const t_len prospective_index = IndexOfFirstUnsetBit(bb->usage);

                    if (prospective_index == -1) {
                        // This block is full, so try again on the next.
                        bb_prev = bb;
                        bb = bb->next;
                        index_from_bb_dist = 0;
                        continue;
                    }

                    *index_from_ptr = +prospective_index; // @todo: This needs to apply the offset!

                    bb->keys[prospective_index] = key;
                    bb->vals[prospective_index] = val;
                    bb->next_indexes[prospective_index] = -1;
                    SetBit(bb->usage, prospective_index);

                    m_entry_cnt++;

                    return ek_hash_map_put_result_added;
                }

                while (index >= m_backing_block_cap) {
                    bb_prev = bb;
                    bb = bb->next;
                    index -= m_backing_block_cap;
                }

                if (m_key_comparator(bb->keys[index], key)) {
                    bb->vals[index] = val;
                    return ek_hash_map_put_result_updated;
                }

                index_from_ptr = &bb->next_indexes[index];
                index_from_bb_dist = 0;
                index = bb->next_indexes[index];
            }
        }

        // Returns true iff the key was found and removed.
        t_b8 Remove(const tp_key_type &key) {
            ZF_ASSERT(m_active);

            const t_len hash_index = KeyToHashIndex(key, m_hash_func, Cap());

            s_ptr<s_backing_block> bb = m_backing_blocks_head;

            t_len index = m_immediate_indexes[hash_index];
            s_ptr<t_len> index_from_ptr = &m_immediate_indexes[hash_index];

            while (bb && index != -1) {
                while (index >= m_backing_block_cap) {
                    bb = bb->next;
                    index -= m_backing_block_cap;
                }

                if (m_key_comparator(bb->keys[index], key)) {
                    *index_from_ptr = bb->next_indexes[index];
                    UnsetBit(bb->usage, index);
                    m_entry_cnt--;
                    return true;
                }

                index_from_ptr = &bb->next_indexes[index];
                index = bb->next_indexes[index];
            }

            return false;
        }

        // Loads key-value pairs into the given pre-allocated arrays.
        void LoadEntries(const s_array<tp_key_type> keys, const s_array<tp_val_type> vals) const {
            ZF_ASSERT(m_active);

            t_len entry_index = 0;

            for (t_len i = 0; i < m_immediate_indexes.Len(); i++) {
                s_ptr<const s_backing_block> bb = m_backing_blocks_head;
                t_len index = m_immediate_indexes[i];

                while (index != -1) {
                    while (index >= m_backing_block_cap) {
                        bb = bb->next;
                        index -= m_backing_block_cap;
                    }

                    keys[entry_index] = bb->keys[index];
                    vals[entry_index] = bb->vals[index];

                    entry_index++;

                    index = bb->next_indexes[index];
                }
            }
        }

        // Allocates the given arrays with the memory arena and loads key-value pairs into them.
        void LoadEntries(s_mem_arena &mem_arena, s_array<tp_key_type> &o_keys, s_array<tp_val_type> &o_vals) const {
            ZF_ASSERT(m_active);

            o_keys = AllocArray<tp_key_type>(m_entry_cnt, mem_arena);
            o_vals = AllocArray<tp_val_type>(m_entry_cnt, mem_arena);
            return LoadEntries(o_keys, o_vals);
        }

    private:
        s_hash_map(const s_hash_map &) = default;

        struct s_backing_block {
            static s_backing_block &Alloc(const t_i32 cap, s_mem_arena &mem_arena) {
                auto &bb = zf::Alloc<s_backing_block>(mem_arena);
                bb.keys = AllocArray<tp_key_type>(cap, mem_arena);
                bb.vals = AllocArray<tp_val_type>(cap, mem_arena);
                bb.next_indexes = AllocArray<t_i32>(cap, mem_arena);
                bb.next_indexes.SetAllTo(-1);
                bb.usage = AllocBitVec(cap, mem_arena);
                return bb;
            }

            s_array<tp_key_type> keys;
            s_array<tp_val_type> vals;
            s_array<t_i32> next_indexes; // -1 means no "next", and a number greater than the BB capacity is referencing a pair on a later block.
            s_bit_vec usage;

            s_ptr<s_backing_block> next;
        };

        static s_ptr<s_backing_block> FindNextBackingBlockOf(s_ptr<s_backing_block> bb, t_i32 index) {
            while (index >= m_backing_block_cap) {
                bb = bb->next;
                index -= m_backing_block_cap;
            }

            return bb;
        }

        t_b8 m_active = false;

        t_hash_func<tp_key_type> m_hash_func = nullptr;
        t_bin_comparator<tp_key_type> m_key_comparator = nullptr;

        t_i32 m_entry_cnt = 0;

        s_array<t_i32> m_immediate_indexes;

        t_i32 m_backing_block_cap = 0;
        s_ptr<s_backing_block> m_backing_blocks_head;

        s_ptr<s_mem_arena> m_mem_arena;

        template <typename tp_other_key_type, typename tp_other_val_type>
        friend s_hash_map<tp_other_key_type, tp_other_val_type> CreateHashMap(const t_hash_func<tp_other_key_type> hash_func, s_mem_arena &mem_arena, const t_len cap, const t_bin_comparator<tp_other_key_type> key_comparator);
    };

    // The provided hash function has to map a key to an integer 0 or higher. The given memory arena will be saved and used for allocating new memory for entries when needed.
    template <typename tp_key_type, typename tp_val_type>
    s_hash_map<tp_key_type, tp_val_type> CreateHashMap(const t_hash_func<tp_key_type> hash_func, s_mem_arena &mem_arena, const t_i32 cap = g_hash_map_cap_default, const t_bin_comparator<tp_key_type> key_comparator = DefaultBinComparator) {
        ZF_ASSERT(hash_func);
        ZF_ASSERT(key_comparator);

        s_hash_map<tp_key_type, tp_val_type> hm;
        hm.m_hash_func = hash_func;
        hm.m_key_comparator = key_comparator;
        hm.m_immediate_indexes = AllocArray<t_i32>(cap, mem_arena);
        hm.m_immediate_indexes.SetAllTo(-1);
        hm.m_backing_block_cap = AlignForward(cap, 8);
        hm.m_mem_arena = &mem_arena;
        hm.m_active = true;

        return hm;
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

        t_i32 cap = 0;

        if (!stream.ReadItem(cap)) {
            return false;
        }

        t_i32 entry_cnt = 0;

        if (!stream.ReadItem(entry_cnt)) {
            return false;
        }

        o_hm = CreateHashMap<tp_key_type, tp_val_type>(hm_hash_func, hm_mem_arena, cap, hm_key_comparator);

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
#endif
}
