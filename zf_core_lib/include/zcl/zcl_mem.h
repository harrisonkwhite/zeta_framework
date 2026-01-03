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
    concept co_array_elem = co_simple<tp_type> && (!co_const<tp_type>);

    template <co_array_elem tp_type>
    struct s_array_rdonly {
        using t_elem = tp_type;

        const tp_type *raw;
        t_i32 len;

        s_array_rdonly() = default;
        s_array_rdonly(const tp_type *const raw, const t_i32 len) : raw(raw), len(len) {}

        t_i32 SizeInBytes() const {
            return ZF_SIZE_OF(tp_type) * len;
        }

        // @todo: Consider replacing with explicit function for safety.
        const tp_type &operator[](const t_i32 index) const {
            ZF_REQUIRE(index >= 0 && index < len);
            return raw[index];
        }

        s_array_rdonly<t_u8> AsByteArray() const {
            return {reinterpret_cast<const t_u8 *>(raw), SizeInBytes()};
        }
    };

    template <co_array_elem tp_type>
    struct s_array_mut {
        using t_elem = tp_type;

        tp_type *raw;
        t_i32 len;

        s_array_mut() = default;
        s_array_mut(tp_type *const raw, const t_i32 len) : raw(raw), len(len) {}

        t_i32 SizeInBytes() const {
            return ZF_SIZE_OF(tp_type) * len;
        }

        operator s_array_rdonly<tp_type>() const {
            return {raw, len};
        }

        tp_type &operator[](const t_i32 index) const {
            ZF_REQUIRE(index >= 0 && index < len);
            return raw[index];
        }

        s_array_mut<t_u8> AsByteArray() const {
            return {reinterpret_cast<t_u8 *>(raw), SizeInBytes()};
        }
    };

    template <co_array_elem tp_type, t_i32 tp_len>
    struct s_static_array {
        using t_elem = tp_type;

        static constexpr t_i32 g_len = tp_len;

        tp_type raw[tp_len];

        operator s_array_mut<tp_type>() {
            return {raw, tp_len};
        }

        operator s_array_rdonly<tp_type>() const {
            return {raw, tp_len};
        }

        s_array_mut<tp_type> AsNonstatic() {
            return {raw, tp_len};
        }

        s_array_rdonly<tp_type> AsNonstatic() const {
            return {raw, tp_len};
        }

        tp_type &operator[](const t_i32 index) {
            ZF_REQUIRE(index >= 0 && index < tp_len);
            return raw[index];
        }

        const tp_type &operator[](const t_i32 index) const {
            ZF_REQUIRE(index >= 0 && index < tp_len);
            return raw[index];
        }
    };

    template <typename tp_type>
    concept co_array_mut = requires { typename tp_type::t_elem; } && co_same<t_cvref_removed<tp_type>, s_array_mut<typename tp_type::t_elem>>;

    template <typename tp_type>
    concept co_array_rdonly = requires { typename tp_type::t_elem; } && co_same<t_cvref_removed<tp_type>, s_array_rdonly<typename tp_type::t_elem>>;

    template <typename tp_type>
    concept co_array = co_array_mut<tp_type> || co_array_rdonly<tp_type>;

    template <co_array tp_arr_type>
    inline const t_comparator_bin<tp_arr_type> g_array_comparator_bin =
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

        s_bit_vec_rdonly() = default;
        s_bit_vec_rdonly(const t_u8 *const bytes_raw, const t_i32 bit_cnt) : bytes_raw(bytes_raw), bit_cnt(bit_cnt) {}
        s_bit_vec_rdonly(const s_array_rdonly<t_u8> bytes) : bytes_raw(bytes.raw), bit_cnt(BytesToBits(bytes.len)) {}

        s_array_rdonly<t_u8> Bytes() const {
            return {bytes_raw, BitsToBytes(bit_cnt)};
        }
    };

    struct s_bit_vec_mut {
        t_u8 *bytes_raw;
        t_i32 bit_cnt;

        s_bit_vec_mut() = default;
        s_bit_vec_mut(t_u8 *const bytes_raw, const t_i32 bit_cnt) : bytes_raw(bytes_raw), bit_cnt(bit_cnt) {}
        s_bit_vec_mut(const s_array_mut<t_u8> bytes) : bytes_raw(bytes.raw), bit_cnt(BytesToBits(bytes.len)) {}

        s_array_mut<t_u8> Bytes() const {
            return {bytes_raw, BitsToBytes(bit_cnt)};
        }

        operator s_bit_vec_rdonly() const {
            return {bytes_raw, bit_cnt};
        }
    };

    template <t_i32 tp_bit_cnt>
    struct s_static_bit_vec {
        static constexpr t_i32 g_bit_cnt = tp_bit_cnt;

        s_static_array<t_u8, BitsToBytes(tp_bit_cnt)> bytes;

        operator s_bit_vec_mut() {
            return {bytes.raw, tp_bit_cnt};
        }

        operator s_bit_vec_rdonly() const {
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

    template <co_simple tp_type>
    inline void ClearItem(tp_type *const item, const t_u8 val) {
        Clear(item, ZF_SIZE_OF(tp_type), val);
    }

    constexpr t_u8 g_poison_uninitted = 0xCD;
    constexpr t_u8 g_poison_freed = 0xDD;

#ifdef ZF_DEBUG
    inline void PoisonUnitted(void *const buf, const t_i32 buf_size) {
        Clear(buf, buf_size, g_poison_uninitted);
    }

    template <co_simple tp_type>
    inline void PoisonUnittedItem(tp_type *const item) {
        ClearItem(item, g_poison_uninitted);
    }

    inline void PoisonFreed(void *const buf, const t_i32 buf_size) {
        Clear(buf, buf_size, g_poison_freed);
    }

    template <co_simple tp_type>
    inline void PoisonFreedItem(tp_type *const item) {
        ClearItem(item, g_poison_freed);
    }
#else
    inline void PoisonUnitted(void *const buf, const t_i32 buf_size) {}
    template <co_simple tp_type> inline void PoisonUnittedItem(tp_type *const item) {}
    inline void PoisonFreed(void *const buf, const t_i32 buf_size) {}
    template <co_simple tp_type> inline void PoisonFreedItem(tp_type *const item) {}
#endif

    // Does not allocate any arena memory (blocks) upfront.
    inline s_arena CreateArena(const t_i32 block_min_size = MegabytesToBytes(1)) {
        return {.block_min_size = block_min_size};
    }

    // Frees all arena memory. It is valid to call this even if no pushing was done.
    void Destroy(s_arena *const arena);

    // Will lazily allocate memory as needed. Allocation failure is treated as fatal and causes an abort - you don't need to check for nullptr.
    void *Push(s_arena *const arena, const t_i32 size, const t_i32 alignment);

    template <co_simple tp_type>
    tp_type *PushItem(s_arena *const arena) {
        return static_cast<tp_type *>(Push(arena, ZF_SIZE_OF(tp_type), ZF_ALIGN_OF(tp_type)));
    }

    template <co_simple tp_type>
    tp_type *PushItemZeroed(s_arena *const arena) {
        const auto result = static_cast<tp_type *>(Push(arena, ZF_SIZE_OF(tp_type), ZF_ALIGN_OF(tp_type)));
        ClearItem(result, 0);
        return result;
    }

    template <co_simple tp_type>
    s_array_mut<tp_type> PushArray(s_arena *const arena, const t_i32 len) {
        ZF_ASSERT(len >= 0);

        if (len == 0) {
            return {};
        }

        const auto raw = static_cast<tp_type *>(Push(arena, ZF_SIZE_OF(tp_type) * len, ZF_ALIGN_OF(tp_type)));
        return {raw, len};
    }

    template <co_simple tp_type>
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
    s_array_rdonly<tp_type> Slice(const s_array_rdonly<tp_type> arr, const t_i32 beg, const t_i32 end) {
        ZF_ASSERT(beg >= 0 && beg <= arr.len);
        ZF_ASSERT(end >= beg && end <= arr.len);

        return {arr.raw + beg, end - beg};
    }

    template <typename tp_type>
    s_array_rdonly<tp_type> SliceFrom(const s_array_rdonly<tp_type> arr, const t_i32 beg) {
        ZF_ASSERT(beg >= 0 && beg <= arr.len);
        return {arr.raw + beg, arr.len - beg};
    }

    template <typename tp_type>
    s_array_mut<tp_type> Slice(const s_array_mut<tp_type> arr, const t_i32 beg, const t_i32 end) {
        ZF_ASSERT(beg >= 0 && beg <= arr.len);
        ZF_ASSERT(end >= beg && end <= arr.len);

        return {arr.raw + beg, end - beg};
    }

    template <typename tp_type>
    s_array_mut<tp_type> SliceFrom(const s_array_mut<tp_type> arr, const t_i32 beg) {
        ZF_ASSERT(beg >= 0 && beg <= arr.len);
        return {arr.raw + beg, arr.len - beg};
    }

    template <co_array tp_arr_type>
    auto CloneArray(const tp_arr_type arr_to_clone, s_arena *const arena) {
        const auto arr = PushArray<typename tp_arr_type::t_elem>(arena, arr_to_clone.len);
        CopyAll(arr, arr_to_clone);
        return arr;
    }

    template <co_simple tp_type>
    s_array_mut<t_u8> AsBytes(tp_type &val) {
        return {reinterpret_cast<t_u8 *>(&val), ZF_SIZE_OF(val)};
    }

    template <co_simple tp_type>
    s_array_rdonly<t_u8> AsBytes(const tp_type &val) {
        return {reinterpret_cast<const t_u8 *>(&val), ZF_SIZE_OF(val)};
    }

    // Creates a byte-sized bitmask with only a single bit set.
    inline t_u8 ByteBitmaskSingle(const t_i32 bit_index) {
        ZF_ASSERT(bit_index >= 0 && bit_index < 8);
        return static_cast<t_u8>(1 << bit_index);
    }

    // Creates a byte-sized bitmask with only bits in the range [begin_bit_index, end_bit_index) set.
    inline t_u8 ByteBitmaskRanged(const t_i32 begin_bit_index, const t_i32 end_bit_index = 8) {
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

    inline t_i32 LastByteBitCount(const s_bit_vec_rdonly bv) {
        return ((bv.bit_cnt - 1) % 8) + 1;
    }

    // Gives a mask of the last byte in which only excess bits are unset.
    inline t_u8 LastByteMask(const s_bit_vec_rdonly bv) {
        return ByteBitmaskRanged(0, LastByteBitCount(bv));
    }

    inline t_b8 IsBitSet(const s_bit_vec_rdonly bv, const t_i32 index) {
        ZF_ASSERT(index >= 0 && index < bv.bit_cnt);
        return bv.Bytes()[index / 8] & ByteBitmaskSingle(index % 8);
    }

    inline void SetBit(const s_bit_vec_mut bv, const t_i32 index) {
        ZF_ASSERT(index >= 0 && index < bv.bit_cnt);
        bv.Bytes()[index / 8] |= ByteBitmaskSingle(index % 8);
    }

    inline void UnsetBit(const s_bit_vec_mut bv, const t_i32 index) {
        ZF_ASSERT(index >= 0 && index < bv.bit_cnt);
        bv.Bytes()[index / 8] &= ~ByteBitmaskSingle(index % 8);
    }

    t_b8 IsAnyBitSet(const s_bit_vec_rdonly bv);

    inline t_b8 AreAllBitsUnset(const s_bit_vec_rdonly bv) {
        return bv.bit_cnt > 0 && !IsAnyBitSet(bv);
    }

    t_b8 AreAllBitsSet(const s_bit_vec_rdonly bv);

    inline t_b8 IsAnyBitUnset(const s_bit_vec_rdonly bv) {
        return bv.bit_cnt > 0 && !AreAllBitsSet(bv);
    }

    void SetAllBits(const s_bit_vec_mut bv);
    void UnsetAllBits(const s_bit_vec_mut bv);

    // Sets all bits in the range [begin_bit_index, end_bit_index).
    void SetBitsInRange(const s_bit_vec_mut bv, const t_i32 begin_bit_index, const t_i32 end_bit_index);

    void ApplyMaskToBits(const s_bit_vec_mut targ, const s_bit_vec_rdonly mask, const e_bitwise_mask_op op);

    // Shifts left only by 1. Returns the discarded bit as 0 or 1.
    t_u8 ShiftBitsLeft(const s_bit_vec_mut bv);

    void ShiftBitsLeftBy(const s_bit_vec_mut bv, const t_i32 amount);

    void RotBitsLeftBy(const s_bit_vec_mut bv, const t_i32 amount);

    // Shifts right only by 1. Returns the carry bit.
    t_u8 ShiftBitsRight(const s_bit_vec_mut bv);

    void ShiftBitsRightBy(const s_bit_vec_mut bv, const t_i32 amount);

    void RotBitsRightBy(const s_bit_vec_mut bv, const t_i32 amount);

    // Returns -1 if all bits are unset.
    t_i32 FindIndexOfFirstSetBit(const s_bit_vec_rdonly bv, const t_i32 from = 0);

    // Returns -1 if all bits are set.
    t_i32 FindIndexOfFirstUnsetBit(const s_bit_vec_rdonly bv, const t_i32 from = 0);

    t_i32 CountSetBits(const s_bit_vec_rdonly bv);

    inline t_i32 CountUnsetBits(const s_bit_vec_rdonly bv) {
        return bv.bit_cnt - CountSetBits(bv);
    }

    // pos is the walker state, initialise it to the bit index you want to start from.
    // o_index is assigned the index of the set bit to process.
    // Returns false iff the walk is complete.
    t_b8 WalkSetBits(const s_bit_vec_rdonly bv, t_i32 *const pos, t_i32 *const o_index);

    // pos is the walker state, initialise it to the bit index you want to start from.
    // o_index is assigned the index of the unset bit to process.
    // Returns false iff the walk is complete.
    t_b8 WalkUnsetBits(const s_bit_vec_rdonly bv, t_i32 *const pos, t_i32 *const o_index);

#define ZF_WALK_SET_BITS(bv, index) for (t_i32 ZF_CONCAT(walk_pos_l, __LINE__) = 0, index; WalkSetBits(bv, &ZF_CONCAT(walk_pos_l, __LINE__), &index);)
#define ZF_WALK_UNSET_BITS(bv, index) for (t_i32 ZF_CONCAT(walk_pos_l, __LINE__) = 0, index; WalkUnsetBits(bv, &ZF_CONCAT(walk_pos_l, __LINE__), &index);)

    // ============================================================
}
