#pragma once

#include <new>
#include <zc/zc_debug.h>

namespace zf {
    constexpr t_size Kilobytes(const t_size x) { return (static_cast<t_size>(1) << 10) * x; }
    constexpr t_size Megabytes(const t_size x) { return (static_cast<t_size>(1) << 20) * x; }
    constexpr t_size Gigabytes(const t_size x) { return (static_cast<t_size>(1) << 30) * x; }
    constexpr t_size Terabytes(const t_size x) { return (static_cast<t_size>(1) << 40) * x; }

    constexpr t_size BitsToBytes(const t_size x) { return (x + 7) / 8; }
    constexpr t_size BytesToBits(const t_size x) { return x * 8; }

    constexpr t_b8 IsAlignmentValid(const t_size n) {
        // Is it a power of 2?
        return n > 0 && (n & (n - 1)) == 0;
    }

    constexpr t_size AlignForward(const t_size n, const t_size alignment) {
        return (n + alignment - 1) & ~(alignment - 1);
    }

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
