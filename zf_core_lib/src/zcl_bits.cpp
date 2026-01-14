#include <zcl/zcl_bits.h>

#include <zcl/zcl_algos.h>

namespace zcl {
    t_b8 BitsetCheckAnySet(const t_bitset_rdonly bs) {
        if (bs.bit_cnt == 0) {
            return false;
        }

        const auto first_bytes = array_slice(BitsetGetBytes(bs), 0, BitsetGetBytes(bs).len - 1);

        if (!CheckAllEqual(first_bytes, 0)) {
            return true;
        }

        return (BitsetGetBytes(bs)[BitsetGetBytes(bs).len - 1] & BitsetGetLastByteMask(bs)) != 0;
    }

    t_b8 BitsetCheckAllSet(const t_bitset_rdonly bs) {
        if (bs.bit_cnt == 0) {
            return false;
        }

        const auto first_bytes = array_slice(BitsetGetBytes(bs), 0, BitsetGetBytes(bs).len - 1);

        if (!CheckAllEqual(first_bytes, 0xFF)) {
            return false;
        }

        const auto last_byte_mask = BitsetGetLastByteMask(bs);
        return (BitsetGetBytes(bs)[BitsetGetBytes(bs).len - 1] & last_byte_mask) == last_byte_mask;
    }

    void BitsetSetAll(const t_bitset_mut bs) {
        if (bs.bit_cnt == 0) {
            return;
        }

        const auto first_bytes = array_slice(BitsetGetBytes(bs), 0, BitsetGetBytes(bs).len - 1);
        SetAllTo(first_bytes, 0xFF);

        BitsetGetBytes(bs)[BitsetGetBytes(bs).len - 1] |= BitsetGetLastByteMask(bs);
    }

    void BitsetUnsetAll(const t_bitset_mut bs) {
        if (bs.bit_cnt == 0) {
            return;
        }

        const auto first_bytes = array_slice(BitsetGetBytes(bs), 0, BitsetGetBytes(bs).len - 1);
        SetAllTo(first_bytes, 0);

        BitsetGetBytes(bs)[BitsetGetBytes(bs).len - 1] &= ~BitsetGetLastByteMask(bs);
    }

    void BitsetSetRange(const t_bitset_mut bs, const t_i32 begin_bit_index, const t_i32 end_bit_index) {
        ZCL_ASSERT(begin_bit_index >= 0 && begin_bit_index < bs.bit_cnt);
        ZCL_ASSERT(end_bit_index >= begin_bit_index && end_bit_index <= bs.bit_cnt);

        const t_i32 begin_elem_index = begin_bit_index / 8;
        const t_i32 end_elem_index = bits_to_bytes(end_bit_index);

        for (t_i32 i = begin_elem_index; i < end_elem_index; i++) {
            const t_i32 bit_offs = i * 8;
            const t_i32 begin_bit_index_rel = begin_bit_index - bit_offs;
            const t_i32 end_bit_index_rel = end_bit_index - bit_offs;

            const t_i32 set_range_begin = calc_max(begin_bit_index_rel, 0);
            const t_i32 set_range_end = calc_min(end_bit_index_rel, 8);

            BitsetGetBytes(bs)[i] |= ByteBitmaskCreateRange(set_range_begin, set_range_end);
        }
    }

    void BitsetApplyMask(const t_bitset_mut bs, const t_bitset_rdonly mask, const t_bitwise_mask_op op) {
        ZCL_ASSERT(bs.bit_cnt == mask.bit_cnt);

        if (bs.bit_cnt == 0) {
            return;
        }

        switch (op) {
        case ek_bitwise_mask_op_and:
            for (t_i32 i = 0; i < BitsetGetBytes(bs).len; i++) {
                BitsetGetBytes(bs)[i] &= BitsetGetBytes(mask)[i];
            }

            break;

        case ek_bitwise_mask_op_or:
            for (t_i32 i = 0; i < BitsetGetBytes(bs).len; i++) {
                BitsetGetBytes(bs)[i] |= BitsetGetBytes(mask)[i];
            }

            break;

        case ek_bitwise_mask_op_xor:
            for (t_i32 i = 0; i < BitsetGetBytes(bs).len; i++) {
                BitsetGetBytes(bs)[i] ^= BitsetGetBytes(mask)[i];
            }

            break;

        case ek_bitwise_mask_op_andnot:
            for (t_i32 i = 0; i < BitsetGetBytes(bs).len; i++) {
                BitsetGetBytes(bs)[i] &= ~BitsetGetBytes(mask)[i];
            }

            break;
        }

        BitsetGetBytes(bs)[BitsetGetBytes(bs).len - 1] &= BitsetGetLastByteMask(bs);
    }

