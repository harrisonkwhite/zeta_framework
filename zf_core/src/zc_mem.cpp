#include <zc/zc_mem.h>

#include <cstdlib>
#include <cstring>

namespace zf {
    static void*& ChunkNextPtr(void* const chunk, const t_size chunk_size) {
        return *reinterpret_cast<void**>(static_cast<t_u8*>(chunk) + chunk_size - sizeof(void*));
    }

    static void FreeChunks(void* const chunk, const t_size chunk_size) {
        if (!chunk) {
            return;
        }

        FreeChunks(ChunkNextPtr(chunk, chunk_size), chunk_size);
        free(chunk);
    }

    static void ClearChunks(void* const chunk, const t_size chunk_size) {
        if (!chunk) {
            return;
        }

        ClearChunks(ChunkNextPtr(chunk, chunk_size), chunk_size);
        memset(chunk, 0, static_cast<size_t>(chunk_size - ZF_SIZE_OF(void*)));
    }

    t_b8 MakeMemArena(s_mem_arena& o_ma, const t_size chunk_size) {
        ZF_ASSERT(chunk_size > 0);

        const auto head_chunk = calloc(static_cast<size_t>(chunk_size), 1);

        if (!head_chunk) {
            return false;
        }

        o_ma = {
            .chunk_size = chunk_size,
            .head_chunk = head_chunk,
            .cur_chunk = head_chunk,
            .cur_chunk_offs = 0
        };

        return true;
    }

    void ReleaseMemArena(s_mem_arena& ma) {
        FreeChunks(ma.head_chunk, ma.chunk_size);
        ma = {};
    }

    void* PushToMemArena(s_mem_arena& ma, const t_size size, const t_size alignment) {
        ZF_ASSERT(size > 0);
        ZF_ASSERT_MSG(size <= ma.chunk_size - ZF_SIZE_OF(void*), "Memory arena push size cannot fit within chunk size (excluding pointer at end)!");
        ZF_ASSERT(IsAlignmentValid(alignment));

        const t_size cur_chunk_offs_aligned = AlignForward(ma.cur_chunk_offs, alignment);
        const t_size cur_chunk_offs_next = cur_chunk_offs_aligned + size;

        if (cur_chunk_offs_next > ma.chunk_size - ZF_SIZE_OF(void*)) {
            auto& cur_chunk_next_ptr = ChunkNextPtr(ma.cur_chunk, ma.chunk_size);

            if (!cur_chunk_next_ptr) {
                const auto next_chunk = calloc(static_cast<size_t>(ma.chunk_size), 1);

                if (!next_chunk) {
                    return nullptr;
                }

                cur_chunk_next_ptr = next_chunk;
            }

            ma.cur_chunk = cur_chunk_next_ptr;
            ma.cur_chunk_offs = 0;

            return PushToMemArena(ma, size, alignment);
        }

        ma.cur_chunk_offs = cur_chunk_offs_next;

        return static_cast<t_u8*>(ma.cur_chunk) + cur_chunk_offs_aligned;
    }

    void ClearMemArena(s_mem_arena& ma) {
        ClearChunks(ma.head_chunk, ma.chunk_size);
        ma.cur_chunk = ma.head_chunk;
        ma.cur_chunk_offs = 0;
    }
}
