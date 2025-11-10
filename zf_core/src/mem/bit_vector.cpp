#include <zc/mem/bit_vector.h>

namespace zf {
    t_b8 c_bit_vector::Init(c_mem_arena& mem_arena, const t_size bit_cnt) {
        ZF_ASSERT(bit_cnt > 0);

        if (!m_bytes.Init(mem_arena, BitsToBytes(bit_cnt))) {
            return false;
        }

        m_bit_cnt = bit_cnt;

        return true;
    }

    t_size c_bit_vector::FindFirstSetBit(const t_size from) const {
        ZF_ASSERT(from >= 0 && from <= m_bit_cnt); // Intentionally allowing the upper bound here for the case of iteration.

        static constexpr s_static_array<t_size, 256> lg_mappings = {{
            -1, // 0000 0000
            0, // 0000 0001
            1, // 0000 0010
            0, // 0000 0011
            2, // 0000 0100
            0, // 0000 0101
            1, // 0000 0110
            0, // 0000 0111
            3, // 0000 1000
            0, // 0000 1001
            1, // 0000 1010
            0, // 0000 1011
            2, // 0000 1100
            0, // 0000 1101
            1, // 0000 1110
            0, // 0000 1111
            4, // 0001 0000
            0, // 0001 0001
            1, // 0001 0010
            0, // 0001 0011
            2, // 0001 0100
            0, // 0001 0101
            1, // 0001 0110
            0, // 0001 0111
            3, // 0001 1000
            0, // 0001 1001
            1, // 0001 1010
            0, // 0001 1011
            2, // 0001 1100
            0, // 0001 1101
            1, // 0001 1110
            0, // 0001 1111
            5, // 0010 0000
            0, // 0010 0001
            1, // 0010 0010
            0, // 0010 0011
            2, // 0010 0100
            0, // 0010 0101
            1, // 0010 0110
            0, // 0010 0111
            3, // 0010 1000
            0, // 0010 1001
            1, // 0010 1010
            0, // 0010 1011
            2, // 0010 1100
            0, // 0010 1101
            1, // 0010 1110
            0, // 0010 1111
            4, // 0011 0000
            0, // 0011 0001
            1, // 0011 0010
            0, // 0011 0011
            2, // 0011 0100
            0, // 0011 0101
            1, // 0011 0110
            0, // 0011 0111
            3, // 0011 1000
            0, // 0011 1001
            1, // 0011 1010
            0, // 0011 1011
            2, // 0011 1100
            0, // 0011 1101
            1, // 0011 1110
            0, // 0011 1111
            6, // 0100 0000
            0, // 0100 0001
            1, // 0100 0010
            0, // 0100 0011
            2, // 0100 0100
            0, // 0100 0101
            1, // 0100 0110
            0, // 0100 0111
            3, // 0100 1000
            0, // 0100 1001
            1, // 0100 1010
            0, // 0100 1011
            2, // 0100 1100
            0, // 0100 1101
            1, // 0100 1110
            0, // 0100 1111
            4, // 0101 0000
            0, // 0101 0001
            1, // 0101 0010
            0, // 0101 0011
            2, // 0101 0100
            0, // 0101 0101
            1, // 0101 0110
            0, // 0101 0111
            3, // 0101 1000
            0, // 0101 1001
            1, // 0101 1010
            0, // 0101 1011
            2, // 0101 1100
            0, // 0101 1101
            1, // 0101 1110
            0, // 0101 1111
            5, // 0110 0000
            0, // 0110 0001
            1, // 0110 0010
            0, // 0110 0011
            2, // 0110 0100
            0, // 0110 0101
            1, // 0110 0110
            0, // 0110 0111
            3, // 0110 1000
            0, // 0110 1001
            1, // 0110 1010
            0, // 0110 1011
            2, // 0110 1100
            0, // 0110 1101
            1, // 0110 1110
            0, // 0110 1111
            4, // 0111 0000
            0, // 0111 0001
            1, // 0111 0010
            0, // 0111 0011
            2, // 0111 0100
            0, // 0111 0101
            1, // 0111 0110
            0, // 0111 0111
            3, // 0111 1000
            0, // 0111 1001
            1, // 0111 1010
            0, // 0111 1011
            2, // 0111 1100
            0, // 0111 1101
            1, // 0111 1110
            0, // 0111 1111
            7, // 1000 0000
            0, // 1000 0001
            1, // 1000 0010
            0, // 1000 0011
            2, // 1000 0100
            0, // 1000 0101
            1, // 1000 0110
            0, // 1000 0111
            3, // 1000 1000
            0, // 1000 1001
            1, // 1000 1010
            0, // 1000 1011
            2, // 1000 1100
            0, // 1000 1101
            1, // 1000 1110
            0, // 1000 1111
            4, // 1001 0000
            0, // 1001 0001
            1, // 1001 0010
            0, // 1001 0011
            2, // 1001 0100
            0, // 1001 0101
            1, // 1001 0110
            0, // 1001 0111
            3, // 1001 1000
            0, // 1001 1001
            1, // 1001 1010
            0, // 1001 1011
            2, // 1001 1100
            0, // 1001 1101
            1, // 1001 1110
            0, // 1001 1111
            5, // 1010 0000
            0, // 1010 0001
            1, // 1010 0010
            0, // 1010 0011
            2, // 1010 0100
            0, // 1010 0101
            1, // 1010 0110
            0, // 1010 0111
            3, // 1010 1000
            0, // 1010 1001
            1, // 1010 1010
            0, // 1010 1011
            2, // 1010 1100
            0, // 1010 1101
            1, // 1010 1110
            0, // 1010 1111
            4, // 1011 0000
            0, // 1011 0001
            1, // 1011 0010
            0, // 1011 0011
            2, // 1011 0100
            0, // 1011 0101
            1, // 1011 0110
            0, // 1011 0111
            3, // 1011 1000
            0, // 1011 1001
            1, // 1011 1010
            0, // 1011 1011
            2, // 1011 1100
            0, // 1011 1101
            1, // 1011 1110
            0, // 1011 1111
            6, // 1100 0000
            0, // 1100 0001
            1, // 1100 0010
            0, // 1100 0011
            2, // 1100 0100
            0, // 1100 0101
            1, // 1100 0110
            0, // 1100 0111
            3, // 1100 1000
            0, // 1100 1001
            1, // 1100 1010
            0, // 1100 1011
            2, // 1100 1100
            0, // 1100 1101
            1, // 1100 1110
            0, // 1100 1111
            4, // 1101 0000
            0, // 1101 0001
            1, // 1101 0010
            0, // 1101 0011
            2, // 1101 0100
            0, // 1101 0101
            1, // 1101 0110
            0, // 1101 0111
            3, // 1101 1000
            0, // 1101 1001
            1, // 1101 1010
            0, // 1101 1011
            2, // 1101 1100
            0, // 1101 1101
            1, // 1101 1110
            0, // 1101 1111
            5, // 1110 0000
            0, // 1110 0001
            1, // 1110 0010
            0, // 1110 0011
            2, // 1110 0100
            0, // 1110 0101
            1, // 1110 0110
            0, // 1110 0111
            3, // 1110 1000
            0, // 1110 1001
            1, // 1110 1010
            0, // 1110 1011
            2, // 1110 1100
            0, // 1110 1101
            1, // 1110 1110
            0, // 1110 1111
            4, // 1111 0000
            0, // 1111 0001
            1, // 1111 0010
            0, // 1111 0011
            2, // 1111 0100
            0, // 1111 0101
            1, // 1111 0110
            0, // 1111 0111
            3, // 1111 1000
            0, // 1111 1001
            1, // 1111 1010
            0, // 1111 1011
            2, // 1111 1100
            0, // 1111 1101
            1, // 1111 1110
            0 // 1111 1111
        }};

        if (m_bit_cnt == 0 || from == m_bit_cnt) {
            return m_bit_cnt;
        }

        const t_size starting_byte_index = from / 8;
        const t_u8 starting_byte_old = m_bytes[starting_byte_index];

        const t_u8 last_byte_old = m_bytes[m_bytes.Len() - 1];

        // Mask out the bits we don't care about.
        // These need to be updated only after BOTH bytes have been saved, because they could be the same!
        m_bytes[starting_byte_index] &= BitRangeMask(from % 8);
        m_bytes[m_bytes.Len() - 1] &= LastByteMask();

        // Check each byte for a set bit.
        const t_size res = [this]() {
            for (t_size i = 0; i < m_bytes.Len(); i++) {
                const t_size bi = lg_mappings[m_bytes[i]];

                if (bi != -1) {
                    return (8 * i) + bi;
                }
            }

            return static_cast<t_size>(-1);
        }();

        // Restore bytes.
        m_bytes[starting_byte_index] = starting_byte_old;
        m_bytes[m_bytes.Len() - 1] = last_byte_old;

        return res;
    }

