#include <zc/zc_allocators.h>

namespace zf {
    t_b8 s_mem_arena::Init(const t_size size) {
        ZF_ASSERT(!buf);
        ZF_ASSERT(size > 0);

        buf = calloc(static_cast<size_t>(size), 1);

        if (!buf) {
            return false;
        }

        this->size = size;

        return true;
    }

    void s_mem_arena::Release() {
        ZF_ASSERT(buf);
        free(buf);
    }

    void* s_mem_arena::PushRaw(const t_size size, const t_size alignment) {
        ZF_ASSERT(buf);
        ZF_ASSERT(size > 0);
        ZF_ASSERT(IsAlignmentValid(alignment));

        const t_size offs_aligned = AlignForward(offs, alignment);
        const t_size offs_next = offs_aligned + size;

        if (offs_next > this->size) {
            return nullptr;
        }

        offs = offs_next;

        return static_cast<t_s8*>(buf) + offs_aligned;
    }
}
