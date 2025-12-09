#include <zcl/zcl_mem.h>

#include <cstdlib>
#include <zcl/zcl_io.h>

namespace zf {
    // ============================================================
    // @section: Memory Arenas
    // ============================================================
    t_b8 CreateMemArena(const t_len size, s_mem_arena *o_ma) {
        ZF_ASSERT(size >= 0);

        MarkUninitted(o_ma);

        if (size > 0) {
            const auto buf = Alloc(size);

            if (!buf) {
                return false;
            }

            o_ma->buf = buf;
        } else {
            o_ma->buf = nullptr;
        }

        o_ma->size = size;
        o_ma->offs = 0;

        return true;
    }

    void DestroyMemArena(s_mem_arena *const ma) {
        Free(ma->buf, ma->size);
    }

    void *PushToMemArena(s_mem_arena *const ma, const t_len size, const t_len alignment) {
        ZF_ASSERT(size >= 0);

        if (size == 0) {
            ZF_ASSERT(alignment == 0);
            return nullptr;
        }

        ZF_ASSERT(IsAlignmentValid(alignment));

        const t_len offs_aligned = AlignForward(ma->offs, alignment);
        const t_len offs_next = offs_aligned + size;

        if (offs_next > ma->size) {
            return nullptr;
        }

        ma->offs = offs_next;

        return static_cast<t_u8 *>(ma->buf) + offs_aligned;
    }

    // ============================================================
    // @section: Bits
    // ============================================================
    static t_len IndexOfFirstSetBitHelper(const s_bit_vec_rdonly bv, const t_len from, const t_u8 xor_mask) {
        ZF_ASSERT(IsBitVecValid(bv));
        ZF_ASSERT(from >= 0 && from <= bv.bit_cnt); // Intentionally allowing the upper bound
                                                    // here for the case of iteration.

        // Map of each possible byte to the index of the first set bit, or -1 for the first
        // case.
        static constexpr s_static_array<t_len, 256> g_mappings = {{-1, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0}};

        const t_len begin_byte_index = from / 8;

        for (t_len i = begin_byte_index; i < bv.bytes.len; i++) {
            t_u8 byte = bv.bytes[i];

            if (i == begin_byte_index) {
                byte &= BitmaskRange(from % 8);
            }

            if (i == bv.bytes.len - 1) {
                byte &= BitVecLastByteMask(bv);
            }

            const t_len bi = g_mappings[byte ^ xor_mask];

            if (bi != -1) {
                return (8 * i) + bi;
            }
        }

        return -1;
    }

    t_len IndexOfFirstSetBit(const s_bit_vec_rdonly bv, const t_len from) {
        return IndexOfFirstSetBitHelper(bv, from, 0);
    }

    t_len IndexOfFirstUnsetBit(const s_bit_vec_rdonly bv, const t_len from) {
        return IndexOfFirstSetBitHelper(bv, from, 0xFF);
    }

    t_len CntSetBits(const s_bit_vec_rdonly bv) {
        ZF_ASSERT(IsBitVecValid(bv));

        // Map of each possible byte to the number of set bits in it.
        static constexpr s_static_array<t_len, 256> g_mappings = {{0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8}};

        t_len res = 0;

        if (bv.bytes.len > 0) {
            for (t_len i = 0; i < bv.bytes.len - 1; i++) {
                res += g_mappings[bv.bytes[i]];
            }

            res += g_mappings[bv.bytes[bv.bytes.len - 1] & BitVecLastByteMask(bv)];
        }

        return res;
    }
}
