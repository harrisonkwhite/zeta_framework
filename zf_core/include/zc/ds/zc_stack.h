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
    s_array<tp_type> StackAsArray(const s_stack<tp_type> stack) {
        return Slice(stack.backing_arr, 0, stack.len);
    }

    template<typename tp_type>
    t_b8 IsStackEmpty(const s_stack<tp_type> stack) {
        return stack.len == 0;
    }

    template<typename tp_type, t_size tp_cap>
    t_b8 IsStackEmpty(const s_static_stack<tp_type, tp_cap>& stack) {
        return stack.len == 0;
    }

    template<typename tp_type>
    t_b8 IsStackFull(const s_stack<tp_type> stack) {
        return stack.len == stack.backing_arr.len;
    }

    template<typename tp_type, t_size tp_cap>
    t_b8 IsStackFull(const s_static_stack<tp_type, tp_cap>& stack) {
        return stack.len == tp_cap;
    }

    template<typename tp_type>
    tp_type& StackPush(const s_array<tp_type> backing_arr, t_size& len, const tp_type& val) {
        ZF_ASSERT(len >= 0 && len < backing_arr.len);

        backing_arr[len] = val;
        len++;
        return backing_arr[len - 1];
    }

    template<typename tp_type>
    tp_type& StackPush(s_stack<tp_type>& stack, const tp_type& val) {
        return StackPush(stack.backing_arr, stack.len, val);
    }

    template<typename tp_type, t_size tp_cap>
    tp_type& StackPush(s_static_stack<tp_type, tp_cap>& stack, const tp_type& val) {
        return StackPush(static_cast<s_array<tp_type>>(stack.backing_arr), stack.len, val);
    }

    template<typename tp_type>
    tp_type StackPop(const s_array<tp_type> backing_arr, t_size& len) {
        ZF_ASSERT(len > 0 && len <= backing_arr.len);

        len--;
        return backing_arr[len];
    }

    template<typename tp_type>
    tp_type StackPop(s_stack<tp_type>& stack) {
        return StackPop(stack.backing_arr, stack.len);
    }

    template<typename tp_type, t_size tp_cap>
    tp_type StackPop(s_static_stack<tp_type, tp_cap>& stack) {
        return StackPop(static_cast<s_array<tp_type>>(stack.backing_arr), stack.len);
    }

    template<typename tp_type>
    t_b8 MakeStack(s_mem_arena& mem_arena, const t_size cap, s_stack<tp_type>& o_stack, const t_size len = 0) {
        ZF_ASSERT(cap > 0 && len >= 0 && len <= cap);

        s_array<tp_type> backing_arr;

        if (!MakeArray(mem_arena, cap, backing_arr)) {
            return false;
        }

        o_stack = {backing_arr, len};

        return true;
    }
}
