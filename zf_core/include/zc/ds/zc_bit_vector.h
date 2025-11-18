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

    struct s_bit_vector_rdonly {
        constexpr s_bit_vector_rdonly() = default;
        constexpr s_bit_vector_rdonly(const s_array<const t_u8> bytes)
            : bytes(bytes), bit_cnt(BytesToBits(bytes.Len())) {}
        constexpr s_bit_vector_rdonly(const s_array<const t_u8> bytes, const t_size bit_cnt)
            : bytes(bytes), bit_cnt(bit_cnt) {
            ZF_ASSERT(bytes.Len() == BitsToBytes(bit_cnt));
        }

        constexpr s_array<const t_u8> Bytes() const {
            return bytes;
        }

        constexpr t_size BitCount() const {
            return bit_cnt;
        }

    private:
        s_array<const t_u8> bytes;
        t_size bit_cnt = 0;
    };

    struct s_bit_vector {
        constexpr s_bit_vector() = default;
        constexpr s_bit_vector(const s_array<t_u8> bytes)
            : bytes(bytes), bit_cnt(BytesToBits(bytes.Len())) {}
        constexpr s_bit_vector(const s_array<t_u8> bytes, const t_size bit_cnt)
            : bytes(bytes), bit_cnt(bit_cnt) {
            ZF_ASSERT(bytes.Len() == BitsToBytes(bit_cnt));
        }

        constexpr s_array<t_u8> Bytes() const {
            return bytes;
        }

        constexpr t_size BitCount() const {
            return bit_cnt;
        }

        constexpr operator s_bit_vector_rdonly() const {
            return {bytes, bit_cnt};
        }

    private:
        s_array<t_u8> bytes;
        t_size bit_cnt = 0;
    };

    template<t_size tp_bit_cnt>
    struct s_static_bit_vector {
        s_static_array<t_u8, BitsToBytes(tp_bit_cnt)> bytes;

        constexpr operator s_bit_vector() {
            return {bytes, tp_bit_cnt};
        }

        constexpr operator s_bit_vector_rdonly() const {
            return {bytes, tp_bit_cnt};
        }
    };

    void ShiftBitsLeft(const s_bit_vector bv, const t_size amount = 1);
    void RotBitsLeft(const s_bit_vector bv, const t_size amount = 1);

    t_b8 MakeBitVector(s_mem_arena& mem_arena, const t_size bit_cnt, s_bit_vector& o_bv);

    // @todo: Right (either arithmetic or logical) shift and right rotate.

    t_size FindFirstSetBit(const s_bit_vector_rdonly bv, const t_size from = 0, const t_b8 inverted = false);

    inline t_size FindFirstUnsetBit(const s_bit_vector_rdonly bv, const t_size from = 0) {
        return FindFirstSetBit(bv, from, true);
    }

    constexpr t_b8 IsBitSet(const s_bit_vector_rdonly bv, const t_size index) {
        ZF_ASSERT(index >= 0 && index < bv.BitCount());
        return bv.Bytes()[index / 8] & (1 << (index % 8));
    }

    constexpr void SetBit(const s_bit_vector bv, const t_size index) {
        ZF_ASSERT(index >= 0 && index < bv.BitCount());
        bv.Bytes()[index / 8] |= (1 << (index % 8));
    }

    constexpr void UnsetBit(const s_bit_vector bv, const t_size index) {
        ZF_ASSERT(index >= 0 && index < bv.BitCount());
        bv.Bytes()[index / 8] &= ~(1 << (index % 8));
    }

    inline t_u8 BitVectorLastByteBitmask(const s_bit_vector_rdonly bv) {
        ZF_ASSERT(bv.BitCount() > 0);

        const t_size bit_cnt = bv.BitCount() - (8 * (bv.Bytes().Len() - 1));
        return ByteBitmask(0, bit_cnt);
    }

    inline t_u8 BitVectorLastByte(const s_bit_vector_rdonly bv) {
        ZF_ASSERT(bv.BitCount() > 0);
        return bv.Bytes()[bv.Bytes().Len() - 1] & BitVectorLastByteBitmask(bv);
    }

    inline t_b8 AreAllBitsSet(const s_bit_vector_rdonly bv) {
        ZF_ASSERT(bv.BitCount() > 0);

        for (t_size i = 0; i < bv.Bytes().Len() - 1; i++) {
            if (bv.Bytes()[i] != 0xFF) {
                return false;
            }
        }

        return BitVectorLastByte(bv) == 0xFF;
    }

    inline t_b8 IsAnyBitSet(const s_bit_vector_rdonly bv) {
        for (t_size i = 0; i < bv.Bytes().Len() - 1; i++) {
            if (bv.Bytes()[i] != 0) {
                return true;
            }
        }

        return BitVectorLastByte(bv) != 0;
    }

    inline t_b8 AreAllBitsUnset(const s_bit_vector_rdonly bv) {
        return !IsAnyBitSet(bv);
    }

    inline t_b8 IsAnyBitUnset(const s_bit_vector_rdonly bv) {
        return !AreAllBitsSet(bv);
    }
}
