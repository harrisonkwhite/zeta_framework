#pragma once

#include <new>
#include <initializer_list>
#include <zcl/zcl_basic.h>

namespace zf {
    // Is n a power of 2?
    constexpr t_b8 IsAlignmentValid(const t_len n) {
        return n > 0 && (n & (n - 1)) == 0;
    }

    // Take n up to the next multiple of the alignment.
    constexpr t_len AlignForward(const t_len n, const t_len alignment) {
        ZF_ASSERT(IsAlignmentValid(alignment));
        return (n + alignment - 1) & ~(alignment - 1);
    }

    template <typename tp_type>
    struct s_ptr {
    public:
        constexpr s_ptr() = default;
        constexpr s_ptr(tp_type *const raw) : m_raw(raw) {}

        constexpr tp_type *Raw() const {
            return m_raw;
        }

        constexpr operator tp_type *() const {
            return m_raw;
        }

        constexpr operator t_b8() const {
            return m_raw != nullptr;
        }

        constexpr tp_type &operator*() const {
            ZF_ASSERT(m_raw);
            return *m_raw;
        }

        constexpr tp_type *operator->() const {
            ZF_ASSERT(m_raw);
            return m_raw;
        }

        constexpr tp_type &operator[](const t_len index) const {
            ZF_ASSERT(m_raw);
            return m_raw[index];
        }

        constexpr t_b8 operator==(const s_ptr<tp_type> other) const {
            return m_raw == other.m_raw;
        }

        constexpr t_b8 operator!=(const s_ptr<tp_type> other) const {
            return m_raw != other.m_raw;
        }

        constexpr s_ptr<tp_type> operator+(const t_len offs) const {
            return {m_raw + offs};
        }

        constexpr s_ptr<tp_type> operator-(const t_len offs) const {
            return {m_raw - offs};
        }

        constexpr s_ptr<tp_type> &operator++() {
            m_raw++;
            return *this;
        }

        constexpr s_ptr<tp_type> &operator--() {
            m_raw--;
            return *this;
        }

        constexpr s_ptr<tp_type> &operator+=(const t_len offs) {
            m_raw += offs;
            return *this;
        }

        constexpr s_ptr<tp_type> &operator-=(const t_len offs) {
            m_raw -= offs;
            return *this;
        }

        constexpr operator s_ptr<const tp_type>() const
            requires(!s_is_const<tp_type>::g_val)
        {
            return {m_raw};
        }

    private:
        tp_type *m_raw = nullptr;
    };

    template <>
    struct s_ptr<const void> {
    public:
        constexpr s_ptr() = default;
        constexpr s_ptr(const void *const raw) : m_raw(raw) {}

        constexpr const void *Raw() const {
            return m_raw;
        }

        constexpr operator const void *() const {
            return m_raw;
        }

        constexpr operator t_b8() const {
            return m_raw != nullptr;
        }

        constexpr t_b8 operator==(const s_ptr<const void> other) const {
            return m_raw == other.m_raw;
        }

        constexpr t_b8 operator!=(const s_ptr<const void> other) const {
            return m_raw != other.m_raw;
        }

        template <typename tp_type>
        explicit constexpr operator s_ptr<const tp_type>() const {
            return {static_cast<const tp_type *>(m_raw)};
        }

    private:
        const void *m_raw = nullptr;
    };

    template <>
    struct s_ptr<void> {
    public:
        constexpr s_ptr() = default;
        constexpr s_ptr(void *const raw) : m_raw(raw) {}

        constexpr void *Raw() const {
            return m_raw;
        }

        constexpr operator void *() const {
            return m_raw;
        }

        constexpr operator t_b8() const {
            return m_raw != nullptr;
        }

        constexpr t_b8 operator==(const s_ptr<void> other) const {
            return m_raw == other.m_raw;
        }

        constexpr t_b8 operator!=(const s_ptr<void> other) const {
            return m_raw != other.m_raw;
        }

        constexpr operator s_ptr<const void>() const {
            return {m_raw};
        }

        template <typename tp_type>
        explicit constexpr operator s_ptr<tp_type>() const {
            return {static_cast<tp_type *>(m_raw)};
        }

    private:
        void *m_raw = nullptr;
    };

    struct s_mem_arena {
    public:
        s_mem_arena() = default;

