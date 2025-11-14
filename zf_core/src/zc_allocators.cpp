#include <zc/zc_allocators.h>

namespace zf {
    t_b8 c_mem_arena::Init(const t_size size) {
        ZF_ASSERT(!m_buf);
        ZF_ASSERT(size > 0);

        m_buf = calloc(static_cast<size_t>(size), 1);

        if (!m_buf) {
            return false;
        }

        m_size = size;

        return true;
    }

    void c_mem_arena::Release() {
        ZF_ASSERT(m_buf);
        free(m_buf);
    }

    void* c_mem_arena::PushRaw(const t_size size, const t_size alignment) {
        ZF_ASSERT(m_buf);
        ZF_ASSERT(size > 0);
        ZF_ASSERT(IsAlignmentValid(alignment));

        const t_size offs_aligned = AlignForward(m_offs, alignment);
        const t_size offs_next = offs_aligned + size;

        if (offs_next > m_size) {
            return nullptr;
        }

        m_offs = offs_next;

        return static_cast<t_s8*>(m_buf) + offs_aligned;
    }
}
