#pragma once

#include <zcl/zcl_io.h>
#include <zcl/zcl_mem.h>
#include <zcl/zcl_strs.h>

namespace zf {
    // ============================================================
    // @section: List
    // ============================================================
    template <typename tp_type>
    struct s_list {
        s_array<tp_type> backing_arr;
        t_size len;

        tp_type &operator[](const t_size index) const {
            ZF_ASSERT(index < len);
            return backing_arr[index];
        }
    };

    template <typename tp_type, t_size tp_cap>
    struct s_static_list {
        s_static_array<tp_type, tp_cap> backing_arr;
        t_size len;

        tp_type &operator[](const t_size index) {
            ZF_ASSERT(index < len);
            return backing_arr[index];
        }

        const tp_type &operator[](const t_size index) const {
            ZF_ASSERT(index < len);
            return backing_arr[index];
        }
    };

    template <typename tp_type>
    s_array<tp_type> ListAsArray(const s_list<tp_type> list) {
        return Slice(list.backing_arr, 0, list.len);
    }

    template <typename tp_type, t_size tp_cap>
    s_array<tp_type> ListAsArray(s_static_list<tp_type, tp_cap> &list) {
        return Slice(ToNonstaticArray(list.backing_arr), 0, list.len);
    }

    template <typename tp_type, t_size tp_cap>
    s_array_rdonly<tp_type> ListAsArray(const s_static_list<tp_type, tp_cap> &list) {
        return Slice(ToNonstaticArray(list.backing_arr), 0, list.len);
    }

    template <typename tp_type>
    t_b8 IsListEmpty(const s_list<tp_type> list) {
        return list.len == 0;
    }

    template <typename tp_type, t_size tp_cap>
    t_b8 IsListEmpty(const s_static_list<tp_type, tp_cap> &list) {
        return list.len == 0;
    }

    template <typename tp_type>
    t_b8 IsListFull(const s_list<tp_type> list) {
        return list.len == list.backing_arr.len;
    }

    template <typename tp_type, t_size tp_cap>
    t_b8 IsListFull(const s_static_list<tp_type, tp_cap> &list) {
        return list.len == tp_cap;
    }

    template <typename tp_type>
    tp_type &ListAppend(const s_array<tp_type> backing_arr, t_size &len, const tp_type &val) {
        ZF_ASSERT(len >= 0 && len < backing_arr.len);

        backing_arr[len] = val;
        len++;
        return backing_arr[len - 1];
    }

    template <typename tp_type>
    tp_type &ListAppend(s_list<tp_type> &list, const tp_type &val) {
        return ListAppend(list.backing_arr, list.len, val);
    }

    template <typename tp_type, t_size tp_cap>
    tp_type &ListAppend(s_static_list<tp_type, tp_cap> &list, const tp_type &val) {
        return ListAppend(ToNonstaticArray(list.backing_arr), list.len, val);
    }

    template <typename tp_type>
    void ListInsert(const s_array<tp_type> backing_arr, t_size &len, const t_size index,
                    const tp_type &val) {
        ZF_ASSERT(len >= 0 && len < backing_arr.len);
        ZF_ASSERT(index >= 0 && index <= len);

        for (t_size i = len; i > index; i--) {
            backing_arr[len] = backing_arr[len - 1];
        }

        len++;
        backing_arr[index] = val;
    }

    template <typename tp_type>
    void ListInsert(s_list<tp_type> &list, const t_size index, const tp_type &val) {
        ListInsert(list.backing_arr, list.len, index, val);
    }

    template <typename tp_type, t_size tp_cap>
    void ListInsert(s_static_list<tp_type, tp_cap> &list, const t_size index,
                    const tp_type &val) {
        ListInsert(ToNonstaticArray(list.backing_arr), list.len, index, val);
    }

    template <typename tp_type>
    tp_type ListRemoveLast(const s_array<tp_type> backing_arr, t_size &len) {
        ZF_ASSERT(len > 0 && len <= backing_arr.len);

        len--;
        return backing_arr[len];
    }

