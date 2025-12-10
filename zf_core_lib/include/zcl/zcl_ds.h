#pragma once

#include <zcl/zcl_mem.h>

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

        tp_type *Append(const tp_type &val) {
            ZF_ASSERT(!IsFull());

            m_backing_arr[m_len] = val;
            m_len++;
            return &m_backing_arr[m_len - 1];
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
    [[nodiscard]] t_b8 CreateList(const t_len cap, s_mem_arena *const mem_arena, s_list<tp_type> *const o_list, const t_len len = 0) {
        ZF_ASSERT(cap > 0 && len >= 0 && len <= cap);

        s_array<tp_type> backing_arr;

        if (!AllocArray(cap, mem_arena, &backing_arr)) {
            return false;
        }

        *o_list = {backing_arr, len};

        return true;
    }

    // ============================================================
    // @section: Hash Map
    // ============================================================
#if 0
    template <typename tp_type>
    using t_hash_func = t_len (*)(const tp_type &key);

    enum class e_hash_map_put_result : t_i32 {
        error,
        added,
        updated
    };

    template <typename tp_type>
    t_len KeyToHashIndex(const tp_type &key, const t_hash_func<tp_type> hash_func, const t_len table_size) {
        ZF_ASSERT(hash_func);
        ZF_ASSERT(table_size > 0);

        const t_len val = hash_func(key);
        ZF_ASSERT(val >= 0);

        return val % table_size;
    }

    constexpr t_len g_hash_map_immediate_cap_default = 32;
    constexpr t_len g_hash_map_backing_block_cap_default = 32;

    template <typename tp_key_type, typename tp_val_type>
    struct s_hash_map {
    public:
        // The provided hash function has to map a key to an integer 0 or higher. The immediate capacity is the total number of upfront slots (i.e. the maximum possible number of slots for which an O(1) access of a value from a key can happen). The given memory arena will be used also for allocating new backing blocks when they're needed.
        [[nodiscard]] t_b8 Init(const t_hash_func<tp_key_type> hash_func, s_mem_arena *const mem_arena, const t_len immediate_cap = g_hash_map_immediate_cap_default, const t_len backing_block_cap = g_hash_map_backing_block_cap_default, const t_bin_comparator<tp_key_type> key_comparator = DefaultBinComparator) {
            ZF_ASSERT(!m_initted);

            *this = {
                .m_hash_func = hash_func,
                .m_key_comparator = key_comparator,
                .m_backing_block_cap = backing_block_cap,
                .m_mem_arena = mem_arena,
            };

    #if 0
            if (!AllocArray(, immediate_cap, &mem_arena, &m_immediate_indexes)) {
                return false;
            }

            SetAllTo(m_immediate_indexes, static_cast<t_len>(-1));

            m_initted = true;
    #endif

            return true;
        }

        [[nodiscard]] t_b8 Get(const tp_key_type &key, tp_val_type *const o_val = nullptr) {
            const t_len hash_index = KeyToHashIndex(key, hm.hash_func, hm.immediate_indexes.len);
            return HashMapBackingBlockGet(hm.backing_blocks_head, key, hm.key_comparator, hm.immediate_indexes[hash_index], o_val);
        }

    #if 0
        // Returns true iff no error occurred.
        [[nodiscard]] e_hash_map_put_result Put(const tp_key_type &key, const tp_val_type &val) {
            const t_len hash_index = KeyToHashIndex(key, hm.hash_func, hm.immediate_indexes.len);

            const auto res = HashMapBackingBlockPut(hm.backing_blocks_head, key, hm.key_comparator, val, hm.immediate_indexes[hash_index], hm.backing_block_cap, *hm.mem_arena);

            if (res == ek_hash_map_put_result_added) {
                hm.kv_pair_cnt++;
            }

            return res;
        }

        // Returns true iff the key was found and removed.
        t_b8 Remove(const tp_key_type key) {
            const t_len hash_index = KeyToHashIndex(key, hm.hash_func, hm.immediate_indexes.len);

            if (HashMapBackingBlockRemove(hm.backing_blocks_head, key, hm.immediate_indexes[hash_index])) {
                hm.kv_pair_cnt--;
                return true;
            }

            return false;
        }
    #endif

    private:
        struct s_backing_block {
            s_array<tp_key_type> keys = {};
            s_array<tp_val_type> vals = {};
            s_array<t_len> next_indexes = {};
            s_bit_vec usage = {};

            s_backing_block *next = nullptr;
        };

        t_b8 m_initted = false;

        t_hash_func<tp_key_type> m_hash_func = nullptr;
        t_bin_comparator<tp_key_type> m_key_comparator = nullptr;

        t_len m_kv_pair_cnt = 0;

        s_array<t_len> m_immediate_indexes = {};

        t_len m_backing_block_cap = 0;
        s_backing_block *m_backing_blocks_head = nullptr;

        s_mem_arena *m_mem_arena = nullptr;
    }
#endif

#if 0
    template <typename tp_type>
    using t_hash_func = t_len (*)(const tp_type &key);

    // This is an FNV-1a implementation.
    constexpr t_hash_func<s_str_rdonly> g_str_hash_func =
        [](const s_str_rdonly &key) constexpr {
            const t_u32 offs_basis = 2166136261u;
            const t_u32 prime = 16777619u;

            t_u32 hash = offs_basis;

            for (t_len i = 0; i < key.bytes.len; i++) {
                hash ^= key.bytes[i];
                hash *= prime;
            }

            return static_cast<t_len>(static_cast<t_u64>(hash) & 0x7FFFFFFFFFFFFFFFull);
        };

    template <typename tp_key_type, typename tp_val_type>
    struct s_hash_map_backing_block {
        s_array<tp_key_type> keys;
        s_array<tp_val_type> vals;
        s_array<t_len> next_indexes;

        s_bit_vec usage;

        s_hash_map_backing_block *next;
    };

    template <typename tp_key_type, typename tp_val_type>
    [[nodiscard]] t_b8 InitHashMapBackingBlock(
        s_hash_map_backing_block<tp_key_type, tp_val_type> *bb, const t_len cap,
        s_mem_arena *const mem_arena) {
        ZF_ASSERT(cap >= 0);

        ZeroOut(bb);

        if (cap == 0) {
            return true; // @todo: Should be able to remove this once InitArray()
                         // accepts 0.
        }

        if (!InitArray(&bb->keys, cap, mem_arena)) {
            return false;
        }

        if (!InitArray(&bb->vals, cap, mem_arena)) {
            return false;
        }

        if (!InitArray(&bb->next_indexes, cap, mem_arena)) {
            return false;
        }

        if (!InitBitVec(&bb->usage, cap, mem_arena)) {
            return false;
        }

        return true;
    }

    template <typename tp_key_type, typename tp_val_type>
    t_len HashMapBackingBlockCap(
        const s_hash_map_backing_block<tp_key_type, tp_val_type> *const bb) {
        return bb->keys.len;
    }

    template <typename tp_key_type, typename tp_val_type>
    [[nodiscard]] t_b8 HashMapBackingBlockGet(
        const s_hash_map_backing_block<tp_key_type, tp_val_type> *bb, const tp_key_type key,
        const t_bin_comparator<tp_key_type> key_comparator, t_len index,
        tp_val_type *const o_val) {
        if (!bb || index == -1) {
            return false;
        }

        ZF_ASSERT(index >= 0);

        while (index >= HashMapBackingBlockCap(bb)) {
            ZF_ASSERT(bb->next);
            bb = bb->next;
            index -= HashMapBackingBlockCap(bb);
        }

        if (key_comparator(bb->keys[index], key)) {
            if (o_val) {
                *o_val = bb->vals[index];
            }

            return true;
        }

        return HashMapBackingBlockGet(bb, key, key_comparator, bb->next_indexes[index], o_val);
    }

    enum e_hash_map_put_result : t_i32 {
        ek_hash_map_put_result_error,
        ek_hash_map_put_result_added,
        ek_hash_map_put_result_updated
    };

    template <typename tp_key_type, typename tp_val_type>
    [[nodiscard]] e_hash_map_put_result HashMapBackingBlockPut(
        s_hash_map_backing_block<tp_key_type, tp_val_type> *bb, const tp_key_type key,
        const t_bin_comparator<tp_key_type> key_comparator, const tp_val_type val,
        t_len &index, const t_len new_bb_cap, s_mem_arena &new_bb_mem_arena) {
        ZF_ASSERT(index >= -1);

        if (!bb) {
            bb = PushToMemArena<s_hash_map_backing_block<tp_key_type, tp_val_type>>(
                &new_bb_mem_arena);

            if (!bb || !InitHashMapBackingBlock(bb, new_bb_cap, &new_bb_mem_arena)) {
                return ek_hash_map_put_result_error;
            }
        }

        if (index == -1) {
            const t_len prospective_index = IndexOfFirstUnsetBit(bb->usage);

            if (prospective_index == -1) {
                return HashMapBackingBlockPut(bb->next, key, key_comparator, val, index,
                                              new_bb_cap, new_bb_mem_arena);
            }

            index = prospective_index;

            bb->keys[index] = key;
            bb->vals[index] = val;
            bb->next_indexes[index] = -1;
            SetBit(bb->usage, index);

            return ek_hash_map_put_result_added;
        }

        t_len index_copy = index;

        while (index_copy >= HashMapBackingBlockCap(bb)) {
            ZF_ASSERT(bb->next);
            bb = bb->next;
            index_copy -= HashMapBackingBlockCap(bb);
        }

        if (key_comparator(bb->keys[index_copy], key)) {
            bb->vals[index_copy] = val;
            return ek_hash_map_put_result_updated;
        }

        return HashMapBackingBlockPut(bb, key, key_comparator, val,
                                      bb->next_indexes[index_copy], new_bb_cap,
                                      new_bb_mem_arena);
    }

    template <typename tp_key_type, typename tp_val_type>
    t_b8 HashMapBackingBlockRemove(s_hash_map_backing_block<tp_key_type, tp_val_type> *bb,
                                   const tp_key_type key,
                                   const t_bin_comparator<tp_key_type> key_comparator,
                                   t_len &index) {
        ZF_ASSERT(index >= -1);

        if (!bb || index == -1) {
            return false;
        }

        t_len index_copy = index;

        while (index_copy >= HashMapBackingBlockCap(bb)) {
            ZF_ASSERT(bb->next);
            bb = bb->next;
            index_copy -= HashMapBackingBlockCap(bb);
        }

        if (key_comparator(bb->keys[index_copy], key)) {
            index = bb->next_indexes[index_copy];
            UnsetBit(bb->usage, index_copy);
            return true;
        }

        return HashMapBackingBlockRemove(bb, key, bb->next_indexes[index_copy]);
    }

    template <typename tp_key_type, typename tp_val_type>
    struct s_hash_map {
        t_hash_func<tp_key_type> hash_func;
        t_bin_comparator<tp_key_type> key_comparator;

        t_len kv_pair_cnt;

        s_array<t_len> immediate_indexes;

        t_len backing_block_cap;
        s_hash_map_backing_block<tp_key_type, tp_val_type> *backing_blocks_head;

        s_mem_arena *mem_arena;
    };

    constexpr t_len g_hash_map_immediate_cap_default = 32;
    constexpr t_len g_hash_map_backing_block_cap_default = 32;

    // The provided hash function has to map a key to an integer 0 or higher.
    // The immediate capacity is the total number of upfront slots (i.e. the maximum
    // possible number of slots for which an O(1) access of a value from a key can
    // happen). The given memory arena will be used also for allocating new backing blocks when
    // they're needed.
    template <typename tp_key_type, typename tp_val_type>
    [[nodiscard]] t_b8 InitHashMap(
        s_hash_map<tp_key_type, tp_val_type> *const hm,
        const t_hash_func<tp_key_type> hash_func, s_mem_arena &mem_arena,
        const t_len immediate_cap = g_hash_map_immediate_cap_default,
        const t_len backing_block_cap = g_hash_map_backing_block_cap_default,
        const t_bin_comparator<tp_key_type> key_comparator = DefaultBinComparator) {
        ZeroOut(hm);

        m_hash_func = hash_func;
        m_key_comparator = key_comparator;
        m_backing_block_cap = backing_block_cap;
        m_mem_arena = &mem_arena;

        if (!InitArray(&m_immediate_indexes, immediate_cap, &mem_arena)) {
            return false;
        }

        SetAllTo(m_immediate_indexes, static_cast<t_len>(-1));

        return true;
    }

    template <typename tp_type>
    t_len KeyToHashIndex(const tp_type &key, const t_hash_func<tp_type> hash_func,
                          const t_len table_size) {
        ZF_ASSERT(hash_func);
        ZF_ASSERT(table_size > 0);

        const t_len val = hash_func(key);
        ZF_ASSERT(val >= 0);

        return val % table_size;
    }

    template <typename tp_key_type, typename tp_val_type>
    [[nodiscard]] t_b8 HashMapGet(const s_hash_map<tp_key_type, tp_val_type> &hm,
                                  const tp_key_type &key, tp_val_type *const o_val = nullptr) {
        const t_len hash_index = KeyToHashIndex(key, hm.hash_func, hm.immediate_indexes.len);
        return HashMapBackingBlockGet(hm.backing_blocks_head, key, hm.key_comparator,
                                      hm.immediate_indexes[hash_index], o_val);
    }

    // Returns true iff no error occurred.
    template <typename tp_key_type, typename tp_val_type>
    [[nodiscard]] e_hash_map_put_result HashMapPut(s_hash_map<tp_key_type, tp_val_type> &hm,
                                                   const tp_key_type &key,
                                                   const tp_val_type &val) {
        const t_len hash_index = KeyToHashIndex(key, hm.hash_func, hm.immediate_indexes.len);

        const auto res = HashMapBackingBlockPut(hm.backing_blocks_head, key, hm.key_comparator,
                                                val, hm.immediate_indexes[hash_index],
                                                hm.backing_block_cap, *hm.mem_arena);

        if (res == ek_hash_map_put_result_added) {
            hm.kv_pair_cnt++;
        }

        return res;
    }

    // Returns true iff the key was found and removed.
    template <typename tp_key_type, typename tp_val_type>
    t_b8 HashMapRemove(s_hash_map<tp_key_type, tp_val_type> &hm, const tp_key_type key) {
        const t_len hash_index = KeyToHashIndex(key, hm.hash_func, hm.immediate_indexes.len);

        if (HashMapBackingBlockRemove(hm.backing_blocks_head, key,
                                      hm.immediate_indexes[hash_index])) {
            hm.kv_pair_cnt--;
            return true;
        }

        return false;
    }

    // This DOES NOT serialize the hash function, binary comparator function, or
    // memory arena!
    template <typename tp_key_type, typename tp_val_type>
    [[nodiscard]] t_b8 SerializeHashMap(s_stream &stream,
                                        const s_hash_map<tp_key_type, tp_val_type> &hm) {
        if (!StreamWriteItem(stream, hm.kv_pair_cnt) &&
            !SerializeArray(stream, hm.immediate_indexes) &&
            !StreamWriteItem(stream, hm.backing_block_cap)) {
            return false;
        }

        const auto bb_cnt = [&hm]() {
            t_len res = 0;

            auto bb = hm.backing_blocks_head;

            while (bb) {
                res++;
                bb = bb->next;
            }

            return res;
        }();

        StreamWriteItem(stream, bb_cnt);

        auto bb = hm.backing_blocks_head;

        while (bb) {
            if (!SerializeArray(stream, bb->keys) && !SerializeArray(stream, bb->vals) &&
                !SerializeArray(stream, bb->next_indexes) &&
                !SerializeBitVec(stream, bb->usage)) {
                return false;
            }

            bb = bb->next;
        }

        return true;
    }

    template <typename tp_key_type, typename tp_val_type>
    [[nodiscard]] t_b8 DeserializeHashMap(s_mem_arena &mem_arena, s_stream &stream,
                                          const t_hash_func<tp_key_type> hash_func,
                                          const t_bin_comparator<tp_key_type> key_comparator,
                                          s_hash_map<tp_key_type, tp_val_type> &o_hm) {
        ZeroOut(&o_hm);

        if (!StreamReadItem(stream, o_hm.kv_pair_cnt) &&
            !DeserializeArray(stream, mem_arena, o_hm.immediate_indexes) &&
            !StreamReadItem(stream, o_hm.backing_block_cap)) {
            return false;
        }

        // Deserialise backing blocks.
        t_len bb_cnt;

        if (!StreamReadItem(stream, bb_cnt)) {
            return false;
        }

        auto bb_ptr_to_update = &o_hm.backing_blocks_head;

        for (t_len i = 0; i < bb_cnt; i++) {
            const auto bb =
                PushToMemArena<s_hash_map_backing_block<tp_key_type, tp_val_type>>(&mem_arena);

            if (!bb) {
                return false;
            }

            *bb_ptr_to_update = bb;

            ZeroOut(bb);

            if (!DeserializeArray(stream, mem_arena, bb->keys) &&
                !DeserializeArray(stream, mem_arena, bb->vals) &&
                !DeserializeArray(stream, mem_arena, bb->next_indexes) &&
                !DeserializeBitVec(stream, mem_arena, bb->usage)) {
                return false;
            }

            bb_ptr_to_update = &bb->next;
        }

        return true;
    }
#endif
}
