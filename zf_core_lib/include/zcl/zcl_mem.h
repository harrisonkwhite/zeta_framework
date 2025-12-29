#pragma once

#include <new>
#include <zcl/zcl_basic.h>

namespace zf {
    constexpr t_i32 KilobytesToBytes(const t_i32 n) {
        return (1 << 10) * n;
    }

    constexpr t_i32 MegabytesToBytes(const t_i32 n) {
        return (1 << 20) * n;
    }

    constexpr t_i32 GigabytesToBytes(const t_i32 n) {
        return (1 << 30) * n;
    }

    constexpr t_i32 BitsToBytes(const t_i32 n) {
        return (n + 7) / 8;
    }

    constexpr t_i32 BytesToBits(const t_i32 x) {
        return x * 8;
    }

    // Is n a power of 2?
    constexpr t_b8 IsAlignmentValid(const t_i32 n) {
        return n > 0 && (n & (n - 1)) == 0;
    }

    // Take n up to the next multiple of the alignment.
    constexpr t_i32 AlignForward(const t_i32 n, const t_i32 alignment) {
        ZF_ASSERT(IsAlignmentValid(alignment));
        return (n + alignment - 1) & ~(alignment - 1);
    }

    template <typename tp_type>
    struct s_ptr {
        tp_type *raw = nullptr;

        constexpr s_ptr() = default;
        constexpr s_ptr(tp_type *const raw) : raw(raw) {}

        constexpr operator tp_type *() const {
            return raw;
        }

        constexpr operator t_b8() const {
            return raw != nullptr;
        }

        constexpr tp_type &operator*() const {
            ZF_REQUIRE(raw);
            return *raw;
        }

        constexpr tp_type *operator->() const {
            ZF_REQUIRE(raw);
            return raw;
        }

        constexpr tp_type &operator[](const t_i32 index) const {
            ZF_REQUIRE(raw);
            return raw[index];
        }

        constexpr t_b8 operator==(const s_ptr<tp_type> other) const {
            return raw == other.raw;
        }

        constexpr t_b8 operator!=(const s_ptr<tp_type> other) const {
            return raw != other.raw;
        }

        constexpr s_ptr<tp_type> operator+(const t_i32 offs) const {
            return {raw + offs};
        }

        constexpr s_ptr<tp_type> operator-(const t_i32 offs) const {
            return {raw - offs};
        }

        constexpr s_ptr<tp_type> &operator++() {
            raw++;
            return *this;
        }

        constexpr s_ptr<tp_type> &operator--() {
            raw--;
            return *this;
        }

        constexpr s_ptr<tp_type> &operator+=(const t_i32 offs) {
            raw += offs;
            return *this;
        }

        constexpr s_ptr<tp_type> &operator-=(const t_i32 offs) {
            raw -= offs;
            return *this;
        }

        constexpr operator s_ptr<const tp_type>() const
            requires(!s_is_const<tp_type>::g_val)
        {
            return {raw};
        }
    };

    template <>
    struct s_ptr<const void> {
    public:
        const void *raw = nullptr;

        constexpr s_ptr() = default;
        constexpr s_ptr(const void *const raw) : raw(raw) {}

        template <typename tp_type>
        constexpr s_ptr(const s_ptr<const tp_type> ptr) : raw(ptr.raw) {}

        constexpr operator const void *() const {
            return raw;
        }

        constexpr operator t_b8() const {
            return raw != nullptr;
        }

        constexpr t_b8 operator==(const s_ptr<const void> other) const {
            return raw == other.raw;
        }

        constexpr t_b8 operator!=(const s_ptr<const void> other) const {
            return raw != other.raw;
        }

        template <typename tp_type>
        explicit constexpr operator s_ptr<const tp_type>() const {
            return {static_cast<const tp_type *>(raw)};
        }
    };

    template <>
    struct s_ptr<void> {
        void *raw;

        constexpr s_ptr() = default;
        constexpr s_ptr(void *const raw) : raw(raw) {}

