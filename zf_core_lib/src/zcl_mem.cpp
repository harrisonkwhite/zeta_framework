#include <zcl/zcl_mem.h>

#include <cstdlib>
#include <zcl/zcl_algos.h>

namespace zf::mem {
    void arena_destroy(t_arena *const arena) {
        const auto f = [](const auto self, t_arena_block *const block) {
            if (!block) {
                return;
            }

            self(self, block->next);

            poison_freed(block->buf, block->buf_size);
            free(block->buf);

            poison_freed_item(block);
            free(block);
        };

        f(f, arena->blocks_head);

        *arena = {};
    }

    static t_arena_block *arena_create_block(const t_i32 buf_size) {
        ZF_REQUIRE(buf_size > 0);

        const auto block = static_cast<t_arena_block *>(malloc(sizeof(t_arena_block)));

        if (!block) {
            ZF_FATAL();
        }

        poison_uninitted_item(block);

        block->buf = malloc(static_cast<size_t>(buf_size));

        if (!block->buf) {
            ZF_FATAL();
        }

        poison_uninitted(block->buf, buf_size);

        block->buf_size = buf_size;
        block->next = nullptr;

        return block;
    }

    void *arena_push(t_arena *const arena, const t_i32 size, const t_i32 alignment) {
        ZF_ASSERT(size > 0 && is_alignment_valid(alignment));

        if (!arena->blocks_head) {
            arena->blocks_head = arena_create_block(max(size, arena->block_min_size));
            arena->block_cur = arena->blocks_head;
            return arena_push(arena, size, alignment);
        }

        const t_i32 offs_aligned = align_forward(arena->block_cur_offs, alignment);
        const t_i32 offs_next = offs_aligned + size;

        if (offs_next > arena->block_cur->buf_size) {
            if (!arena->block_cur->next) {
                arena->block_cur->next = arena_create_block(max(size, arena->block_min_size));
            }

            arena->block_cur = arena->block_cur->next;
            arena->block_cur_offs = 0;

            return arena_push(arena, size, alignment);
        }

        arena->block_cur_offs = offs_next;

        void *const result = static_cast<t_u8 *>(arena->block_cur->buf) + offs_aligned;
        poison_uninitted(result, size);

        return result;
    }

    void arena_rewind(t_arena *const arena) {
        arena->block_cur = arena->blocks_head;
        arena->block_cur_offs = 0;

#ifdef ZF_DEBUG
        // Poison all block buffers.
        const t_arena_block *block = arena->blocks_head;

        while (block) {
            poison_freed(block->buf, block->buf_size);
            block = block->next;
        }
#endif
    }

    t_b8 is_any_bit_set(const t_bitset_rdonly bs) {
        if (bs.bit_cnt == 0) {
            return false;
        }

        const auto first_bytes = array_slice(bitset_get_bytes(bs), 0, bitset_get_bytes(bs).len - 1);

        if (!array_do_all_equal(first_bytes, 0)) {
            return true;
        }

        return (bitset_get_bytes(bs)[bitset_get_bytes(bs).len - 1] & bitset_get_last_byte_mask(bs)) != 0;
    }

    t_b8 are_all_bits_set(const t_bitset_rdonly bs) {
        if (bs.bit_cnt == 0) {
            return false;
        }

        const auto first_bytes = array_slice(bitset_get_bytes(bs), 0, bitset_get_bytes(bs).len - 1);

        if (!array_do_all_equal(first_bytes, 0xFF)) {
            return false;
        }

        const auto last_byte_mask = bitset_get_last_byte_mask(bs);
        return (bitset_get_bytes(bs)[bitset_get_bytes(bs).len - 1] & last_byte_mask) == last_byte_mask;
    }

    void set_all_bits(const t_bitset_mut bs) {
        if (bs.bit_cnt == 0) {
            return;
        }

        const auto first_bytes = array_slice(bitset_get_bytes(bs), 0, bitset_get_bytes(bs).len - 1);
        array_set_all_to(first_bytes, 0xFF);

        bitset_get_bytes(bs)[bitset_get_bytes(bs).len - 1] |= bitset_get_last_byte_mask(bs);
    }