    template <typename tp_type>
    tp_type ListRemoveLast(s_list<tp_type> &list) {
        return ListRemoveLast(list.backing_arr, list.len);
    }

    template <typename tp_type, t_size tp_cap>
    tp_type ListRemoveLast(s_static_list<tp_type, tp_cap> &list) {
        return ListRemoveLast(ToNonstaticArray(list.backing_arr), list.len);
    }

    template <typename tp_type>
    void ListRemoveSwapback(const s_array<tp_type> backing_arr, t_size &len,
                            const t_size index) {
        ZF_ASSERT(len > 0 && len <= backing_arr.len);
        ZF_ASSERT(index >= 0 && index < len);

        backing_arr[index] = backing_arr[len - 1];
        len--;
    }

    template <typename tp_type>
    void ListRemoveSwapback(s_list<tp_type> &list, const t_size index) {
        ListRemoveSwapback(list.backing_arr, list.len, index);
    }

    template <typename tp_type, t_size tp_cap>
    void ListRemoveSwapback(s_static_list<tp_type, tp_cap> &list, const t_size index) {
        ListRemoveSwapback(ToNonstaticArray(list.backing_arr), list.len, index);
    }

    template <typename tp_type>
    void ListRemove(const s_array<tp_type> backing_arr, t_size &len, const t_size index) {
        ZF_ASSERT(len > 0 && len <= backing_arr.len);
        ZF_ASSERT(index >= 0 && index < len);

        Copy(Slice(backing_arr, index, len - 1), Slice(backing_arr, index + 1, len));
        len--;
    }

    template <typename tp_type>
    void ListRemove(s_list<tp_type> &list, const t_size index) {
        ListRemove(list.backing_arr, list.len, index);
    }

    template <typename tp_type, t_size tp_cap>
    void ListRemove(s_static_list<tp_type, tp_cap> &list, const t_size index) {
        ListRemove(ToNonstaticArray(list.backing_arr), list.len, index);
    }

    template <typename tp_type>
    [[nodiscard]] t_b8 MakeList(s_mem_arena &mem_arena, const t_size cap,
                                s_list<tp_type> &o_list, const t_size len = 0) {
        ZF_ASSERT(cap > 0 && len >= 0 && len <= cap);

        o_list = {.len = len};

        return InitArray(mem_arena, cap, o_list.backing_arr);
    }

    // ============================================================
    // @section: Stack
    // ============================================================
    template <typename tp_type>
    struct s_stack {
        s_array<tp_type> backing_arr;
        t_size height;

        tp_type &operator[](const t_size index) const {
            ZF_ASSERT(index < height);
            return backing_arr[index];
        }
    };

    template <typename tp_type, t_size tp_cap>
    struct s_static_stack {
        s_static_array<tp_type, tp_cap> backing_arr;
        t_size height;

        tp_type &operator[](const t_size index) {
            ZF_ASSERT(index < height);
            return backing_arr[index];
        }

        const tp_type &operator[](const t_size index) const {
            ZF_ASSERT(index < height);
            return backing_arr[index];
        }
    };

    template <typename tp_type>
    s_array<tp_type> StackAsArray(const s_stack<tp_type> stack) {
        return Slice(stack.backing_arr, 0, stack.height);
    }

    template <typename tp_type>
    t_b8 IsStackEmpty(const s_stack<tp_type> stack) {
        return stack.height == 0;
    }

    template <typename tp_type, t_size tp_cap>
    t_b8 IsStackEmpty(const s_static_stack<tp_type, tp_cap> &stack) {
        return stack.height == 0;
    }

    template <typename tp_type>
    t_b8 IsStackFull(const s_stack<tp_type> stack) {
        return stack.height == stack.backing_arr.len;
    }

    template <typename tp_type, t_size tp_cap>
    t_b8 IsStackFull(const s_static_stack<tp_type, tp_cap> &stack) {
        return stack.height == tp_cap;
    }