        template <typename tp_type>
        constexpr s_ptr(const s_ptr<tp_type> ptr) : raw(ptr.raw) {}

        constexpr operator void *() const {
            return raw;
        }

        constexpr operator t_b8() const {
            return raw != nullptr;
        }

        constexpr t_b8 operator==(const s_ptr<void> other) const {
            return raw == other.raw;
        }

        constexpr t_b8 operator!=(const s_ptr<void> other) const {
            return raw != other.raw;
        }

        constexpr operator s_ptr<const void>() const {
            return {raw};
        }

        template <typename tp_type>
        explicit constexpr operator s_ptr<tp_type>() const {
            return {static_cast<tp_type *>(raw)};
        }
    };

    struct s_mem_arena {
        // Frees all arena memory and completely resets the arena state. It is valid to call this even if no pushing was done.
        void Release();

        // Will lazily allocate memory as needed. Allocation failure is treated as fatal - you don't need to check for nullptr.
        s_ptr<void> Push(const t_i32 size, const t_i32 alignment);

        // DOES NOT FREE ARENA MEMORY. Simply rewinds the arena to the beginning of its memory and overwrites from there.
        void Rewind() {
            m_block_cur = m_blocks_head;
            m_block_cur_offs = 0;
        }

    private:
        struct s_block {
            s_ptr<void> buf = nullptr;
            t_i32 buf_size = 0;

            s_ptr<s_block> next = nullptr;
        };

        static s_ptr<s_block> CreateBlock(const t_i32 buf_size);

        s_ptr<s_block> m_blocks_head = nullptr;
        s_ptr<s_block> m_block_cur = nullptr;
        t_i32 m_block_cur_offs = 0;
        t_i32 m_block_min_buf_size = MegabytesToBytes(1);
    };

    template <typename tp_type, typename... tp_constructor_args>
    tp_type &Alloc(s_mem_arena &arena, tp_constructor_args &&...args) {
        static_assert(std::is_trivially_destructible_v<tp_type>);

        const auto ptr = static_cast<s_ptr<tp_type>>(arena.Push(ZF_SIZE_OF(tp_type), ZF_ALIGN_OF(tp_type)));
        new (ptr) tp_type(static_cast<tp_constructor_args &&>(args)...);
        return *ptr;
    }

    // ============================================================
    // @section: Arrays
    // ============================================================

    template <typename tp_type>
    struct s_array_rdonly {
        static_assert(!s_is_const<tp_type>::g_val);

        using t_elem = tp_type;

        constexpr s_array_rdonly() = default;

        constexpr s_array_rdonly(const s_ptr<const tp_type> ptr, const t_i32 len) : m_ptr(ptr), m_len(len) {
            ZF_ASSERT((ptr || len == 0) && len >= 0);
        }

        constexpr s_ptr<const tp_type> Ptr() const {
            return m_ptr;
        }

        constexpr t_i32 Len() const {
            return m_len;
        }

        constexpr t_i32 SizeInBytes() const {
            return ZF_SIZE_OF(tp_type) * m_len;
        }

        constexpr const tp_type &operator[](const t_i32 index) const {
            ZF_REQUIRE(index >= 0 && index < m_len);
            return m_ptr[index];
        }

        constexpr const tp_type &Last() const {
            return operator[](m_len - 1);
        }

        constexpr s_array_rdonly<tp_type> Slice(const t_i32 beg, const t_i32 end) const {
            ZF_ASSERT(beg >= 0 && beg <= m_len);
            ZF_ASSERT(end >= beg && end <= m_len);

            return {m_ptr + beg, end - beg};
        }

        constexpr s_array_rdonly<tp_type> SliceFrom(const t_i32 beg) const {
            ZF_ASSERT(beg >= 0 && beg <= m_len);
            return {m_ptr + beg, m_len - beg};
        }

        constexpr s_array_rdonly<t_u8> ToByteArray() const {
            return {reinterpret_cast<const t_u8 *>(m_ptr.raw), SizeInBytes()};
        }

