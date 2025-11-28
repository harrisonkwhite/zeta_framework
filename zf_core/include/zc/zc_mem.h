#pragma once

#include <cstring>
#include <zc/zc_basic.h>

namespace zf {
    constexpr t_size Kilobytes(const t_size x) { return (static_cast<t_size>(1) << 10) * x; }
    constexpr t_size Megabytes(const t_size x) { return (static_cast<t_size>(1) << 20) * x; }
    constexpr t_size Gigabytes(const t_size x) { return (static_cast<t_size>(1) << 30) * x; }
    constexpr t_size Terabytes(const t_size x) { return (static_cast<t_size>(1) << 40) * x; }

    constexpr t_size BitsToBytes(const t_size x) { return (x + 7) / 8; }
    constexpr t_size BytesToBits(const t_size x) { return x * 8; }

    // Is n a power of 2?
    constexpr t_b8 IsAlignmentValid(const t_size n) {
        return n > 0 && (n & (n - 1)) == 0;
    }

    // Take n up to the next multiple of the alignment.
    constexpr t_size AlignForward(const t_size n, const t_size alignment) {
        ZF_ASSERT(IsAlignmentValid(alignment));
        return (n + alignment - 1) & ~(alignment - 1);
    }

    // Prohibiting pointer use here for safety.
    template<typename tp_type>
    constexpr void ZeroOut(tp_type& val) {
        static_assert(!s_is_ptr<tp_type>::g_val);
        memset(&val, 0, sizeof(val));
    }

    // ============================================================
    // @section: Memory Arenas
    // ============================================================
    struct s_mem_arena {
        void* buf;
        t_size size;
        t_size offs;
    };

    [[nodiscard]] t_b8 AllocMemArena(const t_size size, s_mem_arena& o_ma);
    void FreeMemArena(s_mem_arena& ma);
    void* PushToMemArena(s_mem_arena& ma, const t_size size, const t_size alignment);

    template<typename tp_type>
    tp_type* PushToMemArena(s_mem_arena& ma, const t_size cnt = 1) {
        ZF_ASSERT(cnt >= 1);

        const auto buf = PushToMemArena(ma, ZF_SIZE_OF(tp_type) * cnt, alignof(tp_type));
        return static_cast<tp_type*>(buf);
    }

    inline void RewindMemArena(s_mem_arena& ma, const t_size offs) {
        ZF_ASSERT(offs >= 0 && offs <= ma.offs);
        memset(static_cast<t_u8*>(ma.buf) + offs, 0, static_cast<size_t>(ma.offs - offs));
        ma.offs = offs;
    }

    [[nodiscard]] inline t_b8 MakeSubMemArena(s_mem_arena& parent_ma, const t_size size, s_mem_arena& o_ma) {
        o_ma = {
            .buf = PushToMemArena(parent_ma, size, 1),
            .size = size
        };

        return o_ma.buf != nullptr;
    }

    // ============================================================
    // @section: Arrays
    // ============================================================
    template<typename tp_type>
    struct s_array_rdonly {
        static_assert(!s_is_const<tp_type>::g_val);

        using t_elem = tp_type;

        const tp_type* buf_raw;
        t_size len;

        constexpr const tp_type& operator[](const t_size index) const {
            ZF_ASSERT(index >= 0 && index < len);
            return buf_raw[index];
        }
    };

    template<typename tp_type>
    struct s_array {
        static_assert(!s_is_const<tp_type>::g_val);

        using t_elem = tp_type;

        tp_type* buf_raw;
        t_size len;

        constexpr tp_type& operator[](const t_size index) const {
            ZF_ASSERT(index >= 0 && index < len);
            return buf_raw[index];
        }

        constexpr operator s_array_rdonly<tp_type>() const {
            return {buf_raw, len};
        }
    };

    template<typename tp_type, t_size tp_len>
    struct s_static_array {
        static_assert(!s_is_const<tp_type>::g_val);

        using t_elem = tp_type;

        static constexpr t_size g_len = tp_len;

        tp_type buf_raw[tp_len];

        constexpr s_static_array() = default;

        template<t_size tp_other_len>
        constexpr s_static_array(const tp_type (&buf_raw)[tp_other_len]) {
            static_assert(tp_other_len == tp_len);

            for (t_size i = 0; i < tp_other_len; i++) {
                this->buf_raw[i] = buf_raw[i];
            }
        }

        constexpr tp_type& operator[](const t_size index) {
            ZF_ASSERT(index >= 0 && index < tp_len);
            return buf_raw[index];
        }

