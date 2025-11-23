#include <zc/zc_mem.h>

#include <cstdlib>

namespace zf {
    t_b8 AllocMemArena(const t_size size, s_mem_arena& o_ma) {
        ZF_ASSERT(size > 0);

        const auto buf = calloc(static_cast<size_t>(size), 1);

        if (!buf) {
            return false;
        }

        o_ma = {
            .buf = buf,
            .size = size,
            .offs = 0
        };

        return true;
    }

    void FreeMemArena(s_mem_arena& ma) {
        free(ma.buf);
        ma = {};
    }

    void* PushToMemArena(s_mem_arena& ma, const t_size size, const t_size alignment) {
        ZF_ASSERT(size > 0);
        ZF_ASSERT(IsAlignmentValid(alignment));

        const t_size offs_aligned = AlignForward(ma.offs, alignment);
        const t_size offs_next = offs_aligned + size;

        if (offs_next > ma.size) {
            return nullptr;
        }

        ma.offs = offs_next;

        return static_cast<t_u8*>(ma.buf) + offs_aligned;
    }
}
