#include <zc/zc_mem.h>

#include <cstdlib>
#include <cstring>

namespace zf {
    [[nodiscard]] t_b8 MakeMemArena(const t_size size, s_mem_arena& o_ma) {
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

    void ReleaseMemArena(s_mem_arena& ma) {
        ZF_ASSERT(ma.buf);
        free(ma.buf);
        ma = {};
    }

    void RewindMemArena(s_mem_arena& ma, const t_size offs) {
        ZF_ASSERT(ma.buf);
        ZF_ASSERT(offs >= 0 && offs <= ma.offs);

        memset(static_cast<t_u8*>(ma.buf) + offs, 0, static_cast<size_t>(ma.offs - offs));
        ma.offs = offs;
    }

    void* PushToMemArena(s_mem_arena& ma, const t_size size, const t_size alignment) {
        ZF_ASSERT(ma.buf);
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
