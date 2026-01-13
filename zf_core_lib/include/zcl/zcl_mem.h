#pragma once

#include <zcl/zcl_basic.h>

namespace zcl::mem {
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

    constexpr t_b8 check_set(const t_bitset_rdonly bs, const t_i32 index) {
        ZF_ASSERT(index >= 0 && index < bs.bit_cnt);
        return bitset_get_bytes(bs)[index / 8] & byte_bitmask_create_single(index % 8);
    }

    constexpr void set(const t_bitset_mut bs, const t_i32 index) {
        ZF_ASSERT(index >= 0 && index < bs.bit_cnt);
        bitset_get_bytes(bs)[index / 8] |= byte_bitmask_create_single(index % 8);
    }

    constexpr void unset(const t_bitset_mut bs, const t_i32 index) {
        ZF_ASSERT(index >= 0 && index < bs.bit_cnt);
        bitset_get_bytes(bs)[index / 8] &= ~byte_bitmask_create_single(index % 8);
    }

    t_b8 check_any_set(const t_bitset_rdonly bs);

    inline t_b8 check_all_unset(const t_bitset_rdonly bs) {
        return bs.bit_cnt > 0 && !check_any_set(bs);
    }

    t_b8 check_all_set(const t_bitset_rdonly bs);

    inline t_b8 check_any_unset(const t_bitset_rdonly bs) {
        return bs.bit_cnt > 0 && !check_all_set(bs);
    }

    void set_all(const t_bitset_mut bs);
    void unset_all(const t_bitset_mut bs);

    // Sets all bits in the range [begin_bit_index, end_bit_index).
    void set_range(const t_bitset_mut bs, const t_i32 begin_bit_index, const t_i32 end_bit_index);

    enum t_bitwise_mask_op : t_i32 {
        ek_bitwise_mask_op_and,
        ek_bitwise_mask_op_or,
        ek_bitwise_mask_op_xor,
        ek_bitwise_mask_op_andnot
    };

    void apply_mask(const t_bitset_mut bs, const t_bitset_rdonly mask, const t_bitwise_mask_op op);

    void shift_left(const t_bitset_mut bs, const t_i32 amount = 1);
    void rot_left(const t_bitset_mut bs, const t_i32 amount = 1);

    void shift_right(const t_bitset_mut bs, const t_i32 amount = 1);
    void rot_right(const t_bitset_mut bs, const t_i32 amount = 1);

    // Returns the index of the found set bit, or -1 if all bits are unset.
    t_i32 find_first_set_bit(const t_bitset_rdonly bs, const t_i32 from = 0);

    // Returns the index of the found unset bit, or -1 if all bits are unset.
    t_i32 find_first_unset_bit(const t_bitset_rdonly bs, const t_i32 from = 0);

    t_i32 count_set(const t_bitset_rdonly bs);

    inline t_i32 count_unset(const t_bitset_rdonly bs) {
        return bs.bit_cnt - count_set(bs);
    }

    // pos is the walker state, initialise it to the bit index you want to start from.
    // o_index is assigned the index of the set bit to process.
    // Returns false iff the walk is complete.
    t_b8 walk_all_set(const t_bitset_rdonly bs, t_i32 *const pos, t_i32 *const o_index);

    // pos is the walker state, initialise it to the bit index you want to start from.
    // o_index is assigned the index of the unset bit to process.
    // Returns false iff the walk is complete.
    t_b8 walk_all_unset(const t_bitset_rdonly bs, t_i32 *const pos, t_i32 *const o_index);

#define ZF_WALK_SET_BITS(bs, index) for (zcl::t_i32 ZF_CONCAT(walk_pos_l, __LINE__) = 0, index; zcl::mem::walk_all_set(bs, &ZF_CONCAT(walk_pos_l, __LINE__), &index);)
#define ZF_WALK_UNSET_BITS(bs, index) for (zcl::t_i32 ZF_CONCAT(walk_pos_l, __LINE__) = 0, index; zcl::mem::walk_all_unset(bs, &ZF_CONCAT(walk_pos_l, __LINE__), &index);)
}

// @temp
namespace zcl {
    [[nodiscard]] inline t_b8 stream_serialize_bitset(const t_stream stream, const mem::t_bitset_rdonly bv) {
        if (!stream_write_item(stream, bv.bit_cnt)) {
            return false;
        }

        if (!stream_write_items_of_array(stream, mem::bitset_get_bytes(bv))) {
            return false;
        }

        return true;
    }

    [[nodiscard]] inline t_b8 stream_deserialize_bitset(const t_stream stream, t_arena *const bv_arena, mem::t_bitset_mut *const o_bv) {
        t_i32 bit_cnt;

        if (!stream_read_item(stream, &bit_cnt)) {
            return false;
        }

        *o_bv = mem::bitset_create(bit_cnt, bv_arena);

        if (!stream_read_items_into_array(stream, mem::bitset_get_bytes(*o_bv), mem::bitset_get_bytes(*o_bv).len)) {
            return false;
        }

        return true;
    }
}
