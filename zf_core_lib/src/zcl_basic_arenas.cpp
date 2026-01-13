#include <zcl/zcl_basic.h>

namespace zcl {
    void arena_destroy(t_arena *const arena) {
        ZCL_REQUIRE(arena->type == ek_arena_type_blockbased);

        const auto f = [](const auto self, t_arena_block *const block) {
            if (!block) {
                return;
            }

            self(self, block->next);

            free(block->buf);
            free(block);
        };

        f(f, arena->type_data.blockbased.blocks_head);

        *arena = {};
    }

    static t_arena_block *arena_create_block(const t_i32 buf_size) {
        ZCL_REQUIRE(buf_size > 0);

        const auto block = static_cast<t_arena_block *>(malloc(sizeof(t_arena_block)));

        if (!block) {
            ZCL_FATAL();
        }

        zero_clear_item(block);

        block->buf = malloc(static_cast<size_t>(buf_size));

        if (!block->buf) {
            ZCL_FATAL();
        }

#ifdef ZCL_DEBUG
        memset(block->buf, k_arena_poison, static_cast<size_t>(block->buf_size));
#elif
        // Explicitly touch the memory to trigger page faults NOW.
        zero_clear(block->buf, buf_size);
#endif

        block->buf_size = buf_size;

        return block;
    }

    void *arena_push(t_arena *const arena, const t_i32 size, const t_i32 alignment) {
        ZCL_ASSERT(size > 0 && alignment_check_valid(alignment));

        switch (arena->type) {
        case ek_arena_type_blockbased: {
            const auto blockbased = &arena->type_data.blockbased;

            if (!blockbased->blocks_head) {
                blockbased->blocks_head = arena_create_block(calc_max(size, blockbased->block_min_size));
                blockbased->block_cur = blockbased->blocks_head;
                return arena_push(arena, size, alignment);
            }

            const t_i32 offs_aligned = align_forward(blockbased->block_cur_offs, alignment);
            const t_i32 offs_next = offs_aligned + size;

            if (offs_next > blockbased->block_cur->buf_size) {
                if (!blockbased->block_cur->next) {
                    blockbased->block_cur->next = arena_create_block(calc_max(size, blockbased->block_min_size));
                }

                blockbased->block_cur = blockbased->block_cur->next;
                blockbased->block_cur_offs = 0;

                return arena_push(arena, size, alignment);
            }

            blockbased->block_cur_offs = offs_next;

            void *const result = static_cast<t_u8 *>(blockbased->block_cur->buf) + offs_aligned;
            zero_clear(result, size);

            return result;
        }

        case ek_arena_type_wrapping: {
            const auto wrapping = &arena->type_data.wrapping;

            const t_i32 offs_aligned = align_forward(wrapping->buf_offs, alignment);
            const t_i32 offs_next = offs_aligned + size;

            if (offs_next > wrapping->buf_size) {
                ZCL_FATAL();
            }

            wrapping->buf_offs = offs_next;

            void *const result = static_cast<t_u8 *>(wrapping->buf) + offs_aligned;
            zero_clear(result, size);

            return result;
        }

        default:
            ZCL_UNREACHABLE();
        }
    }

    void arena_rewind(t_arena *const arena) {
        switch (arena->type) {
        case ek_arena_type_blockbased: {
            const auto blockbased = &arena->type_data.blockbased;

#ifdef ZCL_DEBUG
            // Poison all memory to be rewinded.
            if (blockbased->block_cur) {
                const t_arena_block *block = blockbased->blocks_head;

                while (block != blockbased->block_cur) {
                    memset(block->buf, k_arena_poison, static_cast<size_t>(block->buf_size));
                    block = block->next;
                }

                memset(blockbased->block_cur->buf, k_arena_poison, static_cast<size_t>(blockbased->block_cur_offs));
            }
#endif

            blockbased->block_cur = blockbased->blocks_head;
            blockbased->block_cur_offs = 0;

            break;
        }

        case ek_arena_type_wrapping: {
            const auto wrapping = &arena->type_data.wrapping;
            zero_clear(wrapping->buf, wrapping->buf_offs);
            wrapping->buf_offs = 0;
            break;
        }

        default:
            ZCL_UNREACHABLE();
        }
    }
}