        s_mem_arena(const s_mem_arena &) = delete;
        s_mem_arena &operator=(const s_mem_arena &) = delete;

        [[nodiscard]] t_b8 Init(const t_len size);
        [[nodiscard]] t_b8 InitAsChild(const t_len size, s_mem_arena &par);

        t_b8 IsInitted() const {
            return m_buf;
        }

        void Release();

        s_ptr<void> Push(const t_len size, const t_len alignment);

        void Rewind(const t_len offs) {
            ZF_ASSERT(IsInitted());
            ZF_ASSERT(offs >= 0 && offs <= m_offs);

            m_offs = offs;
        }

    private:
        s_ptr<void> m_buf = nullptr;
        t_len m_size = 0;
        t_len m_offs = 0;
        t_b8 m_is_child = false; // Invalid to free the buffer if it is.
    };

    template <typename tp_type>
    s_ptr<tp_type> Alloc(s_mem_arena &mem_arena) {
        const auto ptr = static_cast<s_ptr<tp_type>>(mem_arena.Push(ZF_SIZE_OF(tp_type), ZF_ALIGN_OF(tp_type)));

        if (ptr) {
            new (ptr) tp_type();
        }

        return ptr;
    }

    // ============================================================
    // @section: Arrays
    // ============================================================
    template <typename tp_type>
    struct s_array;

    template <typename tp_type>
    struct s_array_rdonly {
        static_assert(!s_is_const<tp_type>::g_val);

    public:
        constexpr s_array_rdonly() = default;

        constexpr s_array_rdonly(const s_ptr<const tp_type> ptr, const t_len len) : m_ptr(ptr), m_len(len) {
            ZF_ASSERT((ptr || len == 0) && len >= 0);
        }

        constexpr s_ptr<const tp_type> Ptr() const {
            return m_ptr;
        }

        constexpr t_len Len() const {
            return m_len;
        }

        constexpr t_b8 IsEmpty() const {
            return m_len == 0;
        }

        constexpr t_len SizeInBytes() const {
            return ZF_SIZE_OF(tp_type) * m_len;
        }

        constexpr const tp_type &operator[](const t_len index) const {
            ZF_ASSERT(index >= 0 && index < m_len);
            return m_ptr[index];
        }

        constexpr const tp_type &Last() const {
            return operator[](m_len - 1);
        }

        constexpr s_array_rdonly<tp_type> Slice(const t_len beg, const t_len end) const {
            ZF_ASSERT(beg >= 0 && beg <= m_len);
            ZF_ASSERT(end >= beg && end <= m_len);

            return {&m_ptr[beg], end - beg};
        }

        constexpr s_array_rdonly<t_u8> ToBytes() const {
            return {reinterpret_cast<const t_u8 *>(m_ptr.Raw()), SizeInBytes()};
        }

        constexpr t_b8 DoAllEqual(const tp_type &val, const t_bin_comparator<tp_type> comparator = DefaultBinComparator) const {
            ZF_ASSERT(comparator);

            if (m_len == 0) {
                return false;
            }

            for (t_len i = 0; i < m_len; i++) {
                if (!comparator(m_ptr[i], val)) {
                    return false;
                }
            }

            return true;
        }

        constexpr t_b8 DoAnyEqual(const tp_type &val, const t_bin_comparator<tp_type> comparator = DefaultBinComparator) const {
            ZF_ASSERT(comparator);

            for (t_len i = 0; i < m_len; i++) {
                if (comparator(m_ptr[i], val)) {
                    return true;
                }
            }

            return false;
        }

        constexpr void CopyTo(const s_array<tp_type> other, const t_b8 truncate = false) const {
            if (!truncate) {
                ZF_ASSERT(other.Len() >= m_len);

                for (t_len i = 0; i < m_len; i++) {
                    other[i] = m_ptr[i];
                }
            } else {
                const auto min = ZF_MIN(m_len, other.Len());

                for (t_len i = 0; i < min; i++) {
                    other[i] = m_ptr[i];
                }
            }
        }

    private:
        s_ptr<const tp_type> m_ptr = nullptr;
        t_len m_len = 0;
    };

    template <typename tp_type>
    struct s_array {
        static_assert(!s_is_const<tp_type>::g_val);

    public:
        constexpr s_array() = default;

