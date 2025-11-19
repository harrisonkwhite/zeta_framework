#pragma once

#include <zc/ds/zc_array.h>

namespace zf {
    template<typename tp_type>
    struct s_queue {
        static_assert(!s_is_const<tp_type>::g_value);

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
        static_assert(!s_is_const<tp_type>::g_value);

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
    tp_type Dequeue(const s_array<const tp_type> backing_arr, t_size& begin_index, t_size& len) {
        ZF_ASSERT(len > 0 && len <= backing_arr.len);

        const tp_type val = backing_arr[begin_index];
        begin_index = (begin_index + 1) % backing_arr.len;
        len--;
        return val;
    }

    template<typename tp_type>
    tp_type Dequeue(s_queue<tp_type>& queue) {
        return Dequeue(static_cast<s_array<const tp_type>>(queue.backing_arr), queue.begin_index, queue.len);
    }

    template<typename tp_type, t_size tp_cap>
    tp_type Dequeue(s_static_queue<tp_type, tp_cap>& queue) {
        return Dequeue(static_cast<s_array<const tp_type>>(queue.backing_arr), queue.begin_index, queue.len);
    }

    template<typename tp_type>
    t_b8 MakeQueue(s_mem_arena& mem_arena, const t_size cap, s_queue<tp_type>& o_queue, const t_size len = 0) {
        ZF_ASSERT(cap > 0 && len >= 0 && len <= cap);

        s_array<tp_type> backing_arr;

        if (!MakeArray(mem_arena, cap, backing_arr)) {
            return false;
        }

        o_queue = {
            .backing_arr = backing_arr,
            .begin_index = 0,
            .len = len
        };

        return true;
    }
}
