#pragma once

#include <zcl/zcl_io.h>

namespace zf {
    template <typename tp_type>
    struct s_list {
        static_assert(!s_is_const<tp_type>::g_val);

    public:
        s_list() = default;

        s_list(const s_array<tp_type> backing_arr, const t_len len) : m_backing_arr(backing_arr), m_len(len) {
            ZF_ASSERT(len >= 0 && len <= backing_arr.len);
        }

        s_array<tp_type> BackingArray() {
            return m_backing_arr;
        }

        t_len Len() const {
            return m_len;
        }

        t_len Cap() const {
            return m_backing_arr.len;
        }

        t_b8 IsEmpty() const {
            return m_len == 0;
        }

        t_b8 IsFull() const {
            return m_len == m_backing_arr.len;
        }

        tp_type &operator[](const t_len index) const {
            ZF_ASSERT(index >= 0 && index < m_len);
            return m_backing_arr[index];
        }

        tp_type &Last() const {
            return operator[](m_len - 1);
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
                m_backing_arr[m_len] = m_backing_arr[m_len - 1];
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
            ZF_ASSERT(m_len > 0 && m_len <= m_backing_arr.len);
            ZF_ASSERT(index >= 0 && index < m_len);

            m_backing_arr[index] = m_backing_arr[m_len - 1];
            m_len--;
        }

        tp_type RemoveLast() {
            ZF_ASSERT(m_len > 0 && m_len <= m_backing_arr.len);

            m_len--;
            return m_backing_arr[m_len];
        }

    private:
        s_array<tp_type> m_backing_arr = {};
        t_len m_len = 0;
    };

    template <typename tp_type>
    [[nodiscard]] t_b8 CreateList(const t_len cap, s_mem_arena &mem_arena, s_list<tp_type> &o_list, const t_len len = 0) {
        ZF_ASSERT(cap > 0 && len >= 0 && len <= cap);

        s_array<tp_type> backing_arr;

        if (!AllocArray(cap, mem_arena, backing_arr)) {
            return false;
        }

        o_list = {backing_arr, len};

        return true;
    }

    // ============================================================
    // @section: Hash Map
    // ============================================================
    template <typename tp_type>
    using t_hash_func = t_len (*)(const tp_type &key);

    // This is an FNV-1a implementation.
    constexpr t_hash_func<s_str_rdonly> g_str_hash_func =
        [](const s_str_rdonly &key) constexpr {
            const t_u32 offs_basis = 2166136261u;
            const t_u32 prime = 16777619u;

            t_u32 hash = offs_basis;

            for (t_len i = 0; i < key.bytes.Len(); i++) {
                hash ^= static_cast<t_u8>(key.bytes[i]);
                hash *= prime;
            }

            return static_cast<t_len>(static_cast<t_u64>(hash) & 0x7FFFFFFFFFFFFFFFull);
        };

    template <typename tp_type>
    t_len KeyToHashIndex(const tp_type &key, const t_hash_func<tp_type> hash_func, const t_len cap) {
        ZF_ASSERT(hash_func);
        ZF_ASSERT(cap > 0);

        const t_len val = hash_func(key);
        ZF_ASSERT(val >= 0);

        return val % cap;
    }

    enum e_hash_map_put_result : t_i32 {
        ek_hash_map_put_result_error,
        ek_hash_map_put_result_added,
        ek_hash_map_put_result_updated
    };

    template <typename tp_key_type, typename tp_val_type>
    struct s_hash_map {
    public:
        // The provided hash function has to map a key to an integer 0 or higher. The given memory arena will be saved and used for allocating new memory for entries when needed.
        [[nodiscard]] t_b8 Init(const t_hash_func<tp_key_type> hash_func, s_mem_arena &mem_arena, const t_len cap = 32, const t_bin_comparator<tp_key_type> key_comparator = DefaultBinComparator) {
            ZF_ASSERT(!m_initted);

            *this = {
                .m_hash_func = hash_func,
                .m_key_comparator = key_comparator,
                .m_backing_block_cap = AlignForward(cap, 8),
                .m_mem_arena = mem_arena,
            };

            if (!AllocArray(cap, mem_arena, m_immediate_indexes)) {
                return false;
            }

            m_immediate_indexes.SetAllTo(-1);

            m_initted = true;

            return true;
        }

        t_b8 IsInitted() const {
            return m_initted;
        }

        t_len Cap() const {
            ZF_ASSERT(m_initted);
            return m_immediate_indexes.Len();
        }

        t_len EntryCount() const {
            ZF_ASSERT(m_initted);
            return m_entry_cnt;
        }