    static t_u8 BitsetShiftLeftSingle(const t_bitset_mut bs) {
        ZCL_ASSERT(bits_to_bytes(bs.bit_cnt) == BitsetGetBytes(bs).len);

        if (bs.bit_cnt == 0) {
            return 0;
        }

        t_u8 discard = 0;

        for (t_i32 i = 0; i < BitsetGetBytes(bs).len; i++) {
            const t_i32 bits_in_byte = i == BitsetGetBytes(bs).len - 1 ? BitsetGetLastByteBitCount(bs) : 8;
            const t_u8 discard_last = discard;
            discard = (BitsetGetBytes(bs)[i] & ByteBitmaskCreateSingle(bits_in_byte - 1)) >> (bits_in_byte - 1);
            BitsetGetBytes(bs)[i] <<= 1;
            BitsetGetBytes(bs)[i] |= discard_last;
        }

        BitsetGetBytes(bs)[BitsetGetBytes(bs).len - 1] &= BitsetGetLastByteMask(bs);

        return discard;
    }

    void BitsetShiftLeft(const t_bitset_mut bs, const t_i32 amount) {
        ZCL_ASSERT(amount >= 0);

        // @speed: :(

        for (t_i32 i = 0; i < amount; i++) {
            BitsetShiftLeftSingle(bs);
        }
    }

    void BitsetRotateLeft(const t_bitset_mut bs, const t_i32 amount) {
        ZCL_ASSERT(amount >= 0);

        if (bs.bit_cnt == 0) {
            return;
        }

        // @speed: :(

        for (t_i32 i = 0; i < amount; i++) {
            const auto discard = BitsetShiftLeftSingle(bs);

            if (discard) {
                BitsetSet(bs, 0);
            } else {
                BitsetUnset(bs, 0);
            }
        }
    }

    static t_u8 BitsetShiftRightSingle(const t_bitset_mut bs) {
        ZCL_ASSERT(bits_to_bytes(bs.bit_cnt) == BitsetGetBytes(bs).len);

        if (bs.bit_cnt == 0) {
            return 0;
        }

        BitsetGetBytes(bs)[BitsetGetBytes(bs).len - 1] &= BitsetGetLastByteMask(bs); // Drop any excess bits so we don't accidentally shift a 1 in.

        t_u8 discard = 0;

        for (t_i32 i = BitsetGetBytes(bs).len - 1; i >= 0; i--) {
            const t_i32 bits_in_byte = i == BitsetGetBytes(bs).len - 1 ? BitsetGetLastByteBitCount(bs) : 8;
            const t_u8 discard_last = discard;
            discard = BitsetGetBytes(bs)[i] & ByteBitmaskCreateSingle(0);
            BitsetGetBytes(bs)[i] >>= 1;

            if (discard_last) {
                BitsetGetBytes(bs)[i] |= ByteBitmaskCreateSingle(bits_in_byte - 1);
            }
        }

        return discard;
    }

    void BitsetShiftRight(const t_bitset_mut bs, const t_i32 amount) {
        ZCL_ASSERT(amount >= 0);

        // @speed: :(

        for (t_i32 i = 0; i < amount; i++) {
            BitsetShiftRightSingle(bs);
        }
    }

    void BitsetRotateRight(const t_bitset_mut bs, const t_i32 amount) {
        ZCL_ASSERT(amount >= 0);

        if (bs.bit_cnt == 0) {
            return;
        }

        // @speed: :(

        for (t_i32 i = 0; i < amount; i++) {
            const auto discard = BitsetShiftRightSingle(bs);

            if (discard) {
                BitsetSet(bs, bs.bit_cnt - 1);
            } else {
                BitsetUnset(bs, bs.bit_cnt - 1);
            }
        }
    }

    // ============================================================