    template <typename tp_type>
    tp_type &StackTop(const s_array<tp_type> backing_arr, const t_size height) {
        ZF_ASSERT(height > 0 && height <= backing_arr.len);
        return backing_arr[height - 1];
    }

    template <typename tp_type>
    tp_type &StackTop(const s_stack<tp_type> &stack) {
        return StackTop(stack.backing_arr, stack.height);
    }

    template <typename tp_type, t_size tp_cap>
    tp_type &StackTop(s_static_stack<tp_type, tp_cap> &stack) {
        return StackTop(ToNonstaticArray(stack.backing_arr), stack.height);
    }

    template <typename tp_type>
    tp_type &StackPush(const s_array<tp_type> backing_arr, t_size &height,
                       const tp_type &val) {
        ZF_ASSERT(height >= 0 && height < backing_arr.len);

        backing_arr[height] = val;
        height++;
        return backing_arr[height - 1];
    }

    template <typename tp_type>
    tp_type &StackPush(s_stack<tp_type> &stack, const tp_type &val) {
        return StackPush(stack.backing_arr, stack.height, val);
    }

    template <typename tp_type, t_size tp_cap>
    tp_type &StackPush(s_static_stack<tp_type, tp_cap> &stack, const tp_type &val) {
        return StackPush(ToNonstaticArray(stack.backing_arr), stack.height, val);
    }

    template <typename tp_type>
    tp_type StackPop(const s_array<tp_type> backing_arr, t_size &height) {
        ZF_ASSERT(height > 0 && height <= backing_arr.len);

        height--;
        return backing_arr[height];
    }

    template <typename tp_type>
    tp_type StackPop(s_stack<tp_type> &stack) {
        return StackPop(stack.backing_arr, stack.height);
    }

    template <typename tp_type, t_size tp_cap>
    tp_type StackPop(s_static_stack<tp_type, tp_cap> &stack) {
        return StackPop(ToNonstaticArray(stack.backing_arr), stack.height);
    }

    template <typename tp_type>
    t_b8 MakeStack(s_mem_arena &mem_arena, const t_size cap, s_stack<tp_type> &o_stack,
                   const t_size height = 0) {
        ZF_ASSERT(cap > 0 && height >= 0 && height <= cap);

        o_stack = {.height = height};

        return InitArray(mem_arena, cap, o_stack.backing_arr);
    }

    // ============================================================
    // @section: Queue
    // ============================================================
    template <typename tp_type>
    struct s_queue {
        s_array<tp_type> backing_arr;
        t_size begin_index;
        t_size len;

        tp_type &operator[](const t_size index) const {
            ZF_ASSERT(index < len);
            return backing_arr[index];
        }
    };

    template <typename tp_type, t_size tp_cap>
    struct s_static_queue {
        s_static_array<tp_type, tp_cap> backing_arr;
        t_size begin_index;
        t_size len;

        tp_type &operator[](const t_size index) {
            ZF_ASSERT(index < len);
            return backing_arr[index];
        }

        const tp_type &operator[](const t_size index) const {
            ZF_ASSERT(index < len);
            return backing_arr[index];
        }
    };

    template <typename tp_type>
    t_b8 IsQueueEmpty(const s_queue<tp_type> queue) {
        return queue.len == 0;
    }

    template <typename tp_type, t_size tp_cap>
    t_b8 IsQueueEmpty(const s_static_queue<tp_type, tp_cap> &queue) {
        return queue.len == 0;
    }

    template <typename tp_type>
    t_b8 IsQueueFull(const s_queue<tp_type> queue) {
        return queue.len == queue.backing_arr.len;
    }

    template <typename tp_type, t_size tp_cap>
    t_b8 IsQueueFull(const s_static_queue<tp_type, tp_cap> &queue) {
        return queue.len == tp_cap;
    }

