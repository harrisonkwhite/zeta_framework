#pragma once

#include <zc/essential.h>

namespace zf {
    class c_bit_vector_view {
    public:
        c_bit_vector_view() = default;

        c_bit_vector_view(const c_array<const t_u8> bytes, const t_size bit_cnt) : m_bytes(bytes), m_bit_cnt(bit_cnt) {
            ZF_ASSERT(bit_cnt >= 0);
            ZF_ASSERT(bytes.len == BitsToBytes(bit_cnt));
        }

        c_array<const t_u8> Bytes() const {
            return m_bytes;
        }

        t_size BitCount() const {
            return m_bit_cnt;
        }

    private:
        c_array<const t_u8> m_bytes;
        t_size m_bit_cnt = 0;
    };

    class c_bit_vector {
    public:
        c_bit_vector() = default;

        c_bit_vector(const c_array<t_u8> bytes, const t_size bit_cnt) : m_bytes(bytes), m_bit_cnt(bit_cnt) {
            ZF_ASSERT(bit_cnt >= 0);
            ZF_ASSERT(bytes.len == BitsToBytes(bit_cnt));
        }

        c_array<t_u8> Bytes() const {
            return m_bytes;
        }

        t_size BitCount() const {
            return m_bit_cnt;
        }

    private:
        c_array<t_u8> m_bytes;
        t_size m_bit_cnt = 0;
    };

    template<co_unsigned_integral tp_type>
    constexpr tp_type BitMask(const t_size index) {
        ZF_ASSERT(index >= 0 && index < ZF_SIZE_IN_BITS(tp_type));
        return static_cast<tp_type>(static_cast<tp_type>(1) << index);
    }

    template<co_unsigned_integral tp_type>
    constexpr tp_type BitRangeMask(const t_size begin_index, const t_size end_index = ZF_SIZE_IN_BITS(tp_type)) {
        ZF_ASSERT(end_index >= 0 && end_index <= ZF_SIZE_IN_BITS(tp_type));
        ZF_ASSERT(begin_index >= 0 && begin_index <= end_index);

        const t_size range_len = end_index - begin_index;
        const auto mask_at_bottom = static_cast<tp_type>(BitMask<tp_type>(range_len) - 1);
        return static_cast<tp_type>(mask_at_bottom << begin_index);
    }

    template<co_unsigned_integral tp_type>
    t_b8 IsBitSet(const tp_type& val, const t_size index) {
        ZF_ASSERT(index >= 0 && index < ZF_SIZE_IN_BITS(tp_type));
        return val & BitMask<t_u8>(index);
    }

    inline t_b8 IsBitSet(const c_bit_vector_view val, const t_size index) {
        ZF_ASSERT(index < val.BitCount());
        return IsBitSet(val.Bytes()[index / 8], index % 8);
    }

    template<co_unsigned_integral tp_type>
    void SetBit(tp_type& val, const t_size index) {
        ZF_ASSERT(index >= 0 && index < ZF_SIZE_IN_BITS(tp_type));
        val |= BitMask<t_u8>(index);
    }

    inline void SetBit(const c_bit_vector val, const t_size index) {
        ZF_ASSERT(index < val.BitCount());
        SetBit(val.Bytes()[index / 8], index % 8);
    }

    template<co_unsigned_integral tp_type>
    void UnsetBit(tp_type& val, const t_size index) {
        ZF_ASSERT(index >= 0 && index < ZF_SIZE_IN_BITS(tp_type));
        val &= ~BitMask<t_u8>(index);
    }

    inline void UnsetBit(const c_bit_vector val, const t_size index) {
        ZF_ASSERT(index < val.BitCount());
        UnsetBit(val.Bytes()[index / 8], index % 8);
    }

#if 0
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
#endif
}
