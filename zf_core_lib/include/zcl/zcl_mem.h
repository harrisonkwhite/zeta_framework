#pragma once

#include <cstring>
#include <zcl/zcl_basic.h>

namespace zf {
    // ============================================================
    // @section: Types and Globals

    struct s_arena_block {
        void *buf;
        t_i32 buf_size;

        s_arena_block *next;
    };

    struct s_arena {
        s_arena_block *blocks_head;
        s_arena_block *block_cur;
        t_i32 block_cur_offs;
        t_i32 block_min_size;
    };

    template <typename tp_type>
    struct s_array_rdonly {
        static_assert(!s_is_const<tp_type>::g_val);

        using t_elem = tp_type;

        const tp_type *raw;
        t_i32 len;

        constexpr s_array_rdonly() = default;
        constexpr s_array_rdonly(const tp_type *const raw, const t_i32 len) : raw(raw), len(len) {}

        constexpr t_i32 SizeInBytes() const {
            return ZF_SIZE_OF(tp_type) * len;
        }

        constexpr const tp_type &operator[](const t_i32 index) const {
            ZF_REQUIRE(index >= 0 && index < len);
            return raw[index];
        }

        constexpr s_array_rdonly<t_u8> AsByteArray() const {
            return {reinterpret_cast<const t_u8 *>(raw), SizeInBytes()};
        }
    };

    template <typename tp_type>
    struct s_array_mut {
        static_assert(!s_is_const<tp_type>::g_val);

        using t_elem = tp_type;

        tp_type *raw;
        t_i32 len;

        constexpr s_array_mut() = default;
        constexpr s_array_mut(tp_type *const raw, const t_i32 len) : raw(raw), len(len) {}

        constexpr t_i32 SizeInBytes() const {
            return ZF_SIZE_OF(tp_type) * len;
        }

        constexpr operator s_array_rdonly<tp_type>() const {
            return {raw, len};
        }

        constexpr tp_type &operator[](const t_i32 index) const {
            ZF_REQUIRE(index >= 0 && index < len);
            return raw[index];
        }

        constexpr s_array_mut<t_u8> AsByteArray() const {
            return {reinterpret_cast<t_u8 *>(raw), SizeInBytes()};
        }
    };

    template <typename tp_type, t_i32 tp_len>
    struct s_static_array {
        static_assert(!s_is_const<tp_type>::g_val);

        using t_elem = tp_type;

        static constexpr t_i32 g_len = tp_len;

        tp_type raw[tp_len];

        constexpr operator s_array_mut<tp_type>() {
            return {raw, tp_len};
        }

        constexpr operator s_array_rdonly<tp_type>() const {
            return {raw, tp_len};
        }

        constexpr s_array_mut<tp_type> AsNonstatic() {
            return {raw, tp_len};
        }