    private:
        s_ptr<const tp_type> m_ptr = nullptr;
        t_i32 m_len = 0;
    };

    template <typename tp_type>
    struct s_array_mut {
        static_assert(!s_is_const<tp_type>::g_val);

        using t_elem = tp_type;

        constexpr s_array_mut() = default;

        constexpr s_array_mut(const s_ptr<tp_type> ptr, const t_i32 len) : m_ptr(ptr), m_len(len) {
            ZF_ASSERT((ptr || len == 0) && len >= 0);
        }

        constexpr s_ptr<tp_type> Ptr() const {
            return m_ptr;
        }

        constexpr t_i32 Len() const {
            return m_len;
        }

        constexpr t_i32 SizeInBytes() const {
            return ZF_SIZE_OF(tp_type) * m_len;
        }

        constexpr operator s_array_rdonly<tp_type>() const {
            return {m_ptr, m_len};
        }

        constexpr tp_type &operator[](const t_i32 index) const {
            ZF_REQUIRE(index >= 0 && index < m_len);
            return m_ptr[index];
        }

        constexpr tp_type &Last() const {
            return operator[](m_len - 1);
        }

        constexpr s_array_mut<tp_type> Slice(const t_i32 beg, const t_i32 end) const {
            ZF_ASSERT(beg >= 0 && beg <= m_len);
            ZF_ASSERT(end >= beg && end <= m_len);

            return {m_ptr + beg, end - beg};
        }

        constexpr s_array_mut<tp_type> SliceFrom(const t_i32 beg) const {
            ZF_ASSERT(beg >= 0 && beg <= m_len);
            return {m_ptr + beg, m_len - beg};
        }

        constexpr s_array_mut<t_u8> ToByteArray() const {
            return {reinterpret_cast<t_u8 *>(m_ptr.raw), SizeInBytes()};
        }

    private:
        s_ptr<tp_type> m_ptr = nullptr;
        t_i32 m_len = 0;
    };

    template <typename tp_type, t_i32 tp_len>
    struct s_static_array {
        static_assert(!s_is_const<tp_type>::g_val);

        using t_elem = tp_type;

        static constexpr t_i32 g_len = tp_len;

        tp_type raw[tp_len];

        constexpr s_static_array() = default;

        template <t_i32 tp_other_len>
        constexpr s_static_array(const tp_type (&other_raw)[tp_other_len]) {
            static_assert(tp_other_len == tp_len);

            for (t_i32 i = 0; i < tp_other_len; i++) {
                this->raw[i] = other_raw[i];
            }
        }

        constexpr operator s_array_mut<tp_type>() {
            return {raw, tp_len};
        }

        constexpr operator s_array_rdonly<tp_type>() const {
            return {raw, tp_len};
        }

        constexpr s_array_mut<tp_type> ToNonstatic() {
            return {raw, tp_len};
        }

        constexpr s_array_rdonly<tp_type> ToNonstatic() const {
            return {raw, tp_len};
        }

        constexpr tp_type &operator[](const t_i32 index) {
            ZF_REQUIRE(index >= 0 && index < tp_len);
            return raw[index];
        }

        constexpr const tp_type &operator[](const t_i32 index) const {
            ZF_REQUIRE(index >= 0 && index < tp_len);
            return raw[index];
        }

        constexpr tp_type &Last() {
            return operator[](tp_len - 1);
        }

        constexpr const tp_type &Last() const {
            return operator[](tp_len - 1);
        }
    };

    template <typename tp_type>
    struct s_is_nonstatic_mut_array {
        static constexpr t_b8 g_val = false;
    };

    template <typename tp_type>
    struct s_is_nonstatic_mut_array<s_array_mut<tp_type>> {
        static constexpr t_b8 g_val = true;
    };

    template <typename tp_type>
    struct s_is_nonstatic_rdonly_array {
        static constexpr t_b8 g_val = false;
    };

