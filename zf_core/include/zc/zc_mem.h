#pragma once

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
        t_size chunk_size;
        void* head_chunk;
        void* cur_chunk;
        t_size cur_chunk_offs;
    };

    [[nodiscard]] t_b8 MakeMemArena(s_mem_arena& o_ma, const t_size chunk_size = Megabytes(1));
    void ReleaseMemArena(s_mem_arena& ma);
    void RewindMemArena(s_mem_arena& ma, const t_size offs);
    void* PushToMemArena(s_mem_arena& ma, const t_size size, const t_size alignment);
    void ClearMemArena(s_mem_arena& ma); // Zeroes out all the arena memory and goes back to the start but does not do any freeing.

    template<typename tp_type>
    tp_type* PushToMemArena(s_mem_arena& ma, const t_size cnt = 1) {
        ZF_ASSERT(ma.head_chunk);
        ZF_ASSERT(cnt >= 1);

        const auto buf = PushToMemArena(ma, ZF_SIZE_OF(tp_type) * cnt, alignof(tp_type));
        return static_cast<tp_type*>(buf);
    }
}