    t_size c_bit_vector::FindFirstUnsetBit(const t_size from) const {
        ZF_ASSERT(from >= 0 && from <= m_bit_cnt); // Intentionally allowing the upper bound here for the case of iteration.

        // Flip all bits then check for first set bit.
        // @speed: This approach is probably not ideal.
        for (t_size i = 0; i < m_bytes[i]; i++) {
            m_bytes[i] ^= 0xFF;
        }

        const t_size res = FindFirstSetBit(from);

        // Flip the bits back.
        for (t_size i = 0; i < m_bytes[i]; i++) {
            m_bytes[i] ^= 0xFF;
        }

        return res;
    }

    void c_bit_vector::ApplyAnd(const c_bit_vector mask) const {
        ZF_ASSERT(m_bit_cnt == mask.m_bit_cnt);

        if (m_bit_cnt == 0) {
            return;
        }

        for (t_size i = 0; i < m_bytes.Len() - 1; i++) {
            m_bytes[i] &= mask.m_bytes[i];
        }

        m_bytes[m_bytes.Len() - 1] &= mask.m_bytes[mask.m_bytes.Len() - 1] & LastByteMask();
    }

    void c_bit_vector::ApplyOr(const c_bit_vector mask) const {
        ZF_ASSERT(m_bit_cnt == mask.m_bit_cnt);

        if (m_bit_cnt == 0) {
            return;
        }

        for (t_size i = 0; i < m_bytes.Len() - 1; i++) {
            m_bytes[i] |= mask.m_bytes[i];
        }

        m_bytes[m_bytes.Len() - 1] |= mask.m_bytes[mask.m_bytes.Len() - 1] & LastByteMask();
    }

    void c_bit_vector::ApplyXor(const c_bit_vector mask) const {
        ZF_ASSERT(m_bit_cnt == mask.m_bit_cnt);

        if (m_bit_cnt == 0) {
            return;
        }

        for (t_size i = 0; i < m_bytes.Len() - 1; i++) {
            m_bytes[i] ^= mask.m_bytes[i];
        }

        m_bytes[m_bytes.Len() - 1] ^= mask.m_bytes[mask.m_bytes.Len() - 1] & LastByteMask();
    }
}
