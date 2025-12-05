#pragma once

#include <zcl/zcl_mem.h>
#include <zcl/zcl_io.h>
#include <zcl/zcl_strs.h>

namespace zf {
    // ============================================================
    // @section: List
    // ============================================================
    template<typename tp_type>
    struct s_list {
        s_array<tp_type> backing_arr;
        t_size len;

        tp_type& operator[](const t_size index) const {
            ZF_ASSERT(index < len);
            return backing_arr[index];
        }
    };

    template<typename tp_type, t_size tp_cap>
    struct s_static_list {
        s_static_array<tp_type, tp_cap> backing_arr;
        t_size len;

        tp_type& operator[](const t_size index) {
            ZF_ASSERT(index < len);
            return backing_arr[index];
        }

        const tp_type& operator[](const t_size index) const {
            ZF_ASSERT(index < len);
            return backing_arr[index];
        }
    };

    template<typename tp_type>
    s_array<tp_type> ListAsArray(const s_list<tp_type> list) {
        return Slice(list.backing_arr, 0, list.len);
    }

    template<typename tp_type, t_size tp_cap>
    s_array<tp_type> ListAsArray(s_static_list<tp_type, tp_cap>& list) {
        return Slice(ToNonstatic(list.backing_arr), 0, list.len);
    }

    template<typename tp_type, t_size tp_cap>
    s_array_rdonly<tp_type> ListAsArray(const s_static_list<tp_type, tp_cap>& list) {
        return Slice(ToNonstatic(list.backing_arr), 0, list.len);
    }

    template<typename tp_type>
    t_b8 IsListEmpty(const s_list<tp_type> list) {
        return list.len == 0;
    }

    template<typename tp_type, t_size tp_cap>
    t_b8 IsListEmpty(const s_static_list<tp_type, tp_cap>& list) {
        return list.len == 0;
    }

    template<typename tp_type>
    t_b8 IsListFull(const s_list<tp_type> list) {
        return list.len == list.backing_arr.len;
    }

    template<typename tp_type, t_size tp_cap>
    t_b8 IsListFull(const s_static_list<tp_type, tp_cap>& list) {
        return list.len == tp_cap;
    }

    template<typename tp_type>
    tp_type& ListAppend(const s_array<tp_type> backing_arr, t_size& len, const tp_type& val) {
        ZF_ASSERT(len >= 0 && len < backing_arr.len);

        backing_arr[len] = val;
        len++;
        return backing_arr[len - 1];
    }

    template<typename tp_type>
    tp_type& ListAppend(s_list<tp_type>& list, const tp_type& val) {
        return ListAppend(list.backing_arr, list.len, val);
    }

    template<typename tp_type, t_size tp_cap>
    tp_type& ListAppend(s_static_list<tp_type, tp_cap>& list, const tp_type& val) {
        return ListAppend(ToNonstatic(list.backing_arr), list.len, val);
    }

    template<typename tp_type>
    void ListInsert(const s_array<tp_type> backing_arr, t_size& len, const t_size index, const tp_type& val) {
        ZF_ASSERT(len >= 0 && len < backing_arr.len);
        ZF_ASSERT(index >= 0 && index <= len);

        for (t_size i = len; i > index; i--) {
            backing_arr[len] = backing_arr[len - 1];
        }

        len++;
        backing_arr[index] = val;
    }

    template<typename tp_type>
    void ListInsert(s_list<tp_type>& list, const t_size index, const tp_type& val) {
        ListInsert(list.backing_arr, list.len, index, val);
    }

    template<typename tp_type, t_size tp_cap>
    void ListInsert(s_static_list<tp_type, tp_cap>& list, const t_size index, const tp_type& val) {
        ListInsert(ToNonstatic(list.backing_arr), list.len, index, val);
    }

    template<typename tp_type>
    tp_type ListRemoveLast(const s_array<tp_type> backing_arr, t_size& len) {
        ZF_ASSERT(len > 0 && len <= backing_arr.len);

        len--;
        return backing_arr[len];
    }

    template<typename tp_type>
    tp_type ListRemoveLast(s_list<tp_type>& list) {
        return ListRemoveLast(list.backing_arr, list.len);
    }

    template<typename tp_type, t_size tp_cap>
    tp_type ListRemoveLast(s_static_list<tp_type, tp_cap>& list) {
        return ListRemoveLast(ToNonstatic(list.backing_arr), list.len);
    }

