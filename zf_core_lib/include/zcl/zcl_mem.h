#pragma once

#include <cstring>
#include <zcl/zcl_basic.h>

namespace zf {
    // ============================================================
    // @section: Types and Globals

    struct s_arena_block {
        void *buf;
        I32 buf_size;

        s_arena_block *next;
    };

    struct s_arena {
        s_arena_block *blocks_head;
        s_arena_block *block_cur;
        I32 block_cur_offs;
        I32 block_min_size;
    };

    template <typename tp_type>
    concept co_array_elem = Simple<tp_type> && (!Const<tp_type>);

    template <co_array_elem tp_type>
    struct s_array_rdonly {
        using t_elem = tp_type;

        const tp_type *raw;
        I32 len;

        // @todo: Consider replacing with explicit function for safety.
        constexpr const tp_type &operator[](const I32 index) const {
            ZF_REQUIRE(index >= 0 && index < len);
            return raw[index];
        }
    };

    template <co_array_elem tp_type>
    struct s_array_mut {
        using t_elem = tp_type;

        tp_type *raw;
        I32 len;

        constexpr tp_type &operator[](const I32 index) const {
            ZF_REQUIRE(index >= 0 && index < len);
            return raw[index];
        }

        constexpr operator s_array_rdonly<tp_type>() const {
            return {raw, len};
        }
    };

    template <co_array_elem tp_type, I32 tp_len>
    struct s_static_array {
        using t_elem = tp_type;

        static constexpr I32 g_len = tp_len;

        tp_type raw[tp_len];

        constexpr tp_type &operator[](const I32 index) {
            ZF_REQUIRE(index >= 0 && index < tp_len);
            return raw[index];
        }

        constexpr const tp_type &operator[](const I32 index) const {
            ZF_REQUIRE(index >= 0 && index < tp_len);
            return raw[index];
        }

        constexpr operator s_array_mut<tp_type>() {
            return {raw, g_len};
        }

        constexpr operator s_array_rdonly<tp_type>() const {
            return {raw, g_len};
        }
    };

    template <typename tp_type>
    concept co_array_mut = requires { typename tp_type::t_elem; } && co_same<CVRefRemoved<tp_type>, s_array_mut<typename tp_type::t_elem>>;

    template <typename tp_type>
    concept co_array_rdonly = requires { typename tp_type::t_elem; } && co_same<CVRefRemoved<tp_type>, s_array_rdonly<typename tp_type::t_elem>>;

    template <typename tp_type>
    concept co_array = co_array_mut<tp_type> || co_array_rdonly<tp_type>;

    template <co_array tp_arr_type>
    inline const t_comparator_bin<tp_arr_type> g_array_comparator_bin =
        [](const tp_arr_type &a, const tp_arr_type &b) {
            if (a.len != b.len) {
                return false;
            }

            for (I32 i = 0; i < a.len; i++) {
                if (a[i] != b[i]) {
                    return false;
                }
            }

            return true;
        };

    struct s_bit_vec_rdonly {
        const U8 *bytes_raw;
        I32 bit_cnt;

        s_array_rdonly<U8> Bytes() const {
            return {bytes_raw, get_bits_to_bytes(bit_cnt)};
        }
    };

    struct s_bit_vec_mut {
        U8 *bytes_raw;
        I32 bit_cnt;

        s_array_mut<U8> Bytes() const {
            return {bytes_raw, get_bits_to_bytes(bit_cnt)};
        }

        constexpr operator s_bit_vec_rdonly() const {
            return {bytes_raw, bit_cnt};
        }
    };

    template <I32 tp_bit_cnt>
    struct s_static_bit_vec {
        static constexpr I32 g_bit_cnt = tp_bit_cnt;

        s_static_array<U8, get_bits_to_bytes(tp_bit_cnt)> bytes;

        constexpr operator s_bit_vec_mut() {
            return {bytes.raw, g_bit_cnt};
        }

        constexpr operator s_bit_vec_rdonly() const {
            return {bytes.raw, g_bit_cnt};
        }
    };

    enum e_bitwise_mask_op : I32 {
        ek_bitwise_mask_op_and,
        ek_bitwise_mask_op_or,
        ek_bitwise_mask_op_xor,
        ek_bitwise_mask_op_andnot
    };

    // ============================================================


    // ============================================================
    // @section: Functions

    inline void Clear(void *const buf, const I32 buf_size, const U8 val) {
        ZF_ASSERT(buf_size >= 0);
        memset(buf, val, static_cast<size_t>(buf_size));
    }

    template <Simple tp_type>
    inline void ClearItem(tp_type *const item, const U8 val) {
        Clear(item, ZF_SIZE_OF(tp_type), val);
    }

