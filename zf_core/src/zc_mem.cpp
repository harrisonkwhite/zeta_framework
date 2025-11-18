#include <zc/zc_mem.h>

#include <cstring>

namespace zf {
    [[nodiscard]] t_b8 MakeMemArena(const t_size size, s_mem_arena& o_ma) {
        ZF_ASSERT(size > 0);

        const auto buf = calloc(static_cast<size_t>(size), 1);

        if (!buf) {
            return false;
        }

        o_ma = {buf, size};

        return true;
    }

    void ReleaseMemArena(s_mem_arena& ma) {
        ZF_ASSERT(ma.Buf());
        free(ma.Buf());
        ma = {};
    }

    void RewindMemArena(s_mem_arena& ma, const t_size offs) {
        ZF_ASSERT(ma.Buf());
        ZF_ASSERT(offs >= 0 && offs <= ma.Offs());
        memset(static_cast<t_u8*>(ma.Buf()) + offs, 0, static_cast<size_t>(ma.Offs() - offs));
        ma.SetOffs(offs);
    }

    void* PushToMemArena(s_mem_arena& ma, const t_size size, const t_size alignment) {
        ZF_ASSERT(ma.Buf());
        ZF_ASSERT(size > 0);
        ZF_ASSERT(IsAlignmentValid(alignment));

        const t_size offs_aligned = AlignForward(ma.Offs(), alignment);
        const t_size offs_next = offs_aligned + size;

        if (offs_next > ma.Size()) {
            return nullptr;
        }

        ma.SetOffs(offs_next);

        return static_cast<t_u8*>(ma.Buf()) + offs_aligned;
    }
}