        constexpr s_array(const s_ptr<tp_type> ptr, const t_len len) : m_ptr(ptr), m_len(len) {
            ZF_ASSERT((ptr || len == 0) && len >= 0);
        }

        constexpr s_ptr<tp_type> Ptr() const {
            return m_ptr;
        }

        constexpr t_len Len() const {
            return m_len;
        }

        constexpr t_b8 IsEmpty() const {
            return m_len == 0;
        }

        constexpr t_len SizeInBytes() const {
            return ZF_SIZE_OF(tp_type) * m_len;
        }

        constexpr operator s_array_rdonly<tp_type>() const {
            return {m_ptr, m_len};
        }

        constexpr tp_type &operator[](const t_len index) const {
            ZF_ASSERT(index >= 0 && index < m_len);
            return m_ptr[index];
        }

        constexpr tp_type &Last() const {
            return operator[](m_len - 1);
        }

        constexpr s_array<tp_type> Slice(const t_len beg, const t_len end) const {
            ZF_ASSERT(beg >= 0 && beg <= m_len);
            ZF_ASSERT(end >= beg && end <= m_len);

            return {m_ptr + beg, end - beg};
        }

        constexpr s_array<t_u8> ToBytes() const {
            return {reinterpret_cast<t_u8 *>(m_ptr.Raw()), SizeInBytes()};
        }

        constexpr t_b8 DoAllEqual(const tp_type &val, const t_bin_comparator<tp_type> comparator = DefaultBinComparator) const {
            return static_cast<const s_array_rdonly<tp_type>>(*this).DoAllEqual(val, comparator);
        }

        constexpr t_b8 DoAnyEqual(const tp_type &val, const t_bin_comparator<tp_type> comparator = DefaultBinComparator) const {
            return static_cast<const s_array_rdonly<tp_type>>(*this).DoAnyEqual(val, comparator);
        }

        constexpr void SetAllTo(const tp_type &val) const {
            for (t_len i = 0; i < m_len; i++) {
                m_ptr[i] = val;
            }
        }

        constexpr void CopyTo(const s_array<tp_type> other, const t_b8 truncate = false) const {
            return static_cast<const s_array_rdonly<tp_type>>(*this).CopyTo(other, truncate);
        }

        constexpr void Reverse() const {
            for (t_len i = 0; i < m_len / 2; i++) {
                Swap(m_ptr[i], m_ptr[m_len - 1 - i]);
            }
        }

    private:
        s_ptr<tp_type> m_ptr = nullptr;
        t_len m_len = 0;
    };

    template <typename tp_type, t_len tp_len>
    struct s_static_array {
        static_assert(!s_is_const<tp_type>::g_val);
        static_assert(tp_len > 0);

        static constexpr t_len g_len = tp_len;

        tp_type raw[tp_len] = {};

        constexpr s_static_array() = default;

        template <t_len tp_other_len>
        constexpr s_static_array(const tp_type (&raw)[tp_other_len]) {
            static_assert(tp_other_len == tp_len);

            for (t_len i = 0; i < tp_other_len; i++) {
                this->raw[i] = raw[i];
            }
        }

        constexpr s_static_array(const std::initializer_list<tp_type> init) {
            ZF_ASSERT(init.size() == tp_len);

            t_len i = 0;

            for (const auto &v : init) {
                raw[i] = v;
                i++;
            }
        }

        constexpr operator s_array<tp_type>() {
            return {raw, tp_len};
        }

        constexpr operator s_array_rdonly<tp_type>() const {
            return {raw, tp_len};
        }

        constexpr tp_type &operator[](const t_len index) {
            ZF_ASSERT(index >= 0 && index < tp_len);
            return raw[index];
        }

        constexpr const tp_type &operator[](const t_len index) const {
            ZF_ASSERT(index >= 0 && index < tp_len);
            return raw[index];
        }
    };

    template <typename tp_type>
    struct s_is_nonstatic_array {
        static constexpr t_b8 g_val = false;
    };

    template <typename tp_type>
    struct s_is_nonstatic_array<s_array<tp_type>> {
        static constexpr t_b8 g_val = true;
    };

    template <typename tp_type>
    struct s_is_nonstatic_array<s_array_rdonly<tp_type>> {
        static constexpr t_b8 g_val = true;
    };

