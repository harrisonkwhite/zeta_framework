#pragma once

#include <cstring>
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

    constexpr t_i32 BytesToBits(const t_i32 n) {
        return n * 8;
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

    inline void Clear(void *const buf, const t_i32 buf_size, const t_u8 val) {
        ZF_ASSERT(buf_size >= 0);
        memset(buf, val, static_cast<size_t>(buf_size));
    }

    template <typename tp_type>
    inline void ClearItem(tp_type *const item, const t_u8 val) {
        Clear(item, ZF_SIZE_OF(*item), val);
    }

    constexpr t_u8 g_poison_uninitted = 0xCD;
    constexpr t_u8 g_poison_freed = 0xDD;

#ifdef ZF_DEBUG
    inline void PoisonUnitted(void *const buf, const t_i32 buf_size) {
        Clear(buf, buf_size, g_poison_uninitted);
    }

    template <typename tp_type>
    inline void PoisonUnittedItem(tp_type *const item) {
        ClearItem(item, g_poison_uninitted);
    }

    inline void PoisonFreed(void *const buf, const t_i32 buf_size) {
        Clear(buf, buf_size, g_poison_freed);
    }

    template <typename tp_type>
    inline void PoisonFreedItem(tp_type *const item) {
        ClearItem(item, g_poison_freed);
    }

    #define ZF_DEFINE_UNINITTED(type, name) \
        type name;                          \
        PoisonUnittedItem(&name)
#else
    inline void PoisonUnitted(void *const buf, const t_i32 buf_size) {}
    template <typename tp_type> inline void PoisonUnittedItem(tp_type *const item) {}
    inline void PoisonFreed(void *const buf, const t_i32 buf_size) {}
    template <typename tp_type> inline void PoisonFreedItem(tp_type *const item) {}

    #define ZF_DEFINE_UNINITTED(type, name) static_cast<void>(0)
#endif


    // ============================================================
    // @section: Arenas

    class c_arena {
    public:
        // Frees all arena memory and resets object state. It is valid to call this even if no pushing was done.
        void Release();

        // Will lazily allocate memory as needed. Allocation failure is treated as fatal and causes an abort - you don't need to check for nullptr.
        void *Push(const t_i32 size, const t_i32 alignment);

        // DOES NOT FREE ANY ARENA MEMORY. Simply rewinds the arena to the beginning of its allocated memory (if any) to overwrite from there.
        void Rewind() {
            m_block_cur = m_blocks_head;
            m_block_cur_offs = 0;
        }

    private:
        static constexpr t_i32 g_block_min_size = MegabytesToBytes(1);

        struct s_block {
            void *buf;
            t_i32 buf_size;

            s_block *next;
        };

        s_block *m_blocks_head = nullptr;
        s_block *m_block_cur = nullptr;
        t_i32 m_block_cur_offs = 0;

        static s_block *CreateBlock(const t_i32 buf_size);
    };

    // ============================================================


    template <typename tp_type>
    tp_type *Alloc(c_arena *const arena) {
        return static_cast<tp_type *>(arena->Push(ZF_SIZE_OF(tp_type), ZF_ALIGN_OF(tp_type)));
    }


    // ============================================================
    // @section: Arrays

    template <typename tp_type>
    class c_array_rdonly {
        static_assert(!s_is_const<tp_type>::g_val);

    public:
        using t_elem = tp_type;

        constexpr c_array_rdonly() = default;

        constexpr c_array_rdonly(const tp_type *const raw, const t_i32 len) : m_raw(raw), m_len(len) {
            ZF_ASSERT((raw || len == 0) && len >= 0);
        }

        constexpr const tp_type *Raw() const { return m_raw; }
        constexpr t_i32 Len() const { return m_len; }

        constexpr t_i32 SizeInBytes() const {
            return ZF_SIZE_OF(tp_type) * m_len;
        }

        constexpr const tp_type &operator[](const t_i32 index) const {
            ZF_REQUIRE(index >= 0 && index < m_len);
            return m_raw[index];
        }

        constexpr c_array_rdonly<tp_type> Slice(const t_i32 beg, const t_i32 end) const {
            ZF_ASSERT(beg >= 0 && beg <= m_len);
            ZF_ASSERT(end >= beg && end <= m_len);

            return {m_raw + beg, end - beg};
        }

        constexpr c_array_rdonly<tp_type> SliceFrom(const t_i32 beg) const {
            ZF_ASSERT(beg >= 0 && beg <= m_len);
            return {m_raw + beg, m_len - beg};
        }

        constexpr c_array_rdonly<t_u8> AsByteArray() const {
            return {reinterpret_cast<const t_u8 *>(m_raw), SizeInBytes()};
        }

    private:
        const tp_type *m_raw = nullptr;
        t_i32 m_len = 0;
    };

    template <typename tp_type>
    class c_array_mut {
        static_assert(!s_is_const<tp_type>::g_val);

    public:
        using t_elem = tp_type;

        constexpr c_array_mut() = default;

        constexpr c_array_mut(tp_type *const raw, const t_i32 len) : m_raw(raw), m_len(len) {
            ZF_ASSERT((raw || len == 0) && len >= 0);
        }

        constexpr tp_type *Raw() const { return m_raw; }
        constexpr t_i32 Len() const { return m_len; }

        constexpr t_i32 SizeInBytes() const {
            return ZF_SIZE_OF(tp_type) * m_len;
        }

        constexpr operator c_array_rdonly<tp_type>() const {
            return {m_raw, m_len};
        }

        constexpr tp_type &operator[](const t_i32 index) const {
            ZF_REQUIRE(index >= 0 && index < m_len);
            return m_raw[index];
        }

        constexpr c_array_mut<tp_type> Slice(const t_i32 beg, const t_i32 end) const {
            ZF_ASSERT(beg >= 0 && beg <= m_len);
            ZF_ASSERT(end >= beg && end <= m_len);

            return {m_raw + beg, end - beg};
        }

        constexpr c_array_mut<tp_type> SliceFrom(const t_i32 beg) const {
            ZF_ASSERT(beg >= 0 && beg <= m_len);
            return {m_raw + beg, m_len - beg};
        }

        constexpr c_array_mut<t_u8> AsByteArray() const {
            return {reinterpret_cast<t_u8 *>(m_raw), SizeInBytes()};
        }

    private:
        tp_type *m_raw = nullptr;
        t_i32 m_len = 0;
    };

    template <typename tp_type, t_i32 tp_len>
    struct s_static_array {
        static_assert(!s_is_const<tp_type>::g_val);

        using t_elem = tp_type;

        static constexpr t_i32 g_len = tp_len;

        tp_type raw[tp_len] = {};

        constexpr operator c_array_mut<tp_type>() {
            return {raw, tp_len};
        }

        constexpr operator c_array_rdonly<tp_type>() const {
            return {raw, tp_len};
        }

        constexpr c_array_mut<tp_type> AsNonstatic() {
            return {raw, tp_len};
        }

        constexpr c_array_rdonly<tp_type> AsNonstatic() const {
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
    };

    template <typename tp_type>
    struct s_is_nonstatic_mut_array {
        static constexpr t_b8 g_val = false;
    };

    template <typename tp_type>
    struct s_is_nonstatic_mut_array<c_array_mut<tp_type>> {
        static constexpr t_b8 g_val = true;
    };

    template <typename tp_type>
    struct s_is_nonstatic_rdonly_array {
        static constexpr t_b8 g_val = false;
    };

    template <typename tp_type>
    struct s_is_nonstatic_rdonly_array<c_array_rdonly<tp_type>> {
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

    template <co_array_nonstatic tp_arr_type>
    t_b8 IsArrayValid(const tp_arr_type arr) {
        return arr.Len() >= 0;
    }

    template <typename tp_type>
    c_array_mut<tp_type> AllocArray(const t_i32 len, c_arena *const arena) {
        ZF_ASSERT(len >= 0);

        if (len == 0) {
            return {};
        }

        return {static_cast<tp_type *>(arena->Push(ZF_SIZE_OF(tp_type) * len, ZF_ALIGN_OF(tp_type))), len};
    }

    template <co_array_nonstatic tp_arr_type>
    auto AllocArrayClone(const tp_arr_type arr_to_clone, c_arena *const arena) {
        const auto arr = AllocArray<typename tp_arr_type::t_elem>(arr_to_clone.Len(), arena);
        Copy(arr, arr_to_clone);
        return arr;
    }

    template <co_array_nonstatic tp_arr_type>
    constexpr t_b8 DoAllEqual(const tp_arr_type arr, const typename tp_arr_type::t_elem &val, const t_bin_comparator<typename tp_arr_type::t_elem> comparator = DefaultBinComparator) {
        ZF_ASSERT(arr.Len() >= 0);

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
        ZF_ASSERT(arr.Len() >= 0);

        for (t_i32 i = 0; i < arr.Len(); i++) {
            if (comparator(arr[i], val)) {
                return true;
            }
        }

        return false;
    }

    template <co_array_nonstatic_mut tp_arr_type>
    constexpr void SetAllTo(const tp_arr_type arr, const typename tp_arr_type::t_elem &val) {
        ZF_ASSERT(arr.Len() >= 0);

        for (t_i32 i = 0; i < arr.Len(); i++) {
            arr[i] = val;
        }
    }

    // @todo: Maybe flip dest and src around?
    template <co_array_nonstatic_mut tp_dest_arr_type, co_array_nonstatic tp_src_arr_type>
    constexpr void Copy(const tp_dest_arr_type dest, const tp_src_arr_type src, const t_b8 allow_truncation = false) {
        static_assert(s_is_same<typename tp_dest_arr_type::t_elem, typename tp_src_arr_type::t_elem>::g_val);

        ZF_ASSERT(IsArrayValid(dest));
        ZF_ASSERT(IsArrayValid(src));

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

        ZF_ASSERT(IsArrayValid(a));
        ZF_ASSERT(IsArrayValid(b));

        const auto min_len = ZF_MIN(a.Len(), b.Len());

        for (t_i32 i = 0; i < min_len; i++) {
            const t_i32 comp = comparator(a[i], b[i]);

            if (comp != 0) {
                return comp;
            }
        }

        return 0;
    }

    // ============================================================


    template <typename tp_type>
    constexpr c_array_mut<t_u8> ToBytes(tp_type &val) {
        return {reinterpret_cast<t_u8 *>(&val), ZF_SIZE_OF(val)};
    }

    template <typename tp_type>
    constexpr c_array_rdonly<t_u8> ToBytes(const tp_type &val) {
        return {reinterpret_cast<const t_u8 *>(&val), ZF_SIZE_OF(val)};
    }


    // ============================================================
    // @section: Bits

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
        c_array_rdonly<t_u8> bytes;
        t_i32 bit_cnt;

        constexpr s_bit_vec_rdonly() = default;
        constexpr s_bit_vec_rdonly(const c_array_rdonly<t_u8> bytes, const t_i32 bit_cnt) : bytes(bytes), bit_cnt(bit_cnt) {}
        constexpr s_bit_vec_rdonly(const c_array_rdonly<t_u8> bytes) : bytes(bytes), bit_cnt(BytesToBits(bytes.Len())) {}

        constexpr t_i32 LastByteBitCount() const {
            return ((bit_cnt - 1) % 8) + 1;
        }

        // Gives a mask of the last byte in which only excess bits are unset.
        constexpr t_u8 LastByteMask() const {
            return BitmaskRange(0, LastByteBitCount());
        }
    };

    struct s_bit_vec_mut {
        constexpr s_bit_vec_mut() = default;
        constexpr s_bit_vec_mut(const c_array_mut<t_u8> bytes, const t_i32 bit_cnt) : bytes(bytes), bit_cnt(bit_cnt) {}
        constexpr s_bit_vec_mut(const c_array_mut<t_u8> bytes) : bytes(bytes), bit_cnt(BytesToBits(bytes.Len())) {}

        constexpr t_i32 LastByteBitCount() const {
            return ((bit_cnt - 1) % 8) + 1;
        }

        // Gives a mask of the last byte in which only excess bits are unset.
        constexpr t_u8 LastByteMask() const {
            return BitmaskRange(0, LastByteBitCount());
        }

        constexpr operator s_bit_vec_rdonly() const {
            return {bytes, bit_cnt};
        }

        c_array_mut<t_u8> bytes = {};
        t_i32 bit_cnt = 0;
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

    inline t_b8 IsValid(const s_bit_vec_rdonly bv) {
        return IsArrayValid(bv.bytes) && BitsToBytes(bv.bit_cnt) == bv.bytes.Len();
    }

    inline s_bit_vec_mut CreateBitVec(const t_i32 bit_cnt, c_arena *const arena) {
        ZF_ASSERT(bit_cnt >= 0);
        return {AllocArray<t_u8>(BitsToBytes(bit_cnt), arena), bit_cnt};
    }

    constexpr t_b8 IsBitSet(const s_bit_vec_rdonly bv, const t_i32 index) {
        ZF_ASSERT(index >= 0 && index < bv.bit_cnt);
        return bv.bytes[index / 8] & BitmaskSingle(index % 8);
    }

    constexpr void SetBit(const s_bit_vec_mut bv, const t_i32 index) {
        ZF_ASSERT(index >= 0 && index < bv.bit_cnt);
        bv.bytes[index / 8] |= BitmaskSingle(index % 8);
    }

    constexpr void UnsetBit(const s_bit_vec_mut bv, const t_i32 index) {
        ZF_ASSERT(index >= 0 && index < bv.bit_cnt);
        bv.bytes[index / 8] &= ~BitmaskSingle(index % 8);
    }

    constexpr t_b8 IsAnyBitSet(const s_bit_vec_rdonly bv) {
        ZF_ASSERT(BitsToBytes(bv.bit_cnt) == bv.bytes.Len());

        if (bv.bit_cnt == 0) {
            return false;
        }

        const auto first_bytes = bv.bytes.Slice(0, bv.bytes.Len() - 1);

        if (!DoAllEqual(first_bytes, 0)) {
            return true;
        }

        return (bv.bytes[bv.bytes.Len() - 1] & bv.LastByteMask()) != 0;
    }

    constexpr t_b8 AreAllBitsSet(const s_bit_vec_rdonly bv) {
        ZF_ASSERT(BitsToBytes(bv.bit_cnt) == bv.bytes.Len());

        if (bv.bit_cnt == 0) {
            return false;
        }

        const auto first_bytes = bv.bytes.Slice(0, bv.bytes.Len() - 1);

        if (!DoAllEqual(first_bytes, 0xFF)) {
            return false;
        }

        const auto last_byte_mask = bv.LastByteMask();
        return (bv.bytes[bv.bytes.Len() - 1] & last_byte_mask) == last_byte_mask;
    }

    constexpr t_b8 AreAllBitsUnset(const s_bit_vec_rdonly bv) {
        if (bv.bit_cnt == 0) {
            return false;
        }

        return !IsAnyBitSet(bv);
    }

    constexpr t_b8 IsAnyBitUnset(const s_bit_vec_rdonly bv) {
        if (bv.bit_cnt == 0) {
            return false;
        }

        return !AreAllBitsSet(bv);
    }

    constexpr void SetAllBits(const s_bit_vec_mut bv) {
        ZF_ASSERT(BitsToBytes(bv.bit_cnt) == bv.bytes.Len());

        if (bv.bit_cnt == 0) {
            return;
        }

        const auto first_bytes = bv.bytes.Slice(0, bv.bytes.Len() - 1);
        SetAllTo(first_bytes, 0xFF);

        bv.bytes[bv.bytes.Len() - 1] |= bv.LastByteMask();
    }

    constexpr void UnsetAllBits(const s_bit_vec_mut bv) {
        ZF_ASSERT(BitsToBytes(bv.bit_cnt) == bv.bytes.Len());

        if (bv.bit_cnt == 0) {
            return;
        }

        const auto first_bytes = bv.bytes.Slice(0, bv.bytes.Len() - 1);
        SetAllTo(first_bytes, 0);

        bv.bytes[bv.bytes.Len() - 1] &= ~bv.LastByteMask();
    }

    // Sets all bits in the range [begin_bit_index, end_bit_index).
    constexpr void SetBitsInRange(const s_bit_vec_mut bv, const t_i32 begin_bit_index, const t_i32 end_bit_index) {
        ZF_ASSERT(BitsToBytes(bv.bit_cnt) == bv.bytes.Len());
        ZF_ASSERT(begin_bit_index >= 0 && begin_bit_index < bv.bit_cnt);
        ZF_ASSERT(end_bit_index >= begin_bit_index && end_bit_index <= bv.bit_cnt);

        const t_i32 begin_elem_index = begin_bit_index / 8;
        const t_i32 end_elem_index = BitsToBytes(end_bit_index);

        for (t_i32 i = begin_elem_index; i < end_elem_index; i++) {
            const t_i32 bit_offs = i * 8;
            const t_i32 begin_bit_index_rel = begin_bit_index - bit_offs;
            const t_i32 end_bit_index_rel = end_bit_index - bit_offs;

            const t_i32 set_range_begin = ZF_MAX(begin_bit_index_rel, 0);
            const t_i32 set_range_end = ZF_MIN(end_bit_index_rel, 8);

            bv.bytes[i] |= BitmaskRange(set_range_begin, set_range_end);
        }
    }

    enum e_bitwise_mask_op : t_i32 {
        ek_bitwise_mask_op_and,
        ek_bitwise_mask_op_or,
        ek_bitwise_mask_op_xor,
        ek_bitwise_mask_op_andnot
    };

    // @todo: Swap dest and src.
    constexpr void ApplyMaskToBits(const s_bit_vec_mut targ, const s_bit_vec_rdonly mask, const e_bitwise_mask_op op) {
        ZF_ASSERT(BitsToBytes(targ.bit_cnt) == targ.bytes.Len());
        ZF_ASSERT(BitsToBytes(mask.bit_cnt) == mask.bytes.Len());
        ZF_ASSERT(targ.bit_cnt == mask.bit_cnt);

        if (targ.bit_cnt == 0) {
            return;
        }

        switch (op) {
        case ek_bitwise_mask_op_and:
            for (t_i32 i = 0; i < targ.bytes.Len(); i++) {
                targ.bytes[i] &= mask.bytes[i];
            }

            break;

        case ek_bitwise_mask_op_or:
            for (t_i32 i = 0; i < targ.bytes.Len(); i++) {
                targ.bytes[i] |= mask.bytes[i];
            }

            break;

        case ek_bitwise_mask_op_xor:
            for (t_i32 i = 0; i < targ.bytes.Len(); i++) {
                targ.bytes[i] ^= mask.bytes[i];
            }

            break;

        case ek_bitwise_mask_op_andnot:
            for (t_i32 i = 0; i < targ.bytes.Len(); i++) {
                targ.bytes[i] &= ~mask.bytes[i];
            }

            break;
        }

        targ.bytes[targ.bytes.Len() - 1] &= targ.LastByteMask();
    }

    // Shifts left only by 1. Returns the discarded bit as 0 or 1.
    constexpr t_u8 ShiftBitsLeft(const s_bit_vec_mut bv) {
        ZF_ASSERT(BitsToBytes(bv.bit_cnt) == bv.bytes.Len());

        if (bv.bit_cnt == 0) {
            return 0;
        }

        t_u8 discard = 0;

        for (t_i32 i = 0; i < bv.bytes.Len(); i++) {
            const t_i32 bits_in_byte = i == bv.bytes.Len() - 1 ? bv.LastByteBitCount() : 8;
            const t_u8 discard_last = discard;
            discard = (bv.bytes[i] & BitmaskSingle(bits_in_byte - 1)) >> (bits_in_byte - 1);
            bv.bytes[i] <<= 1;
            bv.bytes[i] |= discard_last;
        }

        bv.bytes[bv.bytes.Len() - 1] &= bv.LastByteMask();

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

        if (bv.bit_cnt == 0) {
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
        ZF_ASSERT(BitsToBytes(bv.bit_cnt) == bv.bytes.Len());

        if (bv.bit_cnt == 0) {
            return 0;
        }

        bv.bytes[bv.bytes.Len() - 1] &= bv.LastByteMask(); // Drop any excess bits so we don't accidentally shift a 1 in.

        t_u8 discard = 0;

        for (t_i32 i = bv.bytes.Len() - 1; i >= 0; i--) {
            const t_i32 bits_in_byte = i == bv.bytes.Len() - 1 ? bv.LastByteBitCount() : 8;
            const t_u8 discard_last = discard;
            discard = bv.bytes[i] & BitmaskSingle(0);
            bv.bytes[i] >>= 1;

            if (discard_last) {
                bv.bytes[i] |= BitmaskSingle(bits_in_byte - 1);
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

        if (bv.bit_cnt == 0) {
            return;
        }

        // @speed: :(

        for (t_i32 i = 0; i < amount; i++) {
            const auto discard = ShiftBitsRight(bv);

            if (discard) {
                SetBit(bv, bv.bit_cnt - 1);
            } else {
                UnsetBit(bv, bv.bit_cnt - 1);
            }
        }
    }

    t_i32 IndexOfFirstSetBit(const s_bit_vec_rdonly bv, const t_i32 from = 0);   // Returns -1 if all bits are unset.
    t_i32 IndexOfFirstUnsetBit(const s_bit_vec_rdonly bv, const t_i32 from = 0); // Returns -1 if all bits are set.

    t_i32 CountSetBits(const s_bit_vec_rdonly bv);

    inline t_i32 CountUnsetBits(const s_bit_vec_rdonly bv) {
        return bv.bit_cnt - CountSetBits(bv);
    }

    // pos is the walker state, initialise it to the bit index you want to start from.
    // o_index is assigned the index of the set bit to process.
    // Returns false iff the walk is complete.
    // You can use the ZF_FOR_EACH_SET_BIT macro for more brevity if you like.
    inline t_b8 WalkSetBits(const s_bit_vec_rdonly bv, t_i32 *const pos, t_i32 *const o_index) {
        ZF_ASSERT(*pos >= 0 && *pos <= bv.bit_cnt);

        *o_index = IndexOfFirstSetBit(bv, *pos);

        if (*o_index == -1) {
            return false;
        }

        *pos = *o_index + 1;

        return true;
    }

    // pos is the walker state, initialise it to the bit index you want to start from.
    // o_index is assigned the index of the unset bit to process.
    // Returns false iff the walk is complete.
    // You can use the ZF_FOR_EACH_UNSET_BIT macro for more brevity if you like.
    inline t_b8 WalkUnsetBits(const s_bit_vec_rdonly bv, t_i32 *const pos, t_i32 *const o_index) {
        ZF_ASSERT(*pos >= 0 && *pos <= bv.bit_cnt);

        *o_index = IndexOfFirstUnsetBit(bv, *pos);

        if (*o_index == -1) {
            return false;
        }

        *pos = *o_index + 1;

        return true;
    }

#define ZF_WALK_SET_BITS(bv, index) for (t_i32 ZF_CONCAT(walk_pos_l, __LINE__) = 0, index; WalkSetBits(bv, &ZF_CONCAT(walk_pos_l, __LINE__), &index);)
#define ZF_WALK_UNSET_BITS(bv, index) for (t_i32 ZF_CONCAT(walk_pos_l, __LINE__) = 0, index; WalkUnsetBits(bv, &ZF_CONCAT(walk_pos_l, __LINE__), &index);)

    // ============================================================
}