        constexpr const tp_type& operator[](const t_size index) const {
            ZF_ASSERT(index >= 0 && index < tp_len);
            return buf_raw[index];
        }

        constexpr operator s_array<tp_type>() {
            return {buf_raw, tp_len};
        }

        constexpr operator s_array_rdonly<tp_type>() const {
            return {buf_raw, tp_len};
        }
    };

    template<typename tp_type> struct s_is_array { static constexpr t_b8 g_val = false; };
    template<typename tp_type> struct s_is_array<s_array<tp_type>> { static constexpr t_b8 g_val = true; };
    template<typename tp_type> struct s_is_array<const s_array<tp_type>> { static constexpr t_b8 g_val = true; };
    template<typename tp_type> struct s_is_array<s_array_rdonly<tp_type>> { static constexpr t_b8 g_val = true; };
    template<typename tp_type> struct s_is_array<const s_array_rdonly<tp_type>> { static constexpr t_b8 g_val = true; };
    template<typename tp_type, t_size tp_len> struct s_is_array<s_static_array<tp_type, tp_len>> { static constexpr t_b8 g_val = true; };
    template<typename tp_type, t_size tp_len> struct s_is_array<const s_static_array<tp_type, tp_len>> { static constexpr t_b8 g_val = true; };

    template<typename tp_type> struct s_is_mut_array { static constexpr t_b8 g_val = false; };
    template<typename tp_type> struct s_is_mut_array<s_array<tp_type>> { static constexpr t_b8 g_val = true; };
    template<typename tp_type> struct s_is_mut_array<const s_array<tp_type>> { static constexpr t_b8 g_val = true; };
    template<typename tp_type, t_size tp_len> struct s_is_mut_array<s_static_array<tp_type, tp_len>> { static constexpr t_b8 g_val = true; };

    template<typename tp_type> concept c_array = s_is_array<tp_type>::g_val;
    template<typename tp_type> concept c_array_mut = s_is_mut_array<tp_type>::g_val;
    template<typename tp_type> concept c_array_rdonly = s_is_array<tp_type>::g_val && !s_is_mut_array<tp_type>::g_val;

    template<typename tp_type>
    constexpr tp_type* ArrayRaw(const s_array<tp_type> arr) {
        return arr.buf_raw;
    }

    template<typename tp_type>
    constexpr const tp_type* ArrayRaw(const s_array_rdonly<tp_type> arr) {
        return arr.buf_raw;
    }

    template<typename tp_type, t_size tp_len>
    constexpr tp_type* ArrayRaw(s_static_array<tp_type, tp_len>& arr) {
        return arr.buf_raw;
    }

    template<typename tp_type, t_size tp_len>
    constexpr const tp_type* ArrayRaw(const s_static_array<tp_type, tp_len>& arr) {
        return arr.buf_raw;
    }

    template<typename tp_type>
    constexpr t_size ArrayLen(const s_array<tp_type> arr) {
        return arr.len;
    }

    template<typename tp_type>
    constexpr t_size ArrayLen(const s_array_rdonly<tp_type> arr) {
        return arr.len;
    }

    template<typename tp_type, t_size tp_len>
    constexpr t_size ArrayLen(s_static_array<tp_type, tp_len>& arr) {
        return tp_len;
    }

    template<typename tp_type, t_size tp_len>
    constexpr t_size ArrayLen(const s_static_array<tp_type, tp_len>& arr) {
        return tp_len;
    }

    template<c_array_mut tp_type>
    constexpr s_array<typename tp_type::t_elem> ToNonstatic(tp_type& arr) {
        return {arr.buf_raw, ArrayLen(arr)};
    }

    template<c_array_rdonly tp_type>
    constexpr s_array_rdonly<typename tp_type::t_elem> ToNonstatic(tp_type& arr) {
        return {arr.buf_raw, ArrayLen(arr)};
    }

    template<c_array tp_type>
    constexpr t_b8 IsArrayEmpty(tp_type& arr) {
        return ArrayLen(arr) == 0;
    }

    template<c_array tp_type>
    constexpr t_size ArraySizeInBytes(tp_type& arr) {
        return ZF_SIZE_OF(typename tp_type::t_elem) * ArrayLen(arr);
    }

    template<typename tp_type>
    t_b8 MakeArray(s_mem_arena& mem_arena, const t_size len, s_array<tp_type>& o_arr) {
        ZF_ASSERT(len > 0);

        o_arr = {
            .buf_raw = PushToMemArena<tp_type>(mem_arena, len),
            .len = len
        };

        return o_arr.buf_raw != nullptr;
    }