    void unset_all_bits(const t_bitset_mut bs) {
        if (bs.bit_cnt == 0) {
            return;
        }

        const auto first_bytes = array_slice(bitset_get_bytes(bs), 0, bitset_get_bytes(bs).len - 1);
        array_set_all_to(first_bytes, 0);

        bitset_get_bytes(bs)[bitset_get_bytes(bs).len - 1] &= ~bitset_get_last_byte_mask(bs);
    }

    void set_bits_in_range(const t_bitset_mut bs, const t_i32 begin_bit_index, const t_i32 end_bit_index) {
        ZF_ASSERT(begin_bit_index >= 0 && begin_bit_index < bs.bit_cnt);
        ZF_ASSERT(end_bit_index >= begin_bit_index && end_bit_index <= bs.bit_cnt);

        const t_i32 begin_elem_index = begin_bit_index / 8;
        const t_i32 end_elem_index = convert_bits_to_bytes(end_bit_index);

        for (t_i32 i = begin_elem_index; i < end_elem_index; i++) {
            const t_i32 bit_offs = i * 8;
            const t_i32 begin_bit_index_rel = begin_bit_index - bit_offs;
            const t_i32 end_bit_index_rel = end_bit_index - bit_offs;

            const t_i32 set_range_begin = max(begin_bit_index_rel, 0);
            const t_i32 set_range_end = min(end_bit_index_rel, 8);

            bitset_get_bytes(bs)[i] |= create_byte_bitmask_range(set_range_begin, set_range_end);
        }
    }

    void apply_mask_to_bits(const t_bitset_mut targ, const t_bitset_rdonly mask, const t_bitwise_mask_op op) {
        ZF_ASSERT(targ.bit_cnt == mask.bit_cnt);

        if (targ.bit_cnt == 0) {
            return;
        }

        switch (op) {
        case ec_bitwise_mask_op_and:
            for (t_i32 i = 0; i < bitset_get_bytes(targ).len; i++) {
                bitset_get_bytes(targ)[i] &= bitset_get_bytes(mask)[i];
            }

            break;

        case ec_bitwise_mask_op_or:
            for (t_i32 i = 0; i < bitset_get_bytes(targ).len; i++) {
                bitset_get_bytes(targ)[i] |= bitset_get_bytes(mask)[i];
            }

            break;

        case ec_bitwise_mask_op_xor:
            for (t_i32 i = 0; i < bitset_get_bytes(targ).len; i++) {
                bitset_get_bytes(targ)[i] ^= bitset_get_bytes(mask)[i];
            }

            break;

        case ec_bitwise_mask_op_andnot:
            for (t_i32 i = 0; i < bitset_get_bytes(targ).len; i++) {
                bitset_get_bytes(targ)[i] &= ~bitset_get_bytes(mask)[i];
            }

            break;
        }

        bitset_get_bytes(targ)[bitset_get_bytes(targ).len - 1] &= bitset_get_last_byte_mask(targ);
    }

    static t_u8 shift_bits_left(const t_bitset_mut bs) {
        ZF_ASSERT(convert_bits_to_bytes(bs.bit_cnt) == bitset_get_bytes(bs).len);

        if (bs.bit_cnt == 0) {
            return 0;
        }

        t_u8 discard = 0;

        for (t_i32 i = 0; i < bitset_get_bytes(bs).len; i++) {
            const t_i32 bits_in_byte = i == bitset_get_bytes(bs).len - 1 ? bitset_get_last_byte_bit_cnt(bs) : 8;
            const t_u8 discard_last = discard;
            discard = (bitset_get_bytes(bs)[i] & create_byte_bitmask_single(bits_in_byte - 1)) >> (bits_in_byte - 1);
            bitset_get_bytes(bs)[i] <<= 1;
            bitset_get_bytes(bs)[i] |= discard_last;
        }

        bitset_get_bytes(bs)[bitset_get_bytes(bs).len - 1] &= bitset_get_last_byte_mask(bs);

        return discard;
    }