    template <typename tp_type>
    concept c_nonstatic_array = s_is_nonstatic_array<tp_type>::g_val;

    template <typename tp_type>
    struct s_is_nonstatic_mut_array {
        static constexpr t_b8 g_val = false;
    };

    template <typename tp_type>
    struct s_is_nonstatic_mut_array<s_array<tp_type>> {
        static constexpr t_b8 g_val = true;
    };

    template <typename tp_type>
    concept c_nonstatic_mut_array = s_is_nonstatic_array<tp_type>::g_val;

    template <typename tp_type>
    [[nodiscard]] t_b8 AllocArray(const t_len len, s_mem_arena &mem_arena, s_array<tp_type> &o_arr) {
        ZF_ASSERT(len > 0);

        const auto ptr = static_cast<s_ptr<tp_type>>(mem_arena.Push(ZF_SIZE_OF(tp_type) * len, ZF_ALIGN_OF(tp_type)));

        if (!ptr) {
            return false;
        }

        for (t_len i = 0; i < len; i++) {
            new (&ptr[i]) tp_type();
        }

        o_arr = {ptr, len};

        return true;
    }

    template <c_nonstatic_array tp_type>
    [[nodiscard]] t_b8 AllocArrayClone(const tp_type arr_to_clone, s_mem_arena &mem_arena, s_array<typename tp_type::t_elem> &o_arr) {
        if (!AllocArray(arr_to_clone.Len(), mem_arena, o_arr)) {
            return false;
        }

        arr_to_clone.CopyTo(*o_arr);

        return true;
    }

    template <typename tp_type>
    constexpr s_array<t_u8> ToBytes(tp_type &val) {
        return {reinterpret_cast<t_u8 *>(&val), ZF_SIZE_OF(val)};
    }

    template <typename tp_type>
    constexpr s_array_rdonly<t_u8> ToBytes(const tp_type &val) {
        return {reinterpret_cast<const t_u8 *>(&val), ZF_SIZE_OF(val)};
    }

    // ============================================================
    // @section: Bits
    // ============================================================
    constexpr t_len Kilobytes(const t_len x) {
        return (static_cast<t_len>(1) << 10) * x;
    }

    constexpr t_len Megabytes(const t_len x) {
        return (static_cast<t_len>(1) << 20) * x;
    }

    constexpr t_len Gigabytes(const t_len x) {
        return (static_cast<t_len>(1) << 30) * x;
    }

    constexpr t_len Terabytes(const t_len x) {
        return (static_cast<t_len>(1) << 40) * x;
    }

    constexpr t_len BitsToBytes(const t_len x) {
        return (x + 7) / 8;
    }

    constexpr t_len BytesToBits(const t_len x) {
        return x * 8;
    }

    // Creates a bitmask with only a single bit set.
    constexpr t_u8 BitmaskSingle(const t_len bit_index) {
        ZF_ASSERT(bit_index >= 0 && bit_index < 8);
        return static_cast<t_u8>(1 << bit_index);
    }

    // Creates a bitmask with only bits in the range [begin_bit_index, end_bit_index) set.
    constexpr t_u8 BitmaskRange(const t_len begin_bit_index, const t_len end_bit_index = 8) {
        ZF_ASSERT(begin_bit_index >= 0 && begin_bit_index < 8);
        ZF_ASSERT(end_bit_index >= begin_bit_index && end_bit_index <= 8);

        if (end_bit_index - begin_bit_index == 8) {
            return 0xFF;
        }

        const auto bits_at_bottom = static_cast<t_u8>((1 << (end_bit_index - begin_bit_index)) - 1);
        return static_cast<t_u8>(bits_at_bottom << begin_bit_index);
    }

    struct s_bit_vec_rdonly {
    public:
        constexpr s_bit_vec_rdonly() = default;

        constexpr s_bit_vec_rdonly(const s_array_rdonly<t_u8> bytes, const t_len bit_cnt) : m_bytes(bytes), m_bit_cnt(bit_cnt) {
            ZF_ASSERT(BitsToBytes(bit_cnt) == bytes.Len());
        }

        constexpr s_bit_vec_rdonly(const s_array_rdonly<t_u8> bytes) : m_bytes(bytes), m_bit_cnt(BytesToBits(bytes.Len())) {}