    template <typename tp_type>
    tp_type &Enqueue(const s_array<tp_type> backing_arr, const t_size begin_index, t_size &len,
                     const tp_type &val) {
        ZF_ASSERT(len >= 0 && len < backing_arr.len);

        tp_type &enqueued = backing_arr[(begin_index + len) % backing_arr.len];
        enqueued = val;
        len++;
        return enqueued;
    }

    template <typename tp_type>
    tp_type &Enqueue(s_queue<tp_type> &queue, const tp_type &val) {
        return Enqueue(queue.backing_arr, queue.begin_index, queue.len, val);
    }

    template <typename tp_type, t_size tp_cap>
    tp_type &Enqueue(s_static_queue<tp_type, tp_cap> &queue, const tp_type &val) {
        return Enqueue(ToNonstaticArray(queue.backing_arr), queue.begin_index, queue.len, val);
    }

    template <typename tp_type>
    tp_type Dequeue(const s_array_rdonly<tp_type> backing_arr, t_size &begin_index,
                    t_size &len) {
        ZF_ASSERT(len > 0 && len <= backing_arr.len);

        const tp_type val = backing_arr[begin_index];
        begin_index = (begin_index + 1) % backing_arr.len;
        len--;
        return val;
    }

    template <typename tp_type>
    tp_type Dequeue(s_queue<tp_type> &queue) {
        return Dequeue(ToNonstaticArray(queue.backing_arr), queue.begin_index, queue.len);
    }

    template <typename tp_type, t_size tp_cap>
    tp_type Dequeue(s_static_queue<tp_type, tp_cap> &queue) {
        return Dequeue(ToNonstaticArray(queue.backing_arr), queue.begin_index, queue.len);
    }

    template <typename tp_type>
    [[nodiscard]] t_b8 MakeQueue(s_mem_arena &mem_arena, const t_size cap,
                                 s_queue<tp_type> &o_queue, const t_size len = 0) {
        ZF_ASSERT(cap > 0 && len >= 0 && len <= cap);

        o_queue = {.len = len};

        return InitArray(mem_arena, cap, o_queue.backing_arr);
    }

    // ============================================================
    // @section: Hash Map
    // ============================================================
    template <typename tp_type>
    using t_hash_func = t_size (*)(const tp_type &key);

    // This is an FNV-1a implementation.
    constexpr t_hash_func<s_str_rdonly> g_str_hash_func =
        [](const s_str_rdonly &key) constexpr {
            const t_u32 offs_basis = 2166136261u;
            const t_u32 prime = 16777619u;

            t_u32 hash = offs_basis;

            for (t_size i = 0; i < key.bytes.len; i++) {
                hash ^= key.bytes[i];
                hash *= prime;
            }

            return static_cast<t_size>(static_cast<t_u64>(hash) & 0x7FFFFFFFFFFFFFFFull);
        };

    template <typename tp_key_type, typename tp_val_type>
    struct s_hash_map_backing_block {
        s_array<tp_key_type> keys;
        s_array<tp_val_type> vals;
        s_array<t_size> next_indexes;

        s_bit_vec usage;

        s_hash_map_backing_block *next;
    };

