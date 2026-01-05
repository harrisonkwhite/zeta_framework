#pragma once

#include <zcl/zcl_basic.h>

namespace zf::mem {
    constexpr t_i32 f_kilobytes_to_bytes(const t_i32 n) { return (1 << 10) * n; }
    constexpr t_i32 f_megabytes_to_bytes(const t_i32 n) { return (1 << 20) * n; }
    constexpr t_i32 f_gigabytes_to_bytes(const t_i32 n) { return (1 << 30) * n; }
    constexpr t_i32 f_bits_to_bytes(const t_i32 n) { return (n + 7) / 8; }
    constexpr t_i32 f_bytes_to_bits(const t_i32 n) { return n * 8; }

    // Is n a power of 2?
    constexpr t_b8 f_is_alignment_valid(const t_i32 n) {
        return n > 0 && (n & (n - 1)) == 0;
    }

    // Take n up to the next multiple of the alignment.
    constexpr t_i32 f_align_forward(const t_i32 n, const t_i32 alignment) {
        ZF_ASSERT(f_is_alignment_valid(alignment));
        return (n + alignment - 1) & ~(alignment - 1);
    }

    template <c_simple tp_type>
    t_array_mut<t_u8> f_get_as_bytes(tp_type &val) {
        return {reinterpret_cast<t_u8 *>(&val), ZF_SIZE_OF(val)};
    }

    template <c_simple tp_type>
    t_array_rdonly<t_u8> f_get_as_bytes(const tp_type &val) {
        return {reinterpret_cast<const t_u8 *>(&val), ZF_SIZE_OF(val)};
    }

    template <c_array_elem tp_elem_type>
    t_array_mut<t_u8> f_get_array_as_byte_array(const t_array_mut<tp_elem_type> arr) {
        return {reinterpret_cast<t_u8 *>(arr.raw), f_array_get_size_in_bytes(arr)};
    }

    template <c_array_elem tp_elem_type>
    t_array_rdonly<t_u8> f_get_array_as_byte_array(const t_array_rdonly<tp_elem_type> arr) {
        return {reinterpret_cast<const t_u8 *>(arr.raw), f_array_get_size_in_bytes(arr)};
    }

    // Creates a byte-sized bitmask with only a single bit set.
    inline t_u8 f_make_byte_bitmask_single(const t_i32 bit_index) {
        ZF_ASSERT(bit_index >= 0 && bit_index < 8);
        return static_cast<t_u8>(1 << bit_index);
    }

    // Creates a byte-sized bitmask with only bits in the range [begin_bit_index, end_bit_index) set.
    inline t_u8 f_make_byte_bitmask_range(const t_i32 begin_bit_index, const t_i32 end_bit_index = 8) {
        ZF_ASSERT(begin_bit_index >= 0 && begin_bit_index < 8);
        ZF_ASSERT(end_bit_index >= begin_bit_index && end_bit_index <= 8);

        if (end_bit_index - begin_bit_index == 8) {
            return 0xFF;
        }

        const auto bits_at_bottom = static_cast<t_u8>((1 << (end_bit_index - begin_bit_index)) - 1);
        return static_cast<t_u8>(bits_at_bottom << begin_bit_index);
    }

    inline void f_clear(void *const buf, const t_i32 buf_size, const t_u8 val) {
        ZF_ASSERT(buf_size >= 0);
        memset(buf, val, static_cast<size_t>(buf_size));
    }

    template <c_simple tp_type>
    inline void f_clear_item(tp_type *const item, const t_u8 val) {
        f_clear(item, ZF_SIZE_OF(tp_type), val);
    }

#ifdef ZF_DEBUG
    constexpr t_u8 g_poison_uninitted = 0xCD;
    constexpr t_u8 g_poison_freed = 0xDD;

    inline void f_poison_uninitted(void *const buf, const t_i32 buf_size) {
        f_clear(buf, buf_size, g_poison_uninitted);
    }

    template <c_simple tp_type>
    inline void f_poison_uninitted_item(tp_type *const item) {
        f_clear_item(item, g_poison_uninitted);
    }

    inline void f_poison_freed(void *const buf, const t_i32 buf_size) {
        f_clear(buf, buf_size, g_poison_freed);
    }

    template <c_simple tp_type>
    inline void f_poison_freed_item(tp_type *const item) {
        f_clear_item(item, g_poison_freed);
    }
#else
    inline void f_poison_uninitted(void *const buf, const t_i32 buf_size) {}
    template <co_simple tp_type> inline void f_poison_uninitted_item(tp_type *const item) {}
    inline void f_poison_freed(void *const buf, const t_i32 buf_size) {}
    template <co_simple tp_type> inline void f_poison_freed_item(tp_type *const item) {}
#endif


    // ============================================================
    // @section: Arenas

    struct t_arena_block {
        void *buf;
        t_i32 buf_size;

        t_arena_block *next;
    };

    struct t_arena {
        t_arena_block *blocks_head;
        t_arena_block *block_cur;
        t_i32 block_cur_offs;
        t_i32 block_min_size;
    };