        constexpr s_array_rdonly<t_u8> Bytes() const {
            return m_bytes;
        }

        constexpr t_len BitCount() const {
            return m_bit_cnt;
        }

        constexpr t_len LastByteBitCount() const {
            return ((m_bit_cnt - 1) % 8) + 1;
        }

        // Gives a mask of the last byte in which only excess bits are unset.
        constexpr t_u8 LastByteMask() const {
            return BitmaskRange(0, LastByteBitCount());
        }

    private:
        s_array_rdonly<t_u8> m_bytes = {};
        t_len m_bit_cnt = 0;
    };

    struct s_bit_vec {
    public:
        constexpr s_bit_vec() = default;

        constexpr s_bit_vec(const s_array<t_u8> bytes, const t_len bit_cnt) : m_bytes(bytes), m_bit_cnt(bit_cnt) {
            ZF_ASSERT(BitsToBytes(bit_cnt) == bytes.Len());
        }

        constexpr s_bit_vec(const s_array<t_u8> bytes) : m_bytes(bytes), m_bit_cnt(BytesToBits(bytes.Len())) {}

        constexpr operator s_bit_vec_rdonly() const {
            return {m_bytes, m_bit_cnt};
        }

        constexpr s_array<t_u8> Bytes() const {
            return m_bytes;
        }

        constexpr t_len BitCount() const {
            return m_bit_cnt;
        }

        constexpr t_len LastByteBitCount() const {
            return ((m_bit_cnt - 1) % 8) + 1;
        }

        // Gives a mask of the last byte in which only excess bits are unset.
        constexpr t_u8 LastByteMask() const {
            return BitmaskRange(0, LastByteBitCount());
        }

    private:
        s_array<t_u8> m_bytes = {};
        t_len m_bit_cnt = 0;
    };

    template <t_len tp_bit_cnt>
    struct s_static_bit_vec {
        static_assert(tp_bit_cnt > 0);

        static constexpr t_len g_bit_cnt = tp_bit_cnt;

        s_static_array<t_u8, BitsToBytes(tp_bit_cnt)> bytes = {};

        constexpr operator s_bit_vec() {
            return {bytes, tp_bit_cnt};
        }

        constexpr operator s_bit_vec_rdonly() const {
            return {bytes, tp_bit_cnt};
        }
    };

    [[nodiscard]] inline t_b8 CreateBitVec(const t_len bit_cnt, s_mem_arena &mem_arena, s_bit_vec &o_bv) {
        ZF_ASSERT(bit_cnt > 0);

        s_array<t_u8> bytes;

        if (!AllocArray(BitsToBytes(bit_cnt), mem_arena, bytes)) {
            return false;
        }

        o_bv = {bytes, bit_cnt};

        return true;
    }

    constexpr t_b8 IsBitSet(const s_bit_vec_rdonly bv, const t_len index) {
        ZF_ASSERT(index >= 0 && index < bv.BitCount());
        return bv.Bytes()[index / 8] & BitmaskSingle(index % 8);
    }

    constexpr void SetBit(const s_bit_vec bv, const t_len index) {
        ZF_ASSERT(index >= 0 && index < bv.BitCount());
        bv.Bytes()[index / 8] |= BitmaskSingle(index % 8);
    }

    constexpr void UnsetBit(const s_bit_vec bv, const t_len index) {
        ZF_ASSERT(index >= 0 && index < bv.BitCount());
        bv.Bytes()[index / 8] &= ~BitmaskSingle(index % 8);
    }

    constexpr t_b8 IsAnyBitSet(const s_bit_vec_rdonly bv) {
        if (bv.BitCount() == 0) {
            return false;
        }

        const auto first_bytes = bv.Bytes().Slice(0, bv.Bytes().Len() - 1);

        if (!first_bytes.DoAllEqual(0)) {
            return true;
        }

        return (bv.Bytes().Last() & bv.LastByteMask()) != 0;
    }

    constexpr t_b8 AreAllBitsSet(const s_bit_vec_rdonly bv) {
        if (bv.BitCount() == 0) {
            return false;
        }

        const auto first_bytes = bv.Bytes().Slice(0, bv.Bytes().Len() - 1);

        if (!first_bytes.DoAllEqual(0xFF)) {
            return false;
        }

        const auto last_byte_mask = bv.LastByteMask();
        return (bv.Bytes().Last() & last_byte_mask) == last_byte_mask;
    }