    template<typename tp_type>
    void ListRemoveSwapback(const s_array<tp_type> backing_arr, t_size& len, const t_size index) {
        ZF_ASSERT(len > 0 && len <= backing_arr.len);
        ZF_ASSERT(index >= 0 && index < len);

        backing_arr[index] = backing_arr[len - 1];
        len--;
    }

    template<typename tp_type>
    void ListRemoveSwapback(s_list<tp_type>& list, const t_size index) {
        ListRemoveSwapback(list.backing_arr, list.len, index);
    }

    template<typename tp_type, t_size tp_cap>
    void ListRemoveSwapback(s_static_list<tp_type, tp_cap>& list, const t_size index) {
        ListRemoveSwapback(ToNonstatic(list.backing_arr), list.len, index);
    }

    template<typename tp_type>
    void ListRemove(const s_array<tp_type> backing_arr, t_size& len, const t_size index) {
        ZF_ASSERT(len > 0 && len <= backing_arr.len);
        ZF_ASSERT(index >= 0 && index < len);

        Copy(Slice(backing_arr, index, len - 1), Slice(backing_arr, index + 1, len));
        len--;
    }

    template<typename tp_type>
    void ListRemove(s_list<tp_type>& list, const t_size index) {
        ListRemove(list.backing_arr, list.len, index);
    }

    template<typename tp_type, t_size tp_cap>
    void ListRemove(s_static_list<tp_type, tp_cap>& list, const t_size index) {
        ListRemove(ToNonstatic(list.backing_arr), list.len, index);
    }

    template<typename tp_type>
    [[nodiscard]] t_b8 MakeList(s_mem_arena& mem_arena, const t_size cap, s_list<tp_type>& o_list, const t_size len = 0) {
        ZF_ASSERT(cap > 0 && len >= 0 && len <= cap);

        o_list = {
            .len = len
        };

        return MakeArray(mem_arena, cap, o_list.backing_arr);
    }

    // ============================================================
    // @section: Stack
    // ============================================================
    template<typename tp_type>
    struct s_stack {
        s_array<tp_type> backing_arr;
        t_size height;

        tp_type& operator[](const t_size index) const {
            ZF_ASSERT(index < height);
            return backing_arr[index];
        }
    };

    template<typename tp_type, t_size tp_cap>
    struct s_static_stack {
        s_static_array<tp_type, tp_cap> backing_arr;
        t_size height;

        tp_type& operator[](const t_size index) {
            ZF_ASSERT(index < height);
            return backing_arr[index];
        }

        const tp_type& operator[](const t_size index) const {
            ZF_ASSERT(index < height);
            return backing_arr[index];
        }
    };

    template<typename tp_type>
    s_array<tp_type> StackAsArray(const s_stack<tp_type> stack) {
        return Slice(stack.backing_arr, 0, stack.height);
    }

    template<typename tp_type>
    t_b8 IsStackEmpty(const s_stack<tp_type> stack) {
        return stack.height == 0;
    }

    template<typename tp_type, t_size tp_cap>
    t_b8 IsStackEmpty(const s_static_stack<tp_type, tp_cap>& stack) {
        return stack.height == 0;
    }

    template<typename tp_type>
    t_b8 IsStackFull(const s_stack<tp_type> stack) {
        return stack.height == stack.backing_arr.len;
    }

    template<typename tp_type, t_size tp_cap>
    t_b8 IsStackFull(const s_static_stack<tp_type, tp_cap>& stack) {
        return stack.height == tp_cap;
    }

    template<typename tp_type>
    tp_type& StackTop(const s_array<tp_type> backing_arr, const t_size height) {
        ZF_ASSERT(height > 0 && height <= backing_arr.len);
        return backing_arr[height - 1];
    }

    template<typename tp_type>
    tp_type& StackTop(const s_stack<tp_type>& stack) {
        return StackTop(stack.backing_arr, stack.height);
    }

    template<typename tp_type, t_size tp_cap>
    tp_type& StackTop(s_static_stack<tp_type, tp_cap>& stack) {
        return StackTop(ToNonstatic(stack.backing_arr), stack.height);
    }

    template<typename tp_type>
    tp_type& StackPush(const s_array<tp_type> backing_arr, t_size& height, const tp_type& val) {
        ZF_ASSERT(height >= 0 && height < backing_arr.len);

        backing_arr[height] = val;
        height++;
        return backing_arr[height - 1];
    }