    template<c_array tp_type>
    t_b8 MakeArrayClone(s_mem_arena& mem_arena, tp_type& arr_to_clone, s_array<typename tp_type::t_elem>& o_arr) {
        ZF_ASSERT(!IsArrayEmpty(arr_to_clone));

        if (!MakeArray(mem_arena, ArrayLen(arr_to_clone), o_arr)) {
            return false;
        }

        Copy(o_arr, arr_to_clone);

        return true;
    }

    template<c_array_mut tp_type>
    constexpr s_array<typename tp_type::t_elem> Slice(tp_type& arr, const t_size beg, const t_size end) {
        ZF_ASSERT(beg >= 0 && beg <= ArrayLen(arr));
        ZF_ASSERT(end >= beg && end <= ArrayLen(arr));

        return {ArrayRaw(arr) + beg, end - beg};
    }

    template<c_array_rdonly tp_type>
    constexpr s_array_rdonly<typename tp_type::t_elem> Slice(tp_type& arr, const t_size beg, const t_size end) {
        ZF_ASSERT(beg >= 0 && beg <= ArrayLen(arr));
        ZF_ASSERT(end >= beg && end <= ArrayLen(arr));

        return {ArrayRaw(arr) + beg, end - beg};
    } 

    template<c_array_mut tp_dest_type, c_array tp_src_type>
    constexpr void Copy(tp_dest_type& dest, tp_src_type& src) {
        static_assert(s_is_same<typename tp_dest_type::t_elem, typename tp_src_type::t_elem>::g_val);

        ZF_ASSERT(ArrayLen(dest) >= ArrayLen(src));

        for (t_size i = 0; i < ArrayLen(src); i++) {
            dest[i] = src[i];
        }
    }

    template<c_array_mut tp_dest_type, c_array tp_src_type>
    constexpr void CopyReverse(tp_dest_type& dest, tp_src_type& src) {
        static_assert(s_is_same<typename tp_dest_type::t_elem, typename tp_src_type::t_elem>::g_val);

        ZF_ASSERT(ArrayLen(dest) >= ArrayLen(src));

        for (t_size i = ArrayLen(src) - 1; i >= 0; i--) {
            dest[i] = src[i];
        }
    }

    template<c_array_mut tp_type>
    constexpr void Reverse(tp_type& arr) {
        for (t_size i = 0; i < ArrayLen(arr) / 2; i++) {
            Swap(arr[i], arr[ArrayLen(arr) - 1 - i]);
        }
    }

    template<c_array tp_type>
    constexpr t_b8 AreAllEqualTo(tp_type& arr, const typename tp_type::t_elem& val, const t_bin_comparator<typename tp_type::t_elem> comparator = DefaultBinComparator) {
        ZF_ASSERT(comparator);

        for (t_size i = 0; i < ArrayLen(arr); i++) {
            if (!comparator(arr[i], val)) {
                return false;
            }
        }

        return true;
    }

    template<c_array tp_type>
    constexpr t_b8 AreAnyEqualTo(tp_type& arr, const typename tp_type::t_elem& val, const t_bin_comparator<typename tp_type::t_elem> comparator = DefaultBinComparator) {
        ZF_ASSERT(comparator);

        for (t_size i = 0; i < ArrayLen(arr); i++) {
            if (comparator(arr[i], val)) {
                return true;
            }
        }

        return false;
    }

    template<c_array_mut tp_type>
    constexpr void SetAllTo(tp_type& arr, const typename tp_type::t_elem& val) {
        for (t_size i = 0; i < ArrayLen(arr); i++) {
            arr[i] = val;
        }
    }

    template<typename tp_type>
    constexpr s_array<t_u8> ToBytes(tp_type& item) {
        return {reinterpret_cast<t_u8*>(&item), ZF_SIZE_OF(item)};
    }

    template<typename tp_type>
    constexpr s_array_rdonly<t_u8> ToBytes(const tp_type& item) {
        return {reinterpret_cast<const t_u8*>(&item), ZF_SIZE_OF(item)};
    }

    template<c_array_mut tp_type>
    constexpr s_array<t_u8> ToByteArray(tp_type& arr) {
        return {reinterpret_cast<t_u8*>(ArrayRaw(arr)), ArraySizeInBytes(arr)};
    }

