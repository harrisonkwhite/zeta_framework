#pragma once

#include <new>
#include <zcl/zcl_basic.h>

namespace zf {
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

    // Is n a power of 2?
    constexpr t_b8 IsAlignmentValid(const t_len n) {
        return n > 0 && (n & (n - 1)) == 0;
    }

    // Take n up to the next multiple of the alignment.
    constexpr t_len AlignForward(const t_len n, const t_len alignment) {
        ZF_ASSERT(IsAlignmentValid(alignment));
        return (n + alignment - 1) & ~(alignment - 1);
    }

    struct s_mem_arena {
    public:
        s_mem_arena() = default;

        s_mem_arena(const s_mem_arena &) = delete;
        s_mem_arena &operator=(const s_mem_arena &) = delete;

        [[nodiscard]] t_b8 Init(const t_len size);
        [[nodiscard]] t_b8 InitAsChild(const t_len size, s_mem_arena *const par);

        t_b8 IsInitted() const {
            return m_buf;
        }

        void Release();

        void *PushRaw(const t_len size, const t_len alignment);

        template <typename tp_type>
        tp_type *Push(const t_len cnt = 1) {
            ZF_ASSERT(IsInitted());

            const auto buf = static_cast<tp_type *>(PushRaw(ZF_SIZE_OF(tp_type) * cnt, ZF_ALIGN_OF(tp_type)));

            if (buf) {
                for (t_len i = 0; i < cnt; i++) {
                    new (&buf[i]) tp_type();
                }
            }

            return buf;
        }

        void Rewind(const t_len offs);

    private:
        void *m_buf = nullptr;
        t_len m_size = 0;
        t_len m_offs = 0;
        t_b8 m_is_child = false; // Invalid to free the buffer if it is.
    };

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

        constexpr s_array_rdonly(const tp_type *const buf, const t_len len) : m_buf(buf), m_len(len) {
            ZF_ASSERT((buf || len == 0) && len >= 0);
        }

        constexpr const tp_type *Buf() const {
            return m_buf;
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
            return m_buf[index];
        }

        constexpr s_array_rdonly<tp_type> Slice(const t_len beg, const t_len end) const {
            ZF_ASSERT(beg >= 0 && beg <= m_len);
            ZF_ASSERT(end >= beg && end <= m_len);

            return {m_buf + beg, end - beg};
        }

        constexpr t_b8 DoAllEqual(const tp_type &val, const t_bin_comparator<tp_type> comparator = DefaultBinComparator) const {
            ZF_ASSERT(comparator);

            if (m_len == 0) {
                return false;
            }

            for (t_len i = 0; i < m_len; i++) {
                if (!comparator(m_buf[i], val)) {
                    return false;
                }
            }

            return true;
        }

        constexpr t_b8 DoAnyEqual(const tp_type &val, const t_bin_comparator<tp_type> comparator = DefaultBinComparator) const {
            ZF_ASSERT(comparator);

            for (t_len i = 0; i < m_len; i++) {
                if (comparator(m_buf[i], val)) {
                    return true;
                }
            }

            return false;
        }

        constexpr void CopyTo(const s_array<tp_type> other, const t_b8 truncate = false) const {
            if (!truncate) {
                ZF_ASSERT(other.Len() >= m_len);

                for (t_len i = 0; i < m_len; i++) {
                    other[i] = m_buf[i];
                }
            } else {
                const auto min = ZF_MIN(m_len, other.Len());

                for (t_len i = 0; i < min; i++) {
                    other[i] = m_buf[i];
                }
            }
        }

