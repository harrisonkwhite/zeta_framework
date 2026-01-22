#include <zcl/zcl_basic.h>

namespace zcl {
    void ArenaDestroy(t_arena *const arena) {
        ZCL_ASSERT(arena->type == ek_arena_type_block_based);

        const auto f = [](const auto self, t_arena_block *const block) {
            if (!block) {
                return;
            }

            self(self, block->next);

            free(block->buf);
            free(block);
        };

        f(f, arena->type_data.block_based.blocks_head);

        *arena = {};
    }

    static t_arena_block *ArenaCreateBlock(const t_i32 buf_size) {
        ZCL_ASSERT(buf_size > 0);

        const auto block = static_cast<t_arena_block *>(malloc(sizeof(t_arena_block)));

        if (!block) {
            ZCL_FATAL();
        }

        ZeroClearItem(block);

        block->buf = malloc(static_cast<size_t>(buf_size));

        if (!block->buf) {
            ZCL_FATAL();
        }

#ifdef ZCL_DEBUG
        memset(block->buf, k_arena_poison, static_cast<size_t>(block->buf_size));
#else
        // Explicitly touch the memory to trigger page faults NOW.
        ZeroClear(block->buf, buf_size);
#endif

        block->buf_size = buf_size;

        return block;
    }

    void *ArenaPush(t_arena *const arena, const t_i32 size, const t_i32 alignment) {
        ZCL_ASSERT(size > 0 && AlignmentCheckValid(alignment));

        switch (arena->type) {
        case ek_arena_type_block_based: {
            const auto block_based = &arena->type_data.block_based;

            if (!block_based->blocks_head) {
                block_based->blocks_head = ArenaCreateBlock(CalcMax(size, block_based->block_min_size));
                block_based->block_cur = block_based->blocks_head;
                return ArenaPush(arena, size, alignment);
            }

            const t_i32 offs_aligned = AlignForward(block_based->block_cur_offs, alignment);
            const t_i32 offs_next = offs_aligned + size;

            if (offs_next > block_based->block_cur->buf_size) {
                if (!block_based->block_cur->next) {
                    block_based->block_cur->next = ArenaCreateBlock(CalcMax(size, block_based->block_min_size));
                }

                block_based->block_cur = block_based->block_cur->next;
                block_based->block_cur_offs = 0;

                return ArenaPush(arena, size, alignment);
            }

            block_based->block_cur_offs = offs_next;

            void *const result = static_cast<t_u8 *>(block_based->block_cur->buf) + offs_aligned;
            ZeroClear(result, size);

            return result;
        }

        case ek_arena_type_wrapping: {
            const auto wrapping = &arena->type_data.wrapping;

            const t_i32 offs_aligned = AlignForward(wrapping->buf_offs, alignment);
            const t_i32 offs_next = offs_aligned + size;

            if (offs_next > wrapping->buf_size) {
                ZCL_FATAL();
            }

            wrapping->buf_offs = offs_next;

            void *const result = static_cast<t_u8 *>(wrapping->buf) + offs_aligned;
            ZeroClear(result, size);

            return result;
        }

        default:
            ZCL_UNREACHABLE();
        }
    }

    void ArenaRewind(t_arena *const arena) {
        switch (arena->type) {
        case ek_arena_type_block_based: {
            const auto block_based = &arena->type_data.block_based;

#ifdef ZCL_DEBUG
            // Poison all memory to be rewinded.
            if (block_based->block_cur) {
                const t_arena_block *block = block_based->blocks_head;

                while (block != block_based->block_cur) {
                    memset(block->buf, k_arena_poison, static_cast<size_t>(block->buf_size));
                    block = block->next;
                }

                memset(block_based->block_cur->buf, k_arena_poison, static_cast<size_t>(block_based->block_cur_offs));
            }
#endif

            block_based->block_cur = block_based->blocks_head;
            block_based->block_cur_offs = 0;

            break;
        }

        case ek_arena_type_wrapping: {
            const auto wrapping = &arena->type_data.wrapping;
            ZeroClear(wrapping->buf, wrapping->buf_offs);
            wrapping->buf_offs = 0;
            break;
        }

        default:
            ZCL_UNREACHABLE();
        }
    }
}
