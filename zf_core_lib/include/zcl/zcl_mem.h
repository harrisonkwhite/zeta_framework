#pragma once

#include <zcl/zcl_basic.h>

namespace zf::mem {
    constexpr t_i32 kilobytes_to_bytes(const t_i32 n) { return (1 << 10) * n; }
    constexpr t_i32 megabytes_to_bytes(const t_i32 n) { return (1 << 20) * n; }
    constexpr t_i32 gigabytes_to_bytes(const t_i32 n) { return (1 << 30) * n; }
    constexpr t_i32 bits_to_bytes(const t_i32 n) { return (n + 7) / 8; }
    constexpr t_i32 bytes_to_bits(const t_i32 n) { return n * 8; }

    // Is n a power of 2?
    constexpr t_b8 alignment_check_valid(const t_i32 n) {
        return n > 0 && (n & (n - 1)) == 0;
    }

    // Take n up to the next multiple of the alignment.
    constexpr t_i32 align_forward(const t_i32 n, const t_i32 alignment) {
        ZF_ASSERT(alignment_check_valid(alignment));
        return (n + alignment - 1) & ~(alignment - 1);
    }

    template <c_simple tp_type>
    t_array_mut<t_u8> to_bytes(tp_type &val) {
        return {reinterpret_cast<t_u8 *>(&val), ZF_SIZE_OF(val)};
    }

    template <c_simple tp_type>
    t_array_rdonly<t_u8> to_bytes(const tp_type &val) {
        return {reinterpret_cast<const t_u8 *>(&val), ZF_SIZE_OF(val)};
    }

    template <c_array_elem tp_elem_type>
    t_array_mut<t_u8> array_to_byte_array(const t_array_mut<tp_elem_type> arr) {
        return {reinterpret_cast<t_u8 *>(arr.raw), array_get_size_in_bytes(arr)};
    }

    template <c_array_elem tp_elem_type>
    t_array_rdonly<t_u8> array_to_byte_array(const t_array_rdonly<tp_elem_type> arr) {
        return {reinterpret_cast<const t_u8 *>(arr.raw), array_get_size_in_bytes(arr)};
    }

    // Creates a byte-sized bitmask with only a single bit set.
    constexpr t_u8 byte_bitmask_create_single(const t_i32 bit_index) {
        ZF_ASSERT(bit_index >= 0 && bit_index < 8);
        return static_cast<t_u8>(1 << bit_index);
    }

    // Creates a byte-sized bitmask with only bits in the range [begin_bit_index, end_bit_index) set.
    constexpr t_u8 byte_bitmask_create_range(const t_i32 begin_bit_index, const t_i32 end_bit_index = 8) {
        ZF_ASSERT(begin_bit_index >= 0 && begin_bit_index < 8);
        ZF_ASSERT(end_bit_index >= begin_bit_index && end_bit_index <= 8);

        if (end_bit_index - begin_bit_index == 8) {
            return 0xFF;
        }

        const auto bits_at_bottom = static_cast<t_u8>((1 << (end_bit_index - begin_bit_index)) - 1);
        return static_cast<t_u8>(bits_at_bottom << begin_bit_index);
    }

    inline void zero_clear(void *const buf, const t_i32 buf_size) {
        ZF_ASSERT(buf_size >= 0);
        memset(buf, 0, static_cast<size_t>(buf_size));
    }

    template <c_simple tp_type>
    void zero_clear_item(tp_type *const item) {
        zero_clear(item, ZF_SIZE_OF(tp_type));
    }

    constexpr t_b8 zero_check(void *const buf, const t_i32 buf_size) {
        ZF_ASSERT(buf_size >= 0);

        const auto buf_bytes = static_cast<t_u8 *>(buf);

        for (t_i32 i = 0; i < buf_size; i++) {
            if (buf_bytes[i]) {
                return false;
            }
        }

        return true;
    }

    template <c_simple tp_type>
    constexpr t_b8 zero_check_item(tp_type *const item) {
        return zero_check(item, ZF_SIZE_OF(tp_type));
    }


    // ============================================================
    // @section: Arenas

    struct t_arena_block {
        void *buf;
        t_i32 buf_size;

        t_arena_block *next;
    };

    enum t_arena_type : t_i32 {
        ec_arena_type_invalid,
        ec_arena_type_blockbased, // Owns its memory, which is organised as a linked list of dynamically allocated blocks. New blocks are allocated as needed. @todo: Probably should not expose implementation details.
        ec_arena_type_wrapping    // Non-owning and non-reallocating. Useful if you want to leverage a stack-allocated buffer for example. @todo: Probably not a good name.
    };

    struct t_arena {
        t_arena_type type;

