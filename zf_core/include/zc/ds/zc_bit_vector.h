#pragma once

#include <zc/ds/zc_array.h>

namespace zf {
    constexpr t_u8 ByteBitmask(const t_size bit_index) {
        ZF_ASSERT(bit_index >= 0 && bit_index < 8);
        return static_cast<t_u8>(1 << bit_index);
    }

    constexpr t_u8 ByteBitmask(const t_size begin_bit_index, const t_size end_bit_index) {
        ZF_ASSERT(begin_bit_index >= 0 && begin_bit_index < 8);
        ZF_ASSERT(end_bit_index > begin_bit_index && end_bit_index <= 8);

        const auto bits_at_bottom = static_cast<t_u8>((1 << (end_bit_index - begin_bit_index)) - 1);
        return static_cast<t_u8>(bits_at_bottom << begin_bit_index);
    }

    class c_bit_vector_rdonly {
    public:
        constexpr c_bit_vector_rdonly() = default;
        constexpr c_bit_vector_rdonly(const c_array<const t_u8> bytes)
            : m_bytes(bytes), m_bit_cnt(BytesToBits(bytes.Len())) {}
        constexpr c_bit_vector_rdonly(const c_array<const t_u8> bytes, const t_size bit_cnt)
            : m_bytes(bytes), m_bit_cnt(bit_cnt) {
            ZF_ASSERT(bytes.Len() == BitsToBytes(bit_cnt));
        }

        constexpr c_array<const t_u8> Bytes() const {
            return m_bytes;
        }

        constexpr t_size BitCount() const {
            return m_bit_cnt;
        }

    private:
        c_array<const t_u8> m_bytes;
        t_size m_bit_cnt;
    };

    class c_bit_vector {
    public:
        constexpr c_bit_vector() = default;
        constexpr c_bit_vector(const c_array<t_u8> bytes)
            : m_bytes(bytes), m_bit_cnt(BytesToBits(bytes.Len())) {}
        constexpr c_bit_vector(const c_array<t_u8> bytes, const t_size bit_cnt)
            : m_bytes(bytes), m_bit_cnt(bit_cnt) {
            ZF_ASSERT(bytes.Len() == BitsToBytes(bit_cnt));
        }

        constexpr c_array<t_u8> Bytes() const {
            return m_bytes;
        }

        constexpr t_size BitCount() const {
            return m_bit_cnt;
        }

        constexpr operator c_bit_vector_rdonly() const {
            return {m_bytes, m_bit_cnt};
        }

    private:
        c_array<t_u8> m_bytes;
        t_size m_bit_cnt;
    };

    template<t_size tp_bit_cnt>
    struct s_static_bit_vector {
        s_static_array<t_u8, BitsToBytes(tp_bit_cnt)> bytes;

        constexpr operator c_bit_vector() {
            return {bytes, tp_bit_cnt};
        }

        constexpr operator c_bit_vector_rdonly() const {
            return {bytes, tp_bit_cnt};
        }
    };

    t_b8 MakeBitVector(c_mem_arena& mem_arena, const t_size bit_cnt, c_bit_vector& o_bv);

    inline t_u8 BitVectorLastByteBitmask(const c_bit_vector_rdonly bv) {
        ZF_ASSERT(bv.BitCount() > 0);

        const t_size bit_cnt = bv.BitCount() - (8 * (bv.Bytes().Len() - 1));
        return ByteBitmask(0, bit_cnt);
    }

    inline t_u8 BitVectorLastByte(const c_bit_vector_rdonly bv) {
        ZF_ASSERT(bv.BitCount() > 0);
        return bv.Bytes()[bv.Bytes().Len() - 1] & BitVectorLastByteBitmask(bv);
    }

    inline t_b8 AreAllBitsSet(const c_bit_vector_rdonly bv) {
        ZF_ASSERT(bv.BitCount() > 0);

        for (t_size i = 0; i < bv.Bytes().Len() - 1; i++) {
            if (bv.Bytes()[i] != 0xFF) {
                return false;
            }
        }

        return BitVectorLastByte(bv) == 0xFF;
    }

    inline t_b8 IsAnyBitSet(const c_bit_vector_rdonly bv) {
        for (t_size i = 0; i < bv.Bytes().Len() - 1; i++) {
            if (bv.Bytes()[i] != 0) {
                return true;
            }
        }

        return BitVectorLastByte(bv) != 0;
    }

    inline t_b8 AreAllBitsUnset(const c_bit_vector_rdonly bv) {
        return !IsAnyBitSet(bv);
    }

    inline t_b8 IsAnyBitUnset(const c_bit_vector_rdonly bv) {
        return !AreAllBitsSet(bv);
    }

    void ShiftLeft(const c_bit_vector bv, const t_size amount = 1);
    void RotLeft(const c_bit_vector bv, const t_size amount = 1);

    // @todo: Right (either arithmetic or logical) shift and right rotate.

    t_size FindFirstSetBit(const c_bit_vector_rdonly bv, const t_size from = 0, const t_b8 inverted = false);

    inline t_size FindFirstUnsetBit(const c_bit_vector_rdonly bv, const t_size from = 0) {
        return FindFirstSetBit(bv, from, true);
    }

    constexpr t_b8 IsBitSet(const c_bit_vector_rdonly bv, const t_size index) {
        ZF_ASSERT(index >= 0 && index < bv.BitCount());
        return bv.Bytes()[index / 8] & (1 << (index % 8));
    }

    constexpr void SetBit(const c_bit_vector bv, const t_size index) {
        ZF_ASSERT(index >= 0 && index < bv.BitCount());
        bv.Bytes()[index / 8] |= (1 << (index % 8));
    }

    constexpr void UnsetBit(const c_bit_vector bv, const t_size index) {
        ZF_ASSERT(index >= 0 && index < bv.BitCount());
        bv.Bytes()[index / 8] &= ~(1 << (index % 8));
    }
}