        // Returns true iff an entry with the key was found. Leave o_val as nullptr if you don't care about getting the value. o_val is untouched if the key is not found.
        [[nodiscard]] t_b8 Get(const tp_key_type &key, tp_val_type *const o_val = nullptr) {
            ZF_ASSERT(m_initted);

            const t_len hash_index = KeyToHashIndex(key, m_hash_func, Cap());

            s_backing_block *bb = m_backing_blocks_head;
            t_len index = m_immediate_indexes[hash_index];

            while (bb && index != -1) {
                while (index >= m_backing_block_cap) {
                    bb = bb->next;
                    index -= m_backing_block_cap;
                }

                if (key_comparator(bb->keys[index], key)) {
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
        [[nodiscard]] e_hash_map_put_result Put(const tp_key_type &key, const tp_val_type &val) {
            ZF_ASSERT(m_initted);

            const t_len hash_index = KeyToHashIndex(key, m_hash_func, Cap());

            s_backing_block *bb = m_backing_blocks_head;
            s_backing_block *bb_prev = nullptr;

            t_len index = m_immediate_indexes[hash_index];
            t_len *const index_to_update = nullptr;

            while (true) {
                if (!bb) {
                    // Try creating a new backing block.
                    bb = Alloc<s_backing_block>(m_mem_arena);

                    if (!bb || !bb->Init(m_backing_block_cap, m_mem_arena)) {
                        return ek_hash_map_put_result_error;
                    }

                    if (bb_prev) {
                        bb_prev->next = bb;
                    }
                }

                if (index == -1) {
                    // We've reached the end of the chain without finding the key, so we need to create a new pair.
                    if (!index_to_update) {
                        index_to_update = &bb->next_indexes[index];
                    }

                    const t_len prospective_index = IndexOfFirstUnsetBit(bb->usage);

                    if (prospective_index == -1) {
                        // This block is full, so try again on the next.
                        bb_prev = bb;
                        bb = bb->next;
                        continue;
                    }

                    *index_to_update = prospective_index;

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

                if (key_comparator(bb->keys[index], key)) {
                    bb->vals[index] = val;
                    return ek_hash_map_put_result_updated;
                }

                index = bb->next_indexes[index];
            }
        }

        // Returns true iff the key was found and removed.
        t_b8 Remove(const tp_key_type &key) {
            ZF_ASSERT(m_initted);

            const t_len hash_index = KeyToHashIndex(key, m_hash_func, Cap());

            s_backing_block *bb = m_backing_blocks_head;

            t_len *index = &m_immediate_indexes[hash_index];

            while (bb && *index != -1) {
                while (*index >= m_backing_block_cap) {
                    bb = bb->next;
                    index -= m_backing_block_cap;
                }

                if (key_comparator(bb->keys[index], key)) {
                    *index = bb->next_indexes[index];
                    UnsetBit(bb->usage, index);
                    m_entry_cnt--;
                    return true;
                }

                index = &bb->next_indexes[index];
            }

            return false;
        }

        // Loads key-value pairs into the given pre-allocated arrays.
        [[nodiscard]] t_b8 LoadEntries(const s_array<tp_key_type> keys, const s_array<tp_val_type> vals) const {
            ZF_ASSERT(m_initted);

            t_len entry_index = 0;

            for (t_len i = 0; i < m_immediate_indexes.Len(); i++) {
                const s_backing_block *bb = m_backing_blocks_head;
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

            return true;
        }

        // Allocates the given arrays with the memory arena and loads key-value pairs into them.
        [[nodiscard]] t_b8 LoadEntries(s_mem_arena &mem_arena, s_array<tp_key_type> &o_keys, s_array<tp_val_type> &o_vals) const {
            ZF_ASSERT(m_initted);

            if (!AllocArray(m_entry_cnt, mem_arena, o_keys) || !AllocArray(m_entry_cnt, mem_arena, o_vals)) {
                return false;
            }

            return LoadEntries(o_keys, o_vals);
        }

    private:
        struct s_backing_block {
            s_array<tp_key_type> keys = {};
            s_array<tp_val_type> vals = {};
            s_array<t_len> next_indexes = {}; // -1 means no "next", and a number greater than the BB capacity is referencing a pair on a later block.
            s_bit_vec usage = {};

            s_backing_block *next = nullptr;

            [[nodiscard]] t_b8 Init(const t_len cap, s_mem_arena &mem_arena) {
                if (!AllocArray(cap, mem_arena, keys)) {
                    return false;
                }

                if (!AllocArray(cap, mem_arena, vals)) {
                    return false;
                }

                if (!AllocArray(cap, mem_arena, next_indexes)) {
                    return false;
                }

                if (!CreateBitVec(cap, mem_arena, usage)) {
                    return false;
                }

                return true;
            }
        };

        t_b8 m_initted = false;

        t_hash_func<tp_key_type> m_hash_func = nullptr;
        t_bin_comparator<tp_key_type> m_key_comparator = nullptr;

        t_len m_entry_cnt = 0;

        s_array<t_len> m_immediate_indexes = {};

        t_len m_backing_block_cap = 0;
        s_backing_block *m_backing_blocks_head = nullptr;

        s_mem_arena *m_mem_arena = nullptr;
    };

    template <typename tp_key_type, typename tp_val_type>
    [[nodiscard]] t_b8 SerializeHashMap(s_stream &stream, const s_hash_map<tp_key_type, tp_val_type> &hm, s_mem_arena &temp_mem_arena) {
        const t_len cap = hm.Cap();

        if (!stream.WriteItem(cap)) {
            return false;
        }

        const t_len entry_cnt = hm.EntryCount();

        if (!stream.WriteItem(entry_cnt)) {
            return false;
        }

        s_array<tp_key_type> keys;
        s_array<tp_val_type> vals;

        if (!hm.LoadEntries(temp_mem_arena, keys, vals)) {
            return false;
        }

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
        t_len cap;

        if (!stream.ReadItem(cap)) {
            return false;
        }

        t_len entry_cnt;

        if (!stream.ReadItem(entry_cnt)) {
            return false;
        }

        if (!o_hm->Init(hm_hash_func, hm_mem_arena, cap, hm_key_comparator)) {
            return false;
        }

        s_array<tp_key_type> keys;
        s_array<tp_val_type> vals;

        if (!AllocArray(entry_cnt, temp_mem_arena, keys) || !AllocArray(entry_cnt, temp_mem_arena, vals)) {
            return false;
        }

        if (!stream.ReadItemsIntoArray(keys, entry_cnt)) {
            return false;
        }

        if (!stream.ReadItemsIntoArray(vals, entry_cnt)) {
            return false;
        }

        for (t_len i = 0; i < entry_cnt; i++) {
            if (o_hm->Put(keys[i], vals[i]) == ek_hash_map_put_result_error) {
                return false;
            }
        }

        return true;
    }
}
