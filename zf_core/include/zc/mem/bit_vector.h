#pragma once

#include <zc/mem/mem.h>
#include <zc/mem/arrays.h>

namespace zf {
    class c_bit_vector {
    public:
        c_bit_vector() = default;

        c_bit_vector(const c_array<t_u8> bytes, const t_u64 bit_cnt) : m_bytes(bytes), m_bit_cnt(bit_cnt) {
            ZF_ASSERT(bytes.Len() == static_cast<t_s32>(BitsToBytes(bit_cnt)));
        }

        [[nodiscard]] t_b8 Init(c_mem_arena& mem_arena, const t_u64 bit_cnt);

        t_s32 IndexOfFirstSetBit(const t_u64 from = 0) const;
        t_s32 IndexOfFirstUnsetBit(const t_u64 from = 0) const;

        void ApplyAnd(const c_bit_vector mask) const;
        void ApplyOr(const c_bit_vector mask) const;
        void ApplyXor(const c_bit_vector mask) const;

        void SetBit(const t_u64 index) const {
            ZF_ASSERT(index < m_bit_cnt);
            m_bytes[static_cast<t_s32>(index / 8)] |= BitMask(index);
        }

        void UnsetBit(const t_u64 index) const {
            ZF_ASSERT(index < m_bit_cnt);
            m_bytes[static_cast<t_s32>(index / 8)] &= ~BitMask(index);
        }

        t_b8 IsBitSet(const t_u64 index) const {
            ZF_ASSERT(index < m_bit_cnt);
            return m_bytes[static_cast<t_s32>(index / 8)] & BitMask(index);
        }

    private:
        c_array<t_u8> m_bytes;
        t_u64 m_bit_cnt = 0;

        t_u8 LastByteMask() const {
            const t_u64 last_byte_bit_cnt = m_bit_cnt % 8 == 0 ? 8 : m_bit_cnt % 8;
            return BitRangeMask(0, last_byte_bit_cnt);
        }
    };

    template<t_u64 tp_bit_cnt>
    class c_static_bit_vector {
    public:
        static_assert(tp_bit_cnt > 0, "Invalid bit count for bit vector!");

        void SetBit(const t_u64 index) {
            c_bit_vector(m_bytes, tp_bit_cnt).SetBit(index);
        }

        void UnsetBit(const t_u64 index) {
            c_bit_vector(m_bytes, tp_bit_cnt).UnsetBit(index);
        }

        t_b8 IsBitSet(const t_u64 index) const {
            return c_bit_vector(m_bytes, tp_bit_cnt).IsBitSet(index);
        }

        t_s32 IndexOfFirstSetBit(const t_u64 from = 0) const {
            return c_bit_vector(m_bytes, tp_bit_cnt).IndexOfFirstSetBit(from);
        }

        t_s32 IndexOfFirstUnsetBit(const t_u64 from = 0) const {
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
