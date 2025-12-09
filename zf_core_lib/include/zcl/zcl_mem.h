#pragma once

#include <zcl/zcl_basic.h>

#include <cstdlib>
#include <cstring>

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

    template <typename tp_type>
    void Clear(tp_type *const val, const t_u8 byte = 0) {
        static_assert(!s_is_ptr<tp_type>::g_val);
        ZF_ASSERT(val);
        memset(val, byte, sizeof(*val));
    }

    template <typename tp_type>
    t_b8 IsClear(const tp_type *const val, const t_u8 byte = 0) {
        static_assert(!s_is_ptr<tp_type>::g_val);
        ZF_ASSERT(val);

        const auto val_bytes = reinterpret_cast<const t_u8 *>(val);

        for (t_len i = 0; i < ZF_SIZE_OF(*val); i++) {
            if (val_bytes[i] != byte) {
                return false;
            }
        }

        return true;
    }

#ifdef ZF_DEBUG
    constexpr t_u8 g_uninitted_byte = 0xCD;
    constexpr t_u8 g_freed_byte = 0xDD;

    template <typename tp_type>
    t_b8 IsUninitted(tp_type *const val) {
        return IsClear(val, g_uninitted_byte);
    }

    #define ZF_STACK_GUARD(type, name) \
        type name;                     \
        zf::Clear(&name, zf::g_uninitted_byte)
#else
    template <typename tp_type>
    t_b8 IsUninitted(const tp_type *const val) {}

    #define ZF_STACK_GUARD(type, name) type name