    template <typename tp_type>
    struct s_is_nonstatic_rdonly_array<s_array_rdonly<tp_type>> {
        static constexpr t_b8 g_val = true;
    };

    template <typename tp_type> concept co_array_nonstatic_mut = s_is_nonstatic_mut_array<tp_type>::g_val;
    template <typename tp_type> concept co_array_nonstatic_rdonly = s_is_nonstatic_rdonly_array<tp_type>::g_val;
    template <typename tp_type> concept co_array_nonstatic = co_array_nonstatic_mut<tp_type> || co_array_nonstatic_rdonly<tp_type>;

    template <co_array_nonstatic tp_arr_type>
    constexpr t_bin_comparator<tp_arr_type> g_array_bin_comparator =
        [](const tp_arr_type &a, const tp_arr_type &b) {
            if (a.Len() != b.Len()) {
                return false;
            }

            for (t_i32 i = 0; i < a.Len(); i++) {
                if (a[i] != b[i]) {
                    return false;
                }
            }

            return true;
        };

    template <typename tp_type>
    s_array_mut<tp_type> AllocArray(const t_i32 len, s_mem_arena &mem_arena) {
        static_assert(std::is_trivially_destructible_v<tp_type>);

        ZF_ASSERT(len >= 0);

        if (len == 0) {
            return {};
        }

        const auto ptr = static_cast<s_ptr<tp_type>>(mem_arena.Push(ZF_SIZE_OF(tp_type) * len, ZF_ALIGN_OF(tp_type)));

        for (t_i32 i = 0; i < len; i++) {
            new (&ptr[i]) tp_type();
        }

        return {ptr, len};
    }

    template <co_array_nonstatic tp_arr_type>
    auto AllocArrayClone(const tp_arr_type arr_to_clone, s_mem_arena &mem_arena) {
        const auto arr = AllocArray<typename tp_arr_type::t_elem>(arr_to_clone.Len(), mem_arena);
        Copy(arr, arr_to_clone);
        return arr;
    }

    template <co_array_nonstatic tp_arr_type>
    constexpr t_b8 DoAllEqual(const tp_arr_type arr, const typename tp_arr_type::t_elem &val, const t_bin_comparator<typename tp_arr_type::t_elem> comparator = DefaultBinComparator) {
        ZF_ASSERT(comparator);

        if (arr.Len() == 0) {
            return false;
        }

        for (t_i32 i = 0; i < arr.Len(); i++) {
            if (!comparator(arr[i], val)) {
                return false;
            }
        }

        return true;
    }

    template <co_array_nonstatic tp_arr_type>
    constexpr t_b8 DoAnyEqual(const tp_arr_type arr, const typename tp_arr_type::t_elem &val, const t_bin_comparator<typename tp_arr_type::t_elem> comparator = DefaultBinComparator) {
        ZF_ASSERT(comparator);

        for (t_i32 i = 0; i < arr.Len(); i++) {
            if (comparator(arr[i], val)) {
                return true;
            }
        }

        return false;
    }

    template <co_array_nonstatic_mut tp_arr_type>
    constexpr void SetAllTo(const tp_arr_type arr, const typename tp_arr_type::t_elem &val) {
        for (t_i32 i = 0; i < arr.Len(); i++) {
            arr[i] = val;
        }
    }

    // @todo: Maybe flip dest and src around?
    template <co_array_nonstatic_mut tp_dest_arr_type, co_array_nonstatic tp_src_arr_type>
    constexpr void Copy(const tp_dest_arr_type dest, const tp_src_arr_type src, const t_b8 allow_truncation = false) {
        static_assert(s_is_same<typename tp_dest_arr_type::t_elem, typename tp_src_arr_type::t_elem>::g_val);

        if (!allow_truncation) {
            ZF_ASSERT(dest.Len() >= src.Len());

            for (t_i32 i = 0; i < src.Len(); i++) {
                dest[i] = src[i];
            }
        } else {
            const auto min_len = ZF_MIN(src.Len(), dest.Len());

            for (t_i32 i = 0; i < min_len; i++) {
                dest[i] = src[i];
            }
        }
    }