    template<c_array_rdonly tp_type>
    constexpr s_array_rdonly<t_u8> ToByteArray(tp_type& arr) {
        return {reinterpret_cast<const t_u8*>(ArrayRaw(arr)), ArraySizeInBytes(arr)};
    }

    // ============================================================
    // @section: Bits
    // ============================================================

    // Creates a bitmask with only a single bit set.
    constexpr t_u8 BitmaskSingle(const t_size bit_index) {
        ZF_ASSERT(bit_index >= 0 && bit_index < 8);
        return static_cast<t_u8>(1 << bit_index);
    }

    // Creates a bitmask with only bits in the range [begin_bit_index, end_bit_index) set.
    constexpr t_u8 BitmaskRange(const t_size begin_bit_index, const t_size end_bit_index = 8) {
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
        t_size bit_cnt;

        constexpr s_bit_vec_rdonly() = default;
        constexpr s_bit_vec_rdonly(const s_array_rdonly<t_u8> bytes, const t_size bit_cnt) : bytes(bytes), bit_cnt(bit_cnt) {}
        constexpr s_bit_vec_rdonly(const s_array_rdonly<t_u8> bytes) : bytes(bytes), bit_cnt(BytesToBits(bytes.len)) {}
    };

    struct s_bit_vec {
        s_array<t_u8> bytes;
        t_size bit_cnt;

        constexpr s_bit_vec() = default;
        constexpr s_bit_vec(const s_array<t_u8> bytes, const t_size bit_cnt) : bytes(bytes), bit_cnt(bit_cnt) {}
        constexpr s_bit_vec(const s_array<t_u8> bytes) : bytes(bytes), bit_cnt(BytesToBits(bytes.len)) {}

        constexpr operator s_bit_vec_rdonly() const {
            return {bytes, bit_cnt};
        }
    };

    template<t_size tp_bit_cnt>
    struct s_static_bit_vec {
        static constexpr t_size g_bit_cnt = tp_bit_cnt;

        s_static_array<t_u8, BitsToBytes(tp_bit_cnt)> bytes;

        constexpr operator s_bit_vec() {
            return {bytes, tp_bit_cnt};
        }

        constexpr operator s_bit_vec_rdonly() const {
            return {bytes, tp_bit_cnt};
        }
    };

    // Bit vector invariant.
    constexpr t_b8 IsBitVecValid(const s_bit_vec_rdonly& bv) {
        return BitsToBytes(bv.bit_cnt) == bv.bytes.len;
    }

    constexpr t_size BitVecLastByteBitCnt(const s_bit_vec_rdonly& bv) {
        ZF_ASSERT(IsBitVecValid(bv));
        return ((bv.bit_cnt - 1) % 8) + 1;
    }

    // Gives a mask of the last byte of the bit vector where only excess bits are unset.
    constexpr t_u8 BitVecLastByteMask(const s_bit_vec_rdonly& bv) {
        ZF_ASSERT(IsBitVecValid(bv));
        return BitmaskRange(0, BitVecLastByteBitCnt(bv));
    }

    [[nodiscard]] inline t_b8 MakeBitVec(s_mem_arena& mem_arena, const t_size bit_cnt, s_bit_vec& o_bv) {
        ZF_ASSERT(bit_cnt > 0);

        o_bv = {};
        o_bv.bit_cnt = bit_cnt;

        return MakeArray(mem_arena, BitsToBytes(o_bv.bit_cnt), o_bv.bytes);
    }

    constexpr t_b8 IsBitSet(const s_bit_vec_rdonly& bv, const t_size index) {
        ZF_ASSERT(IsBitVecValid(bv));
        ZF_ASSERT(index >= 0 && index < bv.bit_cnt);
        return bv.bytes[index / 8] & BitmaskSingle(index % 8);
    }

    constexpr void SetBit(const s_bit_vec& bv, const t_size index) {
        ZF_ASSERT(IsBitVecValid(bv));
        ZF_ASSERT(index >= 0 && index < bv.bit_cnt);
        bv.bytes[index / 8] |= BitmaskSingle(index % 8);
    }

    constexpr void UnsetBit(const s_bit_vec& bv, const t_size index) {
        ZF_ASSERT(IsBitVecValid(bv));
        ZF_ASSERT(index >= 0 && index < bv.bit_cnt);
        bv.bytes[index / 8] &= ~BitmaskSingle(index % 8);
    }

