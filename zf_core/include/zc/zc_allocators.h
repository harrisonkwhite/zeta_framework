#pragma once

#include <new>
#include <zc/zc_debug.h>

namespace zf {
    class c_mem_arena {
    public:
        [[nodiscard]] t_b8 Init(const t_size size);
        void Release();

        void* PushRaw(const t_size size, const t_size alignment);
        template<typename tp_type> tp_type* Push(const t_size cnt = 1);

        t_size Size() const {
            return m_size;
        }

        t_size Offs() const {
            return m_offs;
        }

        void Rewind(const t_size offs) {
            ZF_ASSERT(offs <= m_size);
            m_offs = offs;
        }

    private:
        void* m_buf = nullptr;
        t_size m_size = 0;
        t_size m_offs = 0;
    };

    template<typename tp_type>
    tp_type* c_mem_arena::Push(const t_size cnt) {
        ZF_ASSERT(m_buf);
        ZF_ASSERT(cnt >= 1);

        void* const buf_generic = PushRaw(ZF_SIZE_OF(tp_type) * cnt, alignof(tp_type));

        if (!buf_generic) {
            return nullptr;
        }

        const auto buf = static_cast<tp_type*>(buf_generic);

        for (t_size i = 0; i < cnt; i++) {
            new (&buf[i]) tp_type();
        }

        return buf;
    }
}