    // Does not allocate any arena memory (blocks) upfront.
    inline t_arena f_arena_create(const t_i32 block_min_size = f_megabytes_to_bytes(1)) {
        return {.block_min_size = block_min_size};
    }

    // Frees all arena memory. It is valid to call this even if no pushing was done.
    void f_arena_destroy(t_arena *const arena);

    // Will lazily allocate memory as needed. Allocation failure is treated as fatal and causes an abort - you don't need to check for nullptr.
    void *f_arena_push(t_arena *const arena, const t_i32 size, const t_i32 alignment);

    template <c_simple tp_type>
    tp_type *f_arena_push_item(t_arena *const arena) {
        return static_cast<tp_type *>(f_arena_push(arena, ZF_SIZE_OF(tp_type), ZF_ALIGN_OF(tp_type)));
    }

    template <c_simple tp_type>
    tp_type *f_arena_push_item_zeroed(t_arena *const arena) {
        const auto result = static_cast<tp_type *>(f_arena_push(arena, ZF_SIZE_OF(tp_type), ZF_ALIGN_OF(tp_type)));
        f_clear_item(result, 0);
        return result;
    }

    template <c_array_elem tp_elem_type>
    t_array_mut<tp_elem_type> f_arena_push_array(t_arena *const arena, const t_i32 len) {
        ZF_ASSERT(len >= 0);

        if (len == 0) {
            return {};
        }

        const auto raw = static_cast<tp_elem_type *>(f_arena_push(arena, ZF_SIZE_OF(tp_elem_type) * len, ZF_ALIGN_OF(tp_elem_type)));
        return {raw, len};
    }

    template <c_array_elem tp_elem_type>
    t_array_mut<tp_elem_type> f_arena_push_array_zeroed(t_arena *const arena, const t_i32 len) {
        ZF_ASSERT(len >= 0);

        if (len == 0) {
            return {};
        }

        const t_i32 size = ZF_SIZE_OF(tp_elem_type) * len;
        const auto raw = static_cast<tp_elem_type *>(f_arena_push(arena, size, ZF_ALIGN_OF(tp_elem_type)));
        f_clear(raw, size, 0);
        return {raw, len};
    }

    template <c_array tp_arr_type>
    auto f_arena_push_array_clone(t_arena *const arena, const tp_arr_type arr_to_clone) {
        const auto arr = f_arena_push_array<typename tp_arr_type::t_elem>(arena, arr_to_clone.len);
        f_array_copy(arr, arr_to_clone);
        return arr;
    }

    // Takes the arena offset to the beginning of its allocated memory (if any) to overwrite from there.
    void f_arena_rewind(t_arena *const arena);

    // ============================================================


    // ============================================================
    // @section: Bitsets

    struct t_bitset_rdonly {
        const t_u8 *bytes_raw;
        t_i32 bit_cnt;
    };

    struct t_bitset_mut {
        t_u8 *bytes_raw;
        t_i32 bit_cnt;

        constexpr operator t_bitset_rdonly() const {
            return {bytes_raw, bit_cnt};
        }
    };

    template <t_i32 tp_bit_cnt>
    struct t_static_bitset {
        static constexpr t_i32 g_bit_cnt = tp_bit_cnt;

        t_static_array<t_u8, f_bits_to_bytes(tp_bit_cnt)> bytes;

        constexpr operator t_bitset_mut() { return {bytes.raw, g_bit_cnt}; }
        constexpr operator t_bitset_rdonly() const { return {bytes.raw, g_bit_cnt}; }
    };

    inline t_bitset_mut f_bitset_create(const t_array_mut<t_u8> bytes) {
        return {bytes.raw, f_bytes_to_bits(bytes.len)};
    }

    inline t_bitset_mut f_bitset_create(const t_array_mut<t_u8> bytes, const t_i32 bit_cnt) {
        ZF_ASSERT(bit_cnt >= 0 && bit_cnt <= f_bytes_to_bits(bytes.len));
        return {bytes.raw, bit_cnt};
    }

    inline t_bitset_rdonly f_bitset_create(const t_array_rdonly<t_u8> bytes) {
        return {bytes.raw, f_bytes_to_bits(bytes.len)};
    }

    inline t_bitset_rdonly f_bitset_create(const t_array_rdonly<t_u8> bytes, const t_i32 bit_cnt) {
        ZF_ASSERT(bit_cnt >= 0 && bit_cnt <= f_bytes_to_bits(bytes.len));
        return {bytes.raw, bit_cnt};
    }

    inline t_bitset_mut f_bitset_create(const t_i32 bit_cnt, t_arena *const arena) {
        ZF_ASSERT(bit_cnt >= 0);
        return {f_arena_push_array_zeroed<t_u8>(arena, f_bits_to_bytes(bit_cnt)).raw, bit_cnt};
    }

