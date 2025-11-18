#pragma once

#include <new>
#include <zc/zc_debug.h>

namespace zf {
    struct s_mem_arena {
        s_mem_arena() = default;

        s_mem_arena(void* const buf, const t_size size) : buf(buf), size(size) {
            ZF_ASSERT((!buf && size == 0) || (buf && size > 0));
        }

        void* Buf() const { return buf; }
        t_size Size() const { return size; }
        t_size Offs() const { return offs; }

        void SetOffs(const t_size val) {
            ZF_ASSERT(val >= 0 && val <= size);
            offs = val;
        }

    private:
        void* buf = nullptr;
        t_size size = 0;
        t_size offs = 0;
    };

    [[nodiscard]] t_b8 MakeMemArena(const t_size size, s_mem_arena& o_ma);
    void ReleaseMemArena(s_mem_arena& ma);
    void RewindMemArena(s_mem_arena& ma, const t_size offs);
    void* PushToMemArena(s_mem_arena& ma, const t_size size, const t_size alignment);

    template<typename tp_type>
    tp_type* PushToMemArena(s_mem_arena& ma, const t_size cnt = 1) {
        ZF_ASSERT(ma.Buf());
        ZF_ASSERT(cnt >= 1);

        void* const buf_generic = PushToMemArena(ma, ZF_SIZE_OF(tp_type) * cnt, alignof(tp_type));

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
