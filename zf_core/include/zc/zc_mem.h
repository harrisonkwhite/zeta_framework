#pragma once

#include <cstring>
#include <zc/zc_debug.h>

namespace zf {
    constexpr t_size Kilobytes(const t_size x) { return (static_cast<t_size>(1) << 10) * x; }
    constexpr t_size Megabytes(const t_size x) { return (static_cast<t_size>(1) << 20) * x; }
    constexpr t_size Gigabytes(const t_size x) { return (static_cast<t_size>(1) << 30) * x; }
    constexpr t_size Terabytes(const t_size x) { return (static_cast<t_size>(1) << 40) * x; }

    constexpr t_size BitsToBytes(const t_size x) { return (x + 7) / 8; }
    constexpr t_size BytesToBits(const t_size x) { return x * 8; }

    // Is n a power of 2?
    constexpr t_b8 IsAlignmentValid(const t_size n) {
        return n > 0 && (n & (n - 1)) == 0;
    }

    // Take n up to the next multiple of the alignment.
    constexpr t_size AlignForward(const t_size n, const t_size alignment) {
        ZF_ASSERT(IsAlignmentValid(alignment));
        return (n + alignment - 1) & ~(alignment - 1);
    }

    struct s_mem_arena {
        void* buf;
        t_size size;
        t_size offs;
    };

    [[nodiscard]] t_b8 AllocMemArena(const t_size size, s_mem_arena& o_ma);
    void FreeMemArena(s_mem_arena& ma);
    void* PushToMemArena(s_mem_arena& ma, const t_size size, const t_size alignment);

    template<typename tp_type>
    tp_type* PushToMemArena(s_mem_arena& ma, const t_size cnt = 1) {
        ZF_ASSERT(cnt >= 1);

        const auto buf = PushToMemArena(ma, ZF_SIZE_OF(tp_type) * cnt, alignof(tp_type));
        return static_cast<tp_type*>(buf);
    }

    inline void RewindMemArena(s_mem_arena& ma, const t_size offs) {
        ZF_ASSERT(offs >= 0 && offs <= ma.offs);
        memset(static_cast<t_u8*>(ma.buf) + offs, 0, static_cast<size_t>(ma.offs - offs));
        ma.offs = offs;
    }

#define ZF_DEFER_MEM_ARENA_REWIND(arena) \
    do { \
        const zf::t_size arena_begin_offs = arena.offs; \
        ZF_DEFER({ zf::RewindMemArena(arena, arena_begin_offs); }); \
    } while(0) \

    [[nodiscard]] inline t_b8 MakeSubMemArena(s_mem_arena& parent_ma, const t_size size, s_mem_arena& o_ma) {
        o_ma = {
            .buf = PushToMemArena(parent_ma, size, 1),
            .size = size
        };

        return o_ma.buf != nullptr;
    }
}