    constexpr t_b8 AreAllBitsUnset(const s_bit_vec_rdonly bv) {
        if (bv.BitCount() == 0) {
            return false;
        }

        return !IsAnyBitSet(bv);
    }

    constexpr t_b8 IsAnyBitUnset(const s_bit_vec_rdonly bv) {
        if (bv.BitCount() == 0) {
            return false;
        }

        return !AreAllBitsSet(bv);
    }

    // Sets all bits in the range [begin_bit_index, end_bit_index).
    constexpr void SetBitsInRange(const s_bit_vec bv, const t_len begin_bit_index, const t_len end_bit_index) {
        ZF_ASSERT(begin_bit_index >= 0 && begin_bit_index < bv.BitCount());
        ZF_ASSERT(end_bit_index >= begin_bit_index && end_bit_index <= bv.BitCount());

        const t_len begin_elem_index = begin_bit_index / 8;
        const t_len end_elem_index = BitsToBytes(end_bit_index);

        for (t_len i = begin_elem_index; i < end_elem_index; i++) {
            const t_len bit_offs = i * 8;
            const t_len begin_bit_index_rel = begin_bit_index - bit_offs;
            const t_len end_bit_index_rel = end_bit_index - bit_offs;

            const t_len set_range_begin = ZF_MAX(begin_bit_index_rel, 0);
            const t_len set_range_end = ZF_MIN(end_bit_index_rel, 8);

            bv.Bytes()[i] |= BitmaskRange(set_range_begin, set_range_end);
        }
    }

    enum e_bitwise_mask_op : t_i32 {
        ek_bitwise_mask_op_and,
        ek_bitwise_mask_op_or,
        ek_bitwise_mask_op_xor,
        ek_bitwise_mask_op_andnot
    };

    constexpr void ApplyMask(const s_bit_vec dest, const s_bit_vec_rdonly src, const e_bitwise_mask_op op) {
        ZF_ASSERT(dest.BitCount() == src.BitCount());

        if (dest.BitCount() == 0) {
            return;
        }

        switch (op) {
        case ek_bitwise_mask_op_and:
            for (t_len i = 0; i < dest.Bytes().Len(); i++) {
                dest.Bytes()[i] &= src.Bytes()[i];
            }

            break;

        case ek_bitwise_mask_op_or:
            for (t_len i = 0; i < dest.Bytes().Len(); i++) {
                dest.Bytes()[i] |= src.Bytes()[i];
            }

            break;

        case ek_bitwise_mask_op_xor:
            for (t_len i = 0; i < dest.Bytes().Len(); i++) {
                dest.Bytes()[i] ^= src.Bytes()[i];
            }

            break;

        case ek_bitwise_mask_op_andnot:
            for (t_len i = 0; i < dest.Bytes().Len(); i++) {
                dest.Bytes()[i] &= ~src.Bytes()[i];
            }

            break;
        }

        dest.Bytes().Last() &= dest.LastByteMask();
    }

    // Shifts left only by 1. Returns the discarded bit as 0 or 1.
    constexpr t_u8 ShiftBitsLeft(const s_bit_vec bv) {
        if (bv.BitCount() == 0) {
            return 0;
        }

        t_u8 discard = 0;

        for (t_len i = 0; i < bv.Bytes().Len(); i++) {
            const t_len bits_in_byte = i == bv.Bytes().Len() - 1 ? bv.LastByteBitCount() : 8;
            const t_u8 discard_last = discard;
            discard = (bv.Bytes()[i] & BitmaskSingle(bits_in_byte - 1)) >> (bits_in_byte - 1);
            bv.Bytes()[i] <<= 1;
            bv.Bytes()[i] |= discard_last;
        }

        bv.Bytes().Last() &= bv.LastByteMask();

        return discard;
    }

    constexpr void ShiftBitsLeftBy(const s_bit_vec bv, const t_len amount) {
        ZF_ASSERT(amount >= 0);

        // @speed: :(

        for (t_len i = 0; i < amount; i++) {
            ShiftBitsLeft(bv);
        }
    }

