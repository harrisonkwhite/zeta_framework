#pragma once

#include <zc/zc_seqs.h>
#include <zc/zc_math.h>

namespace zf {
    class c_bit_vector_view {
    public:
        constexpr c_bit_vector_view() = default;

        constexpr c_bit_vector_view(const c_array<const t_u8> bytes)
            : m_bytes(bytes), m_bit_cnt(BytesToBits(bytes.Len())) {}

        constexpr c_bit_vector_view(const c_array<const t_u8> bytes, const t_size bit_cnt)
            : m_bytes(bytes), m_bit_cnt(bit_cnt) {
            ZF_ASSERT(bit_cnt >= 0);
            ZF_ASSERT(bytes.Len() == BitsToBytes(bit_cnt));
        }

        constexpr c_array<const t_u8> Bytes() const {
            return m_bytes;
        }

        constexpr t_size BitCount() const {
            return m_bit_cnt;
        }

    private:
        c_array<const t_u8> m_bytes;
        t_size m_bit_cnt = 0;
    };

    class c_bit_vector {
    public:
        constexpr c_bit_vector() = default;

        constexpr c_bit_vector(const c_array<t_u8> bytes)
            : m_bytes(bytes), m_bit_cnt(BytesToBits(bytes.Len())) {}

        constexpr c_bit_vector(const c_array<t_u8> bytes, const t_size bit_cnt)
            : m_bytes(bytes), m_bit_cnt(bit_cnt) {
            ZF_ASSERT(bit_cnt >= 0);
            ZF_ASSERT(bytes.Len() == BitsToBytes(bit_cnt));
        }

        constexpr c_array<t_u8> Bytes() const {
            return m_bytes;
        }

        constexpr t_size BitCount() const {
            return m_bit_cnt;
        }

    private:
        c_array<t_u8> m_bytes;
        t_size m_bit_cnt = 0;
    };

    template<t_size tp_bit_cnt>
    struct s_static_bit_vector {
        static_assert(tp_bit_cnt > 0);

        s_static_array<t_u8, BitsToBytes(tp_bit_cnt)> bytes;

        constexpr t_size BitCount() const {
            return tp_bit_cnt;
        }

        constexpr operator c_bit_vector() {
            return {bytes, tp_bit_cnt};
        }

        constexpr operator c_bit_vector_view() const {
            return {bytes, tp_bit_cnt};
        }
    };

#if 0
    template<t_size tp_bit_cnt>
    constexpr s_static_bit_vector<tp_bit_cnt> BitMask(const t_size index) {
        ZF_ASSERT(index >= 0 && index < tp_bit_cnt);
        return static_cast<tp_type>(static_cast<tp_type>(1) << index);
    }
#endif

    inline t_b8 IsBitSet(const c_bit_vector_view bv, const t_size index) {
        ZF_ASSERT(index >= 0 && index < bv.BitCount());
        return bv.Bytes()[index / 8] & (1 << (index % 8));
    }

    inline void SetBit(const c_bit_vector bv, const t_size index) {
        ZF_ASSERT(index >= 0 && index < bv.BitCount());
        bv.Bytes()[index / 8] |= (1 << (index % 8));
    }

    inline void UnsetBit(const c_bit_vector bv, const t_size index) {
        ZF_ASSERT(index >= 0 && index < bv.BitCount());
        bv.Bytes()[index / 8] &= ~(1 << (index % 8));
    }

    void ShiftLeft(const c_bit_vector bv, const t_size amount = 1);
    void RotLeft(const c_bit_vector bv, const t_size amount = 1);
    void ShiftRight(const c_bit_vector bv, const t_size amount = 1);
    void RotRight(const c_bit_vector bv, const t_size amount = 1);

#if 0
    t_size FindFirstSetBit(const c_bit_vector_view bv, const t_size index);
    t_size FindFirstUnsetBit(const c_bit_vector_view bv, const t_size index);
#endif

#if 0
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

    inline t_b8 IsBitSet(const c_bit_vector_view bv, const t_size index) {
        ZF_ASSERT(index < bv.BitCount());
        return IsBitSet(bv.Bytes()[index / 8], index % 8);
    }

    template<co_unsigned_integral tp_type>
    void SetBit(tp_type& val, const t_size index) {
        ZF_ASSERT(index >= 0 && index < ZF_SIZE_IN_BITS(tp_type));
        val |= BitMask<t_u8>(index);
    }

    inline void SetBit(const c_bit_vector bv, const t_size index) {
        ZF_ASSERT(index < bv.BitCount());
        SetBit(bv.Bytes()[index / 8], index % 8);
    }

    template<co_unsigned_integral tp_type>
    void UnsetBit(tp_type& val, const t_size index) {
        ZF_ASSERT(index >= 0 && index < ZF_SIZE_IN_BITS(tp_type));
        val &= ~BitMask<t_u8>(index);
    }

    inline void UnsetBit(const c_bit_vector bv, const t_size index) {
        ZF_ASSERT(index < bv.BitCount());
        UnsetBit(bv.Bytes()[index / 8], index % 8);
    }

    template<co_unsigned_integral tp_type> t_s64 FindFirstSetBit(const c_bit_vector bv, const t_size index);
    t_s64 FindFirstSetBit(const c_bit_vector bv, const t_size index);

    template<co_unsigned_integral tp_type> t_s64 FindFirstUnsetBit(const c_bit_vector bv, const t_size index);
    t_s64 FindFirstUnsetBit(const c_bit_vector bv, const t_size index);
#endif
}