    template<typename tp_type>
    tp_type& StackPush(s_stack<tp_type>& stack, const tp_type& val) {
        return StackPush(stack.backing_arr, stack.height, val);
    }

    template<typename tp_type, t_size tp_cap>
    tp_type& StackPush(s_static_stack<tp_type, tp_cap>& stack, const tp_type& val) {
        return StackPush(ToNonstatic(stack.backing_arr), stack.height, val);
    }

    template<typename tp_type>
    tp_type StackPop(const s_array<tp_type> backing_arr, t_size& height) {
        ZF_ASSERT(height > 0 && height <= backing_arr.len);

        height--;
        return backing_arr[height];
    }

    template<typename tp_type>
    tp_type StackPop(s_stack<tp_type>& stack) {
        return StackPop(stack.backing_arr, stack.height);
    }

    template<typename tp_type, t_size tp_cap>
    tp_type StackPop(s_static_stack<tp_type, tp_cap>& stack) {
        return StackPop(ToNonstatic(stack.backing_arr), stack.height);
    }

    template<typename tp_type>
    t_b8 MakeStack(s_mem_arena& mem_arena, const t_size cap, s_stack<tp_type>& o_stack, const t_size height = 0) {
        ZF_ASSERT(cap > 0 && height >= 0 && height <= cap);

        o_stack = {
            .height = height
        };

        return MakeArray(mem_arena, cap, o_stack.backing_arr);
    }

    // ============================================================
    // @section: Queue
    // ============================================================
    template<typename tp_type>
    struct s_queue {
        s_array<tp_type> backing_arr;
        t_size begin_index;
        t_size len;

        tp_type& operator[](const t_size index) const {
            ZF_ASSERT(index < len);
            return backing_arr[index];
        }
    };

    template<typename tp_type, t_size tp_cap>
    struct s_static_queue {
        s_static_array<tp_type, tp_cap> backing_arr;
        t_size begin_index;
        t_size len;

        tp_type& operator[](const t_size index) {
            ZF_ASSERT(index < len);
            return backing_arr[index];
        }

        const tp_type& operator[](const t_size index) const {
            ZF_ASSERT(index < len);
            return backing_arr[index];
        }
    };

    template<typename tp_type>
    t_b8 IsQueueEmpty(const s_queue<tp_type> queue) {
        return queue.len == 0;
    }

    template<typename tp_type, t_size tp_cap>
    t_b8 IsQueueEmpty(const s_static_queue<tp_type, tp_cap>& queue) {
        return queue.len == 0;
    }

    template<typename tp_type>
    t_b8 IsQueueFull(const s_queue<tp_type> queue) {
        return queue.len == queue.backing_arr.len;
    }

    template<typename tp_type, t_size tp_cap>
    t_b8 IsQueueFull(const s_static_queue<tp_type, tp_cap>& queue) {
        return queue.len == tp_cap;
    }

    template<typename tp_type>
    tp_type& Enqueue(const s_array<tp_type> backing_arr, const t_size begin_index, t_size& len, const tp_type& val) {
        ZF_ASSERT(len >= 0 && len < backing_arr.len);

        tp_type& enqueued = backing_arr[(begin_index + len) % backing_arr.len];
        enqueued = val;
        len++;
        return enqueued;
    }

    template<typename tp_type>
    tp_type& Enqueue(s_queue<tp_type>& queue, const tp_type& val) {
        return Enqueue(queue.backing_arr, queue.begin_index, queue.len, val);
    }

    template<typename tp_type, t_size tp_cap>
    tp_type& Enqueue(s_static_queue<tp_type, tp_cap>& queue, const tp_type& val) {
        return Enqueue(ToNonstatic(queue.backing_arr), queue.begin_index, queue.len, val);
    }

    template<typename tp_type>
    tp_type Dequeue(const s_array_rdonly<tp_type> backing_arr, t_size& begin_index, t_size& len) {
        ZF_ASSERT(len > 0 && len <= backing_arr.len);

        const tp_type val = backing_arr[begin_index];
        begin_index = (begin_index + 1) % backing_arr.len;
        len--;
        return val;
    }

    template<typename tp_type>
    tp_type Dequeue(s_queue<tp_type>& queue) {
        return Dequeue(ToNonstatic(queue.backing_arr), queue.begin_index, queue.len);
    }