    template <co_array_nonstatic tp_arr_a_type, co_array_nonstatic tp_arr_b_type>
    constexpr t_i32 Compare(const tp_arr_a_type a, const tp_arr_b_type b, const t_ord_comparator<typename tp_arr_a_type::t_elem> comparator = DefaultOrdComparator) {
        static_assert(s_is_same<typename tp_arr_a_type::t_elem, typename tp_arr_a_type::t_elem>::g_val);

        const auto min_len = ZF_MIN(a.Len(), b.Len());

        for (t_i32 i = 0; i < min_len; i++) {
            const t_i32 comp = comparator(a[i], b[i]);

            if (comp != 0) {
                return comp;
            }
        }

        return 0;
    }

    template <typename tp_type>
    constexpr s_array_mut<t_u8> ToBytes(tp_type &val) {
        return {reinterpret_cast<t_u8 *>(&val), ZF_SIZE_OF(val)};
    }

    template <typename tp_type>
    constexpr s_array_rdonly<t_u8> ToBytes(const tp_type &val) {
        return {reinterpret_cast<const t_u8 *>(&val), ZF_SIZE_OF(val)};
    }

    // ============================================================
    // @section: Bits
    // ============================================================

    // Creates a bitmask with only a single bit set.
    constexpr t_u8 BitmaskSingle(const t_i32 bit_index) {
        ZF_ASSERT(bit_index >= 0 && bit_index < 8);
        return static_cast<t_u8>(1 << bit_index);
    }

    // Creates a bitmask with only bits in the range [begin_bit_index, end_bit_index) set.
    constexpr t_u8 BitmaskRange(const t_i32 begin_bit_index, const t_i32 end_bit_index = 8) {
        ZF_ASSERT(begin_bit_index >= 0 && begin_bit_index < 8);
        ZF_ASSERT(end_bit_index >= begin_bit_index && end_bit_index <= 8);

        if (end_bit_index - begin_bit_index == 8) {
            return 0xFF;
        }

        const auto bits_at_bottom = static_cast<t_u8>((1 << (end_bit_index - begin_bit_index)) - 1);
        return static_cast<t_u8>(bits_at_bottom << begin_bit_index);
    }

    struct s_bit_vec_rdonly {
        constexpr s_bit_vec_rdonly() = default;

        constexpr s_bit_vec_rdonly(const s_array_rdonly<t_u8> bytes, const t_i32 bit_cnt) : m_bytes(bytes), m_bit_cnt(bit_cnt) {
            ZF_ASSERT(BitsToBytes(bit_cnt) == bytes.Len());
        }

        constexpr s_bit_vec_rdonly(const s_array_rdonly<t_u8> bytes) : m_bytes(bytes), m_bit_cnt(BytesToBits(bytes.Len())) {}

        constexpr s_array_rdonly<t_u8> Bytes() const { return m_bytes; }
        constexpr t_i32 BitCount() const { return m_bit_cnt; }

        constexpr t_i32 LastByteBitCount() const {
            return ((m_bit_cnt - 1) % 8) + 1;
        }

        // Gives a mask of the last byte in which only excess bits are unset.
        constexpr t_u8 LastByteMask() const {
            return BitmaskRange(0, LastByteBitCount());
        }

    private:
        s_array_rdonly<t_u8> m_bytes;
        t_i32 m_bit_cnt = 0;
    };

    struct s_bit_vec_mut {
        constexpr s_bit_vec_mut() = default;

        constexpr s_bit_vec_mut(const s_array_mut<t_u8> bytes, const t_i32 bit_cnt) : m_bytes(bytes), m_bit_cnt(bit_cnt) {
            ZF_ASSERT(BitsToBytes(bit_cnt) == bytes.Len());
        }

        constexpr s_bit_vec_mut(const s_array_mut<t_u8> bytes) : m_bytes(bytes), m_bit_cnt(BytesToBits(bytes.Len())) {}