        union {
            struct {
                t_arena_block *blocks_head;
                t_arena_block *block_cur;
                t_i32 block_cur_offs;
                t_i32 block_min_size;
            } blockbased;

            struct {
                void *buf;
                t_i32 buf_size;
                t_i32 buf_offs;
            } wrapping;
        } type_data;
    };

    // Does not allocate any arena memory (blocks) upfront.
    inline t_arena arena_create_blockbased(const t_i32 block_min_size = megabytes_to_bytes(1)) {
        ZF_ASSERT(block_min_size > 0);

        return {
            .type = ec_arena_type_blockbased,
            .type_data = {.blockbased = {.block_min_size = block_min_size}},
        };
    }

    inline t_arena arena_create_wrapping(const t_array_mut<t_u8> bytes) {
        array_set_all_to(bytes, 0);

        return {
            .type = ec_arena_type_wrapping,
            .type_data = {.wrapping = {.buf = bytes.raw, .buf_size = bytes.len}},
        };
    }

    // Frees all arena memory. Only valid for block-based arenas. This can be called even if no pushing was done.
    void arena_destroy(t_arena *const arena);

    // Will lazily allocate memory as needed. Allocation failure is treated as fatal and causes an abort - you don't need to check for nullptr.
    // Returned buffer is guaranteed to be zeroed.
    void *arena_push(t_arena *const arena, const t_i32 size, const t_i32 alignment);

    // Will lazily allocate memory as needed. Allocation failure is treated as fatal and causes an abort - you don't need to check for nullptr.
    // Returned item is guaranteed to be zeroed.
    template <c_simple tp_type>
    tp_type *arena_push_item(t_arena *const arena) {
        return static_cast<tp_type *>(arena_push(arena, ZF_SIZE_OF(tp_type), ZF_ALIGN_OF(tp_type)));
    }

    template <c_array_elem tp_elem_type>
    t_array_mut<tp_elem_type> arena_push_array(t_arena *const arena, const t_i32 len) {
        ZF_ASSERT(len >= 0);

        if (len == 0) {
            return {};
        }

        const t_i32 size = ZF_SIZE_OF(tp_elem_type) * len;
        return {static_cast<tp_elem_type *>(arena_push(arena, size, ZF_ALIGN_OF(tp_elem_type))), len};
    }

    template <c_array tp_arr_type>
    auto arena_push_array_clone(t_arena *const arena, const tp_arr_type arr_to_clone) {
        const auto arr = arena_push_array<typename tp_arr_type::t_elem>(arena, arr_to_clone.len);
        array_copy(arr, arr_to_clone);
        return arr;
    }

    // Takes the arena offset to the beginning of its memory (if any) to overwrite from there.
    void arena_rewind(t_arena *const arena);

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
        static constexpr t_i32 k_bit_cnt = tp_bit_cnt;

        t_static_array<t_u8, bits_to_bytes(tp_bit_cnt)> bytes;