    constexpr t_b8 IsAnyBitSet(const s_bit_vec_rdonly& bv) {
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

    constexpr t_b8 AreAllBitsSet(const s_bit_vec_rdonly& bv) {
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

    constexpr t_b8 AreAllBitsUnset(const s_bit_vec_rdonly& bv) {
        if (bv.bit_cnt == 0) {
            return false;
        }

        return !IsAnyBitSet(bv);
    }

    constexpr t_b8 IsAnyBitUnset(const s_bit_vec_rdonly& bv) {
        if (bv.bit_cnt == 0) {
            return false;
        }

        return !AreAllBitsSet(bv);
    }

    // Sets all bits in the range [begin_bit_index, end_bit_index).
    constexpr void SetBitsInRange(const s_bit_vec& bv, const t_size begin_bit_index, const t_size end_bit_index) {
        ZF_ASSERT(IsBitVecValid(bv));
        ZF_ASSERT(begin_bit_index >= 0 && begin_bit_index < bv.bit_cnt);
        ZF_ASSERT(end_bit_index >= begin_bit_index && end_bit_index <= bv.bit_cnt);

        const t_size begin_elem_index = begin_bit_index / 8;
        const t_size end_elem_index = BitsToBytes(end_bit_index);

        for (t_size i = begin_elem_index; i < end_elem_index; i++) {
            const t_size bit_offs = i * 8;
            const t_size begin_bit_index_rel = begin_bit_index - bit_offs;
            const t_size end_bit_index_rel = end_bit_index - bit_offs;

            const t_size set_range_begin = ZF_MAX(begin_bit_index_rel, 0);
            const t_size set_range_end = ZF_MIN(end_bit_index_rel, 8);

            bv.bytes[i] |= BitmaskRange(set_range_begin, set_range_end);
        }
    }

    enum e_bitwise_mask_op {
        ek_bitwise_mask_op_and,
        ek_bitwise_mask_op_or,
        ek_bitwise_mask_op_xor,
        ek_bitwise_mask_op_andnot
    };

    constexpr void ApplyMask(const s_bit_vec& dest, const s_bit_vec_rdonly& src, const e_bitwise_mask_op op) {
        ZF_ASSERT(IsBitVecValid(dest));
        ZF_ASSERT(IsBitVecValid(src));
        ZF_ASSERT(dest.bit_cnt == src.bit_cnt);

        if (dest.bit_cnt == 0) {
            return;
        }

        switch (op) {
            case ek_bitwise_mask_op_and:
                for (t_size i = 0; i < dest.bytes.len; i++) {
                    dest.bytes[i] &= src.bytes[i];
                }

                break;

            case ek_bitwise_mask_op_or:
                for (t_size i = 0; i < dest.bytes.len; i++) {
                    dest.bytes[i] |= src.bytes[i];
                }

                break;

            case ek_bitwise_mask_op_xor:
                for (t_size i = 0; i < dest.bytes.len; i++) {
                    dest.bytes[i] ^= src.bytes[i];
                }

                break;

            case ek_bitwise_mask_op_andnot:
                for (t_size i = 0; i < dest.bytes.len; i++) {
                    dest.bytes[i] &= ~src.bytes[i];
                }

                break;
        }

        dest.bytes[dest.bytes.len - 1] &= BitVecLastByteMask(dest);
    }

    // Shifts left only by 1. Returns the discarded bit as 0 or 1.
    constexpr t_u8 ShiftBitsLeft(const s_bit_vec& bv) {
        ZF_ASSERT(IsBitVecValid(bv));

        if (bv.bit_cnt == 0) {
            return 0;
        }

        t_u8 discard = 0;

        for (t_size i = 0; i < bv.bytes.len; i++) {
            const t_size bits_in_byte = i == bv.bytes.len - 1 ? BitVecLastByteBitCnt(bv) : 8;
            const t_u8 discard_last = discard;
            discard = (bv.bytes[i] & BitmaskSingle(bits_in_byte - 1)) >> (bits_in_byte - 1);
            bv.bytes[i] <<= 1;
            bv.bytes[i] |= discard_last;
        }

        bv.bytes[bv.bytes.len - 1] &= BitVecLastByteMask(bv);

        return discard;
    }

    constexpr void ShiftBitsLeftBy(const s_bit_vec& bv, const t_size amount) {
        ZF_ASSERT(IsBitVecValid(bv));
        ZF_ASSERT(amount >= 0);

        // @speed: :(

        for (t_size i = 0; i < amount; i++) {
            ShiftBitsLeft(bv);
        }
    }

    constexpr void RotBitsLeftBy(const s_bit_vec& bv, const t_size amount) {
        ZF_ASSERT(IsBitVecValid(bv));
        ZF_ASSERT(amount >= 0);

        if (bv.bit_cnt == 0) {
            return;
        }

        // @speed: :(

        for (t_size i = 0; i < amount; i++) {
            const auto discard = ShiftBitsLeft(bv);

            if (discard) {
                SetBit(bv, 0);
            } else {
                UnsetBit(bv, 0);
            }
        }
    }

    // Shifts right only by 1. Returns the carry bit.
    constexpr t_u8 ShiftBitsRight(const s_bit_vec& bv) {
        ZF_ASSERT(IsBitVecValid(bv));

        if (bv.bit_cnt == 0) {
            return 0;
        }

        bv.bytes[bv.bytes.len - 1] &= BitVecLastByteMask(bv); // Drop any excess bits so we don't accidentally shift a 1 in.

        t_u8 discard = 0;

        for (t_size i = bv.bytes.len - 1; i >= 0; i--) {
            const t_size bits_in_byte = i == bv.bytes.len - 1 ? BitVecLastByteBitCnt(bv) : 8;
            const t_u8 discard_last = discard;
            discard = bv.bytes[i] & BitmaskSingle(0);
            bv.bytes[i] >>= 1;

            if (discard_last) {
                bv.bytes[i] |= BitmaskSingle(bits_in_byte - 1);
            }
        }

        return discard;
    }

    constexpr void ShiftBitsRightBy(const s_bit_vec& bv, const t_size amount) {
        ZF_ASSERT(IsBitVecValid(bv));
        ZF_ASSERT(amount >= 0);

        // @speed: :(

        for (t_size i = 0; i < amount; i++) {
            ShiftBitsRight(bv);
        }
    }

    constexpr void RotBitsRightBy(const s_bit_vec& bv, const t_size amount) {
        ZF_ASSERT(IsBitVecValid(bv));
        ZF_ASSERT(amount >= 0);

        if (bv.bit_cnt == 0) {
            return;
        }

        // @speed: :(

        for (t_size i = 0; i < amount; i++) {
            const auto discard = ShiftBitsRight(bv);

            if (discard) {
                SetBit(bv, bv.bit_cnt - 1);
            } else {
                UnsetBit(bv, bv.bit_cnt - 1);
            }
        }
    }

    t_size IndexOfFirstSetBit(const s_bit_vec_rdonly& bv, const t_size from = 0);
    t_size IndexOfFirstUnsetBit(const s_bit_vec_rdonly& bv, const t_size from = 0);

    t_size CntSetBits(const s_bit_vec_rdonly& bv);

    inline t_size CntUnsetBits(const s_bit_vec_rdonly& bv) {
        return bv.bit_cnt - CntSetBits(bv);
    }

    // pos is the walker state, initialise it to the bit index you want to start from.
    // o_index is assigned the index of the set bit to process.
    // Returns false iff the walk is complete.
    // You can use the ZF_FOR_EACH_SET_BIT macro for more brevity if you like.
    inline t_b8 WalkSetBits(const s_bit_vec_rdonly& bv, t_size& pos, t_size& o_index) {
        ZF_ASSERT(pos >= 0 && pos < bv.bit_cnt);
        o_index = IndexOfFirstSetBit(bv, pos);
        pos = o_index + 1;
        return o_index != -1;
    }

    // pos is the walker state, initialise it to the bit index you want to start from.
    // o_index is assigned the index of the unset bit to process.
    // Returns false iff the walk is complete.
    // You can use the ZF_FOR_EACH_UNSET_BIT macro for more brevity if you like.
    inline t_b8 WalkUnsetBits(const s_bit_vec_rdonly& bv, t_size& pos, t_size& o_index) {
        ZF_ASSERT(pos >= 0 && pos < bv.bit_cnt);
        o_index = IndexOfFirstUnsetBit(bv, pos);
        pos = o_index + 1;
        return o_index != -1;
    }

#define ZF_FOR_EACH_SET_BIT(br, index) for (t_size ZF_CONCAT(p_walk_pos_l, __LINE__) = 0, index; WalkSetBits(br, ZF_CONCAT(p_walk_pos_l, __LINE__), index); )
#define ZF_FOR_EACH_UNSET_BIT(br, index) for (t_size ZF_CONCAT(p_walk_pos_l, __LINE__) = 0, index; WalkUnsetBits(br, ZF_CONCAT(p_walk_pos_l, __LINE__), index); )
}
