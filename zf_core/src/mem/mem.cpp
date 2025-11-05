#include <zc/mem/mem.h>

namespace zf {
    bool c_mem_arena::Init(const size_t size) {
        ZF_ASSERT(!IsInitted());
        ZF_ASSERT(size > 0);

        m_buf = static_cast<t_byte*>(calloc(size, 1));

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

    void* c_mem_arena::Push(const size_t size, const size_t alignment) {
        ZF_ASSERT(IsInitted());

        const size_t offs_aligned = AlignForward(m_offs, alignment);
        const size_t offs_next = offs_aligned + size;

        if (offs_next > m_size) {
            return nullptr;
        }

        m_offs = offs_next;

        return m_buf + offs_aligned;
    }
}