    template <typename tp_key_type, typename tp_val_type>
    [[nodiscard]] t_b8 InitHashMapBackingBlock(
        s_hash_map_backing_block<tp_key_type, tp_val_type> *bb, const t_size cap,
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
    t_size HashMapBackingBlockCap(
        const s_hash_map_backing_block<tp_key_type, tp_val_type> *const bb) {
        return bb->keys.len;
    }

    template <typename tp_key_type, typename tp_val_type>
    [[nodiscard]] t_b8 HashMapBackingBlockGet(
        const s_hash_map_backing_block<tp_key_type, tp_val_type> *bb, const tp_key_type key,
        const t_bin_comparator<tp_key_type> key_comparator, t_size index,
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
        t_size &index, const t_size new_bb_cap, s_mem_arena &new_bb_mem_arena) {
        ZF_ASSERT(index >= -1);

        if (!bb) {
            bb = PushToMemArena<s_hash_map_backing_block<tp_key_type, tp_val_type>>(
                &new_bb_mem_arena);

            if (!bb || !InitHashMapBackingBlock(bb, new_bb_cap, &new_bb_mem_arena)) {
                return ek_hash_map_put_result_error;
            }
        }

        if (index == -1) {
            const t_size prospective_index = IndexOfFirstUnsetBit(bb->usage);

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

        t_size index_copy = index;

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
                                   t_size &index) {
        ZF_ASSERT(index >= -1);

        if (!bb || index == -1) {
            return false;
        }

        t_size index_copy = index;

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

        t_size kv_pair_cnt;

        s_array<t_size> immediate_indexes;

        t_size backing_block_cap;
        s_hash_map_backing_block<tp_key_type, tp_val_type> *backing_blocks_head;

        s_mem_arena *mem_arena;
    };

    constexpr t_size g_hash_map_immediate_cap_default = 32;
    constexpr t_size g_hash_map_backing_block_cap_default = 32;

    // The provided hash function has to map a key to an integer 0 or higher.
    // The immediate capacity is the total number of upfront slots (i.e. the maximum
    // possible number of slots for which an O(1) access of a value from a key can
    // happen). The given memory arena will be used also for allocating new backing blocks when
    // they're needed.
    template <typename tp_key_type, typename tp_val_type>
    [[nodiscard]] t_b8 InitHashMap(
        s_hash_map<tp_key_type, tp_val_type> *const hm,
        const t_hash_func<tp_key_type> hash_func, s_mem_arena &mem_arena,
        const t_size immediate_cap = g_hash_map_immediate_cap_default,
        const t_size backing_block_cap = g_hash_map_backing_block_cap_default,
        const t_bin_comparator<tp_key_type> key_comparator = DefaultBinComparator) {
        ZeroOut(hm);

        hm->hash_func = hash_func;
        hm->key_comparator = key_comparator;
        hm->backing_block_cap = backing_block_cap;
        hm->mem_arena = &mem_arena;

        if (!InitArray(&hm->immediate_indexes, immediate_cap, &mem_arena)) {
            return false;
        }

        SetAllTo(hm->immediate_indexes, static_cast<t_size>(-1));

        return true;
    }

    template <typename tp_type>
    t_size KeyToHashIndex(const tp_type &key, const t_hash_func<tp_type> hash_func,
                          const t_size table_size) {
        ZF_ASSERT(hash_func);
        ZF_ASSERT(table_size > 0);

        const t_size val = hash_func(key);
        ZF_ASSERT(val >= 0);

        return val % table_size;
    }

    template <typename tp_key_type, typename tp_val_type>
    [[nodiscard]] t_b8 HashMapGet(const s_hash_map<tp_key_type, tp_val_type> &hm,
                                  const tp_key_type &key, tp_val_type *const o_val = nullptr) {
        const t_size hash_index = KeyToHashIndex(key, hm.hash_func, hm.immediate_indexes.len);
        return HashMapBackingBlockGet(hm.backing_blocks_head, key, hm.key_comparator,
                                      hm.immediate_indexes[hash_index], o_val);
    }

    // Returns true iff no error occurred.
    template <typename tp_key_type, typename tp_val_type>
    [[nodiscard]] e_hash_map_put_result HashMapPut(s_hash_map<tp_key_type, tp_val_type> &hm,
                                                   const tp_key_type &key,
                                                   const tp_val_type &val) {
        const t_size hash_index = KeyToHashIndex(key, hm.hash_func, hm.immediate_indexes.len);

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
        const t_size hash_index = KeyToHashIndex(key, hm.hash_func, hm.immediate_indexes.len);

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
            t_size res = 0;

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
        t_size bb_cnt;

        if (!StreamReadItem(stream, bb_cnt)) {
            return false;
        }

        auto bb_ptr_to_update = &o_hm.backing_blocks_head;

        for (t_size i = 0; i < bb_cnt; i++) {
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
}