    inline t_array_mut<t_u8> f_bitset_get_bytes(const t_bitset_mut bs) {
        return {bs.bytes_raw, f_bits_to_bytes(bs.bit_cnt)};
    }

    inline t_array_rdonly<t_u8> f_bitset_get_bytes(const t_bitset_rdonly bs) {
        return {bs.bytes_raw, f_bits_to_bytes(bs.bit_cnt)};
    }

    inline t_i32 f_bitset_get_last_byte_bit_cnt(const t_bitset_rdonly bs) {
        return ((bs.bit_cnt - 1) % 8) + 1;
    }

    // Gives a mask of the last byte in which only excess bits are unset.
    inline t_u8 f_bitset_get_last_byte_mask(const t_bitset_rdonly bs) {
        return f_make_byte_bitmask_range(0, f_bitset_get_last_byte_bit_cnt(bs));
    }

    // ============================================================


    inline t_b8 f_is_bit_set(const t_bitset_rdonly bs, const t_i32 index) {
        ZF_ASSERT(index >= 0 && index < bs.bit_cnt);
        return f_bitset_get_bytes(bs)[index / 8] & f_make_byte_bitmask_single(index % 8);
    }

    inline void f_set_bit(const t_bitset_mut bs, const t_i32 index) {
        ZF_ASSERT(index >= 0 && index < bs.bit_cnt);
        f_bitset_get_bytes(bs)[index / 8] |= f_make_byte_bitmask_single(index % 8);
    }

    inline void f_unset_bit(const t_bitset_mut bs, const t_i32 index) {
        ZF_ASSERT(index >= 0 && index < bs.bit_cnt);
        f_bitset_get_bytes(bs)[index / 8] &= ~f_make_byte_bitmask_single(index % 8);
    }

    t_b8 f_is_any_bit_set(const t_bitset_rdonly bs);

    inline t_b8 f_are_all_bits_unset(const t_bitset_rdonly bs) {
        return bs.bit_cnt > 0 && !f_is_any_bit_set(bs);
    }

    t_b8 f_are_all_bits_set(const t_bitset_rdonly bs);

    inline t_b8 f_is_any_bit_unset(const t_bitset_rdonly bs) {
        return bs.bit_cnt > 0 && !f_are_all_bits_set(bs);
    }

    void f_set_all_bits(const t_bitset_mut bs);
    void f_unset_all_bits(const t_bitset_mut bs);

    // Sets all bits in the range [begin_bit_index, end_bit_index).
    void f_set_bits_in_range(const t_bitset_mut bs, const t_i32 begin_bit_index, const t_i32 end_bit_index);

    enum t_bitwise_mask_op : t_i32 {
        ec_bitwise_mask_op_and,
        ec_bitwise_mask_op_or,
        ec_bitwise_mask_op_xor,
        ec_bitwise_mask_op_andnot
    };

    void f_apply_mask_to_bits(const t_bitset_mut targ, const t_bitset_rdonly mask, const t_bitwise_mask_op op);

    void f_shift_bits_left_by(const t_bitset_mut bs, const t_i32 amount = 1);
    void f_rot_bits_left_by(const t_bitset_mut bs, const t_i32 amount = 1);

    void f_shift_bits_right_by(const t_bitset_mut bs, const t_i32 amount = 1);
    void f_rot_bits_right_by(const t_bitset_mut bs, const t_i32 amount = 1);

    // Returns -1 if all bits are unset.
    t_i32 f_get_index_of_first_set_bit(const t_bitset_rdonly bs, const t_i32 from = 0);

    // Returns -1 if all bits are set.
    t_i32 f_get_index_of_first_unset_bit(const t_bitset_rdonly bs, const t_i32 from = 0);

    t_i32 f_count_set_bits(const t_bitset_rdonly bs);

    inline t_i32 f_count_unset_bits(const t_bitset_rdonly bs) {
        return bs.bit_cnt - f_count_set_bits(bs);
    }

    // pos is the walker state, initialise it to the bit index you want to start from.
    // o_index is assigned the index of the set bit to process.
    // Returns false iff the walk is complete.
    t_b8 f_walk_set_bits(const t_bitset_rdonly bs, t_i32 *const pos, t_i32 *const o_index);

    // pos is the walker state, initialise it to the bit index you want to start from.
    // o_index is assigned the index of the unset bit to process.
    // Returns false iff the walk is complete.
    t_b8 f_walk_unset_bits(const t_bitset_rdonly bs, t_i32 *const pos, t_i32 *const o_index);

#define ZF_WALK_SET_BITS(bs, index) for (t_i32 ZF_CONCAT(walk_pos_l, __LINE__) = 0, index; zf::mem::f_walk_set_bits(bs, &ZF_CONCAT(walk_pos_l, __LINE__), &index);)
#define ZF_WALK_UNSET_BITS(bs, index) for (t_i32 ZF_CONCAT(walk_pos_l, __LINE__) = 0, index; zf::mem::f_walk_unset_bits(bs, &ZF_CONCAT(walk_pos_l, __LINE__), &index);)
}
