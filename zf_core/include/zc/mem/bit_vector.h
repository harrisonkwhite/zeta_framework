#pragma once

#include <zc/mem/mem.h>
#include <zc/mem/arrays.h>

namespace zf {
    class c_bit_vector {
    public:
        c_bit_vector() = default;

        c_bit_vector(const c_array<t_u8> bytes, const t_size bit_cnt) : m_bytes(bytes), m_bit_cnt(bit_cnt) {
            ZF_ASSERT(bit_cnt >= 0);
            ZF_ASSERT(bytes.Len() == BitsToBytes(bit_cnt));
        }

        [[nodiscard]] t_b8 Init(c_mem_arena& mem_arena, const t_size bit_cnt);

        t_size FindFirstSetBit(const t_size from = 0) const; // Returns -1 if not found.
        t_size FindFirstUnsetBit(const t_size from = 0) const; // Returns -1 if not found.

        void ApplyAnd(const c_bit_vector mask) const;
        void ApplyOr(const c_bit_vector mask) const;
        void ApplyXor(const c_bit_vector mask) const;

        void SetBit(const t_size index) const {
            ZF_ASSERT(index < m_bit_cnt);
            m_bytes[index / 8] |= BitMask<t_u8>(index);
        }

        void UnsetBit(const t_size index) const {
            ZF_ASSERT(index < m_bit_cnt);
            m_bytes[index / 8] &= ~BitMask<t_u8>(index);
        }

        t_b8 IsBitSet(const t_size index) const {
            ZF_ASSERT(index < m_bit_cnt);
            return m_bytes[index / 8] & BitMask<t_u8>(index);
        }

    private:
        c_array<t_u8> m_bytes;
        t_size m_bit_cnt = 0;

        t_u8 LastByteMask() const {
            const t_size last_byte_bit_cnt = m_bit_cnt % 8 == 0 ? 8 : m_bit_cnt % 8;
            return BitRangeMask<t_u8>(0, last_byte_bit_cnt);
        }
    };

    template<t_size tp_bit_cnt>
    class c_static_bit_vector {
    public:
        static_assert(tp_bit_cnt > 0, "Invalid bit count for bit vector!");

        void SetBit(const t_size index) {
            c_bit_vector(m_bytes, tp_bit_cnt).SetBit(index);
        }

        void UnsetBit(const t_size index) {
            c_bit_vector(m_bytes, tp_bit_cnt).UnsetBit(index);
        }

        t_b8 IsBitSet(const t_size index) const {
            return c_bit_vector(m_bytes, tp_bit_cnt).IsBitSet(index);
        }

        // Returns -1 if not found.
        t_size FindFirstSetBit(const t_size from = 0) const {
            return c_bit_vector(m_bytes, tp_bit_cnt).FindFirstSetBit(from);
        }

        // Returns -1 if not found.
        t_size FindFirstUnsetBit(const t_size from = 0) const {
            return c_bit_vector(m_bytes, tp_bit_cnt).FindFirstUnsetBit(from);
        }

        void ApplyAnd(const c_bit_vector mask) {
            c_bit_vector(m_bytes, tp_bit_cnt).ApplyAnd(mask);
        }

        void ApplyOr(const c_bit_vector mask) {
            c_bit_vector(m_bytes, tp_bit_cnt).ApplyOr(mask);
        }

        void ApplyXor(const c_bit_vector mask) {
            c_bit_vector(m_bytes, tp_bit_cnt).ApplyXor(mask);
        }

    private:
        s_static_array<t_s8, BitsToBytes(tp_bit_cnt)> m_bytes;
    };
}
