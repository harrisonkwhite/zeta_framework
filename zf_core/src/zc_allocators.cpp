#include <zc/zc_allocators.h>

namespace zf {
    [[nodiscard]] t_b8 MakeMemArena(const t_size size, s_mem_arena& o_ma) {
        ZF_ASSERT(size > 0);

        const auto buf = calloc(static_cast<size_t>(size), 1);

        if (!buf) {
            return false;
        }

        o_ma = {};
        o_ma.buf = buf;
        o_ma.size = size;

        return true;
    }

    void ReleaseMemArena(s_mem_arena& ma) {
        ZF_ASSERT(ma.buf);
        free(ma.buf);
        ma = {};
    }

    void* PushRaw(s_mem_arena& ma, const t_size size, const t_size alignment) {
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
