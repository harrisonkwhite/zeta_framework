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

    class c_bit_vector {
    public:
        constexpr c_bit_vector() = default;
        constexpr c_bit_vector(const c_array<t_u8> bytes)
            : bytes(bytes), bit_cnt(BytesToBits(bytes.Len())) {}
        constexpr c_bit_vector(const c_array<t_u8> bytes, const t_size bit_cnt)
            : bytes(bytes), bit_cnt(bit_cnt) {
            ZF_ASSERT(bytes.Len() == BitsToBytes(bit_cnt));
        }

        constexpr c_array<t_u8> Bytes() const {
            return bytes;
        }

        constexpr t_size BitCount() const {
            return bit_cnt;
        }

    private:
        c_array<t_u8> bytes;
        t_size bit_cnt = 0;
    };

    template<t_size tp_bit_cnt>
    struct s_static_bit_vector {
        s_static_array<t_u8, BitsToBytes(tp_bit_cnt)> bytes;

        constexpr operator c_bit_vector() {
            return {bytes, tp_bit_cnt};
        }

        constexpr operator c_bit_vector() const {
            return {bytes, tp_bit_cnt};
        }
    };

    t_b8 MakeBitVector(c_mem_arena& mem_arena, const t_size bit_cnt, c_bit_vector& o_bv);

    void ShiftLeft(const c_bit_vector bv, const t_size amount = 1);
    void RotLeft(const c_bit_vector bv, const t_size amount = 1);
    void ShiftRight(const c_bit_vector bv, const t_size amount = 1);
    void RotRight(const c_bit_vector bv, const t_size amount = 1);

    t_size FindFirstSetBit(const c_bit_vector bv, const t_size from = 0, const t_b8 inverted = false);

    inline t_size FindFirstUnsetBit(const c_bit_vector bv, const t_size from = 0) {
        return FindFirstSetBit(bv, from, true);
    }

    constexpr t_b8 IsBitSet(const c_bit_vector bv, const t_size index) {
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
