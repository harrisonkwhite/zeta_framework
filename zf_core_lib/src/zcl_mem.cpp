#include <zcl/zcl_mem.h>

#include <cstdlib>
#include <zcl/zcl_io.h>

namespace zf {
    // ============================================================
    // @section: Memory Arenas
    // ============================================================
    t_b8 InitMemArena(s_mem_arena *ma, const t_size size) {
        ZF_ASSERT(size >= 0);

        ZeroOut(ma);

        if (size > 0) {
            const auto buf = calloc(static_cast<size_t>(size), 1);

            if (!buf) {
                return false;
            }

            ma->buf = buf;
        }

        ma->size = size;

        return true;
    }

    void ReleaseMemArena(s_mem_arena *const ma) {
        if (ma->buf) {
            free(ma->buf);
        }
    }

    void *PushToMemArena(s_mem_arena *const ma, const t_size size, const t_size alignment) {
        ZF_ASSERT(size >= 0);

        if (size == 0) {
            ZF_ASSERT(alignment == 0);
            return nullptr;
        }

        ZF_ASSERT(IsAlignmentValid(alignment));

        const t_size offs_aligned = AlignForward(ma->offs, alignment);
        const t_size offs_next = offs_aligned + size;

        if (offs_next > ma->size) {
            return nullptr;
        }

        ma->offs = offs_next;

        return static_cast<t_u8 *>(ma->buf) + offs_aligned;
    }

    // ============================================================
    // @section: Bits
    // ============================================================
    static t_size IndexOfFirstSetBitHelper(const s_bit_vec_rdonly &bv, const t_size from,
                                           const t_u8 xor_mask) {
        ZF_ASSERT(IsBitVecValid(bv));
        ZF_ASSERT(from >= 0 && from <= bv.bit_cnt); // Intentionally allowing the upper bound
                                                    // here for the case of iteration.

        // Map of each possible byte to the index of the first set bit, or -1 for the first
        // case.
        static constexpr s_static_array<t_size, 256> g_mappings = {
            {-1, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0,
             1,  0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0,
             2,  0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0,
             1,  0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0,
             3,  0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 7, 0,
             1,  0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0,
             2,  0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0,
             1,  0, 3, 0, 1, 0, 2, 0, 1, 0, 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
             4,  0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0,
             1,  0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0}};

        const t_size begin_byte_index = from / 8;

        for (t_size i = begin_byte_index; i < bv.bytes.len; i++) {
            t_u8 byte = bv.bytes[i];

            if (i == begin_byte_index) {
                byte &= BitmaskRange(from % 8);
            }

            if (i == bv.bytes.len - 1) {
                byte &= BitVecLastByteMask(bv);
            }

            const t_size bi = g_mappings[byte ^ xor_mask];

            if (bi != -1) {
                return (8 * i) + bi;
            }
        }

        return -1;
    }

    t_size IndexOfFirstSetBit(const s_bit_vec_rdonly &bv, const t_size from) {
        return IndexOfFirstSetBitHelper(bv, from, 0);
    }

    t_size IndexOfFirstUnsetBit(const s_bit_vec_rdonly &bv, const t_size from) {
        return IndexOfFirstSetBitHelper(bv, from, 0xFF);
    }

    t_size CntSetBits(const s_bit_vec_rdonly &bv) {
        ZF_ASSERT(IsBitVecValid(bv));

        // Map of each possible byte to the number of set bits in it.
        static constexpr s_static_array<t_size, 256> g_mappings = {
            {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3,
             3, 4, 3, 4, 4, 5, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4,
             3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4,
             4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5,
             3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 1, 2,
             2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5,
             4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4,
             4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
             3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5,
             5, 6, 5, 6, 6, 7, 4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8}};

        t_size res = 0;

        if (bv.bytes.len > 0) {
            for (t_size i = 0; i < bv.bytes.len - 1; i++) {
                res += g_mappings[bv.bytes[i]];
            }

            res += g_mappings[bv.bytes[bv.bytes.len - 1] & BitVecLastByteMask(bv)];
        }

        return res;
    }
}
