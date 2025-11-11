#pragma once

#include <zc/zc_essential.h>

namespace zf {
    class c_bit_vector_view {
    public:
        c_bit_vector_view() = default;

        c_bit_vector_view(const c_array<const t_u8> bytes, const t_size bit_cnt) : m_bytes(bytes), m_bit_cnt(bit_cnt) {
            ZF_ASSERT(bit_cnt >= 0);
            ZF_ASSERT(bytes.Len() == BitsToBytes(bit_cnt));
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
            ZF_ASSERT(bytes.Len() == BitsToBytes(bit_cnt));
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
}
