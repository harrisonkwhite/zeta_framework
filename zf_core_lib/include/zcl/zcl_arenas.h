#pragma once

#include <zcl/zcl_basic.h>

namespace zcl {
    struct t_arena_block {
        void *buf;
        t_i32 buf_size;

        t_arena_block *next;
    };

    enum t_arena_type : t_i32 {
        ek_arena_type_invalid,
        ek_arena_type_blockbased, // Owns its memory, which is organised as a linked list of dynamically allocated blocks. New blocks are allocated as needed.
        ek_arena_type_wrapping    // Non-owning and non-reallocating. Useful if you want to leverage a stack-allocated buffer for example. @todo: Probably not a good name.
    };

    struct t_arena {
        t_arena_type type;

        union {
            struct {
                t_arena_block *blocks_head;
                t_arena_block *block_cur;
                t_i32 block_cur_offs;
                t_i32 block_min_size;
            } blockbased;

            struct {
                void *buf;
                t_i32 buf_size;
                t_i32 buf_offs;
            } wrapping;
        } type_data;
    };

#ifdef ZF_DEBUG
    constexpr t_u8 k_arena_poison = 0xCD; // Memory outside the arena's valid "scope" is set to this for easier debugging.
#endif

    // Does not allocate any arena memory (blocks) upfront.
    inline t_arena arena_create_blockbased(const t_i32 block_min_size = megabytes_to_bytes(1)) {
        ZF_ASSERT(block_min_size > 0);

        return {
            .type = ek_arena_type_blockbased,
            .type_data = {.blockbased = {.block_min_size = block_min_size}},
        };
    }

    inline t_arena arena_create_wrapping(const t_array_mut<t_u8> bytes) {
        array_set_all_to(bytes, 0);

        return {
            .type = ek_arena_type_wrapping,
            .type_data = {.wrapping = {.buf = bytes.raw, .buf_size = bytes.len}},
        };
    }

    // Frees all arena memory. Only valid for block-based arenas. This can be called even if no pushing was done.
    void arena_destroy(t_arena *const arena);

    // Will lazily allocate memory as needed. Allocation failure is treated as fatal and causes an abort - you don't need to check for nullptr.
    // The returned buffer is guaranteed to be zeroed.
    void *arena_push(t_arena *const arena, const t_i32 size, const t_i32 alignment);

    // Will lazily allocate memory as needed. Allocation failure is treated as fatal and causes an abort - you don't need to check for nullptr.
    // The returned item is guaranteed to be zeroed.
    template <c_simple tp_type>
    tp_type *arena_push_item(t_arena *const arena) {
        return static_cast<tp_type *>(arena_push(arena, ZF_SIZE_OF(tp_type), ZF_ALIGN_OF(tp_type)));
    }

    template <c_array_elem tp_elem_type>
    t_array_mut<tp_elem_type> arena_push_array(t_arena *const arena, const t_i32 len) {
        ZF_ASSERT(len >= 0);

        if (len == 0) {
            return {};
        }

        const t_i32 size = ZF_SIZE_OF(tp_elem_type) * len;
        return {static_cast<tp_elem_type *>(arena_push(arena, size, ZF_ALIGN_OF(tp_elem_type))), len};
    }

    template <c_array tp_arr_type>
    auto arena_push_array_clone(t_arena *const arena, const tp_arr_type arr_to_clone) {
        const auto arr = arena_push_array<typename tp_arr_type::t_elem>(arena, arr_to_clone.len);
        array_copy(arr, arr_to_clone);
        return arr;
    }

    // Takes the arena offset to the beginning of its memory (if any) to overwrite from there.
    void arena_rewind(t_arena *const arena);
}