#endif

    inline void *Alloc(const t_len size) {
        ZF_ASSERT(size >= 0);

#ifdef ZF_DEBUG
        const auto buf = malloc(static_cast<size_t>(size));

        if (buf) {
            memset(buf, g_uninitted_byte, static_cast<size_t>(size));
        }

        return buf;
#else
        return malloc(static_cast<size_t>(size));
#endif
    }

    inline void Free(void *const buf, const t_len size) {
        ZF_ASSERT(size >= 0);

        if (!buf) {
            return;
        }

#ifdef ZF_DEBUG
        memset(buf, g_freed_byte, static_cast<size_t>(size));
#endif

        free(buf);
    }

    struct s_mem_arena {
    public:
        [[nodiscard]] t_b8 Init(const t_len size) {
            ZF_ASSERT(!IsInitted());
            ZF_ASSERT(size > 0);

            const auto buf = calloc(static_cast<size_t>(size), 1);

            if (!buf) {
                return false;
            }

            m_buf = buf;
            m_size = size;

            return true;
        }

        [[nodiscard]] t_b8 InitAsChild(const t_len size, s_mem_arena *const par) {
            ZF_ASSERT(!IsInitted());
            ZF_ASSERT(size > 0);

            const auto buf = par->PushRaw(size, 1);

            if (!buf) {
                return false;
            }

            m_buf = buf;
            m_size = size;
            m_is_child = true;

            return true;
        }

        t_b8 IsInitted() const {
            return m_buf;
        }

        void Release() {
            ZF_ASSERT(IsInitted() && !m_is_child);
            free(m_buf);
            *this = {};
        }

        void *PushRaw(const t_len size, const t_len alignment) {
            ZF_ASSERT(IsInitted());
            ZF_ASSERT(size > 0);
            ZF_ASSERT(IsAlignmentValid(alignment));

            const t_len offs_aligned = AlignForward(m_offs, alignment);
            const t_len offs_next = offs_aligned + size;

            if (offs_next > m_size) {
                return nullptr;
            }

            m_offs = offs_next;

            return static_cast<t_u8 *>(m_buf) + offs_aligned;
        }

        template <typename tp_type>
        tp_type *Push(const t_len cnt = 1) {
            ZF_ASSERT(IsInitted());

            const auto buf = PushRaw(ZF_SIZE_OF(tp_type) * cnt, ZF_ALIGN_OF(tp_type));
            return static_cast<tp_type *>(buf);
        }

        void Rewind(const t_len offs) {
            ZF_ASSERT(IsInitted());
            ZF_ASSERT(offs >= 0 && offs <= m_offs);

            memset(static_cast<t_u8 *>(m_buf) + offs, 0, static_cast<size_t>(m_offs - offs));
            m_offs = offs;
        }

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
    struct s_array_rdonly {
        static_assert(!s_is_const<tp_type>::g_val);

        using t_elem = tp_type;

        const tp_type *buf = nullptr;
        t_len len = 0;

        constexpr const tp_type &operator[](const t_len index) const {
            ZF_ASSERT(index >= 0 && index < len);
            return buf[index];
        }
    };

    template <typename tp_type>
    struct s_array {
        static_assert(!s_is_const<tp_type>::g_val);

        using t_elem = tp_type;

        tp_type *buf;
        t_len len;

        constexpr tp_type &operator[](const t_len index) const {
            ZF_ASSERT(index >= 0 && index < len);
            return buf[index];
        }

        constexpr operator s_array_rdonly<tp_type>() const {
            return {buf, len};
        }
    };

    template <typename tp_type, t_len tp_len>
    struct s_static_array {
        static_assert(!s_is_const<tp_type>::g_val);

        static constexpr t_len g_len = tp_len;

        tp_type buf[tp_len];

        constexpr s_static_array() = default;

        template <t_len tp_other_len>
        constexpr s_static_array(const tp_type (&buf)[tp_other_len]) {
            static_assert(tp_other_len == tp_len);

            for (t_len i = 0; i < tp_other_len; i++) {
                this->buf[i] = buf[i];
            }
        }

        constexpr tp_type &operator[](const t_len index) {
            ZF_ASSERT(index >= 0 && index < tp_len);
            return buf[index];
        }

        constexpr const tp_type &operator[](const t_len index) const {
            ZF_ASSERT(index >= 0 && index < tp_len);
            return buf[index];
        }

        constexpr operator s_array<tp_type>() {
            return {buf, tp_len};
        }

        constexpr operator s_array_rdonly<tp_type>() const {
            return {buf, tp_len};
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

    template <c_nonstatic_array tp_type>
    constexpr t_len ArraySizeInBytes(const tp_type arr) {
        return ZF_SIZE_OF(typename tp_type::t_elem) * arr.len;
    }

    template <typename tp_type>
    [[nodiscard]] t_b8 AllocArray(const t_len len, s_mem_arena *const mem_arena, s_array<tp_type> *const o_arr) {
        ZF_ASSERT(len >= 0);
        ZF_ASSERT(IsUninitted(o_arr));

        o_arr->buf = PushToMemArena<tp_type>(mem_arena, len);
        o_arr->len = len;

        return o_arr->buf != nullptr || o_arr->len == 0;
    }

    template <c_nonstatic_array tp_type>
    [[nodiscard]] t_b8 AllocArrayClone(const tp_type arr_to_clone, s_mem_arena *const mem_arena, s_array<typename tp_type::t_elem> *const o_arr) {
        if (!AllocArray(o_arr, arr_to_clone.len, mem_arena)) {
            return false;
        }

        Copy(o_arr, arr_to_clone);

        return true;
    }

    template <typename tp_type>
    constexpr s_array<tp_type> Slice(const s_array<tp_type> arr, const t_len beg, const t_len end) {
        ZF_ASSERT(beg >= 0 && beg <= arr.len);
        ZF_ASSERT(end >= beg && end <= arr.len);

        return {arr.buf + beg, end - beg};
    }

    template <typename tp_type>
    constexpr s_array_rdonly<tp_type> Slice(const s_array_rdonly<tp_type> arr, const t_len beg, const t_len end) {
        ZF_ASSERT(beg >= 0 && beg <= arr.len);
        ZF_ASSERT(end >= beg && end <= arr.len);

        return {arr.buf + beg, end - beg};
    }

    template <c_nonstatic_mut_array tp_dest_type, c_nonstatic_array tp_src_type>
    constexpr void Copy(const tp_dest_type dest, const tp_src_type src) {
        static_assert(s_is_same<typename tp_dest_type::t_elem, typename tp_src_type::t_elem>::g_val);

        ZF_ASSERT(dest.len >= src.len);

        for (t_len i = 0; i < src.len; i++) {
            dest[i] = src[i];
        }
    }

    template <c_nonstatic_mut_array tp_dest_type, c_nonstatic_array tp_src_type>
    constexpr void CopyOrTruncate(const tp_dest_type dest, const tp_src_type src) {
        static_assert(s_is_same<typename tp_dest_type::t_elem, typename tp_src_type::t_elem>::g_val);

        const auto min = ZF_MIN(dest.len, src.len);

        for (t_len i = 0; i < min; i++) {
            dest[i] = src[i];
        }
    }

    template <c_nonstatic_mut_array tp_type>
    constexpr void Reverse(const tp_type arr) {
        for (t_len i = 0; i < arr.len / 2; i++) {
            Swap(arr[i], arr[arr.len - 1 - i]);
        }
    }

    template <c_nonstatic_array tp_type>
    constexpr t_b8 AreAllEqualTo(const tp_type arr, const typename tp_type::t_elem &val, const t_bin_comparator<typename tp_type::t_elem> comparator = DefaultBinComparator) {
        ZF_ASSERT(comparator);

        for (t_len i = 0; i < arr.len; i++) {
            if (!comparator(arr[i], val)) {
                return false;
            }
        }

        return true;
    }

    template <c_nonstatic_array tp_type>
    constexpr t_b8 AreAnyEqualTo(const tp_type arr, const typename tp_type::t_elem &val, const t_bin_comparator<typename tp_type::t_elem> comparator = DefaultBinComparator) {
        ZF_ASSERT(comparator);

        for (t_len i = 0; i < arr.len; i++) {
            if (comparator(arr[i], val)) {
                return true;
            }
        }

        return false;
    }

    template <c_nonstatic_mut_array tp_type>
    constexpr void SetAllTo(const tp_type arr, const typename tp_type::t_elem &val) {
        for (t_len i = 0; i < arr.len; i++) {
            arr[i] = val;
        }
    }

    template <typename tp_type>
    constexpr s_array<t_u8> ToBytes(tp_type &item) {
        return {reinterpret_cast<t_u8 *>(&item), ZF_SIZE_OF(item)};
    }

    template <typename tp_type>
    constexpr s_array_rdonly<t_u8> ToBytes(const tp_type &item) {
        return {reinterpret_cast<const t_u8 *>(&item), ZF_SIZE_OF(item)};
    }

    template <typename tp_type>
    constexpr s_array<t_u8> ToByteArray(const s_array<tp_type> arr) {
        return {reinterpret_cast<t_u8 *>(arr.buf), ArraySizeInBytes(arr)};
    }

    template <typename tp_type>
    constexpr s_array_rdonly<t_u8> ToByteArray(const s_array_rdonly<tp_type> arr) {
        return {reinterpret_cast<const t_u8 *>(arr.buf), ArraySizeInBytes(arr)};
    }
}