    constexpr U8 g_poison_uninitted = 0xCD;
    constexpr U8 g_poison_freed = 0xDD;

#ifdef ZF_DEBUG
    inline void PoisonUnitted(void *const buf, const I32 buf_size) {
        Clear(buf, buf_size, g_poison_uninitted);
    }

    template <Simple tp_type>
    inline void PoisonUnittedItem(tp_type *const item) {
        ClearItem(item, g_poison_uninitted);
    }

    inline void PoisonFreed(void *const buf, const I32 buf_size) {
        Clear(buf, buf_size, g_poison_freed);
    }

    template <Simple tp_type>
    inline void PoisonFreedItem(tp_type *const item) {
        ClearItem(item, g_poison_freed);
    }
#else
    inline void PoisonUnitted(void *const buf, const I32 buf_size) {}
    template <co_simple tp_type> inline void PoisonUnittedItem(tp_type *const item) {}
    inline void PoisonFreed(void *const buf, const I32 buf_size) {}
    template <co_simple tp_type> inline void PoisonFreedItem(tp_type *const item) {}
#endif

    // Does not allocate any arena memory (blocks) upfront.
    inline s_arena CreateArena(const I32 block_min_size = get_megabytes_to_bytes(1)) {
        return {.block_min_size = block_min_size};
    }

    // Frees all arena memory. It is valid to call this even if no pushing was done.
    void ArenaDestroy(s_arena *const arena);

    // Will lazily allocate memory as needed. Allocation failure is treated as fatal and causes an abort - you don't need to check for nullptr.
    void *ArenaPush(s_arena *const arena, const I32 size, const I32 alignment);

    template <Simple tp_type>
    tp_type *ArenaPushItem(s_arena *const arena) {
        return static_cast<tp_type *>(ArenaPush(arena, ZF_SIZE_OF(tp_type), ZF_ALIGN_OF(tp_type)));
    }

    template <Simple tp_type>
    tp_type *mem_push_item_zeroed(s_arena *const arena) {
        const auto result = static_cast<tp_type *>(ArenaPush(arena, ZF_SIZE_OF(tp_type), ZF_ALIGN_OF(tp_type)));
        ClearItem(result, 0);
        return result;
    }

    template <Simple tp_type>
    s_array_mut<tp_type> ArenaPushArray(s_arena *const arena, const I32 len) {
        ZF_ASSERT(len >= 0);

        if (len == 0) {
            return {};
        }

        const auto raw = static_cast<tp_type *>(ArenaPush(arena, ZF_SIZE_OF(tp_type) * len, ZF_ALIGN_OF(tp_type)));
        return {raw, len};
    }

    template <Simple tp_type>
    s_array_mut<tp_type> ArenaPushArrayZeroed(s_arena *const arena, const I32 len) {
        ZF_ASSERT(len >= 0);

        if (len == 0) {
            return {};
        }

        const I32 size = ZF_SIZE_OF(tp_type) * len;
        const auto raw = static_cast<tp_type *>(ArenaPush(arena, size, ZF_ALIGN_OF(tp_type)));
        Clear(raw, size, 0);
        return {raw, len};
    }

    // Takes the arena offset to the beginning of its allocated memory (if any) to overwrite from there.
    void zf_mem_rewind_arena(s_arena *const arena);

    template <typename tp_type>
    s_array_rdonly<tp_type> ArraySlice(const s_array_rdonly<tp_type> arr, const I32 beg, const I32 end) {
        ZF_ASSERT(beg >= 0 && beg <= arr.len);
        ZF_ASSERT(end >= beg && end <= arr.len);

        return {arr.raw + beg, end - beg};
    }

    template <typename tp_type>
    s_array_rdonly<tp_type> ArraySliceFrom(const s_array_rdonly<tp_type> arr, const I32 beg) {
        ZF_ASSERT(beg >= 0 && beg <= arr.len);
        return {arr.raw + beg, arr.len - beg};
    }

    template <typename tp_type>
    s_array_mut<tp_type> ArraySlice(const s_array_mut<tp_type> arr, const I32 beg, const I32 end) {
        ZF_ASSERT(beg >= 0 && beg <= arr.len);
        ZF_ASSERT(end >= beg && end <= arr.len);

        return {arr.raw + beg, end - beg};
    }

    template <typename tp_type>
    s_array_mut<tp_type> ArraySliceFrom(const s_array_mut<tp_type> arr, const I32 beg) {
        ZF_ASSERT(beg >= 0 && beg <= arr.len);
        return {arr.raw + beg, arr.len - beg};
    }