    template<typename tp_type, t_size tp_cap>
    tp_type Dequeue(s_static_queue<tp_type, tp_cap>& queue) {
        return Dequeue(ToNonstatic(queue.backing_arr), queue.begin_index, queue.len);
    }

    template<typename tp_type>
    [[nodiscard]] t_b8 MakeQueue(s_mem_arena& mem_arena, const t_size cap, s_queue<tp_type>& o_queue, const t_size len = 0) {
        ZF_ASSERT(cap > 0 && len >= 0 && len <= cap);

        o_queue = {
            .len = len
        };

        return MakeArray(mem_arena, cap, o_queue.backing_arr);
    }

    // ============================================================
    // @section: Hash Map
    // ============================================================
    template<typename tp_type>
    using t_hash_func = t_size (*)(const tp_type& key);

    // This is an FNV-1a implementation.
    constexpr t_hash_func<s_str_rdonly> g_str_hash_func = [](const s_str_rdonly& key) constexpr {
        const t_u32 offs_basis = 2166136261u;
        const t_u32 prime = 16777619u;

        t_u32 hash = offs_basis;

        for (t_size i = 0; i < key.bytes.len; i++) {
            hash ^= key.bytes[i];
            hash *= prime;
        }

        return static_cast<t_size>(static_cast<t_u64>(hash) & 0x7FFFFFFFFFFFFFFFull);
    };

    template<typename tp_key_type, typename tp_val_type>
    struct s_hash_map_backing_block {
        s_array<tp_key_type> keys;
        s_array<tp_val_type> vals;
        s_array<t_size> next_indexes;

        s_bit_vec usage;

        s_hash_map_backing_block* next;
    };

    template<typename tp_key_type, typename tp_val_type>
    t_size HashMapBackingBlockCap(const s_hash_map_backing_block<tp_key_type, tp_val_type>* const bb) {
        return bb->keys.len;
    }

    template<typename tp_key_type, typename tp_val_type>
    t_b8 HashMapBackingBlockFind(const s_hash_map_backing_block<tp_key_type, tp_val_type>* bb, const t_s32 key, t_size index) {
        if (index == -1) {
            return false;
        }

        ZF_ASSERT(index >= 0);

        while (index >= HashMapBackingBlockCap(bb)) {
            ZF_ASSERT(bb->next);
            bb = bb->next;
            index -= HashMapBackingBlockCap(bb);
        }

        if (bb->keys[index] == key) {
            return true;
        }

        return HashMapBackingBlockFind(bb, key, bb->next_indexes[index]);
    }

    template<typename tp_key_type, typename tp_val_type>
    [[nodiscard]] t_b8 HashMapBackingBlockPut(s_hash_map_backing_block<tp_key_type, tp_val_type>* bb, const t_s32 key, const t_s32 val, t_size& index, s_mem_arena& mem_arena) {
        ZF_ASSERT(index >= -1);

        if (index == -1) {
            const t_size prospective_index = IndexOfFirstUnsetBit(bb->usage);

            if (prospective_index == -1) {
                if (!bb->next) {
                    bb->next = PushToMemArena<s_hash_map_backing_block>(mem_arena);

                    if (!bb->next) {
                        return false;
                    }
                }

                return HashMapBackingBlockPut(bb->next, key, val, index, mem_arena);
            }

            index = prospective_index;

            bb->keys[index] = key;
            bb->vals[index] = val;
            bb->next_indexes[index] = -1;
            SetBit(bb->usage, index);

            return true;
        }

        t_size index_copy = index;

        while (index_copy >= HashMapBackingBlockCap(bb)) {
            ZF_ASSERT(bb->next);
            bb = bb->next;
            index_copy -= HashMapBackingBlockCap(bb);
        }

        if (bb->keys[index_copy] == key) {
            bb->vals[index_copy] = val;
            return true;
        }

        return HashMapBackingBlockPut(bb, key, val, bb->next_indexes[index_copy], mem_arena);
    }

    template<typename tp_key_type, typename tp_val_type>
    t_b8 HashMapBackingBlockRemove(s_hash_map_backing_block<tp_key_type, tp_val_type>* bb, const t_s32 key, t_size& index) {
        ZF_ASSERT(index >= -1);

        if (index == -1) {
            return false;
        }

        t_size index_copy = index;

        while (index_copy >= HashMapBackingBlockCap(bb)) {
            ZF_ASSERT(bb->next);
            bb = bb->next;
            index_copy -= HashMapBackingBlockCap(bb);
        }

        if (bb->keys[index_copy] == key) {
            index = bb->next_indexes[index_copy];
            UnsetBit(bb->usage, index_copy);
            return true;
        }

        return HashMapBackingBlockRemove(bb, key, bb->next_indexes[index_copy]);
    }

