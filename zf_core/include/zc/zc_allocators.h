#pragma once

#include <new>
#include <zc/zc_debug.h>

namespace zf {
    struct s_mem_arena {
        void* buf;
        t_size size;
        t_size offs;

        s_mem_arena() = default;
    };

    [[nodiscard]] t_b8 MakeMemArena(const t_size size, s_mem_arena& o_ma);
    void ReleaseMemArena(s_mem_arena& ma);
    void* PushRaw(s_mem_arena& ma, const t_size size, const t_size alignment);

    template<typename tp_type>
    tp_type* Push(s_mem_arena& ma, const t_size cnt) {
        ZF_ASSERT(ma.buf);
        ZF_ASSERT(cnt >= 1);

        void* const buf_generic = PushRaw(ma, ZF_SIZE_OF(tp_type) * cnt, alignof(tp_type));

        if (!buf_generic) {
            return nullptr;
        }

        const auto buf = static_cast<tp_type*>(buf_generic);

        for (t_size i = 0; i < cnt; i++) {
            new (&buf[i]) tp_type();
        }

        return buf;
    }

    inline void Rewind(s_mem_arena& ma, const t_size offs) {
        ZF_ASSERT(offs >= 0 && offs <= ma.size);
        ma.offs = offs;
    }
}