        constexpr s_array_mut<t_u8> Bytes() const { return m_bytes; }
        constexpr t_i32 BitCount() const { return m_bit_cnt; }

        constexpr t_i32 LastByteBitCount() const {
            return ((m_bit_cnt - 1) % 8) + 1;
        }

        // Gives a mask of the last byte in which only excess bits are unset.
        constexpr t_u8 LastByteMask() const {
            return BitmaskRange(0, LastByteBitCount());
        }

        constexpr operator s_bit_vec_rdonly() const {
            return {m_bytes, m_bit_cnt};
        }

    private:
        s_array_mut<t_u8> m_bytes;
        t_i32 m_bit_cnt = 0;
    };

    template <t_i32 tp_bit_cnt>
    struct s_static_bit_vec {
        static constexpr t_i32 g_bit_cnt = tp_bit_cnt;

        s_static_array<t_u8, BitsToBytes(tp_bit_cnt)> bytes;

        constexpr operator s_bit_vec_mut() {
            return {bytes, tp_bit_cnt};
        }

        constexpr operator s_bit_vec_rdonly() const {
            return {bytes, tp_bit_cnt};
        }
    };

    inline s_bit_vec_mut CreateBitVec(const t_i32 bit_cnt, s_mem_arena &mem_arena) {
        ZF_ASSERT(bit_cnt >= 0);
        return {AllocArray<t_u8>(BitsToBytes(bit_cnt), mem_arena), bit_cnt};
    }

    constexpr t_b8 IsBitSet(const s_bit_vec_rdonly bv, const t_i32 index) {
        ZF_ASSERT(index >= 0 && index < bv.BitCount());
        return bv.Bytes()[index / 8] & BitmaskSingle(index % 8);
    }

    constexpr void SetBit(const s_bit_vec_mut bv, const t_i32 index) {
        ZF_ASSERT(index >= 0 && index < bv.BitCount());
        bv.Bytes()[index / 8] |= BitmaskSingle(index % 8);
    }

    constexpr void UnsetBit(const s_bit_vec_mut bv, const t_i32 index) {
        ZF_ASSERT(index >= 0 && index < bv.BitCount());
        bv.Bytes()[index / 8] &= ~BitmaskSingle(index % 8);
    }

    constexpr t_b8 IsAnyBitSet(const s_bit_vec_rdonly bv) {
        if (bv.BitCount() == 0) {
            return false;
        }

        const auto first_bytes = bv.Bytes().Slice(0, bv.Bytes().Len() - 1);

        if (!DoAllEqual(first_bytes, 0)) {
            return true;
        }

        return (bv.Bytes().Last() & bv.LastByteMask()) != 0;
    }

