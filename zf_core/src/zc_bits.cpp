#include <zc/zc_bits.h>

namespace zf {
    static t_u8 ShiftLeftSingle(const c_bit_vector bv, t_u8 carry = 0) {
        ZF_ASSERT(carry == 0 || carry == 1);

        if (bv.BitCount() == 0) {
            return 0;
        }

        const auto bytes = bv.Bytes();

        const auto make_carry = [](const t_u8 byte, const t_size bit_cnt) {
            return static_cast<t_u8>((byte & (1 << (bit_cnt - 1))) >> (bit_cnt - 1));
        };

        // Shift all bytes except the last.
        for (t_size i = 0; i < bytes.Len() - 1; i++) {
            const t_u8 carry_old = carry;
            carry = make_carry(bytes[i], 8);
            bytes[i] <<= 1;
            bytes[i] |= carry_old;
        }

        // Apply shift to final byte (might have less than 8 bits in the vector).
        const t_size remaining_bit_cnt = bv.BitCount() - (8 * (bytes.Len() - 1));

        t_u8& last_byte = bytes[bytes.Len() - 1];
        const t_u8 carry_old = carry;
        carry = make_carry(last_byte, remaining_bit_cnt);
        last_byte <<= 1;
        last_byte |= carry_old;

        return carry;
    }

    void ShiftLeft(const c_bit_vector bv, const t_size amount) {
        ZF_ASSERT(amount >= 0);

        for (t_size i = 0; i < amount; i++) {
            ShiftLeftSingle(bv, 0);
        }
    }

    void RotLeft(const c_bit_vector bv, const t_size amount) {
        ZF_ASSERT(amount >= 0);

        t_u8 carry = 0;

        for (t_size i = 0; i < amount; i++) {
            carry = ShiftLeftSingle(bv, carry);
        }
    }

#if 0
    t_size FindFirstSetBit(const c_bit_vector bv, const t_size index) {
    }

    t_size FindFirstUnsetBit(const c_bit_vector bv, const t_size index) {
    }
#endif
}
