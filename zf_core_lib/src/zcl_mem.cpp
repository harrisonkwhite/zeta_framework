#include <zcl/zcl_mem.h>

#include <cstring>

namespace zf {
    t_b8 s_mem_arena::Init(const t_len size) {
        ZF_ASSERT(!IsInitted());
        ZF_ASSERT(size > 0);

        const auto buf = calloc(static_cast<size_t>(size), 1);

        if (!buf) {
            return false;
        }

        m_buf = buf;
        m_size = size;

        return true;
    }

    t_b8 s_mem_arena::InitAsChild(const t_len size, s_mem_arena *const par) {
        ZF_ASSERT(!IsInitted());
        ZF_ASSERT(size > 0);

        const auto buf = par->PushRaw(size, 1);

        if (!buf) {
            return false;
        }

        m_buf = buf;
        m_size = size;
        m_is_child = true;

        return true;
    }

    void s_mem_arena::Release() {
        ZF_ASSERT(IsInitted() && !m_is_child);

        free(m_buf);
        m_buf = nullptr;

        m_size = 0;
        m_offs = 0;
        m_is_child = false;
    }

    void *s_mem_arena::PushRaw(const t_len size, const t_len alignment) {
        ZF_ASSERT(IsInitted());
        ZF_ASSERT(size > 0);
        ZF_ASSERT(IsAlignmentValid(alignment));

        const t_len offs_aligned = AlignForward(m_offs, alignment);
        const t_len offs_next = offs_aligned + size;

        if (offs_next > m_size) {
            return nullptr;
        }

        m_offs = offs_next;

        return static_cast<t_u8 *>(m_buf) + offs_aligned;
    }

    void s_mem_arena::Rewind(const t_len offs) {
        ZF_ASSERT(IsInitted());
        ZF_ASSERT(offs >= 0 && offs <= m_offs);

        memset(static_cast<t_u8 *>(m_buf) + offs, 0, static_cast<size_t>(m_offs - offs));
        m_offs = offs;
    }
}