    template <co_array tp_arr_type>
    I32 ArraySizeInBytes(const tp_arr_type arr) {
        return ZF_SIZE_OF(typename tp_arr_type::t_elem) * arr.len;
    }

    template <co_array tp_arr_type>
    auto CloneArray(const tp_arr_type arr_to_clone, s_arena *const arena) {
        const auto arr = ArenaPushArray<typename tp_arr_type::t_elem>(arena, arr_to_clone.len);
        CopyAll(arr, arr_to_clone);
        return arr;
    }

    template <typename tp_type, I32 tp_len>
    s_array_mut<tp_type> AsNonstatic(s_static_array<tp_type, tp_len> &arr) {
        return {arr.raw, arr.g_len};
    }

    template <typename tp_type, I32 tp_len>
    s_array_rdonly<tp_type> AsNonstatic(const s_static_array<tp_type, tp_len> &arr) {
        return {arr.raw, arr.g_len};
    }

    template <Simple tp_type>
    s_array_mut<U8> AsBytes(tp_type &val) {
        return {reinterpret_cast<U8 *>(&val), ZF_SIZE_OF(val)};
    }

    template <Simple tp_type>
    s_array_rdonly<U8> AsBytes(const tp_type &val) {
        return {reinterpret_cast<const U8 *>(&val), ZF_SIZE_OF(val)};
    }

    template <typename tp_type>
    s_array_mut<U8> AsByteArray(const s_array_mut<tp_type> arr) {
        return {reinterpret_cast<U8 *>(arr.raw), ArraySizeInBytes(arr)};
    }

    template <typename tp_type>
    s_array_rdonly<U8> AsByteArray(const s_array_rdonly<tp_type> arr) {
        return {reinterpret_cast<const U8 *>(arr.raw), ArraySizeInBytes(arr)};
    }

    // Creates a byte-sized bitmask with only a single bit set.
    inline U8 ByteBitmaskSingle(const I32 bit_index) {
        ZF_ASSERT(bit_index >= 0 && bit_index < 8);
        return static_cast<U8>(1 << bit_index);
    }

    // Creates a byte-sized bitmask with only bits in the range [begin_bit_index, end_bit_index) set.
    inline U8 ByteBitmaskRanged(const I32 begin_bit_index, const I32 end_bit_index = 8) {
        ZF_ASSERT(begin_bit_index >= 0 && begin_bit_index < 8);
        ZF_ASSERT(end_bit_index >= begin_bit_index && end_bit_index <= 8);

        if (end_bit_index - begin_bit_index == 8) {
            return 0xFF;
        }

        const auto bits_at_bottom = static_cast<U8>((1 << (end_bit_index - begin_bit_index)) - 1);
        return static_cast<U8>(bits_at_bottom << begin_bit_index);
    }

    inline s_bit_vec_mut CreateBitVector(const s_array_mut<U8> bytes) {
        return {bytes.raw, get_bytes_to_bits(bytes.len)};
    }

    inline s_bit_vec_mut CreateBitVector(const s_array_mut<U8> bytes, const I32 bit_cnt) {
        ZF_ASSERT(bit_cnt >= 0 && bit_cnt <= get_bytes_to_bits(bytes.len));
        return {bytes.raw, bit_cnt};
    }

    inline s_bit_vec_rdonly CreateBitVector(const s_array_rdonly<U8> bytes) {
        return {bytes.raw, get_bytes_to_bits(bytes.len)};
    }

    inline s_bit_vec_rdonly CreateBitVector(const s_array_rdonly<U8> bytes, const I32 bit_cnt) {
        ZF_ASSERT(bit_cnt >= 0 && bit_cnt <= get_bytes_to_bits(bytes.len));
        return {bytes.raw, bit_cnt};
    }

    inline s_bit_vec_mut CreateBitVector(const I32 bit_cnt, s_arena *const arena) {
        ZF_ASSERT(bit_cnt >= 0);
        return {ArenaPushArrayZeroed<U8>(arena, get_bits_to_bytes(bit_cnt)).raw, bit_cnt};
    }

    inline s_array_mut<U8> BitVectorBytes(const s_bit_vec_mut bv) {
        return {bv.bytes_raw, get_bits_to_bytes(bv.bit_cnt)};
    }

    inline s_array_rdonly<U8> BitVectorBytes(const s_bit_vec_rdonly bv) {
        return {bv.bytes_raw, get_bits_to_bytes(bv.bit_cnt)};
    }

    template <I32 tp_bit_cnt>
    s_bit_vec_mut AsNonstatic(s_static_bit_vec<tp_bit_cnt> &bv) {
        return {bv.bytes.raw, tp_bit_cnt};
    }

