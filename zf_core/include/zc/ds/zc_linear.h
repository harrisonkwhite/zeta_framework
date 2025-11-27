#pragma once

#include <zc/zc_mem.h>

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
        return ListAppend(static_cast<s_array<tp_type>>(list.backing_arr), list.len, val);
    }

    template<typename tp_type>
    void ListInsert(const s_array<tp_type> backing_arr, t_size& len, const t_size index, const tp_type& val) {
        ZF_ASSERT(len >= 0 && len < backing_arr.len);
        ZF_ASSERT(index >= 0 && index <= len);

        CopyReverse(Slice(backing_arr, index + 1, len + 1), Slice(backing_arr, index, len));

        len++;
        backing_arr[index] = val;
    }

    template<typename tp_type>
    void ListInsert(s_list<tp_type>& list, const t_size index, const tp_type& val) {
        ListInsert(list.backing_arr, list.len, index, val);
    }

    template<typename tp_type, t_size tp_cap>
    void ListInsert(s_static_list<tp_type, tp_cap>& list, const t_size index, const tp_type& val) {
        ListInsert(static_cast<s_array<tp_type>>(list.backing_arr), list.len, index, val);
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
        return ListRemoveLast(static_cast<s_array<tp_type>>(list.backing_arr), list.len);
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
        ListRemoveSwapback(static_cast<s_array<tp_type>>(list.backing_arr), list.len, index);
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
        ListRemove(static_cast<s_array<tp_type>>(list.backing_arr), list.len, index);
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
        return StackPush(static_cast<s_array<tp_type>>(stack.backing_arr), stack.height, val);
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
        return StackPop(static_cast<s_array<tp_type>>(stack.backing_arr), stack.height);
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
        return Enqueue(static_cast<s_array<tp_type>>(queue.backing_arr), queue.begin_index, queue.len, val);
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
        return Dequeue(static_cast<s_array_rdonly<tp_type>>(queue.backing_arr), queue.begin_index, queue.len);
    }

    template<typename tp_type, t_size tp_cap>
    tp_type Dequeue(s_static_queue<tp_type, tp_cap>& queue) {
        return Dequeue(static_cast<s_array_rdonly<tp_type>>(queue.backing_arr), queue.begin_index, queue.len);
    }

    template<typename tp_type>
    [[nodiscard]] t_b8 MakeQueue(s_mem_arena& mem_arena, const t_size cap, s_queue<tp_type>& o_queue, const t_size len = 0) {
        ZF_ASSERT(cap > 0 && len >= 0 && len <= cap);

        o_queue = {
            .len = len
        };

        return MakeArray(mem_arena, cap, o_queue.backing_arr);
    }
}