    static t_i32 BitsetFindFirstSetBitHelper(const t_bitset_rdonly bs, const t_i32 from, const t_u8 xor_mask) {
        ZCL_ASSERT(from >= 0 && from <= bs.bit_cnt); // Intentionally allowing the upper bound here for the case of iteration.

        // Map of each possible byte to the index of the first set bit, or -1 for the first case.
        constexpr t_static_array<t_i32, 256> k_mappings = {{
            -1, // 0000 0000
            0,  // 0000 0001
            1,  // 0000 0010
            0,  // 0000 0011
            2,  // 0000 0100
            0,  // 0000 0101
            1,  // 0000 0110
            0,  // 0000 0111
            3,  // 0000 1000
            0,  // 0000 1001
            1,  // 0000 1010
            0,  // 0000 1011
            2,  // 0000 1100
            0,  // 0000 1101
            1,  // 0000 1110
            0,  // 0000 1111
            4,  // 0001 0000
            0,  // 0001 0001
            1,  // 0001 0010
            0,  // 0001 0011
            2,  // 0001 0100
            0,  // 0001 0101
            1,  // 0001 0110
            0,  // 0001 0111
            3,  // 0001 1000
            0,  // 0001 1001
            1,  // 0001 1010
            0,  // 0001 1011
            2,  // 0001 1100
            0,  // 0001 1101
            1,  // 0001 1110
            0,  // 0001 1111
            5,  // 0010 0000
            0,  // 0010 0001
            1,  // 0010 0010
            0,  // 0010 0011
            2,  // 0010 0100
            0,  // 0010 0101
            1,  // 0010 0110
            0,  // 0010 0111
            3,  // 0010 1000
            0,  // 0010 1001
            1,  // 0010 1010
            0,  // 0010 1011
            2,  // 0010 1100
            0,  // 0010 1101
            1,  // 0010 1110
            0,  // 0010 1111
            4,  // 0011 0000
            0,  // 0011 0001
            1,  // 0011 0010
            0,  // 0011 0011
            2,  // 0011 0100
            0,  // 0011 0101
            1,  // 0011 0110
            0,  // 0011 0111
            3,  // 0011 1000
            0,  // 0011 1001
            1,  // 0011 1010
            0,  // 0011 1011
            2,  // 0011 1100
            0,  // 0011 1101
            1,  // 0011 1110
            0,  // 0011 1111
            6,  // 0100 0000
            0,  // 0100 0001
            1,  // 0100 0010
            0,  // 0100 0011
            2,  // 0100 0100
            0,  // 0100 0101
            1,  // 0100 0110
            0,  // 0100 0111
            3,  // 0100 1000
            0,  // 0100 1001
            1,  // 0100 1010
            0,  // 0100 1011
            2,  // 0100 1100
            0,  // 0100 1101
            1,  // 0100 1110
            0,  // 0100 1111
            4,  // 0101 0000
            0,  // 0101 0001
            1,  // 0101 0010
            0,  // 0101 0011
            2,  // 0101 0100
            0,  // 0101 0101
            1,  // 0101 0110
            0,  // 0101 0111
            3,  // 0101 1000
            0,  // 0101 1001
            1,  // 0101 1010
            0,  // 0101 1011
            2,  // 0101 1100
            0,  // 0101 1101
            1,  // 0101 1110
            0,  // 0101 1111
            5,  // 0110 0000
            0,  // 0110 0001
            1,  // 0110 0010
            0,  // 0110 0011
            2,  // 0110 0100
            0,  // 0110 0101
            1,  // 0110 0110
            0,  // 0110 0111
            3,  // 0110 1000
            0,  // 0110 1001
            1,  // 0110 1010
            0,  // 0110 1011
            2,  // 0110 1100
            0,  // 0110 1101
            1,  // 0110 1110
            0,  // 0110 1111
            4,  // 0111 0000
            0,  // 0111 0001
            1,  // 0111 0010
            0,  // 0111 0011
            2,  // 0111 0100
            0,  // 0111 0101
            1,  // 0111 0110
            0,  // 0111 0111
            3,  // 0111 1000
            0,  // 0111 1001
            1,  // 0111 1010
            0,  // 0111 1011
            2,  // 0111 1100
            0,  // 0111 1101
            1,  // 0111 1110
            0,  // 0111 1111
            7,  // 1000 0000
            0,  // 1000 0001
            1,  // 1000 0010
            0,  // 1000 0011
            2,  // 1000 0100
            0,  // 1000 0101
            1,  // 1000 0110
            0,  // 1000 0111
            3,  // 1000 1000
            0,  // 1000 1001
            1,  // 1000 1010
            0,  // 1000 1011
            2,  // 1000 1100
            0,  // 1000 1101
            1,  // 1000 1110
            0,  // 1000 1111
            4,  // 1001 0000
            0,  // 1001 0001
            1,  // 1001 0010
            0,  // 1001 0011
            2,  // 1001 0100
            0,  // 1001 0101
            1,  // 1001 0110
            0,  // 1001 0111
            3,  // 1001 1000
            0,  // 1001 1001
            1,  // 1001 1010
            0,  // 1001 1011
            2,  // 1001 1100
            0,  // 1001 1101
            1,  // 1001 1110
            0,  // 1001 1111
            5,  // 1010 0000
            0,  // 1010 0001
            1,  // 1010 0010
            0,  // 1010 0011
            2,  // 1010 0100
            0,  // 1010 0101
            1,  // 1010 0110
            0,  // 1010 0111
            3,  // 1010 1000
            0,  // 1010 1001
            1,  // 1010 1010
            0,  // 1010 1011
            2,  // 1010 1100
            0,  // 1010 1101
            1,  // 1010 1110
            0,  // 1010 1111
            4,  // 1011 0000
            0,  // 1011 0001
            1,  // 1011 0010
            0,  // 1011 0011
            2,  // 1011 0100
            0,  // 1011 0101
            1,  // 1011 0110
            0,  // 1011 0111
            3,  // 1011 1000
            0,  // 1011 1001
            1,  // 1011 1010
            0,  // 1011 1011
            2,  // 1011 1100
            0,  // 1011 1101
            1,  // 1011 1110
            0,  // 1011 1111
            6,  // 1100 0000
            0,  // 1100 0001
            1,  // 1100 0010
            0,  // 1100 0011
            2,  // 1100 0100
            0,  // 1100 0101
            1,  // 1100 0110
            0,  // 1100 0111
            3,  // 1100 1000
            0,  // 1100 1001
            1,  // 1100 1010
            0,  // 1100 1011
            2,  // 1100 1100
            0,  // 1100 1101
            1,  // 1100 1110
            0,  // 1100 1111
            4,  // 1101 0000
            0,  // 1101 0001
            1,  // 1101 0010
            0,  // 1101 0011
            2,  // 1101 0100
            0,  // 1101 0101
            1,  // 1101 0110
            0,  // 1101 0111
            3,  // 1101 1000
            0,  // 1101 1001
            1,  // 1101 1010
            0,  // 1101 1011
            2,  // 1101 1100
            0,  // 1101 1101
            1,  // 1101 1110
            0,  // 1101 1111
            5,  // 1110 0000
            0,  // 1110 0001
            1,  // 1110 0010
            0,  // 1110 0011
            2,  // 1110 0100
            0,  // 1110 0101
            1,  // 1110 0110
            0,  // 1110 0111
            3,  // 1110 1000
            0,  // 1110 1001
            1,  // 1110 1010
            0,  // 1110 1011
            2,  // 1110 1100
            0,  // 1110 1101
            1,  // 1110 1110
            0,  // 1110 1111
            4,  // 1111 0000
            0,  // 1111 0001
            1,  // 1111 0010
            0,  // 1111 0011
            2,  // 1111 0100
            0,  // 1111 0101
            1,  // 1111 0110
            0,  // 1111 0111
            3,  // 1111 1000
            0,  // 1111 1001
            1,  // 1111 1010
            0,  // 1111 1011
            2,  // 1111 1100
            0,  // 1111 1101
            1,  // 1111 1110
            0,  // 1111 1111
        }};

        const t_i32 begin_byte_index = from / 8;

        for (t_i32 i = begin_byte_index; i < BitsetGetBytes(bs).len; i++) {
            t_u8 byte = BitsetGetBytes(bs)[i] ^ xor_mask;

            if (i == begin_byte_index) {
                byte &= ByteBitmaskCreateRange(from % 8);
            }

            if (i == BitsetGetBytes(bs).len - 1) {
                byte &= BitsetGetLastByteMask(bs);
            }

            const t_i32 bi = k_mappings[byte];

            if (bi != -1) {
                return (8 * i) + bi;
            }
        }

        return -1;
    }

