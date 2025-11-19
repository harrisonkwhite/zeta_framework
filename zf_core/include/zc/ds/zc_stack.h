#pragma once

#include <zc/ds/zc_array.h>

namespace zf {
    template<typename tp_type>
    struct s_stack {
        static_assert(!s_is_const<tp_type>::g_value);

        s_array<tp_type> backing_arr;
        t_size height;

        tp_type& operator[](const t_size index) const {
            ZF_ASSERT(index < height);
            return backing_arr[index];
        }
    };

    template<typename tp_type, t_size tp_cap>
    struct s_static_stack {
        static_assert(!s_is_const<tp_type>::g_value);

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

        s_array<tp_type> backing_arr;

        if (!MakeArray(mem_arena, cap, backing_arr)) {
            return false;
        }

        o_stack = {backing_arr, height};

        return true;
    }
}