    template<typename tp_key_type, typename tp_val_type>
    struct s_hash_map {
        t_hash_func<tp_key_type> hash_func;
        t_bin_comparator<tp_key_type> key_comparator;

        t_size kv_pair_cnt;

        s_array<t_size> backing_block_immediate_indexes;

        s_hash_map_backing_block<tp_key_type, tp_val_type>* backing_blocks;

        s_mem_arena* mem_arena;
    };

    template<typename tp_type>
    t_size KeyToHashIndex(const tp_type& key, const t_hash_func<tp_type> hash_func, const t_size table_size) {
        ZF_ASSERT(hash_func);
        ZF_ASSERT(table_size > 0);

        const t_size val = hash_func(key);
        ZF_ASSERT(val >= 0);

        return val % table_size;
    }

    template<typename tp_key_type, typename tp_val_type>
    t_b8 HashMapFind(const s_hash_map<tp_key_type, tp_val_type>& hm, const tp_key_type& key) {
        const t_size hash_index = KeyToHashIndex(key, hm.hash_func, hm.backing_block_immediate_indexes.len);
        return HashMapBackingBlockFind(hm.backing_blocks_head, key, hm.backing_block_immediate_indexes[hash_index]);
    }

    // Returns true iff no error occurred.
    template<typename tp_key_type, typename tp_val_type>
    [[nodiscard]] t_b8 HashMapPut(s_hash_map<tp_key_type, tp_val_type>& hm, const tp_key_type& key, const tp_val_type& val) {
        const t_size hash_index = KeyToHashIndex(key, hm.hash_func, hm.backing_block_immediate_indexes.len);
        return HashMapBackingBlockPut(hm.backing_blocks, key, val, hm.backing_block_immediate_indexes[hash_index], *hm.mem_arena);
    }

    // Returns true iff the key was found and removed.
    template<typename tp_key_type, typename tp_val_type>
    t_b8 HashMapRemove(s_hash_map<tp_key_type, tp_val_type>& hm, const t_s32 key) {
        const t_size hash_index = KeyToHashIndex(key, hm.hash_func, hm.backing_block_immediate_indexes.len);
        return DudeMapBackingBlockRemove(hm.backing_blocks_head, key, hm.backing_block_immediate_indexes[hash_index]);
    }

#if 0







    template<typename tp_type>
    using t_hash_func = t_size (*)(const tp_type& key);

    constexpr t_hash_func<s_str_rdonly> g_str_hash_func = [](const s_str_rdonly& key) constexpr {
        // This is an FNV-1a implementation.
        const t_u32 offs_basis = 2166136261u;
        const t_u32 prime = 16777619u;

        t_u32 hash = offs_basis;

        for (t_size i = 0; i < key.bytes.len; i++) {
            hash ^= key.bytes[i];
            hash *= prime;
        }

        return static_cast<t_size>(static_cast<t_u64>(hash) & 0x7FFFFFFFFFFFFFFFull);
    };

    template<typename tp_key_type, typename tp_val_type>
    struct s_hash_map {
        t_hash_func<tp_key_type> hash_func;
        t_bin_comparator<tp_key_type> key_comparator;

        t_size kv_pair_cnt;

        s_array<t_size> backing_store_indexes; // These are what the hash function initially maps to after modulo. They are indexes into "slots" (linked-list nodes) in the backing store below.

        // This is where all the key-value pairs are actually stored.
        struct {
            // Keeping all of these in distinct arrays to not waste space with padding had they been put in a struct.
            s_array<tp_key_type> keys;
            s_array<tp_val_type> vals;
            s_array<t_size> next_indexes; // Like the standard "next" pointer of a linked list node, but for an index specific to this backing store.

            // Each bit indicates whether the slot or "node" is in use.
            s_bit_vec usage;
        } backing_store;
    };










    template<typename tp_type>
    t_size KeyToHashIndex(const tp_type& key, const t_hash_func<tp_type> hash_func, const t_size table_size) {
        ZF_ASSERT(hash_func);
        ZF_ASSERT(table_size > 0);

        const t_size val = hash_func(key);
        ZF_ASSERT(val >= 0);

        return val % table_size;
    }