    t_i32 BitsetFindFirstSetBit(const t_bitset_rdonly bs, const t_i32 from) {
        return BitsetFindFirstSetBitHelper(bs, from, 0);
    }

    t_i32 BitsetFindFirstUnsetBit(const t_bitset_rdonly bs, const t_i32 from) {
        return BitsetFindFirstSetBitHelper(bs, from, 0xFF);
    }

    t_i32 BitsetCountSet(const t_bitset_rdonly bs) {
        // Map of each possible byte to the number of set bits in it.
        constexpr t_static_array<t_i32, 256> k_mappings = {{
            0, // 0000 0000
            1, // 0000 0001
            1, // 0000 0010
            2, // 0000 0011
            1, // 0000 0100
            2, // 0000 0101
            2, // 0000 0110
            3, // 0000 0111
            1, // 0000 1000
            2, // 0000 1001
            2, // 0000 1010
            3, // 0000 1011
            2, // 0000 1100
            3, // 0000 1101
            3, // 0000 1110
            4, // 0000 1111
            1, // 0001 0000
            2, // 0001 0001
            2, // 0001 0010
            3, // 0001 0011
            2, // 0001 0100
            3, // 0001 0101
            3, // 0001 0110
            4, // 0001 0111
            2, // 0001 1000
            3, // 0001 1001
            3, // 0001 1010
            4, // 0001 1011
            3, // 0001 1100
            4, // 0001 1101
            4, // 0001 1110
            5, // 0001 1111
            1, // 0010 0000
            2, // 0010 0001
            2, // 0010 0010
            3, // 0010 0011
            2, // 0010 0100
            3, // 0010 0101
            3, // 0010 0110
            4, // 0010 0111
            2, // 0010 1000
            3, // 0010 1001
            3, // 0010 1010
            4, // 0010 1011
            3, // 0010 1100
            4, // 0010 1101
            4, // 0010 1110
            5, // 0010 1111
            2, // 0011 0000
            3, // 0011 0001
            3, // 0011 0010
            4, // 0011 0011
            3, // 0011 0100
            4, // 0011 0101
            4, // 0011 0110
            5, // 0011 0111
            3, // 0011 1000
            4, // 0011 1001
            4, // 0011 1010
            5, // 0011 1011
            4, // 0011 1100
            5, // 0011 1101
            5, // 0011 1110
            6, // 0011 1111
            1, // 0100 0000
            2, // 0100 0001
            2, // 0100 0010
            3, // 0100 0011
            2, // 0100 0100
            3, // 0100 0101
            3, // 0100 0110
            4, // 0100 0111
            2, // 0100 1000
            3, // 0100 1001
            3, // 0100 1010
            4, // 0100 1011
            3, // 0100 1100
            4, // 0100 1101
            4, // 0100 1110
            5, // 0100 1111
            2, // 0101 0000
            3, // 0101 0001
            3, // 0101 0010
            4, // 0101 0011
            3, // 0101 0100
            4, // 0101 0101
            4, // 0101 0110
            5, // 0101 0111
            3, // 0101 1000
            4, // 0101 1001
            4, // 0101 1010
            5, // 0101 1011
            4, // 0101 1100
            5, // 0101 1101
            5, // 0101 1110
            6, // 0101 1111
            2, // 0110 0000
            3, // 0110 0001
            3, // 0110 0010
            4, // 0110 0011
            3, // 0110 0100
            4, // 0110 0101
            4, // 0110 0110
            5, // 0110 0111
            3, // 0110 1000
            4, // 0110 1001
            4, // 0110 1010
            5, // 0110 1011
            4, // 0110 1100
            5, // 0110 1101
            5, // 0110 1110
            6, // 0110 1111
            3, // 0111 0000
            4, // 0111 0001
            4, // 0111 0010
            5, // 0111 0011
            4, // 0111 0100
            5, // 0111 0101
            5, // 0111 0110
            6, // 0111 0111
            4, // 0111 1000
            5, // 0111 1001
            5, // 0111 1010
            6, // 0111 1011
            5, // 0111 1100
            6, // 0111 1101
            6, // 0111 1110
            7, // 0111 1111
            1, // 1000 0000
            2, // 1000 0001
            2, // 1000 0010
            3, // 1000 0011
            2, // 1000 0100
            3, // 1000 0101
            3, // 1000 0110
            4, // 1000 0111
            2, // 1000 1000
            3, // 1000 1001
            3, // 1000 1010
            4, // 1000 1011
            3, // 1000 1100
            4, // 1000 1101
            4, // 1000 1110
            5, // 1000 1111
            2, // 1001 0000
            3, // 1001 0001
            3, // 1001 0010
            4, // 1001 0011
            3, // 1001 0100
            4, // 1001 0101
            4, // 1001 0110
            5, // 1001 0111
            3, // 1001 1000
            4, // 1001 1001
            4, // 1001 1010
            5, // 1001 1011
            4, // 1001 1100
            5, // 1001 1101
            5, // 1001 1110
            6, // 1001 1111
            2, // 1010 0000
            3, // 1010 0001
            3, // 1010 0010
            4, // 1010 0011
            3, // 1010 0100
            4, // 1010 0101
            4, // 1010 0110
            5, // 1010 0111
            3, // 1010 1000
            4, // 1010 1001
            4, // 1010 1010
            5, // 1010 1011
            4, // 1010 1100
            5, // 1010 1101
            5, // 1010 1110
            6, // 1010 1111
            3, // 1011 0000
            4, // 1011 0001
            4, // 1011 0010
            5, // 1011 0011
            4, // 1011 0100
            5, // 1011 0101
            5, // 1011 0110
            6, // 1011 0111
            4, // 1011 1000
            5, // 1011 1001
            5, // 1011 1010
            6, // 1011 1011
            5, // 1011 1100
            6, // 1011 1101
            6, // 1011 1110
            7, // 1011 1111
            2, // 1100 0000
            3, // 1100 0001
            3, // 1100 0010
            4, // 1100 0011
            3, // 1100 0100
            4, // 1100 0101
            4, // 1100 0110
            5, // 1100 0111
            3, // 1100 1000
            4, // 1100 1001
            4, // 1100 1010
            5, // 1100 1011
            4, // 1100 1100
            5, // 1100 1101
            5, // 1100 1110
            6, // 1100 1111
            3, // 1101 0000
            4, // 1101 0001
            4, // 1101 0010
            5, // 1101 0011
            4, // 1101 0100
            5, // 1101 0101
            5, // 1101 0110
            6, // 1101 0111
            4, // 1101 1000
            5, // 1101 1001
            5, // 1101 1010
            6, // 1101 1011
            5, // 1101 1100
            6, // 1101 1101
            6, // 1101 1110
            7, // 1101 1111
            3, // 1110 0000
            4, // 1110 0001
            4, // 1110 0010
            5, // 1110 0011
            4, // 1110 0100
            5, // 1110 0101
            5, // 1110 0110
            6, // 1110 0111
            4, // 1110 1000
            5, // 1110 1001
            5, // 1110 1010
            6, // 1110 1011
            5, // 1110 1100
            6, // 1110 1101
            6, // 1110 1110
            7, // 1110 1111
            4, // 1111 0000
            5, // 1111 0001
            5, // 1111 0010
            6, // 1111 0011
            5, // 1111 0100
            6, // 1111 0101
            6, // 1111 0110
            7, // 1111 0111
            5, // 1111 1000
            6, // 1111 1001
            6, // 1111 1010
            7, // 1111 1011
            6, // 1111 1100
            7, // 1111 1101
            7, // 1111 1110
            8, // 1111 1111
        }};

        t_i32 result = 0;

        if (BitsetGetBytes(bs).len > 0) {
            for (t_i32 i = 0; i < BitsetGetBytes(bs).len - 1; i++) {
                result += k_mappings[BitsetGetBytes(bs)[i]];
            }

            result += k_mappings[BitsetGetBytes(bs)[BitsetGetBytes(bs).len - 1] & BitsetGetLastByteMask(bs)];
        }

        return result;
    }

    t_b8 BitsetWalkAllSet(const t_bitset_rdonly bs, t_i32 *const pos, t_i32 *const o_index) {
        ZCL_ASSERT(*pos >= 0 && *pos <= bs.bit_cnt);

        *o_index = BitsetFindFirstSetBit(bs, *pos);

        if (*o_index == -1) {
            return false;
        }

        *pos = *o_index + 1;

        return true;
    }

    t_b8 BitsetWalkAllUnset(const t_bitset_rdonly bs, t_i32 *const pos, t_i32 *const o_index) {
        ZCL_ASSERT(*pos >= 0 && *pos <= bs.bit_cnt);

        *o_index = BitsetFindFirstUnsetBit(bs, *pos);

        if (*o_index == -1) {
            return false;
        }

        *pos = *o_index + 1;

        return true;
    }
}