    template <I32 tp_bit_cnt>
    s_bit_vec_rdonly AsNonstatic(const s_static_bit_vec<tp_bit_cnt> &bv) {
        return {bv.bytes.raw, tp_bit_cnt};
    }

    inline I32 LastByteBitCount(const s_bit_vec_rdonly bv) {
        return ((bv.bit_cnt - 1) % 8) + 1;
    }

    // Gives a mask of the last byte in which only excess bits are unset.
    inline U8 LastByteMask(const s_bit_vec_rdonly bv) {
        return ByteBitmaskRanged(0, LastByteBitCount(bv));
    }

    inline B8 IsBitSet(const s_bit_vec_rdonly bv, const I32 index) {
        ZF_ASSERT(index >= 0 && index < bv.bit_cnt);
        return bv.Bytes()[index / 8] & ByteBitmaskSingle(index % 8);
    }

    inline void SetBit(const s_bit_vec_mut bv, const I32 index) {
        ZF_ASSERT(index >= 0 && index < bv.bit_cnt);
        bv.Bytes()[index / 8] |= ByteBitmaskSingle(index % 8);
    }

    inline void UnsetBit(const s_bit_vec_mut bv, const I32 index) {
        ZF_ASSERT(index >= 0 && index < bv.bit_cnt);
        bv.Bytes()[index / 8] &= ~ByteBitmaskSingle(index % 8);
    }

    B8 IsAnyBitSet(const s_bit_vec_rdonly bv);

    inline B8 AreAllBitsUnset(const s_bit_vec_rdonly bv) {
        return bv.bit_cnt > 0 && !IsAnyBitSet(bv);
    }

    B8 AreAllBitsSet(const s_bit_vec_rdonly bv);

    inline B8 IsAnyBitUnset(const s_bit_vec_rdonly bv) {
        return bv.bit_cnt > 0 && !AreAllBitsSet(bv);
    }

    void SetAllBits(const s_bit_vec_mut bv);
    void UnsetAllBits(const s_bit_vec_mut bv);

    // Sets all bits in the range [begin_bit_index, end_bit_index).
    void SetBitsInRange(const s_bit_vec_mut bv, const I32 begin_bit_index, const I32 end_bit_index);

    void ApplyMaskToBits(const s_bit_vec_mut targ, const s_bit_vec_rdonly mask, const e_bitwise_mask_op op);

    // Shifts left only by 1. Returns the discarded bit as 0 or 1.
    U8 ShiftBitsLeft(const s_bit_vec_mut bv);

    void ShiftBitsLeftBy(const s_bit_vec_mut bv, const I32 amount);

    void RotBitsLeftBy(const s_bit_vec_mut bv, const I32 amount);

    // Shifts right only by 1. Returns the carry bit.
    U8 ShiftBitsRight(const s_bit_vec_mut bv);

    void ShiftBitsRightBy(const s_bit_vec_mut bv, const I32 amount);

    void RotBitsRightBy(const s_bit_vec_mut bv, const I32 amount);

    // Returns -1 if all bits are unset.
    I32 FindIndexOfFirstSetBit(const s_bit_vec_rdonly bv, const I32 from = 0);

    // Returns -1 if all bits are set.
    I32 FindIndexOfFirstUnsetBit(const s_bit_vec_rdonly bv, const I32 from = 0);

    I32 CountSetBits(const s_bit_vec_rdonly bv);

    inline I32 CountUnsetBits(const s_bit_vec_rdonly bv) {
        return bv.bit_cnt - CountSetBits(bv);
    }

    // pos is the walker state, initialise it to the bit index you want to start from.
    // o_index is assigned the index of the set bit to process.
    // Returns false iff the walk is complete.
    B8 WalkSetBits(const s_bit_vec_rdonly bv, I32 *const pos, I32 *const o_index);

    // pos is the walker state, initialise it to the bit index you want to start from.
    // o_index is assigned the index of the unset bit to process.
    // Returns false iff the walk is complete.
    B8 WalkUnsetBits(const s_bit_vec_rdonly bv, I32 *const pos, I32 *const o_index);

#define ZF_WALK_SET_BITS(bv, index) for (I32 ZF_CONCAT(walk_pos_l, __LINE__) = 0, index; zf::WalkSetBits(bv, &ZF_CONCAT(walk_pos_l, __LINE__), &index);)
#define ZF_WALK_UNSET_BITS(bv, index) for (I32 ZF_CONCAT(walk_pos_l, __LINE__) = 0, index; zf::WalkUnsetBits(bv, &ZF_CONCAT(walk_pos_l, __LINE__), &index);)

    // ============================================================
}