        constexpr s_array_rdonly<tp_type> AsNonstatic() const {
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
    concept co_array_mut = requires { typename tp_type::t_elem; } && s_is_same<tp_type, s_array_mut<typename tp_type::t_elem>>::g_val;

    template <typename tp_type>
    concept co_array_rdonly = requires { typename tp_type::t_elem; } && s_is_same<tp_type, s_array_rdonly<typename tp_type::t_elem>>::g_val;

    template <typename tp_type>
    concept co_array = co_array_mut<tp_type> || co_array_rdonly<tp_type>;

    template <co_array tp_arr_type>
    constexpr t_bin_comparator<tp_arr_type> g_array_bin_comparator =
        [](const tp_arr_type &a, const tp_arr_type &b) {
            if (a.len != b.len) {
                return false;
            }

            for (t_i32 i = 0; i < a.len; i++) {
                if (a[i] != b[i]) {
                    return false;
                }
            }

            return true;
        };

    struct s_bit_vec_rdonly {
        const t_u8 *bytes_raw;
        t_i32 bit_cnt;

        constexpr s_bit_vec_rdonly() = default;
        constexpr s_bit_vec_rdonly(const t_u8 *const bytes_raw, const t_i32 bit_cnt) : bytes_raw(bytes_raw), bit_cnt(bit_cnt) {}
        constexpr s_bit_vec_rdonly(const s_array_rdonly<t_u8> bytes) : bytes_raw(bytes.raw), bit_cnt(BytesToBits(bytes.len)) {}

        constexpr s_array_rdonly<t_u8> Bytes() const {
            return {bytes_raw, BitsToBytes(bit_cnt)};
        }
    };

    struct s_bit_vec_mut {
        t_u8 *bytes_raw;
        t_i32 bit_cnt;

        constexpr s_bit_vec_mut() = default;
        constexpr s_bit_vec_mut(t_u8 *const bytes_raw, const t_i32 bit_cnt) : bytes_raw(bytes_raw), bit_cnt(bit_cnt) {}
        constexpr s_bit_vec_mut(const s_array_mut<t_u8> bytes) : bytes_raw(bytes.raw), bit_cnt(BytesToBits(bytes.len)) {}

        constexpr s_array_mut<t_u8> Bytes() const {
            return {bytes_raw, BitsToBytes(bit_cnt)};
        }

        constexpr operator s_bit_vec_rdonly() const {
            return {bytes_raw, bit_cnt};
        }
    };

    template <t_i32 tp_bit_cnt>
    struct s_static_bit_vec {
        static constexpr t_i32 g_bit_cnt = tp_bit_cnt;

        s_static_array<t_u8, BitsToBytes(tp_bit_cnt)> bytes;

        constexpr operator s_bit_vec_mut() {
            return {bytes.raw, tp_bit_cnt};
        }

        constexpr operator s_bit_vec_rdonly() const {
            return {bytes.raw, tp_bit_cnt};
        }
    };

    enum e_bitwise_mask_op : t_i32 {
        ek_bitwise_mask_op_and,
        ek_bitwise_mask_op_or,
        ek_bitwise_mask_op_xor,
        ek_bitwise_mask_op_andnot
    };

    // ============================================================


    // ============================================================
    // @section: Functions

    inline void Clear(void *const buf, const t_i32 buf_size, const t_u8 val) {
        ZF_ASSERT(buf_size >= 0);
        memset(buf, val, static_cast<size_t>(buf_size));
    }

    template <typename tp_type>
    inline void ClearItem(tp_type *const item, const t_u8 val) {
        Clear(item, ZF_SIZE_OF(tp_type), val);
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
#else
    inline void PoisonUnitted(void *const buf, const t_i32 buf_size) {}
    template <typename tp_type> inline void PoisonUnittedItem(tp_type *const item) {}
    inline void PoisonFreed(void *const buf, const t_i32 buf_size) {}
    template <typename tp_type> inline void PoisonFreedItem(tp_type *const item) {}
#endif

    // Does not allocate any arena memory (blocks) upfront.
    inline s_arena CreateArena(const t_i32 block_min_size = MegabytesToBytes(1)) {
        return {.block_min_size = block_min_size};
    }

    // Frees all arena memory. It is valid to call this even if no pushing was done.
    void Destroy(s_arena *const arena);

    // Will lazily allocate memory as needed. Allocation failure is treated as fatal and causes an abort - you don't need to check for nullptr.
    void *Push(s_arena *const arena, const t_i32 size, const t_i32 alignment);

    template <typename tp_type>
    tp_type *PushItem(s_arena *const arena) {
        return static_cast<tp_type *>(Push(arena, ZF_SIZE_OF(tp_type), ZF_ALIGN_OF(tp_type)));
    }

    template <typename tp_type>
    tp_type *PushItemZeroed(s_arena *const arena) {
        const auto result = static_cast<tp_type *>(Push(arena, ZF_SIZE_OF(tp_type), ZF_ALIGN_OF(tp_type)));
        ClearItem(result, 0);
        return result;
    }

    template <typename tp_type>
    s_array_mut<tp_type> PushArray(s_arena *const arena, const t_i32 len) {
        ZF_ASSERT(len >= 0);

        if (len == 0) {
            return {};
        }

        const auto raw = static_cast<tp_type *>(Push(arena, ZF_SIZE_OF(tp_type) * len, ZF_ALIGN_OF(tp_type)));
        return {raw, len};
    }

    template <typename tp_type>
    s_array_mut<tp_type> PushArrayZeroed(s_arena *const arena, const t_i32 len) {
        ZF_ASSERT(len >= 0);

        if (len == 0) {
            return {};
        }

        const t_i32 size = ZF_SIZE_OF(tp_type) * len;
        const auto raw = static_cast<tp_type *>(Push(arena, size, ZF_ALIGN_OF(tp_type)));
        Clear(raw, size, 0);
        return {raw, len};
    }

    // Takes the arena offset to the beginning of its allocated memory (if any) to overwrite from there.
    void Rewind(s_arena *const arena);

    template <typename tp_type>
    constexpr s_array_rdonly<tp_type> Slice(const s_array_rdonly<tp_type> arr, const t_i32 beg, const t_i32 end) {
        ZF_ASSERT(beg >= 0 && beg <= arr.len);
        ZF_ASSERT(end >= beg && end <= arr.len);

        return {arr.raw + beg, end - beg};
    }

    template <typename tp_type>
    constexpr s_array_rdonly<tp_type> SliceFrom(const s_array_rdonly<tp_type> arr, const t_i32 beg) {
        ZF_ASSERT(beg >= 0 && beg <= arr.len);
        return {arr.raw + beg, arr.len - beg};
    }

    template <typename tp_type>
    constexpr s_array_mut<tp_type> Slice(const s_array_mut<tp_type> arr, const t_i32 beg, const t_i32 end) {
        ZF_ASSERT(beg >= 0 && beg <= arr.len);
        ZF_ASSERT(end >= beg && end <= arr.len);

        return {arr.raw + beg, end - beg};
    }

    template <typename tp_type>
    constexpr s_array_mut<tp_type> SliceFrom(const s_array_mut<tp_type> arr, const t_i32 beg) {
        ZF_ASSERT(beg >= 0 && beg <= arr.len);
        return {arr.raw + beg, arr.len - beg};
    }

    template <co_array tp_src_arr_type, co_array_mut tp_dest_arr_type>
    constexpr void CopyAll(const tp_src_arr_type src, const tp_dest_arr_type dest, const t_b8 allow_truncation = false) {
        static_assert(s_is_same<typename tp_src_arr_type::t_elem, typename tp_dest_arr_type::t_elem>::g_val);

        if (!allow_truncation) {
            ZF_ASSERT(dest.len >= src.len);

            for (t_i32 i = 0; i < src.len; i++) {
                dest[i] = src[i];
            }
        } else {
            const auto min_len = ZF_MIN(src.len, dest.len);

            for (t_i32 i = 0; i < min_len; i++) {
                dest[i] = src[i];
            }
        }
    }

    template <co_array tp_arr_a_type, co_array tp_arr_b_type>
    constexpr t_i32 CompareAll(const tp_arr_a_type a, const tp_arr_b_type b, const t_ord_comparator<typename tp_arr_a_type::t_elem> comparator = DefaultOrdComparator) {
        static_assert(s_is_same<typename tp_arr_a_type::t_elem, typename tp_arr_a_type::t_elem>::g_val);

        const auto min_len = ZF_MIN(a.len, b.len);

        for (t_i32 i = 0; i < min_len; i++) {
            const t_i32 comp = comparator(a[i], b[i]);

            if (comp != 0) {
                return comp;
            }
        }

        return 0;
    }

    template <co_array tp_arr_type>
    constexpr t_b8 DoAllEqual(const tp_arr_type arr, const typename tp_arr_type::t_elem &val, const t_bin_comparator<typename tp_arr_type::t_elem> comparator = DefaultBinComparator) {
        if (arr.len == 0) {
            return false;
        }

        for (t_i32 i = 0; i < arr.len; i++) {
            if (!comparator(arr[i], val)) {
                return false;
            }
        }

        return true;
    }

    template <co_array tp_arr_type>
    constexpr t_b8 DoAnyEqual(const tp_arr_type arr, const typename tp_arr_type::t_elem &val, const t_bin_comparator<typename tp_arr_type::t_elem> comparator = DefaultBinComparator) {
        for (t_i32 i = 0; i < arr.len; i++) {
            if (comparator(arr[i], val)) {
                return true;
            }
        }

        return false;
    }

    template <co_array_mut tp_arr_type>
    constexpr void SetAllTo(const tp_arr_type arr, const typename tp_arr_type::t_elem &val) {
        for (t_i32 i = 0; i < arr.len; i++) {
            arr[i] = val;
        }
    }

    template <co_array tp_arr_type>
    auto Clone(const tp_arr_type arr_to_clone, s_arena *const arena) {
        const auto arr = PushArray<typename tp_arr_type::t_elem>(arena, arr_to_clone.len);
        CopyAll(arr, arr_to_clone);
        return arr;
    }

    template <typename tp_type>
    s_array_mut<t_u8> AsBytes(tp_type &val) {
        return {reinterpret_cast<t_u8 *>(&val), ZF_SIZE_OF(val)};
    }

    template <typename tp_type>
    s_array_rdonly<t_u8> AsBytes(const tp_type &val) {
        return {reinterpret_cast<const t_u8 *>(&val), ZF_SIZE_OF(val)};
    }

    // Creates a byte-sized bitmask with only a single bit set.
    constexpr t_u8 Bit(const t_i32 bit_index) {
        ZF_ASSERT(bit_index >= 0 && bit_index < 8);
        return static_cast<t_u8>(1 << bit_index);
    }

    // Creates a byte-sized bitmask with only bits in the range [begin_bit_index, end_bit_index) set.
    constexpr t_u8 BitRange(const t_i32 begin_bit_index, const t_i32 end_bit_index = 8) {
        ZF_ASSERT(begin_bit_index >= 0 && begin_bit_index < 8);
        ZF_ASSERT(end_bit_index >= begin_bit_index && end_bit_index <= 8);

        if (end_bit_index - begin_bit_index == 8) {
            return 0xFF;
        }

        const auto bits_at_bottom = static_cast<t_u8>((1 << (end_bit_index - begin_bit_index)) - 1);
        return static_cast<t_u8>(bits_at_bottom << begin_bit_index);
    }

    inline s_bit_vec_mut CreateBitVector(const t_i32 bit_cnt, s_arena *const arena) {
        ZF_ASSERT(bit_cnt >= 0);
        return {PushArrayZeroed<t_u8>(arena, BitsToBytes(bit_cnt)).raw, bit_cnt};
    }

    constexpr t_i32 LastByteBitCount(const s_bit_vec_rdonly bv) {
        return ((bv.bit_cnt - 1) % 8) + 1;
    }

    // Gives a mask of the last byte in which only excess bits are unset.
    constexpr t_u8 LastByteMask(const s_bit_vec_rdonly bv) {
        return BitRange(0, LastByteBitCount(bv));
    }

    constexpr t_b8 IsBitSet(const s_bit_vec_rdonly bv, const t_i32 index) {
        ZF_ASSERT(index >= 0 && index < bv.bit_cnt);
        return bv.Bytes()[index / 8] & Bit(index % 8);
    }

    constexpr void SetBit(const s_bit_vec_mut bv, const t_i32 index) {
        ZF_ASSERT(index >= 0 && index < bv.bit_cnt);
        bv.Bytes()[index / 8] |= Bit(index % 8);
    }

    constexpr void UnsetBit(const s_bit_vec_mut bv, const t_i32 index) {
        ZF_ASSERT(index >= 0 && index < bv.bit_cnt);
        bv.Bytes()[index / 8] &= ~Bit(index % 8);
    }

    constexpr t_b8 CalcIsAnyBitSet(const s_bit_vec_rdonly bv) {
        if (bv.bit_cnt == 0) {
            return false;
        }

        const auto first_bytes = Slice(bv.Bytes(), 0, bv.Bytes().len - 1);

        if (!DoAllEqual(first_bytes, 0)) {
            return true;
        }

        return (bv.Bytes()[bv.Bytes().len - 1] & LastByteMask(bv)) != 0;
    }

    constexpr t_b8 CalcAreAllBitsSet(const s_bit_vec_rdonly bv) {
        if (bv.bit_cnt == 0) {
            return false;
        }

        const auto first_bytes = Slice(bv.Bytes(), 0, bv.Bytes().len - 1);

        if (!DoAllEqual(first_bytes, 0xFF)) {
            return false;
        }

        const auto last_byte_mask = LastByteMask(bv);
        return (bv.Bytes()[bv.Bytes().len - 1] & last_byte_mask) == last_byte_mask;
    }

    constexpr t_b8 CalcAreAllBitsUnset(const s_bit_vec_rdonly bv) {
        if (bv.bit_cnt == 0) {
            return false;
        }

        return !CalcIsAnyBitSet(bv);
    }

    constexpr t_b8 CalcIsAnyBitUnset(const s_bit_vec_rdonly bv) {
        if (bv.bit_cnt == 0) {
            return false;
        }

        return !CalcAreAllBitsSet(bv);
    }

    constexpr void SetAllBits(const s_bit_vec_mut bv) {
        if (bv.bit_cnt == 0) {
            return;
        }

        const auto first_bytes = Slice(bv.Bytes(), 0, bv.Bytes().len - 1);
        SetAllTo(first_bytes, 0xFF);

        bv.Bytes()[bv.Bytes().len - 1] |= LastByteMask(bv);
    }

    constexpr void UnsetAllBits(const s_bit_vec_mut bv) {
        if (bv.bit_cnt == 0) {
            return;
        }

        const auto first_bytes = Slice(bv.Bytes(), 0, bv.Bytes().len - 1);
        SetAllTo(first_bytes, 0);

        bv.Bytes()[bv.Bytes().len - 1] &= ~LastByteMask(bv);
    }

    // Sets all bits in the range [begin_bit_index, end_bit_index).
    constexpr void SetBitsInRange(const s_bit_vec_mut bv, const t_i32 begin_bit_index, const t_i32 end_bit_index) {
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

            bv.Bytes()[i] |= BitRange(set_range_begin, set_range_end);
        }
    }

    // @todo: Swap dest and src.
    constexpr void ApplyMaskToBits(const s_bit_vec_mut targ, const s_bit_vec_rdonly mask, const e_bitwise_mask_op op) {
        ZF_ASSERT(targ.bit_cnt == mask.bit_cnt);

        if (targ.bit_cnt == 0) {
            return;
        }

        switch (op) {
        case ek_bitwise_mask_op_and:
            for (t_i32 i = 0; i < targ.Bytes().len; i++) {
                targ.Bytes()[i] &= mask.Bytes()[i];
            }

            break;

        case ek_bitwise_mask_op_or:
            for (t_i32 i = 0; i < targ.Bytes().len; i++) {
                targ.Bytes()[i] |= mask.Bytes()[i];
            }

            break;

        case ek_bitwise_mask_op_xor:
            for (t_i32 i = 0; i < targ.Bytes().len; i++) {
                targ.Bytes()[i] ^= mask.Bytes()[i];
            }

            break;

        case ek_bitwise_mask_op_andnot:
            for (t_i32 i = 0; i < targ.Bytes().len; i++) {
                targ.Bytes()[i] &= ~mask.Bytes()[i];
            }

            break;
        }

        targ.Bytes()[targ.Bytes().len - 1] &= LastByteMask(targ);
    }

    // Shifts left only by 1. Returns the discarded bit as 0 or 1.
    constexpr t_u8 ShiftBitsLeft(const s_bit_vec_mut bv) {
        ZF_ASSERT(BitsToBytes(bv.bit_cnt) == bv.Bytes().len);

        if (bv.bit_cnt == 0) {
            return 0;
        }

        t_u8 discard = 0;

        for (t_i32 i = 0; i < bv.Bytes().len; i++) {
            const t_i32 bits_in_byte = i == bv.Bytes().len - 1 ? LastByteBitCount(bv) : 8;
            const t_u8 discard_last = discard;
            discard = (bv.Bytes()[i] & Bit(bits_in_byte - 1)) >> (bits_in_byte - 1);
            bv.Bytes()[i] <<= 1;
            bv.Bytes()[i] |= discard_last;
        }

        bv.Bytes()[bv.Bytes().len - 1] &= LastByteMask(bv);

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
        ZF_ASSERT(BitsToBytes(bv.bit_cnt) == bv.Bytes().len);

        if (bv.bit_cnt == 0) {
            return 0;
        }

        bv.Bytes()[bv.Bytes().len - 1] &= LastByteMask(bv); // Drop any excess bits so we don't accidentally shift a 1 in.

        t_u8 discard = 0;

        for (t_i32 i = bv.Bytes().len - 1; i >= 0; i--) {
            const t_i32 bits_in_byte = i == bv.Bytes().len - 1 ? LastByteBitCount(bv) : 8;
            const t_u8 discard_last = discard;
            discard = bv.Bytes()[i] & Bit(0);
            bv.Bytes()[i] >>= 1;

            if (discard_last) {
                bv.Bytes()[i] |= Bit(bits_in_byte - 1);
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

    t_i32 FindIndexOfFirstSetBit(const s_bit_vec_rdonly bv, const t_i32 from = 0);   // Returns -1 if all bits are unset.
    t_i32 FindIndexOfFirstUnsetBit(const s_bit_vec_rdonly bv, const t_i32 from = 0); // Returns -1 if all bits are set.

    t_i32 CountSetBits(const s_bit_vec_rdonly bv);

    inline t_i32 CountUnsetBits(const s_bit_vec_rdonly bv) {
        return bv.bit_cnt - CountSetBits(bv);
    }

    // pos is the walker state, initialise it to the bit index you want to start from.
    // o_index is assigned the index of the set bit to process.
    // Returns false iff the walk is complete.
    inline t_b8 WalkSetBits(const s_bit_vec_rdonly bv, t_i32 *const pos, t_i32 *const o_index) {
        ZF_ASSERT(*pos >= 0 && *pos <= bv.bit_cnt);

        *o_index = FindIndexOfFirstSetBit(bv, *pos);

        if (*o_index == -1) {
            return false;
        }

        *pos = *o_index + 1;

        return true;
    }

    // pos is the walker state, initialise it to the bit index you want to start from.
    // o_index is assigned the index of the unset bit to process.
    // Returns false iff the walk is complete.
    inline t_b8 WalkUnsetBits(const s_bit_vec_rdonly bv, t_i32 *const pos, t_i32 *const o_index) {
        ZF_ASSERT(*pos >= 0 && *pos <= bv.bit_cnt);

        *o_index = FindIndexOfFirstUnsetBit(bv, *pos);

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
