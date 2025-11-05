#pragma once

#include <zc/mem/mem.h>
#include <zc/mem/arrays.h>

namespace zf {
    class c_bit_vector {
    public:
        c_bit_vector() = default;

        c_bit_vector(const c_array<t_u8> bytes, const size_t bit_cnt) : m_bytes(bytes), m_bit_cnt(bit_cnt) {
            assert(bytes.Len() == BitsToBytes(bit_cnt));
        }

        [[nodiscard]] bool Init(c_mem_arena& mem_arena, const size_t bit_cnt);

        int IndexOfFirstSetBit(const size_t from = 0) const;
        int IndexOfFirstUnsetBit(const size_t from = 0) const;

        void ApplyAnd(const c_bit_vector mask) const;
        void ApplyOr(const c_bit_vector mask) const;
        void ApplyXor(const c_bit_vector mask) const;

        void SetBit(const size_t index) const {
            assert(index < m_bit_cnt);
            m_bytes[index / 8] |= BitMask(index);
        }

        void UnsetBit(const size_t index) const {
            assert(index < m_bit_cnt);
            m_bytes[index / 8] &= ~BitMask(index);
        }

        bool IsBitSet(const size_t index) const {
            assert(index < m_bit_cnt);
            return m_bytes[index / 8] & BitMask(index);
        }

    private:
        c_array<t_u8> m_bytes;
        size_t m_bit_cnt = 0;

        t_u8 LastByteMask() const {
            const size_t last_byte_bit_cnt = m_bit_cnt % 8 == 0 ? 8 : m_bit_cnt % 8;
            return BitRangeMask(0, last_byte_bit_cnt);
        }
    };

    template<size_t tp_bit_cnt>
    class c_static_bit_vector {
    public:
        void SetBit(const size_t index) {
            c_bit_vector(m_bytes, tp_bit_cnt).SetBit(index);
        }

        void UnsetBit(const size_t index) {
            c_bit_vector(m_bytes, tp_bit_cnt).UnsetBit(index);
        }

        bool IsBitSet(const size_t index) const {
            return c_bit_vector(m_bytes, tp_bit_cnt).IsBitSet(index);
        }

        int IndexOfFirstSetBit(const size_t from = 0) const {
            return c_bit_vector(m_bytes, tp_bit_cnt).IndexOfFirstSetBit(from);
        }

        int IndexOfFirstUnsetBit(const size_t from = 0) const {
            return c_bit_vector(m_bytes, tp_bit_cnt).IndexOfFirstUnsetBit(from);
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
        s_static_array<t_u8, BitsToBytes(tp_bit_cnt)> m_bytes;
    };
}