    constexpr void RotBitsLeftBy(const s_bit_vec bv, const t_len amount) {
        ZF_ASSERT(amount >= 0);

        if (bv.BitCount() == 0) {
            return;
        }

        // @speed: :(

        for (t_len i = 0; i < amount; i++) {
            const auto discard = ShiftBitsLeft(bv);

            if (discard) {
                SetBit(bv, 0);
            } else {
                UnsetBit(bv, 0);
            }
        }
    }

    // Shifts right only by 1. Returns the carry bit.
    constexpr t_u8 ShiftBitsRight(const s_bit_vec bv) {
        if (bv.BitCount() == 0) {
            return 0;
        }

        bv.Bytes().Last() &= bv.LastByteMask(); // Drop any excess bits so we don't accidentally shift a 1 in.

        t_u8 discard = 0;

        for (t_len i = bv.Bytes().Len() - 1; i >= 0; i--) {
            const t_len bits_in_byte = i == bv.Bytes().Len() - 1 ? bv.LastByteBitCount() : 8;
            const t_u8 discard_last = discard;
            discard = bv.Bytes()[i] & BitmaskSingle(0);
            bv.Bytes()[i] >>= 1;

            if (discard_last) {
                bv.Bytes()[i] |= BitmaskSingle(bits_in_byte - 1);
            }
        }

        return discard;
    }

    constexpr void ShiftBitsRightBy(const s_bit_vec bv, const t_len amount) {
        ZF_ASSERT(amount >= 0);

        // @speed: :(

        for (t_len i = 0; i < amount; i++) {
            ShiftBitsRight(bv);
        }
    }

    constexpr void RotBitsRightBy(const s_bit_vec bv, const t_len amount) {
        ZF_ASSERT(amount >= 0);

        if (bv.BitCount() == 0) {
            return;
        }

        // @speed: :(

        for (t_len i = 0; i < amount; i++) {
            const auto discard = ShiftBitsRight(bv);

            if (discard) {
                SetBit(bv, bv.BitCount() - 1);
            } else {
                UnsetBit(bv, bv.BitCount() - 1);
            }
        }
    }

    t_len IndexOfFirstSetBit(const s_bit_vec_rdonly bv, const t_len from = 0);   // Returns -1 if all bits are unset.
    t_len IndexOfFirstUnsetBit(const s_bit_vec_rdonly bv, const t_len from = 0); // Returns -1 if all bits are set.

    t_len CntSetBits(const s_bit_vec_rdonly bv);

    inline t_len CntUnsetBits(const s_bit_vec_rdonly bv) {
        return bv.BitCount() - CntSetBits(bv);
    }

    // pos is the walker state, initialise it to the bit index you want to start from.
    // o_index is assigned the index of the set bit to process.
    // Returns false iff the walk is complete.
    // You can use the ZF_FOR_EACH_SET_BIT macro for more brevity if you like.
    inline t_b8 WalkSetBits(const s_bit_vec_rdonly bv, t_len &pos, t_len &o_index) {
        ZF_ASSERT(pos >= 0 && pos < bv.BitCount());
        o_index = IndexOfFirstSetBit(bv, pos);
        pos = o_index + 1;
        return o_index != -1;
    }

    // pos is the walker state, initialise it to the bit index you want to start from.
    // o_index is assigned the index of the unset bit to process.
    // Returns false iff the walk is complete.
    // You can use the ZF_FOR_EACH_UNSET_BIT macro for more brevity if you like.
    inline t_b8 WalkUnsetBits(const s_bit_vec_rdonly bv, t_len &pos, t_len &o_index) {
        ZF_ASSERT(pos >= 0 && pos < bv.BitCount());
        o_index = IndexOfFirstUnsetBit(bv, pos);
        pos = o_index + 1;
        return o_index != -1;
    }

#define ZF_FOR_EACH_SET_BIT(bv, index) for (t_len ZF_CONCAT(walk_pos_l, __LINE__) = 0, index; WalkSetBits(bv, ZF_CONCAT(walk_pos_l, __LINE__), index);)
#define ZF_FOR_EACH_UNSET_BIT(bv, index) for (t_len ZF_CONCAT(walk_pos_l, __LINE__) = 0, index; WalkUnsetBits(bv, ZF_CONCAT(walk_pos_l, __LINE__), index);)
}