    private:
        const tp_type *m_buf = nullptr;
        t_len m_len = 0;
    };

    template <typename tp_type>
    struct s_array {
        static_assert(!s_is_const<tp_type>::g_val);

    public:
        constexpr s_array() = default;

        constexpr s_array(tp_type *const buf, const t_len len) : m_buf(buf), m_len(len) {
            ZF_ASSERT((buf || len == 0) && len >= 0);
        }

        constexpr tp_type *Buf() const {
            return m_buf;
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
            return {m_buf, m_len};
        }

        constexpr tp_type &operator[](const t_len index) const {
            ZF_ASSERT(index >= 0 && index < m_len);
            return m_buf[index];
        }

        constexpr s_array<tp_type> Slice(const t_len beg, const t_len end) const {
            ZF_ASSERT(beg >= 0 && beg <= m_len);
            ZF_ASSERT(end >= beg && end <= m_len);

            return {m_buf + beg, end - beg};
        }

        constexpr t_b8 DoAllEqual(const tp_type &val, const t_bin_comparator<tp_type> comparator = DefaultBinComparator) const {
            return reinterpret_cast<const s_array_rdonly<tp_type> *>(this)->DoAllEqual(val, comparator);
        }

        constexpr t_b8 DoAnyEqual(const tp_type &val, const t_bin_comparator<tp_type> comparator = DefaultBinComparator) const {
            return reinterpret_cast<const s_array_rdonly<tp_type> *>(this)->DoAnyEqual(val, comparator);
        }

        constexpr void SetAllTo(const tp_type &val) const {
            for (t_len i = 0; i < m_len; i++) {
                m_buf[i] = val;
            }
        }

        constexpr void CopyTo(const s_array<tp_type> other, const t_b8 truncate = false) const {
            return reinterpret_cast<const s_array_rdonly<tp_type> *>(this)->CopyTo(other, truncate);
        }

        constexpr void Reverse() const {
            for (t_len i = 0; i < m_len / 2; i++) {
                Swap(m_buf[i], m_buf[m_len - 1 - i]);
            }
        }

    private:
        tp_type *m_buf = nullptr;
        t_len m_len = 0;
    };

    template <typename tp_type, t_len tp_len>
    struct s_static_array {
        static_assert(!s_is_const<tp_type>::g_val);

        static constexpr t_len g_len = tp_len;

        tp_type buf[tp_len] = {};

        constexpr s_static_array() = default;

        template <t_len tp_buf_len>
        constexpr s_static_array(const tp_type (&buf)[tp_buf_len]) {
            static_assert(tp_buf_len == tp_len);

            for (t_len i = 0; i < tp_buf_len; i++) {
                this->buf[i] = buf[i];
            }
        }

        constexpr operator s_array<tp_type>() {
            return {buf, tp_len};
        }

        constexpr operator s_array_rdonly<tp_type>() const {
            return {buf, tp_len};
        }

        constexpr tp_type &operator[](const t_len index) {
            ZF_ASSERT(index >= 0 && index < tp_len);
            return buf[index];
        }

        constexpr const tp_type &operator[](const t_len index) const {
            ZF_ASSERT(index >= 0 && index < tp_len);
            return buf[index];
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

    template <typename tp_type, t_len tp_len>
    constexpr s_array<tp_type> ToNonstaticArray(s_static_array<tp_type, tp_len> &arr) {
        return static_cast<s_array<tp_type>>(arr);
    }

    template <typename tp_type, t_len tp_len>
    constexpr s_array_rdonly<tp_type> ToNonstaticArray(const s_static_array<tp_type, tp_len> &arr) {
        return static_cast<s_array_rdonly<tp_type>>(arr);
    }

    template <typename tp_type>
    [[nodiscard]] t_b8 AllocArray(const t_len len, s_mem_arena *const mem_arena, s_array<tp_type> *const o_arr) {
        ZF_ASSERT(len > 0);

        const auto buf = mem_arena->Push<tp_type>(len);

        if (!buf) {
            return false;
        }

        *o_arr = {buf, len};

        return true;
    }

    template <c_nonstatic_array tp_type>
    [[nodiscard]] t_b8 AllocArrayClone(const tp_type arr_to_clone, s_mem_arena *const mem_arena, s_array<typename tp_type::t_elem> *const o_arr) {
        if (!AllocArray(arr_to_clone.Len(), mem_arena, o_arr)) {
            return false;
        }

        arr_to_clone.CopyTo(*o_arr);

        return true;
    }

    template <typename tp_type>
    constexpr s_array<t_u8> ToBytes(tp_type &item) {
        return {reinterpret_cast<t_u8 *>(&item), ZF_SIZE_OF(item)};
    }

    template <typename tp_type>
    constexpr s_array_rdonly<t_u8> ToBytes(const tp_type &item) {
        return {reinterpret_cast<const t_u8 *>(&item), ZF_SIZE_OF(item)};
    }
}
