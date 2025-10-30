#pragma once

#include "zc_mem.h"

namespace zf {
    // @todo: This should be a ZF static array!
    constexpr int g_indexes_of_first_set_bits[256] = {
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
         0  // 1111 1111
    };

    class c_bit_vector {
    public:
        c_bit_vector() = default;

        c_bit_vector(const c_array<t_u8> bytes, const size_t bit_cnt) : m_bytes(bytes), m_bit_cnt(bit_cnt) {
            assert(!bytes.IsEmpty());
            assert(bit_cnt > 0 && bit_cnt <= 8u * bytes.Len());
        }

        bool Init(c_mem_arena& mem_arena, const size_t bit_cnt) {
            assert(bit_cnt > 0);

            *this = {};

            m_bytes = mem_arena.PushArray<t_u8>(BitsToBytes(bit_cnt));

            if (m_bytes.IsEmpty()) {
                return false;
            }

            m_bit_cnt = bit_cnt;

            return true;
        }

        bool IsBitSet(const size_t index) {
            assert(IsInitted());
            assert(index < m_bit_cnt);
            return m_bytes[index / 8] & BitMask(index);
        }

        void SetBit(const size_t index) {
            assert(IsInitted());
            assert(index < m_bit_cnt);
            m_bytes[index / 8] |= BitMask(index);
        }

        void UnsetBit(const size_t index) {
            assert(IsInitted());
            assert(index < m_bit_cnt);
            m_bytes[index / 8] &= ~BitMask(index);
        }

        // @todo: Shift right!!!
        void ShiftLeft(int amount, const bool rot = false) {
            assert(IsInitted());
            assert(amount >= 0);

            while (amount >= 0) {
                const int iter_shift_amount = Min(amount, 8);

                t_u8 carry = 0;

                for (int i = 0; i < m_bytes.Len(); i++) {
                    t_u8& byte = m_bytes[i];
                    const size_t byte_bit_cnt = m_bit_cnt - (8 * i);
                    const t_u8 lower_mask = (1 << iter_shift_amount) - 1;
                    const t_u8 upper_mask = lower_mask << (byte_bit_cnt - iter_shift_amount);
                    const t_u8 carry_old = carry;
                    carry = (byte & upper_mask) >> (byte_bit_cnt - iter_shift_amount);
                    byte = (byte << iter_shift_amount) | carry_old;
                }

                if (rot) {
                    m_bytes[0] |= carry;
                }

                amount -= iter_shift_amount;
            }
        }

        int IndexOfFirstSetBit(const int from = 0) const {
            assert(IsInitted());
            assert(from >= 0 && from < m_bit_cnt);

            int ret = -1;

            const int first_byte_index = from / 8;
            const auto first_byte_saved = m_bytes[first_byte_index];
            m_bytes[first_byte_index] = m_bytes[first_byte_index] & ~((1 << (from % 8)) - 1); // Ignore the initial bits specified.

            for (int i = first_byte_index; i < m_bytes.Len(); i++) {
                const int bi = g_indexes_of_first_set_bits[m_bytes[i]];

                if (bi != -1) {
                    ret = (i * 8) + bi;
                    break;
                }
            }

            m_bytes[first_byte_index] = first_byte_saved;

            return ret < m_bit_cnt ? ret : -1;
        }

        void WriteBitStr(const c_array<char> str_chrs) {
            assert(IsInitted());
            assert(str_chrs.Len() >= m_bit_cnt + 1);

            for (size_t i = 0; i < m_bit_cnt; i++) {
                str_chrs[i] = IsBitSet(i) ? '1' : '0';
            }
        }

    private:
        static t_u8 BitMask(const size_t index) {
            return 1 << (index % 8);
        }

        bool IsInitted() const {
            return !m_bytes.IsEmpty();
        }

        c_array<t_u8> m_bytes;
        size_t m_bit_cnt = 0;
    };
}
