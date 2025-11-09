#include <zc/mem/mem.h>

namespace zf {
    t_b8 c_mem_arena::Init(const t_u64 size) {
        ZF_ASSERT(!IsInitted());
        ZF_ASSERT(size > 0);

        m_buf = static_cast<t_u8*>(calloc(size, 1));

        if (!m_buf) {
            return false;
        }

        m_size = size;

        return true;
    }

    void c_mem_arena::Release() {
        ZF_ASSERT(IsInitted());

        free(m_buf);
        *this = {};
    }

    void* c_mem_arena::Push(const t_u64 size, const t_u64 alignment) {
        ZF_ASSERT(IsInitted());

        const t_u64 offs_aligned = AlignForward(m_offs, alignment);
        const t_u64 offs_next = offs_aligned + size;

        if (offs_next > m_size) {
            return nullptr;
        }

        m_offs = offs_next;

        return m_buf + offs_aligned;
    }
}
