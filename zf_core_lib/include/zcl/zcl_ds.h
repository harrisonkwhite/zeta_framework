#pragma once

#include <zcl/zcl_io.h>

namespace zf {
    template <typename tp_type>
    struct s_list {
        static_assert(!s_is_const<tp_type>::g_val);

    public:
        s_list() = default;

        explicit s_list(const s_array<tp_type> backing_arr, const t_i32 len = 0) : m_backing_arr(backing_arr), m_len(len) {
            ZF_ASSERT(len >= 0 && len <= backing_arr.Len());
        }

        s_list(const s_list &) = delete;

        s_array<tp_type> BackingArray() {
            return m_backing_arr;
        }

        t_i32 Len() const {
            return m_len;
        }

        t_i32 Cap() const {
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

        tp_type &operator[](const t_i32 index) const {
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

        void Insert(const t_i32 index, const tp_type &val) {
            ZF_ASSERT(!IsFull());
            ZF_ASSERT(index >= 0 && index <= m_len);

            for (t_i32 i = m_len; i > index; i--) {
                m_backing_arr[i] = m_backing_arr[i - 1];
            }

            m_len++;
            m_backing_arr[index] = val;
        }

        void Remove(const t_i32 index) {
            ZF_ASSERT(!IsEmpty());
            ZF_ASSERT(index >= 0 && index < m_len);

            m_backing_arr.Slice(index + 1, m_len).CopyTo(m_backing_arr.Slice(index, m_len - 1));
            m_len--;
        }

        void RemoveSwapback(const t_i32 index) {
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
        t_i32 m_len = 0;
    };

    template <typename tp_type, t_i32 tp_cap>
    struct s_static_list : public s_list<tp_type> {
    public:
        s_static_list() : s_list<tp_type>(m_backing_arr) {}

    private:
        s_static_array<tp_type, tp_cap> m_backing_arr = {};
    };

    template <typename tp_type>
    [[nodiscard]] t_b8 CreateList(const t_i32 cap, s_mem_arena &mem_arena, s_list<tp_type> &o_list, const t_i32 len = 0) {
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

        explicit s_stack(const s_array<tp_type> backing_arr, const t_i32 height = 0) : m_backing_arr(backing_arr), m_height(height) {
            ZF_ASSERT(height >= 0 && height <= backing_arr.Len());
        }

        s_stack(const s_stack &) = delete;

        s_array<tp_type> BackingArray() {
            return m_backing_arr;
        }

        t_i32 Height() const {
            return m_height;
        }

        t_i32 Cap() const {
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

        tp_type &operator[](const t_i32 index) const {
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
        t_i32 m_height = 0;
    };

    template <typename tp_type, t_i32 tp_cap>
    struct s_static_stack : public s_stack<tp_type> {
    public:
        s_static_stack() : s_stack<tp_type>(m_backing_arr) {}

    private:
        s_static_array<tp_type, tp_cap> m_backing_arr = {};
    };

    template <typename tp_type>
    [[nodiscard]] t_b8 CreateStack(const t_i32 cap, s_mem_arena &mem_arena, s_stack<tp_type> &o_stack, const t_i32 height = 0) {
        ZF_ASSERT(cap > 0 && height >= 0 && height <= cap);
        o_stack = {AllocArray<tp_type>(cap, mem_arena), height};
        return true;
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

        s_kv_pair_block_seq(const t_i32 block_cap, s_mem_arena &blocks_mem_arena, const t_bin_comparator<tp_key_type> key_comparator = DefaultBinComparator) : m_active(true), m_block_cap(block_cap), m_blocks_mem_arena(&blocks_mem_arena), m_key_comparator(key_comparator) {
            ZF_ASSERT(key_comparator);
        };

        // s_kv_pair_block_seq(const s_kv_pair_block_seq &) = delete;

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
            block.next_indexes.SetAllTo(-1);

            block.usage = AllocBitVec(cap, mem_arena);

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
    public:
        s_hash_map() = default;

        // s_hash_map(const s_hash_map &) = delete;
        // s_hash_map &operator=(const s_hash_map &) = delete;
        // @todo: What to do for this above?

        t_b8 IsActive() const {
            return m_active;
        }

        t_i32 Cap() const {
            ZF_ASSERT(m_active);
            return m_immediate_indexes.Len();
        }

        t_i32 EntryCount() const {
            ZF_ASSERT(m_active);
            return m_kv_pair_block_seq.PairCount();
        }

        [[nodiscard]] t_b8 Find(const tp_key_type &key, s_ptr<tp_val_type> &o_val) const {
            ZF_ASSERT(m_active);

            const t_i32 hash_index = KeyToHashIndex(key, m_hash_func, Cap());
            return m_kv_pair_block_seq.FindInChain(m_immediate_indexes[hash_index], key, o_val);
        }

        t_b8 Exists(const tp_key_type &key) const {
            ZF_ASSERT(m_active);

            const t_i32 hash_index = KeyToHashIndex(key, m_hash_func, Cap());
            return m_kv_pair_block_seq.ExistsInChain(m_immediate_indexes[hash_index], key);
        }

        // Try adding the key-value pair to the hash map or just updating the value if the key is already present.
        e_hash_map_put_result Put(const tp_key_type &key, const tp_val_type &val) {
            ZF_ASSERT(m_active);

            const t_i32 hash_index = KeyToHashIndex(key, m_hash_func, Cap());
            return m_kv_pair_block_seq.PutInChain(m_immediate_indexes[hash_index], key, val) == ek_kv_pair_block_seq_put_result_updated ? ek_hash_map_put_result_updated : ek_hash_map_put_result_added;
        }

        // Returns true iff an entry with the key was found and removed.
        t_b8 Remove(const tp_key_type &key) {
            ZF_ASSERT(m_active);

            const t_i32 hash_index = KeyToHashIndex(key, m_hash_func, Cap());
            return m_kv_pair_block_seq.RemoveInChain(m_immediate_indexes[hash_index], key);
        }

        // Loads all key-value pairs into the given PRE-ALLOCATED arrays.
        void LoadEntries(const s_array<tp_key_type> keys, const s_array<tp_val_type> vals) const {
            ZF_ASSERT(m_active);
            ZF_ASSERT(keys.Len() >= EntryCount() && vals.Len() >= EntryCount());

            t_i32 loaded_cnt = 0;

            for (t_i32 i = 0; i < m_immediate_indexes.Len(); i++) {
                loaded_cnt += m_kv_pair_block_seq.LoadChain(m_immediate_indexes[i], keys.SliceFrom(loaded_cnt), vals.SliceFrom(loaded_cnt));
            }
        }

        // Allocates the given arrays with the memory arena and loads key-value pairs into them.
        void LoadEntries(s_mem_arena &mem_arena, s_array<tp_key_type> &o_keys, s_array<tp_val_type> &o_vals) const {
            ZF_ASSERT(m_active);

            o_keys = AllocArray<tp_key_type>(EntryCount(), mem_arena);
            o_vals = AllocArray<tp_val_type>(EntryCount(), mem_arena);
            return LoadEntries(o_keys, o_vals);
        }

    private:
        t_b8 m_active = false;

        t_hash_func<tp_key_type> m_hash_func = nullptr;

        s_array<t_i32> m_immediate_indexes = {};
        s_kv_pair_block_seq<tp_key_type, tp_val_type> m_kv_pair_block_seq = {};

        template <typename tp_other_key_type, typename tp_other_val_type>
        friend s_hash_map<tp_other_key_type, tp_other_val_type> CreateHashMap(const t_hash_func<tp_other_key_type> hash_func, s_mem_arena &mem_arena, const t_i32 cap, const t_bin_comparator<tp_other_key_type> key_comparator);
    };

    // The provided hash function has to map a key to an integer 0 or higher. The given memory arena will be saved and used for allocating new memory for entries when needed.
    template <typename tp_key_type, typename tp_val_type>
    s_hash_map<tp_key_type, tp_val_type> CreateHashMap(const t_hash_func<tp_key_type> hash_func, s_mem_arena &mem_arena, const t_i32 cap = g_hash_map_cap_default, const t_bin_comparator<tp_key_type> key_comparator = DefaultBinComparator) {
        ZF_ASSERT(hash_func);
        ZF_ASSERT(key_comparator);

        s_hash_map<tp_key_type, tp_val_type> hm;
        hm.m_active = true;
        hm.m_hash_func = hash_func;
        hm.m_immediate_indexes = AllocArray<t_i32>(cap, mem_arena);
        hm.m_immediate_indexes.SetAllTo(-1);
        hm.m_kv_pair_block_seq = {static_cast<t_i32>(AlignForward(cap, 8)), mem_arena, key_comparator};

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
}