    constexpr t_b8 AreAllBitsSet(const s_bit_vec_rdonly bv) {
        if (bv.BitCount() == 0) {
            return false;
        }

        const auto first_bytes = bv.Bytes().Slice(0, bv.Bytes().Len() - 1);

        if (!DoAllEqual(first_bytes, 0xFF)) {
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

    constexpr void SetAllBits(const s_bit_vec_mut bv) {
        if (bv.BitCount() == 0) {
            return;
        }

        const auto first_bytes = bv.Bytes().Slice(0, bv.Bytes().Len() - 1);
        SetAllTo(first_bytes, 0xFF);

        bv.Bytes().Last() |= bv.LastByteMask();
    }

    constexpr void UnsetAllBits(const s_bit_vec_mut bv) {
        if (bv.BitCount() == 0) {
            return;
        }

        const auto first_bytes = bv.Bytes().Slice(0, bv.Bytes().Len() - 1);
        SetAllTo(first_bytes, 0);

        bv.Bytes().Last() &= ~bv.LastByteMask();
    }

    // Sets all bits in the range [begin_bit_index, end_bit_index).
    constexpr void SetBitsInRange(const s_bit_vec_mut bv, const t_i32 begin_bit_index, const t_i32 end_bit_index) {
        ZF_ASSERT(begin_bit_index >= 0 && begin_bit_index < bv.BitCount());
        ZF_ASSERT(end_bit_index >= begin_bit_index && end_bit_index <= bv.BitCount());

        const t_i32 begin_elem_index = begin_bit_index / 8;
        const t_i32 end_elem_index = BitsToBytes(end_bit_index);

        for (t_i32 i = begin_elem_index; i < end_elem_index; i++) {
            const t_i32 bit_offs = i * 8;
            const t_i32 begin_bit_index_rel = begin_bit_index - bit_offs;
            const t_i32 end_bit_index_rel = end_bit_index - bit_offs;

            const t_i32 set_range_begin = ZF_MAX(begin_bit_index_rel, 0);
            const t_i32 set_range_end = ZF_MIN(end_bit_index_rel, 8);

            bv.Bytes()[i] |= BitmaskRange(set_range_begin, set_range_end);
        }
    }

    enum e_bitwise_mask_op : t_i32 {
        ek_bitwise_mask_op_and,
        ek_bitwise_mask_op_or,
        ek_bitwise_mask_op_xor,
        ek_bitwise_mask_op_andnot
    };

    // @todo: Swap dest and src.
    constexpr void ApplyMaskToBits(const s_bit_vec_mut dest, const s_bit_vec_rdonly src, const e_bitwise_mask_op op) {
        ZF_ASSERT(dest.BitCount() == src.BitCount());

        if (dest.BitCount() == 0) {
            return;
        }

        switch (op) {
        case ek_bitwise_mask_op_and:
            for (t_i32 i = 0; i < dest.Bytes().Len(); i++) {
                dest.Bytes()[i] &= src.Bytes()[i];
            }

            break;

        case ek_bitwise_mask_op_or:
            for (t_i32 i = 0; i < dest.Bytes().Len(); i++) {
                dest.Bytes()[i] |= src.Bytes()[i];
            }

            break;

        case ek_bitwise_mask_op_xor:
            for (t_i32 i = 0; i < dest.Bytes().Len(); i++) {
                dest.Bytes()[i] ^= src.Bytes()[i];
            }

            break;

        case ek_bitwise_mask_op_andnot:
            for (t_i32 i = 0; i < dest.Bytes().Len(); i++) {
                dest.Bytes()[i] &= ~src.Bytes()[i];
            }

            break;
        }

        dest.Bytes().Last() &= dest.LastByteMask();
    }

    // Shifts left only by 1. Returns the discarded bit as 0 or 1.
    constexpr t_u8 ShiftBitsLeft(const s_bit_vec_mut bv) {
        if (bv.BitCount() == 0) {
            return 0;
        }

        t_u8 discard = 0;

        for (t_i32 i = 0; i < bv.Bytes().Len(); i++) {
            const t_i32 bits_in_byte = i == bv.Bytes().Len() - 1 ? bv.LastByteBitCount() : 8;
            const t_u8 discard_last = discard;
            discard = (bv.Bytes()[i] & BitmaskSingle(bits_in_byte - 1)) >> (bits_in_byte - 1);
            bv.Bytes()[i] <<= 1;
            bv.Bytes()[i] |= discard_last;
        }

        bv.Bytes().Last() &= bv.LastByteMask();

        return discard;
    }

    constexpr void ShiftBitsLeftBy(const s_bit_vec_mut bv, const t_i32 amount) {
        ZF_ASSERT(amount >= 0);

        // @speed: :(

        for (t_i32 i = 0; i < amount; i++) {
            ShiftBitsLeft(bv);
        }
    }

    constexpr void RotBitsLeftBy(const s_bit_vec_mut bv, const t_i32 amount) {
        ZF_ASSERT(amount >= 0);

        if (bv.BitCount() == 0) {
            return;
        }

        // @speed: :(

        for (t_i32 i = 0; i < amount; i++) {
            const auto discard = ShiftBitsLeft(bv);

            if (discard) {
                SetBit(bv, 0);
            } else {
                UnsetBit(bv, 0);
            }
        }
    }

    // Shifts right only by 1. Returns the carry bit.
    constexpr t_u8 ShiftBitsRight(const s_bit_vec_mut bv) {
        if (bv.BitCount() == 0) {
            return 0;
        }

        bv.Bytes().Last() &= bv.LastByteMask(); // Drop any excess bits so we don't accidentally shift a 1 in.

        t_u8 discard = 0;

        for (t_i32 i = bv.Bytes().Len() - 1; i >= 0; i--) {
            const t_i32 bits_in_byte = i == bv.Bytes().Len() - 1 ? bv.LastByteBitCount() : 8;
            const t_u8 discard_last = discard;
            discard = bv.Bytes()[i] & BitmaskSingle(0);
            bv.Bytes()[i] >>= 1;

            if (discard_last) {
                bv.Bytes()[i] |= BitmaskSingle(bits_in_byte - 1);
            }
        }

        return discard;
    }

    constexpr void ShiftBitsRightBy(const s_bit_vec_mut bv, const t_i32 amount) {
        ZF_ASSERT(amount >= 0);

        // @speed: :(

        for (t_i32 i = 0; i < amount; i++) {
            ShiftBitsRight(bv);
        }
    }

    constexpr void RotBitsRightBy(const s_bit_vec_mut bv, const t_i32 amount) {
        ZF_ASSERT(amount >= 0);

        if (bv.BitCount() == 0) {
            return;
        }

        // @speed: :(

        for (t_i32 i = 0; i < amount; i++) {
            const auto discard = ShiftBitsRight(bv);

            if (discard) {
                SetBit(bv, bv.BitCount() - 1);
            } else {
                UnsetBit(bv, bv.BitCount() - 1);
            }
        }
    }

    t_i32 IndexOfFirstSetBit(const s_bit_vec_rdonly bv, const t_i32 from = 0);   // Returns -1 if all bits are unset.
    t_i32 IndexOfFirstUnsetBit(const s_bit_vec_rdonly bv, const t_i32 from = 0); // Returns -1 if all bits are set.

    t_i32 CountSetBits(const s_bit_vec_rdonly bv);

    inline t_i32 CountUnsetBits(const s_bit_vec_rdonly bv) {
        return bv.BitCount() - CountSetBits(bv);
    }

    // pos is the walker state, initialise it to the bit index you want to start from.
    // o_index is assigned the index of the set bit to process.
    // Returns false iff the walk is complete.
    // You can use the ZF_FOR_EACH_SET_BIT macro for more brevity if you like.
    inline t_b8 WalkSetBits(const s_bit_vec_rdonly bv, t_i32 &pos, t_i32 &o_index) {
        ZF_ASSERT(pos >= 0 && pos < bv.BitCount());
        o_index = IndexOfFirstSetBit(bv, pos);
        pos = o_index + 1;
        return o_index != -1;
    }

    // pos is the walker state, initialise it to the bit index you want to start from.
    // o_index is assigned the index of the unset bit to process.
    // Returns false iff the walk is complete.
    // You can use the ZF_FOR_EACH_UNSET_BIT macro for more brevity if you like.
    inline t_b8 WalkUnsetBits(const s_bit_vec_rdonly bv, t_i32 &pos, t_i32 &o_index) {
        ZF_ASSERT(pos >= 0 && pos < bv.BitCount());
        o_index = IndexOfFirstUnsetBit(bv, pos);
        pos = o_index + 1;
        return o_index != -1;
    }

#define ZF_WALK_SET_BITS(bv, index) for (t_i32 ZF_CONCAT(walk_pos_l, __LINE__) = 0, index; WalkSetBits(bv, ZF_CONCAT(walk_pos_l, __LINE__), index);)
#define ZF_WALK_UNSET_BITS(bv, index) for (t_i32 ZF_CONCAT(walk_pos_l, __LINE__) = 0, index; WalkUnsetBits(bv, ZF_CONCAT(walk_pos_l, __LINE__), index);)
}