    // Returns true iff the key was found.
    // If you don't care about getting a reference to the value and are solely looking for whether it exists, pass nullptr in for o_val.
    template<typename tp_key_type, typename tp_val_type>
    [[nodiscard]] t_b8 HashMapGet(const s_hash_map<tp_key_type, tp_val_type>& hm, const tp_key_type& key, tp_val_type* const o_val = nullptr) {
        const t_size hash_index = KeyToHashIndex(key, hm.hash_func, hm.backing_store_indexes.len);

        const auto bs_get = [&hm, &key, o_val](const auto self, const t_size index) {
            if (index == -1) {
                return false;
            }

            if (hm.key_comparator(hm.backing_store.keys[index], key)) {
                if (o_val) {
                    *o_val = hm.backing_store.vals[index];
                }

                return true;
            }

            return self(self, hm.backing_store.next_indexes[index]);
        };

        return bs_get(bs_get, hm.backing_store_indexes[hash_index]);
    }

    enum e_hash_map_put_result : t_s32 {
        ek_hash_map_put_result_added,
        ek_hash_map_put_result_updated,
        ek_hash_map_put_result_error
    };

    template<typename tp_key_type, typename tp_val_type>
    [[nodiscard]] e_hash_map_put_result HashMapPut(s_hash_map<tp_key_type, tp_val_type>& hm, const tp_key_type& key, const tp_val_type& val) {
        const t_size hash_index = KeyToHashIndex(key, hm.hash_func, hm.backing_store_indexes.len);

        const auto bs_put = [&hm, &key, &val](const auto self, t_size& index) {
            if (index == -1) {
                const t_size prospective_index = IndexOfFirstUnsetBit(hm.backing_store.usage);

                if (prospective_index == -1) {
                    // We're out of room!
                    return ek_hash_map_put_result_error;
                }

                index = prospective_index;

                hm.backing_store.keys[index] = key;
                hm.backing_store.vals[index] = val;
                hm.backing_store.next_indexes[index] = -1;
                SetBit(hm.backing_store.usage, index);

                hm.kv_pair_cnt++;

                return ek_hash_map_put_result_added;
            }

            if (hm.key_comparator(hm.backing_store.keys[index], key)) {
                hm.backing_store.vals[index] = val;
                return ek_hash_map_put_result_updated;
            }

            return self(self, hm.backing_store.next_indexes[index]);
        };

        return bs_put(bs_put, hm.backing_store_indexes[hash_index]);
    }

    // Returns true iff an entry with the given key was found and removed.
    template<typename tp_key_type, typename tp_val_type>
    t_b8 HashMapRemove(const s_hash_map<tp_key_type, tp_val_type>& hm, const tp_key_type& key) {
        const t_size hash_index = KeyToHashIndex(key, hm.hash_func, hm.backing_store_indexes.len);

        const auto bs_remove = [&hm, &key](const auto self, t_size& index) {
            if (index == -1) {
                return false;
            }

            if (hm.key_comparator(hm.backing_store.keys[index], key)) {
                UnsetBit(hm.backing_store.usage, index);
                index = hm.backing_store.next_indexes[index];
                hm.kv_pair_cnt--;
                return true;
            }

            return self(self, hm.backing_store.next_indexes[index]);
        };

        return bs_remove(bs_remove, hm.backing_store_indexes[hash_index]);
    }

    // Returns true iff the destination array had enough room to fit all keys.
    template<typename tp_key_type, typename tp_val_type>
    [[nodiscard]] t_b8 LoadHashMapKeys(const s_hash_map<tp_key_type, tp_val_type>& hm, s_mem_arena& mem_arena, s_array<tp_key_type>& o_keys) {
        if (!MakeArray(mem_arena, hm.kv_pair_cnt, o_keys)) {
            return false;
        }

        s_list<tp_key_type> keys_list = {ToNonstatic(o_keys)};

        const auto add_from_bs = [&hm, &keys_list](const auto self, const t_size index) {
            if (index == -1) {
                return true;
            }
            
            if (IsListFull(keys_list)) {
                return false;
            }

            ListAppend(keys_list, hm.backing_store.keys[index]);
            self(self, hm.backing_store.next_indexes[index]);
        };

        for (t_size i = 0; i < hm.backing_store_indexes.len; i++) {
            if (!add_from_bs(add_from_bs, hm.backing_store_indexes[i])) {
                return false;
            }
        }

        return true;
    }