        constexpr operator t_bitset_mut() { return {bytes.raw, k_bit_cnt}; }
        constexpr operator t_bitset_rdonly() const { return {bytes.raw, k_bit_cnt}; }
    };

    constexpr t_bitset_mut bitset_create(const t_array_mut<t_u8> bytes) {
        return {bytes.raw, bytes_to_bits(bytes.len)};
    }

    constexpr t_bitset_mut bitset_create(const t_array_mut<t_u8> bytes, const t_i32 bit_cnt) {
        ZF_ASSERT(bit_cnt >= 0 && bit_cnt <= bytes_to_bits(bytes.len));
        return {bytes.raw, bit_cnt};
    }

    constexpr t_bitset_rdonly bitset_create(const t_array_rdonly<t_u8> bytes) {
        return {bytes.raw, bytes_to_bits(bytes.len)};
    }

    constexpr t_bitset_rdonly bitset_create(const t_array_rdonly<t_u8> bytes, const t_i32 bit_cnt) {
        ZF_ASSERT(bit_cnt >= 0 && bit_cnt <= bytes_to_bits(bytes.len));
        return {bytes.raw, bit_cnt};
    }

    inline t_bitset_mut bitset_create(const t_i32 bit_cnt, t_arena *const arena) {
        ZF_ASSERT(bit_cnt >= 0);
        return {arena_push_array<t_u8>(arena, bits_to_bytes(bit_cnt)).raw, bit_cnt};
    }

    constexpr t_array_mut<t_u8> bitset_get_bytes(const t_bitset_mut bs) {
        return {bs.bytes_raw, bits_to_bytes(bs.bit_cnt)};
    }

    constexpr t_array_rdonly<t_u8> bitset_get_bytes(const t_bitset_rdonly bs) {
        return {bs.bytes_raw, bits_to_bytes(bs.bit_cnt)};
    }

    constexpr t_i32 bitset_get_last_byte_bit_cnt(const t_bitset_rdonly bs) {
        return ((bs.bit_cnt - 1) % 8) + 1;
    }

    // Gives a mask of the last byte in which only excess bits are unset.
    constexpr t_u8 bitset_get_last_byte_mask(const t_bitset_rdonly bs) {
        return byte_bitmask_create_range(0, bitset_get_last_byte_bit_cnt(bs));
    }

    constexpr t_b8 bitset_check_set(const t_bitset_rdonly bs, const t_i32 index) {
        ZF_ASSERT(index >= 0 && index < bs.bit_cnt);
        return bitset_get_bytes(bs)[index / 8] & byte_bitmask_create_single(index % 8);
    }

    constexpr void bitset_set(const t_bitset_mut bs, const t_i32 index) {
        ZF_ASSERT(index >= 0 && index < bs.bit_cnt);
        bitset_get_bytes(bs)[index / 8] |= byte_bitmask_create_single(index % 8);
    }

    constexpr void bitset_unset(const t_bitset_mut bs, const t_i32 index) {
        ZF_ASSERT(index >= 0 && index < bs.bit_cnt);
        bitset_get_bytes(bs)[index / 8] &= ~byte_bitmask_create_single(index % 8);
    }

    t_b8 bitset_check_any_set(const t_bitset_rdonly bs);

    inline t_b8 bitset_check_all_unset(const t_bitset_rdonly bs) {
        return bs.bit_cnt > 0 && !bitset_check_any_set(bs);
    }

    t_b8 bitset_check_all_set(const t_bitset_rdonly bs);

    inline t_b8 bitset_check_any_unset(const t_bitset_rdonly bs) {
        return bs.bit_cnt > 0 && !bitset_check_all_set(bs);
    }

    void bitset_set_all(const t_bitset_mut bs);
    void bitset_unset_all(const t_bitset_mut bs);

    // Sets all bits in the range [begin_bit_index, end_bit_index).
    void bitset_set_range(const t_bitset_mut bs, const t_i32 begin_bit_index, const t_i32 end_bit_index);

    enum t_bitwise_mask_op : t_i32 {
        ec_bitwise_mask_op_and,
        ec_bitwise_mask_op_or,
        ec_bitwise_mask_op_xor,
        ec_bitwise_mask_op_andnot
    };

    void bitset_apply_mask(const t_bitset_mut bs, const t_bitset_rdonly mask, const t_bitwise_mask_op op);

    void bitset_shift_left(const t_bitset_mut bs, const t_i32 amount = 1);
    void bitset_rot_left(const t_bitset_mut bs, const t_i32 amount = 1);

    void bitset_shift_right(const t_bitset_mut bs, const t_i32 amount = 1);
    void bitset_rot_right(const t_bitset_mut bs, const t_i32 amount = 1);

    // Returns the index of the found set bit, or -1 if all bits are unset.
    t_i32 bitset_find_first_set_bit(const t_bitset_rdonly bs, const t_i32 from = 0);

    // Returns the index of the found unset bit, or -1 if all bits are unset.
    t_i32 bitset_find_first_unset_bit(const t_bitset_rdonly bs, const t_i32 from = 0);

    t_i32 bitset_count_set(const t_bitset_rdonly bs);

    inline t_i32 bitset_count_unset(const t_bitset_rdonly bs) {
        return bs.bit_cnt - bitset_count_set(bs);
    }

    // pos is the walker state, initialise it to the bit index you want to start from.
    // o_index is assigned the index of the set bit to process.
    // Returns false iff the walk is complete.
    t_b8 bitset_walk_all_set(const t_bitset_rdonly bs, t_i32 *const pos, t_i32 *const o_index);

    // pos is the walker state, initialise it to the bit index you want to start from.
    // o_index is assigned the index of the unset bit to process.
    // Returns false iff the walk is complete.
    t_b8 bitset_walk_all_unset(const t_bitset_rdonly bs, t_i32 *const pos, t_i32 *const o_index);

#define ZF_WALK_SET_BITS(bs, index) for (t_i32 ZF_CONCAT(walk_pos_l, __LINE__) = 0, index; zf::mem::bitset_walk_all_set(bs, &ZF_CONCAT(walk_pos_l, __LINE__), &index);)
#define ZF_WALK_UNSET_BITS(bs, index) for (t_i32 ZF_CONCAT(walk_pos_l, __LINE__) = 0, index; zf::mem::bitset_walk_all_unset(bs, &ZF_CONCAT(walk_pos_l, __LINE__), &index);)

    // ============================================================
}