    void shift_bits_left_by(const t_bitset_mut bs, const t_i32 amount) {
        ZF_ASSERT(amount >= 0);

        // @speed: :(

        for (t_i32 i = 0; i < amount; i++) {
            shift_bits_left(bs);
        }
    }

    void rot_bits_left_by(const t_bitset_mut bs, const t_i32 amount) {
        ZF_ASSERT(amount >= 0);

        if (bs.bit_cnt == 0) {
            return;
        }

        // @speed: :(

        for (t_i32 i = 0; i < amount; i++) {
            const auto discard = shift_bits_left(bs);

            if (discard) {
                set_bit(bs, 0);
            } else {
                unset_bit(bs, 0);
            }
        }
    }

    static t_u8 shift_bits_right(const t_bitset_mut bs) {
        ZF_ASSERT(convert_bits_to_bytes(bs.bit_cnt) == bitset_get_bytes(bs).len);

        if (bs.bit_cnt == 0) {
            return 0;
        }

        bitset_get_bytes(bs)[bitset_get_bytes(bs).len - 1] &= bitset_get_last_byte_mask(bs); // Drop any excess bits so we don't accidentally shift a 1 in.

        t_u8 discard = 0;

        for (t_i32 i = bitset_get_bytes(bs).len - 1; i >= 0; i--) {
            const t_i32 bits_in_byte = i == bitset_get_bytes(bs).len - 1 ? bitset_get_last_byte_bit_cnt(bs) : 8;
            const t_u8 discard_last = discard;
            discard = bitset_get_bytes(bs)[i] & create_byte_bitmask_single(0);
            bitset_get_bytes(bs)[i] >>= 1;

            if (discard_last) {
                bitset_get_bytes(bs)[i] |= create_byte_bitmask_single(bits_in_byte - 1);
            }
        }

        return discard;
    }

    void shift_bits_right_by(const t_bitset_mut bs, const t_i32 amount) {
        ZF_ASSERT(amount >= 0);

        // @speed: :(

        for (t_i32 i = 0; i < amount; i++) {
            shift_bits_right(bs);
        }
    }

    void rot_bits_right_by(const t_bitset_mut bs, const t_i32 amount) {
        ZF_ASSERT(amount >= 0);

        if (bs.bit_cnt == 0) {
            return;
        }

        // @speed: :(

        for (t_i32 i = 0; i < amount; i++) {
            const auto discard = shift_bits_right(bs);

            if (discard) {
                set_bit(bs, bs.bit_cnt - 1);
            } else {
                unset_bit(bs, bs.bit_cnt - 1);
            }
        }
    }

    // ============================================================