    template<typename tp_key_type, typename tp_val_type>
    [[nodiscard]] t_b8 LoadHashMapKeys(s_mem_arena& mem_arena, const s_hash_map<tp_key_type, tp_val_type>& hm, s_array<tp_key_type>& o_keys) {
        if (!MakeArray(mem_arena, hm.kv_pair_cnt, o_keys)) {
            return false;
        }

        const t_b8 success = LoadHashMapKeys(hm, o_keys);
        ZF_ASSERT(success);

        return true;
    }

    // The provided hash function has to map a key to an integer 0 or higher.
    // The immediate capacity is the total number of upfront slots (i.e. the maximum possible number of slots for which an O(1) access of a value from a key can happen).
    // The key-value pair capacity is the overall limit of how many key-value pairs this map can ever hold. It obviously has to be equal to or greater than the immediate capacity.
    template<typename tp_key_type, typename tp_val_type>
    [[nodiscard]] t_b8 MakeHashMap(s_mem_arena& mem_arena, const t_hash_func<tp_key_type> hash_func, s_hash_map<tp_key_type, tp_val_type>& o_um, const t_bin_comparator<tp_key_type> key_comparator = DefaultBinComparator, const t_size immediate_cap = 1024, const t_size kv_pair_cap = 1 << 16) {
        ZF_ASSERT(hash_func);
        ZF_ASSERT(key_comparator);
        ZF_ASSERT(immediate_cap > 0 && kv_pair_cap >= immediate_cap);

        o_um = {};

        o_um.hash_func = hash_func;
        o_um.key_comparator = key_comparator;

        if (!MakeArray(mem_arena, immediate_cap, o_um.backing_store_indexes)) {
            return false;
        }

        SetAllTo(o_um.backing_store_indexes, static_cast<t_size>(-1));

        if (!MakeArray(mem_arena, kv_pair_cap, o_um.backing_store.keys)) {
            return false;
        }

        if (!MakeArray(mem_arena, kv_pair_cap, o_um.backing_store.vals)) {
            return false;
        }

        if (!MakeArray(mem_arena, kv_pair_cap, o_um.backing_store.next_indexes)) {
            return false;
        }

        if (!MakeBitVec(mem_arena, kv_pair_cap, o_um.backing_store.usage)) {
            return false;
        }

        return true;
    }

    // This DOES NOT serialize the hash function pointer and binary comparator function pointer!
    template<typename tp_key_type, typename tp_val_type>
    [[nodiscard]] t_b8 SerializeHashMap(s_stream& stream, const s_hash_map<tp_key_type, tp_val_type>& hm) {
        if (!StreamWriteItem(stream, hm.kv_pair_cnt)) {
            return false;
        }

        if (!SerializeArray(stream, hm.backing_store_indexes)) {
            return false;
        }

        if (!SerializeArray(stream, hm.backing_store.keys)) {
            return false;
        }

        if (!SerializeArray(stream, hm.backing_store.vals)) {
            return false;
        }

        if (!SerializeArray(stream, hm.backing_store.next_indexes)) {
            return false;
        }

        if (!SerializeBitVec(stream, hm.backing_store.usage)) {
            return false;
        }

        return true;
    }

    template<typename tp_key_type, typename tp_val_type>
    [[nodiscard]] t_b8 DeserializeHashMap(s_mem_arena& mem_arena, s_stream& stream, const t_hash_func<tp_key_type> hash_func, const t_bin_comparator<tp_key_type> key_comparator, s_hash_map<tp_key_type, tp_val_type>& o_hm) {
        o_hm = {
            .hash_func = hash_func,
            .key_comparator = key_comparator
        };

        if (!StreamReadItem(stream, o_hm.kv_pair_cnt)) {
            return false;
        }

        if (!DeserializeArray(stream, mem_arena, o_hm.backing_store_indexes)) {
            return false;
        }

        if (!DeserializeArray(stream, mem_arena, o_hm.backing_store.keys)) {
            return false;
        }

        if (!DeserializeArray(stream, mem_arena, o_hm.backing_store.vals)) {
            return false;
        }

        if (!DeserializeArray(stream, mem_arena, o_hm.backing_store.next_indexes)) {
            return false;
        }

        if (!DeserializeBitVec(stream, mem_arena, o_hm.backing_store.usage)) {
            return false;
        }

        return true;
    }
#endif
}
