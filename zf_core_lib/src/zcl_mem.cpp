#include <zcl/zcl_mem.h>

#include <cstring>

namespace zf {
    t_b8 s_mem_arena::Init(const t_len size) {
        ZF_ASSERT(!m_initted);
        ZF_ASSERT(size > 0);

        const auto buf = malloc(static_cast<size_t>(size));

        if (!buf) {
            return false;
        }

        m_buf = buf;
        m_size = size;

        m_initted = true;

        return true;
    }

    t_b8 s_mem_arena::InitAsChild(const t_len size, s_mem_arena *const par) {
        ZF_ASSERT(!m_initted);
        ZF_ASSERT(size > 0);

        const auto buf = par->PushRaw(size, 1);

        if (!buf) {
            return false;
        }

        m_parent = par;
        m_buf = buf;
        m_size = size;

        m_initted = true;

        return true;
    }

    void s_mem_arena::Release() {
        ZF_ASSERT(IsActive() && !m_parent);

        free(m_buf);
        m_buf = nullptr;
    }

    void *s_mem_arena::PushRaw(const t_len size, const t_len alignment) {
        ZF_ASSERT(IsActive());
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

    // ============================================================
    // @section: Bits
    // ============================================================
    static t_len IndexOfFirstSetBitHelper(const s_bit_vec_rdonly bv, const t_len from, const t_u8 xor_mask) {
        ZF_ASSERT(from >= 0 && from <= bv.BitCount()); // Intentionally allowing the upper bound here for the case of iteration.

        // Map of each possible byte to the index of the first set bit, or -1 for the first case.
        static constexpr s_static_array<t_len, 256> g_mappings = {{-1, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0}};

        const t_len begin_byte_index = from / 8;

        for (t_len i = begin_byte_index; i < bv.Bytes().Len(); i++) {
            t_u8 byte = bv.Bytes()[i];

            if (i == begin_byte_index) {
                byte &= BitmaskRange(from % 8);
            }

            if (i == bv.Bytes().Len() - 1) {
                byte &= bv.LastByteMask();
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
        // Map of each possible byte to the number of set bits in it.
        static constexpr s_static_array<t_len, 256> g_mappings = {{0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8}};

        t_len res = 0;

        if (bv.Bytes().Len() > 0) {
            for (t_len i = 0; i < bv.Bytes().Len() - 1; i++) {
                res += g_mappings[bv.Bytes()[i]];
            }

            res += g_mappings[bv.Bytes()[bv.Bytes().Len() - 1] & bv.LastByteMask()];
        }

        return res;
    }
}
