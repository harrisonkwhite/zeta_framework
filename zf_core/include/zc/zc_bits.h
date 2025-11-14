#pragma once

#include <zc/zc_seqs.h>
#include <zc/zc_math.h>

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

    struct s_bit_vector_ro {
        constexpr s_bit_vector_ro() = default;
        constexpr s_bit_vector_ro(const c_array<const t_u8> bytes)
            : bytes(bytes), bit_cnt(BytesToBits(bytes.Len())) {}
        constexpr s_bit_vector_ro(const c_array<const t_u8> bytes, const t_size bit_cnt)
            : bytes(bytes), bit_cnt(bit_cnt) {
            ZF_ASSERT(bytes.Len() == BitsToBytes(bit_cnt));
        }

        constexpr c_array<const t_u8> Bytes() const {
            return bytes;
        }

        constexpr t_size BitCount() const {
            return bit_cnt;
        }

    private:
        c_array<const t_u8> bytes;
        t_size bit_cnt = 0;
    };

    struct s_bit_vector_mut {
        constexpr s_bit_vector_mut() = default;

        constexpr s_bit_vector_mut(const c_array<t_u8> bytes)
            : bytes(bytes), bit_cnt(BytesToBits(bytes.Len())) {}

        constexpr s_bit_vector_mut(const c_array<t_u8> bytes, const t_size bit_cnt)
            : bytes(bytes), bit_cnt(bit_cnt) {
            ZF_ASSERT(bit_cnt >= 0);
            ZF_ASSERT(bytes.Len() == BitsToBytes(bit_cnt));
        }

        constexpr c_array<t_u8> Bytes() const {
            return bytes;
        }

        constexpr t_size BitCount() const {
            return bit_cnt;
        }

        constexpr operator s_bit_vector_ro() const {
            return {bytes, bit_cnt};
        }

    private:
        c_array<t_u8> bytes;
        t_size bit_cnt = 0;
    };

    template<t_size tp_bit_cnt>
    struct s_static_bit_vector {
        static_assert(tp_bit_cnt > 0);

        s_static_array<t_u8, BitsToBytes(tp_bit_cnt)> bytes;

        constexpr operator s_bit_vector_mut() {
            return {bytes, tp_bit_cnt};
        }

        constexpr operator s_bit_vector_ro() const {
            return {bytes, tp_bit_cnt};
        }
    };

    inline t_b8 IsBitSet(const s_bit_vector_ro bv, const t_size index) {
        ZF_ASSERT(index >= 0 && index < bv.BitCount());
        return bv.Bytes()[index / 8] & (1 << (index % 8));
    }

    inline void SetBit(const s_bit_vector_mut bv, const t_size index) {
        ZF_ASSERT(index >= 0 && index < bv.BitCount());
        bv.Bytes()[index / 8] |= (1 << (index % 8));
    }

    inline void UnsetBit(const s_bit_vector_mut bv, const t_size index) {
        ZF_ASSERT(index >= 0 && index < bv.BitCount());
        bv.Bytes()[index / 8] &= ~(1 << (index % 8));
    }

    void ShiftLeft(const s_bit_vector_mut bv, const t_size amount = 1);
    void RotLeft(const s_bit_vector_mut bv, const t_size amount = 1);
    void ShiftRight(const s_bit_vector_mut bv, const t_size amount = 1);
    void RotRight(const s_bit_vector_mut bv, const t_size amount = 1);

    t_size FindFirstSetBit(const s_bit_vector_mut bv, const t_size from = 0);
    t_size FindFirstUnsetBit(const s_bit_vector_mut bv, const t_size from = 0);
}
