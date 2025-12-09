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
    void MarkUninitted(tp_type *const val) {
        Clear(val, g_uninitted_byte);
    }

    template <typename tp_type>
    t_b8 IsUninitted(tp_type *const val) {
        return IsClear(val, g_uninitted_byte);
    }

    template <typename tp_type>
    void MarkFreed(tp_type *const val) {
        Clear(val, g_freed_byte);
    }
#else
    template <typename tp_type>
    void MarkUninitted(tp_type *const val) {}

    template <typename tp_type>
    t_b8 IsUninitted(const tp_type *const val) {}

    template <typename tp_type>
    void MarkFreed(tp_type *const val) {}
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

    // ============================================================
    // @section: Memory Arenas
    // ============================================================
    struct s_mem_arena {
        void *buf;
        t_len size;
        t_len offs;
    };

    [[nodiscard]] t_b8 CreateMemArena(const t_len size, s_mem_arena *const o_ma);
    void DestroyMemArena(s_mem_arena *const ma);
    void *PushToMemArena(s_mem_arena *const ma, const t_len size, const t_len alignment);

    template <typename tp_type>
    tp_type *PushToMemArena(s_mem_arena *const ma, const t_len cnt = 1) {
        ZF_ASSERT(cnt >= 0);

        const auto buf = PushToMemArena(ma, ZF_SIZE_OF(tp_type) * cnt, ZF_ALIGN_OF(tp_type));
        return static_cast<tp_type *>(buf);
    }

    inline void RewindMemArena(s_mem_arena *const ma, const t_len offs) {
        ZF_ASSERT(offs >= 0 && offs <= ma->offs);
#ifdef ZF_DEBUG
        memset(static_cast<t_u8 *>(ma->buf) + offs, g_uninitted_byte, static_cast<size_t>(ma->offs - offs));
#endif
        ma->offs = offs;
    }

    [[nodiscard]] inline t_b8 CreateChildMemArena(const t_len size, s_mem_arena *const parent_ma, s_mem_arena *const o_ma) {
        MarkUninitted(o_ma);
        o_ma->buf = PushToMemArena(parent_ma, size, 1);
        o_ma->size = size;
        return o_ma->buf != nullptr;
    }

    // ============================================================
    // @section: Arrays
    // ============================================================
    template <typename tp_type>
    struct s_array_rdonly {
        static_assert(!s_is_const<tp_type>::g_val);

        using t_elem = tp_type;

        const tp_type *buf;
        t_len len;

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

        MarkUninitted(o_arr);

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

    // ============================================================
    // @section: Bits
    // ============================================================

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
        s_array_rdonly<t_u8> bytes;
        t_len bit_cnt;

        constexpr s_bit_vec_rdonly() = default;
        constexpr s_bit_vec_rdonly(const s_array_rdonly<t_u8> bytes, const t_len bit_cnt) : bytes(bytes), bit_cnt(bit_cnt) {}
        constexpr s_bit_vec_rdonly(const s_array_rdonly<t_u8> bytes) : bytes(bytes), bit_cnt(BytesToBits(bytes.len)) {}
    };

    struct s_bit_vec {
        s_array<t_u8> bytes;
        t_len bit_cnt;

        constexpr s_bit_vec() = default;
        constexpr s_bit_vec(const s_array<t_u8> bytes, const t_len bit_cnt) : bytes(bytes), bit_cnt(bit_cnt) {}
        constexpr s_bit_vec(const s_array<t_u8> bytes) : bytes(bytes), bit_cnt(BytesToBits(bytes.len)) {}

        constexpr operator s_bit_vec_rdonly() const {
            return {bytes, bit_cnt};
        }
    };

    template <t_len tp_bit_cnt>
    struct s_static_bit_vec {
        static constexpr t_len g_bit_cnt = tp_bit_cnt;

        s_static_array<t_u8, BitsToBytes(tp_bit_cnt)> bytes;

        constexpr operator s_bit_vec() {
            return {bytes, tp_bit_cnt};
        }

        constexpr operator s_bit_vec_rdonly() const {
            return {bytes, tp_bit_cnt};
        }
    };

    // Bit vector invariant.
    constexpr t_b8 IsBitVecValid(const s_bit_vec_rdonly bv) {
        return BitsToBytes(bv.bit_cnt) == bv.bytes.len;
    }

    constexpr t_len BitVecLastByteBitCnt(const s_bit_vec_rdonly bv) {
        ZF_ASSERT(IsBitVecValid(bv));
        return ((bv.bit_cnt - 1) % 8) + 1;
    }

    // Gives a mask of the last byte of the bit vector where only excess bits are unset.
    constexpr t_u8 BitVecLastByteMask(const s_bit_vec_rdonly bv) {
        ZF_ASSERT(IsBitVecValid(bv));
        return BitmaskRange(0, BitVecLastByteBitCnt(bv));
    }

    [[nodiscard]] inline t_b8 CreateBitVec(const t_len bit_cnt, s_mem_arena *const mem_arena, s_bit_vec *const o_bv) {
        ZF_ASSERT(bit_cnt > 0);

        MarkUninitted(o_bv);

        if (!AllocArray(BitsToBytes(bit_cnt), mem_arena, &o_bv->bytes)) {
            return false;
        }

        SetAllTo(o_bv->bytes, 0);

        o_bv->bit_cnt = bit_cnt;

        return true;
    }

    constexpr t_b8 IsBitSet(const s_bit_vec_rdonly bv, const t_len index) {
        ZF_ASSERT(IsBitVecValid(bv));
        ZF_ASSERT(index >= 0 && index < bv.bit_cnt);
        return bv.bytes[index / 8] & BitmaskSingle(index % 8);
    }

    constexpr void SetBit(const s_bit_vec bv, const t_len index) {
        ZF_ASSERT(IsBitVecValid(bv));
        ZF_ASSERT(index >= 0 && index < bv.bit_cnt);
        bv.bytes[index / 8] |= BitmaskSingle(index % 8);
    }

    constexpr void UnsetBit(const s_bit_vec bv, const t_len index) {
        ZF_ASSERT(IsBitVecValid(bv));
        ZF_ASSERT(index >= 0 && index < bv.bit_cnt);
        bv.bytes[index / 8] &= ~BitmaskSingle(index % 8);
    }

    constexpr t_b8 IsAnyBitSet(const s_bit_vec_rdonly bv) {
        ZF_ASSERT(IsBitVecValid(bv));

        if (bv.bit_cnt == 0) {
            return false;
        }

        const auto first_bytes = Slice(bv.bytes, 0, bv.bytes.len - 1);

        if (!AreAllEqualTo(first_bytes, 0)) {
            return true;
        }

        return (bv.bytes[bv.bytes.len - 1] & BitVecLastByteMask(bv)) != 0;
    }

    constexpr t_b8 AreAllBitsSet(const s_bit_vec_rdonly bv) {
        ZF_ASSERT(IsBitVecValid(bv));

        if (bv.bit_cnt == 0) {
            return false;
        }

        const auto first_bytes = Slice(bv.bytes, 0, bv.bytes.len - 1);

        if (!AreAllEqualTo(first_bytes, 0xFF)) {
            return false;
        }

        const auto last_byte_mask = BitVecLastByteMask(bv);
        return (bv.bytes[bv.bytes.len - 1] & last_byte_mask) == last_byte_mask;
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

    // Sets all bits in the range [begin_bit_index, end_bit_index).
    constexpr void SetBitsInRange(const s_bit_vec bv, const t_len begin_bit_index, const t_len end_bit_index) {
        ZF_ASSERT(IsBitVecValid(bv));
        ZF_ASSERT(begin_bit_index >= 0 && begin_bit_index < bv.bit_cnt);
        ZF_ASSERT(end_bit_index >= begin_bit_index && end_bit_index <= bv.bit_cnt);

        const t_len begin_elem_index = begin_bit_index / 8;
        const t_len end_elem_index = BitsToBytes(end_bit_index);

        for (t_len i = begin_elem_index; i < end_elem_index; i++) {
            const t_len bit_offs = i * 8;
            const t_len begin_bit_index_rel = begin_bit_index - bit_offs;
            const t_len end_bit_index_rel = end_bit_index - bit_offs;

            const t_len set_range_begin = ZF_MAX(begin_bit_index_rel, 0);
            const t_len set_range_end = ZF_MIN(end_bit_index_rel, 8);

            bv.bytes[i] |= BitmaskRange(set_range_begin, set_range_end);
        }
    }

    enum e_bitwise_mask_op {
        ek_bitwise_mask_op_and,
        ek_bitwise_mask_op_or,
        ek_bitwise_mask_op_xor,
        ek_bitwise_mask_op_andnot
    };

    constexpr void ApplyMask(const s_bit_vec dest, const s_bit_vec_rdonly src, const e_bitwise_mask_op op) {
        ZF_ASSERT(IsBitVecValid(dest));
        ZF_ASSERT(IsBitVecValid(src));
        ZF_ASSERT(dest.bit_cnt == src.bit_cnt);

        if (dest.bit_cnt == 0) {
            return;
        }

        switch (op) {
        case ek_bitwise_mask_op_and:
            for (t_len i = 0; i < dest.bytes.len; i++) {
                dest.bytes[i] &= src.bytes[i];
            }

            break;

        case ek_bitwise_mask_op_or:
            for (t_len i = 0; i < dest.bytes.len; i++) {
                dest.bytes[i] |= src.bytes[i];
            }

            break;

        case ek_bitwise_mask_op_xor:
            for (t_len i = 0; i < dest.bytes.len; i++) {
                dest.bytes[i] ^= src.bytes[i];
            }

            break;

        case ek_bitwise_mask_op_andnot:
            for (t_len i = 0; i < dest.bytes.len; i++) {
                dest.bytes[i] &= ~src.bytes[i];
            }

            break;
        }

        dest.bytes[dest.bytes.len - 1] &= BitVecLastByteMask(dest);
    }

    // Shifts left only by 1. Returns the discarded bit as 0 or 1.
    constexpr t_u8 ShiftBitsLeft(const s_bit_vec bv) {
        ZF_ASSERT(IsBitVecValid(bv));

        if (bv.bit_cnt == 0) {
            return 0;
        }

        t_u8 discard = 0;

        for (t_len i = 0; i < bv.bytes.len; i++) {
            const t_len bits_in_byte = i == bv.bytes.len - 1 ? BitVecLastByteBitCnt(bv) : 8;
            const t_u8 discard_last = discard;
            discard = (bv.bytes[i] & BitmaskSingle(bits_in_byte - 1)) >> (bits_in_byte - 1);
            bv.bytes[i] <<= 1;
            bv.bytes[i] |= discard_last;
        }

        bv.bytes[bv.bytes.len - 1] &= BitVecLastByteMask(bv);

        return discard;
    }

    constexpr void ShiftBitsLeftBy(const s_bit_vec bv, const t_len amount) {
        ZF_ASSERT(IsBitVecValid(bv));
        ZF_ASSERT(amount >= 0);

        // @speed: :(

        for (t_len i = 0; i < amount; i++) {
            ShiftBitsLeft(bv);
        }
    }

    constexpr void RotBitsLeftBy(const s_bit_vec bv, const t_len amount) {
        ZF_ASSERT(IsBitVecValid(bv));
        ZF_ASSERT(amount >= 0);

        if (bv.bit_cnt == 0) {
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
        ZF_ASSERT(IsBitVecValid(bv));

        if (bv.bit_cnt == 0) {
            return 0;
        }

        bv.bytes[bv.bytes.len - 1] &= BitVecLastByteMask(bv); // Drop any excess bits so we don't accidentally shift a 1 in.

        t_u8 discard = 0;

        for (t_len i = bv.bytes.len - 1; i >= 0; i--) {
            const t_len bits_in_byte = i == bv.bytes.len - 1 ? BitVecLastByteBitCnt(bv) : 8;
            const t_u8 discard_last = discard;
            discard = bv.bytes[i] & BitmaskSingle(0);
            bv.bytes[i] >>= 1;

            if (discard_last) {
                bv.bytes[i] |= BitmaskSingle(bits_in_byte - 1);
            }
        }

        return discard;
    }

    constexpr void ShiftBitsRightBy(const s_bit_vec bv, const t_len amount) {
        ZF_ASSERT(IsBitVecValid(bv));
        ZF_ASSERT(amount >= 0);

        // @speed: :(

        for (t_len i = 0; i < amount; i++) {
            ShiftBitsRight(bv);
        }
    }

    constexpr void RotBitsRightBy(const s_bit_vec bv, const t_len amount) {
        ZF_ASSERT(IsBitVecValid(bv));
        ZF_ASSERT(amount >= 0);

        if (bv.bit_cnt == 0) {
            return;
        }

        // @speed: :(

        for (t_len i = 0; i < amount; i++) {
            const auto discard = ShiftBitsRight(bv);

            if (discard) {
                SetBit(bv, bv.bit_cnt - 1);
            } else {
                UnsetBit(bv, bv.bit_cnt - 1);
            }
        }
    }

    t_len IndexOfFirstSetBit(const s_bit_vec_rdonly bv, const t_len from = 0);   // Returns -1 if all bits are unset.
    t_len IndexOfFirstUnsetBit(const s_bit_vec_rdonly bv, const t_len from = 0); // Returns -1 if all bits are set.

    t_len CntSetBits(const s_bit_vec_rdonly bv);

    inline t_len CntUnsetBits(const s_bit_vec_rdonly bv) {
        return bv.bit_cnt - CntSetBits(bv);
    }

    // index is assigned the index of the set bit to process.
    // pos is the walker state, initialise it to the bit index you want to start from.
    // Returns false iff the walk is complete.
    // You can use the ZF_FOR_EACH_SET_BIT macro for more brevity if you like.
    inline t_b8 WalkSetBits(const s_bit_vec_rdonly bv, t_len *const index, t_len *const pos) {
        ZF_ASSERT(*pos >= 0 && *pos < bv.bit_cnt);
        *index = IndexOfFirstSetBit(bv, *pos);
        *pos = *index + 1;
        return *index != -1;
    }

    // index is assigned the index of the unset bit to process.
    // pos is the walker state, initialise it to the bit index you want to start from.
    // Returns false iff the walk is complete.
    // You can use the ZF_FOR_EACH_UNSET_BIT macro for more brevity if you like.
    inline t_b8 WalkUnsetBits(const s_bit_vec_rdonly bv, t_len *const index, t_len *const pos) {
        ZF_ASSERT(*pos >= 0 && *pos < bv.bit_cnt);
        *index = IndexOfFirstUnsetBit(bv, *pos);
        *pos = *index + 1;
        return *index != -1;
    }

#define ZF_FOR_EACH_SET_BIT(bv, index) for (t_len ZF_CONCAT(walk_pos_l, __LINE__) = 0, index; WalkSetBits(bv, &index, &ZF_CONCAT(walk_pos_l, __LINE__));)
#define ZF_FOR_EACH_UNSET_BIT(bv, index) for (t_len ZF_CONCAT(walk_pos_l, __LINE__) = 0, index; WalkUnsetBits(bv, &index, &ZF_CONCAT(walk_pos_l, __LINE__));)
}
