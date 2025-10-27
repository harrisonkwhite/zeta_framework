#pragma once

#include "zf_mem.h"

namespace zf {
    class c_bitset {
    public:
        c_bitset() = default;

        c_bitset(const c_array<t_u8> bytes, const size_t bit_cnt) : m_bytes(bytes), m_bit_cnt(bit_cnt) {
            assert(bit_cnt <= 8u * bytes.Len());
        }

        bool IsBitActive(const size_t index) {
            assert(index < m_bit_cnt);
            return m_bytes[index / 8] & BitMask(index);
        }

        void SetBit(const size_t index) {
            assert(index < m_bit_cnt);
            m_bytes[index / 8] |= BitMask(index);
        }

        void UnsetBit(const size_t index) {
            assert(index < m_bit_cnt);
            m_bytes[index / 8] &= ~BitMask(index);
        }

        void ShiftLeft(int amount, const bool rot = false) {
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

        void WriteBitStr(const c_array<char> str_chrs) {
            assert(str_chrs.Len() >= m_bit_cnt + 1);

            for (size_t i = 0; i < m_bit_cnt; i++) {
                str_chrs[i] = IsBitActive(i) ? '1' : '0';
            }
        }

    private:
        t_u8 BitMask(const size_t index) {
            return 1 << (index % 8);
        }

        c_array<t_u8> m_bytes;
        size_t m_bit_cnt;
    };
}