    static t_i32 get_index_of_first_set_bit_helper(const t_bitset_rdonly bs, const t_i32 from, const t_u8 xor_mask) {
        ZF_ASSERT(from >= 0 && from <= bs.bit_cnt); // Intentionally allowing the upper bound here for the case of iteration.

        // Map of each possible byte to the index of the first set bit, or -1 for the first case.
        static const t_static_array<t_i32, 256> g_mappings = {{
            -1, // 0000 0000
            0,  // 0000 0001
            1,  // 0000 0010
            0,  // 0000 0011
            2,  // 0000 0100
            0,  // 0000 0101
            1,  // 0000 0110
            0,  // 0000 0111
            3,  // 0000 1000
            0,  // 0000 1001
            1,  // 0000 1010
            0,  // 0000 1011
            2,  // 0000 1100
            0,  // 0000 1101
            1,  // 0000 1110
            0,  // 0000 1111
            4,  // 0001 0000
            0,  // 0001 0001
            1,  // 0001 0010
            0,  // 0001 0011
            2,  // 0001 0100
            0,  // 0001 0101
            1,  // 0001 0110
            0,  // 0001 0111
            3,  // 0001 1000
            0,  // 0001 1001
            1,  // 0001 1010
            0,  // 0001 1011
            2,  // 0001 1100
            0,  // 0001 1101
            1,  // 0001 1110
            0,  // 0001 1111
            5,  // 0010 0000
            0,  // 0010 0001
            1,  // 0010 0010
            0,  // 0010 0011
            2,  // 0010 0100
            0,  // 0010 0101
            1,  // 0010 0110
            0,  // 0010 0111
            3,  // 0010 1000
            0,  // 0010 1001
            1,  // 0010 1010
            0,  // 0010 1011
            2,  // 0010 1100
            0,  // 0010 1101
            1,  // 0010 1110
            0,  // 0010 1111
            4,  // 0011 0000
            0,  // 0011 0001
            1,  // 0011 0010
            0,  // 0011 0011
            2,  // 0011 0100
            0,  // 0011 0101
            1,  // 0011 0110
            0,  // 0011 0111
            3,  // 0011 1000
            0,  // 0011 1001
            1,  // 0011 1010
            0,  // 0011 1011
            2,  // 0011 1100
            0,  // 0011 1101
            1,  // 0011 1110
            0,  // 0011 1111
            6,  // 0100 0000
            0,  // 0100 0001
            1,  // 0100 0010
            0,  // 0100 0011
            2,  // 0100 0100
            0,  // 0100 0101
            1,  // 0100 0110
            0,  // 0100 0111
            3,  // 0100 1000
            0,  // 0100 1001
            1,  // 0100 1010
            0,  // 0100 1011
            2,  // 0100 1100
            0,  // 0100 1101
            1,  // 0100 1110
            0,  // 0100 1111
            4,  // 0101 0000
            0,  // 0101 0001
            1,  // 0101 0010
            0,  // 0101 0011
            2,  // 0101 0100
            0,  // 0101 0101
            1,  // 0101 0110
            0,  // 0101 0111
            3,  // 0101 1000
            0,  // 0101 1001
            1,  // 0101 1010
            0,  // 0101 1011
            2,  // 0101 1100
            0,  // 0101 1101
            1,  // 0101 1110
            0,  // 0101 1111
            5,  // 0110 0000
            0,  // 0110 0001
            1,  // 0110 0010
            0,  // 0110 0011
            2,  // 0110 0100
            0,  // 0110 0101
            1,  // 0110 0110
            0,  // 0110 0111
            3,  // 0110 1000
            0,  // 0110 1001
            1,  // 0110 1010
            0,  // 0110 1011
            2,  // 0110 1100
            0,  // 0110 1101
            1,  // 0110 1110
            0,  // 0110 1111
            4,  // 0111 0000
            0,  // 0111 0001
            1,  // 0111 0010
            0,  // 0111 0011
            2,  // 0111 0100
            0,  // 0111 0101
            1,  // 0111 0110
            0,  // 0111 0111
            3,  // 0111 1000
            0,  // 0111 1001
            1,  // 0111 1010
            0,  // 0111 1011
            2,  // 0111 1100
            0,  // 0111 1101
            1,  // 0111 1110
            0,  // 0111 1111
            7,  // 1000 0000
            0,  // 1000 0001
            1,  // 1000 0010
            0,  // 1000 0011
            2,  // 1000 0100
            0,  // 1000 0101
            1,  // 1000 0110
            0,  // 1000 0111
            3,  // 1000 1000
            0,  // 1000 1001
            1,  // 1000 1010
            0,  // 1000 1011
            2,  // 1000 1100
            0,  // 1000 1101
            1,  // 1000 1110
            0,  // 1000 1111
            4,  // 1001 0000
            0,  // 1001 0001
            1,  // 1001 0010
            0,  // 1001 0011
            2,  // 1001 0100
            0,  // 1001 0101
            1,  // 1001 0110
            0,  // 1001 0111
            3,  // 1001 1000
            0,  // 1001 1001
            1,  // 1001 1010
            0,  // 1001 1011
            2,  // 1001 1100
            0,  // 1001 1101
            1,  // 1001 1110
            0,  // 1001 1111
            5,  // 1010 0000
            0,  // 1010 0001
            1,  // 1010 0010
            0,  // 1010 0011
            2,  // 1010 0100
            0,  // 1010 0101
            1,  // 1010 0110
            0,  // 1010 0111
            3,  // 1010 1000
            0,  // 1010 1001
            1,  // 1010 1010
            0,  // 1010 1011
            2,  // 1010 1100
            0,  // 1010 1101
            1,  // 1010 1110
            0,  // 1010 1111
            4,  // 1011 0000
            0,  // 1011 0001
            1,  // 1011 0010
            0,  // 1011 0011
            2,  // 1011 0100
            0,  // 1011 0101
            1,  // 1011 0110
            0,  // 1011 0111
            3,  // 1011 1000
            0,  // 1011 1001
            1,  // 1011 1010
            0,  // 1011 1011
            2,  // 1011 1100
            0,  // 1011 1101
            1,  // 1011 1110
            0,  // 1011 1111
            6,  // 1100 0000
            0,  // 1100 0001
            1,  // 1100 0010
            0,  // 1100 0011
            2,  // 1100 0100
            0,  // 1100 0101
            1,  // 1100 0110
            0,  // 1100 0111
            3,  // 1100 1000
            0,  // 1100 1001
            1,  // 1100 1010
            0,  // 1100 1011
            2,  // 1100 1100
            0,  // 1100 1101
            1,  // 1100 1110
            0,  // 1100 1111
            4,  // 1101 0000
            0,  // 1101 0001
            1,  // 1101 0010
            0,  // 1101 0011
            2,  // 1101 0100
            0,  // 1101 0101
            1,  // 1101 0110
            0,  // 1101 0111
            3,  // 1101 1000
            0,  // 1101 1001
            1,  // 1101 1010
            0,  // 1101 1011
            2,  // 1101 1100
            0,  // 1101 1101
            1,  // 1101 1110
            0,  // 1101 1111
            5,  // 1110 0000
            0,  // 1110 0001
            1,  // 1110 0010
            0,  // 1110 0011
            2,  // 1110 0100
            0,  // 1110 0101
            1,  // 1110 0110
            0,  // 1110 0111
            3,  // 1110 1000
            0,  // 1110 1001
            1,  // 1110 1010
            0,  // 1110 1011
            2,  // 1110 1100
            0,  // 1110 1101
            1,  // 1110 1110
            0,  // 1110 1111
            4,  // 1111 0000
            0,  // 1111 0001
            1,  // 1111 0010
            0,  // 1111 0011
            2,  // 1111 0100
            0,  // 1111 0101
            1,  // 1111 0110
            0,  // 1111 0111
            3,  // 1111 1000
            0,  // 1111 1001
            1,  // 1111 1010
            0,  // 1111 1011
            2,  // 1111 1100
            0,  // 1111 1101
            1,  // 1111 1110
            0,  // 1111 1111
        }};

        const t_i32 begin_byte_index = from / 8;

        for (t_i32 i = begin_byte_index; i < bitset_get_bytes(bs).len; i++) {
            t_u8 byte = bitset_get_bytes(bs)[i] ^ xor_mask;

            if (i == begin_byte_index) {
                byte &= create_byte_bitmask_range(from % 8);
            }

            if (i == bitset_get_bytes(bs).len - 1) {
                byte &= bitset_get_last_byte_mask(bs);
            }

            const t_i32 bi = g_mappings[byte];

            if (bi != -1) {
                return (8 * i) + bi;
            }
        }

        return -1;
    }

    t_i32 get_index_of_first_set_bit(const t_bitset_rdonly bs, const t_i32 from) {
        return get_index_of_first_set_bit_helper(bs, from, 0);
    }

    t_i32 get_index_of_first_unset_bit(const t_bitset_rdonly bs, const t_i32 from) {
        return get_index_of_first_set_bit_helper(bs, from, 0xFF);
    }

    t_i32 count_set_bits(const t_bitset_rdonly bs) {
        // Map of each possible byte to the number of set bits in it.
        static const t_static_array<t_i32, 256> g_mappings = {{
            0, // 0000 0000
            1, // 0000 0001
            1, // 0000 0010
            2, // 0000 0011
            1, // 0000 0100
            2, // 0000 0101
            2, // 0000 0110
            3, // 0000 0111
            1, // 0000 1000
            2, // 0000 1001
            2, // 0000 1010
            3, // 0000 1011
            2, // 0000 1100
            3, // 0000 1101
            3, // 0000 1110
            4, // 0000 1111
            1, // 0001 0000
            2, // 0001 0001
            2, // 0001 0010
            3, // 0001 0011
            2, // 0001 0100
            3, // 0001 0101
            3, // 0001 0110
            4, // 0001 0111
            2, // 0001 1000
            3, // 0001 1001
            3, // 0001 1010
            4, // 0001 1011
            3, // 0001 1100
            4, // 0001 1101
            4, // 0001 1110
            5, // 0001 1111
            1, // 0010 0000
            2, // 0010 0001
            2, // 0010 0010
            3, // 0010 0011
            2, // 0010 0100
            3, // 0010 0101
            3, // 0010 0110
            4, // 0010 0111
            2, // 0010 1000
            3, // 0010 1001
            3, // 0010 1010
            4, // 0010 1011
            3, // 0010 1100
            4, // 0010 1101
            4, // 0010 1110
            5, // 0010 1111
            2, // 0011 0000
            3, // 0011 0001
            3, // 0011 0010
            4, // 0011 0011
            3, // 0011 0100
            4, // 0011 0101
            4, // 0011 0110
            5, // 0011 0111
            3, // 0011 1000
            4, // 0011 1001
            4, // 0011 1010
            5, // 0011 1011
            4, // 0011 1100
            5, // 0011 1101
            5, // 0011 1110
            6, // 0011 1111
            1, // 0100 0000
            2, // 0100 0001
            2, // 0100 0010
            3, // 0100 0011
            2, // 0100 0100
            3, // 0100 0101
            3, // 0100 0110
            4, // 0100 0111
            2, // 0100 1000
            3, // 0100 1001
            3, // 0100 1010
            4, // 0100 1011
            3, // 0100 1100
            4, // 0100 1101
            4, // 0100 1110
            5, // 0100 1111
            2, // 0101 0000
            3, // 0101 0001
            3, // 0101 0010
            4, // 0101 0011
            3, // 0101 0100
            4, // 0101 0101
            4, // 0101 0110
            5, // 0101 0111
            3, // 0101 1000
            4, // 0101 1001
            4, // 0101 1010
            5, // 0101 1011
            4, // 0101 1100
            5, // 0101 1101
            5, // 0101 1110
            6, // 0101 1111
            2, // 0110 0000
            3, // 0110 0001
            3, // 0110 0010
            4, // 0110 0011
            3, // 0110 0100
            4, // 0110 0101
            4, // 0110 0110
            5, // 0110 0111
            3, // 0110 1000
            4, // 0110 1001
            4, // 0110 1010
            5, // 0110 1011
            4, // 0110 1100
            5, // 0110 1101
            5, // 0110 1110
            6, // 0110 1111
            3, // 0111 0000
            4, // 0111 0001
            4, // 0111 0010
            5, // 0111 0011
            4, // 0111 0100
            5, // 0111 0101
            5, // 0111 0110
            6, // 0111 0111
            4, // 0111 1000
            5, // 0111 1001
            5, // 0111 1010
            6, // 0111 1011
            5, // 0111 1100
            6, // 0111 1101
            6, // 0111 1110
            7, // 0111 1111
            1, // 1000 0000
            2, // 1000 0001
            2, // 1000 0010
            3, // 1000 0011
            2, // 1000 0100
            3, // 1000 0101
            3, // 1000 0110
            4, // 1000 0111
            2, // 1000 1000
            3, // 1000 1001
            3, // 1000 1010
            4, // 1000 1011
            3, // 1000 1100
            4, // 1000 1101
            4, // 1000 1110
            5, // 1000 1111
            2, // 1001 0000
            3, // 1001 0001
            3, // 1001 0010
            4, // 1001 0011
            3, // 1001 0100
            4, // 1001 0101
            4, // 1001 0110
            5, // 1001 0111
            3, // 1001 1000
            4, // 1001 1001
            4, // 1001 1010
            5, // 1001 1011
            4, // 1001 1100
            5, // 1001 1101
            5, // 1001 1110
            6, // 1001 1111
            2, // 1010 0000
            3, // 1010 0001
            3, // 1010 0010
            4, // 1010 0011
            3, // 1010 0100
            4, // 1010 0101
            4, // 1010 0110
            5, // 1010 0111
            3, // 1010 1000
            4, // 1010 1001
            4, // 1010 1010
            5, // 1010 1011
            4, // 1010 1100
            5, // 1010 1101
            5, // 1010 1110
            6, // 1010 1111
            3, // 1011 0000
            4, // 1011 0001
            4, // 1011 0010
            5, // 1011 0011
            4, // 1011 0100
            5, // 1011 0101
            5, // 1011 0110
            6, // 1011 0111
            4, // 1011 1000
            5, // 1011 1001
            5, // 1011 1010
            6, // 1011 1011
            5, // 1011 1100
            6, // 1011 1101
            6, // 1011 1110
            7, // 1011 1111
            2, // 1100 0000
            3, // 1100 0001
            3, // 1100 0010
            4, // 1100 0011
            3, // 1100 0100
            4, // 1100 0101
            4, // 1100 0110
            5, // 1100 0111
            3, // 1100 1000
            4, // 1100 1001
            4, // 1100 1010
            5, // 1100 1011
            4, // 1100 1100
            5, // 1100 1101
            5, // 1100 1110
            6, // 1100 1111
            3, // 1101 0000
            4, // 1101 0001
            4, // 1101 0010
            5, // 1101 0011
            4, // 1101 0100
            5, // 1101 0101
            5, // 1101 0110
            6, // 1101 0111
            4, // 1101 1000
            5, // 1101 1001
            5, // 1101 1010
            6, // 1101 1011
            5, // 1101 1100
            6, // 1101 1101
            6, // 1101 1110
            7, // 1101 1111
            3, // 1110 0000
            4, // 1110 0001
            4, // 1110 0010
            5, // 1110 0011
            4, // 1110 0100
            5, // 1110 0101
            5, // 1110 0110
            6, // 1110 0111
            4, // 1110 1000
            5, // 1110 1001
            5, // 1110 1010
            6, // 1110 1011
            5, // 1110 1100
            6, // 1110 1101
            6, // 1110 1110
            7, // 1110 1111
            4, // 1111 0000
            5, // 1111 0001
            5, // 1111 0010
            6, // 1111 0011
            5, // 1111 0100
            6, // 1111 0101
            6, // 1111 0110
            7, // 1111 0111
            5, // 1111 1000
            6, // 1111 1001
            6, // 1111 1010
            7, // 1111 1011
            6, // 1111 1100
            7, // 1111 1101
            7, // 1111 1110
            8, // 1111 1111
        }};

        t_i32 result = 0;

        if (bitset_get_bytes(bs).len > 0) {
            for (t_i32 i = 0; i < bitset_get_bytes(bs).len - 1; i++) {
                result += g_mappings[bitset_get_bytes(bs)[i]];
            }

            result += g_mappings[bitset_get_bytes(bs)[bitset_get_bytes(bs).len - 1] & bitset_get_last_byte_mask(bs)];
        }

        return result;
    }

    t_b8 walk_set_bits(const t_bitset_rdonly bs, t_i32 *const pos, t_i32 *const o_index) {
        ZF_ASSERT(*pos >= 0 && *pos <= bs.bit_cnt);

        *o_index = get_index_of_first_set_bit(bs, *pos);

        if (*o_index == -1) {
            return false;
        }

        *pos = *o_index + 1;

        return true;
    }

    t_b8 walk_unset_bits(const t_bitset_rdonly bs, t_i32 *const pos, t_i32 *const o_index) {
        ZF_ASSERT(*pos >= 0 && *pos <= bs.bit_cnt);

        *o_index = get_index_of_first_unset_bit(bs, *pos);

        if (*o_index == -1) {
            return false;
        }

        *pos = *o_index + 1;

        return true;
    }
}
