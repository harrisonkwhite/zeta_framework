#pragma once

#include <cstring>
#include <zcl/zcl_basic.h>

namespace zf {
    // ============================================================
    // @section: Types and Globals

    constexpr t_u8 g_poison_uninitted = 0xCD;
    constexpr t_u8 g_poison_freed = 0xDD;

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

    template <typename tp_type>
    concept c_array_elem = c_simple<tp_type> && (!c_const<tp_type>);

    template <c_array_elem tp_type>
    struct t_array_rdonly {
        using t_elem = tp_type;

        const tp_type *raw;
        t_i32 len;

        // @todo: Consider replacing with explicit function for safety.
        constexpr const tp_type &operator[](const t_i32 index) const {
            ZF_REQUIRE(index >= 0 && index < len);
            return raw[index];
        }
    };

    template <c_array_elem tp_type>
    struct t_array_mut {
        using t_elem = tp_type;

        tp_type *raw;
        t_i32 len;

        constexpr tp_type &operator[](const t_i32 index) const {
            ZF_REQUIRE(index >= 0 && index < len);
            return raw[index];
        }

        constexpr operator t_array_rdonly<tp_type>() const {
            return {raw, len};
        }
    };

    template <c_array_elem tp_type, t_i32 tp_len>
    struct t_static_array {
        using t_elem = tp_type;

        static constexpr t_i32 g_len = tp_len;

        tp_type raw[tp_len];

        constexpr tp_type &operator[](const t_i32 index) {
            ZF_REQUIRE(index >= 0 && index < tp_len);
            return raw[index];
        }

        constexpr const tp_type &operator[](const t_i32 index) const {
            ZF_REQUIRE(index >= 0 && index < tp_len);
            return raw[index];
        }

        constexpr operator t_array_mut<tp_type>() {
            return {raw, g_len};
        }

        constexpr operator t_array_rdonly<tp_type>() const {
            return {raw, g_len};
        }
    };

    template <typename tp_type>
    concept c_array_mut = requires { typename tp_type::t_elem; } && c_same<t_cvref_removed<tp_type>, t_array_mut<typename tp_type::t_elem>>;

    template <typename tp_type>
    concept c_array_rdonly = requires { typename tp_type::t_elem; } && c_same<t_cvref_removed<tp_type>, t_array_rdonly<typename tp_type::t_elem>>;

    template <typename tp_type>
    concept c_array = c_array_mut<tp_type> || c_array_rdonly<tp_type>;

    template <c_array tp_arr_type>
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

    struct t_bit_vec_rdonly {
        const t_u8 *bytes_raw;
        t_i32 bit_cnt;
    };

    struct t_bit_vec_mut {
        t_u8 *bytes_raw;
        t_i32 bit_cnt;

        constexpr operator t_bit_vec_rdonly() const {
            return {bytes_raw, bit_cnt};
        }
    };

    template <t_i32 tp_bit_cnt>
    struct t_static_bit_vec {
        static constexpr t_i32 g_bit_cnt = tp_bit_cnt;

        t_static_array<t_u8, f_bits_to_bytes(tp_bit_cnt)> bytes;

        constexpr operator t_bit_vec_mut() { return {bytes.raw, g_bit_cnt}; }
        constexpr operator t_bit_vec_rdonly() const { return {bytes.raw, g_bit_cnt}; }
    };

    enum t_bitwise_mask_op : t_i32 {
        ec_bitwise_mask_op_and,
        ec_bitwise_mask_op_or,
        ec_bitwise_mask_op_xor,
        ec_bitwise_mask_op_andnot
    };

    // ============================================================


    // ============================================================
    // @section: Functions

    inline void f_mem_clear(void *const buf, const t_i32 buf_size, const t_u8 val) {
        ZF_ASSERT(buf_size >= 0);
        memset(buf, val, static_cast<size_t>(buf_size));
    }

    template <c_simple tp_type>
    inline void f_mem_clear_item(tp_type *const item, const t_u8 val) {
        f_mem_clear(item, ZF_SIZE_OF(tp_type), val);
    }

#ifdef ZF_DEBUG
    inline void f_mem_poison_uninitted(void *const buf, const t_i32 buf_size) {
        f_mem_clear(buf, buf_size, g_poison_uninitted);
    }

    template <c_simple tp_type>
    inline void f_poison_uninitted_item(tp_type *const item) {
        f_mem_clear_item(item, g_poison_uninitted);
    }

    inline void f_poison_freed(void *const buf, const t_i32 buf_size) {
        f_mem_clear(buf, buf_size, g_poison_freed);
    }

    template <c_simple tp_type>
    inline void f_poison_freed_item(tp_type *const item) {
        f_mem_clear_item(item, g_poison_freed);
    }
#else
    inline void f_poison_uninitted(void *const buf, const t_i32 buf_size) {}
    template <co_simple tp_type> inline void f_poison_uninitted_item(tp_type *const item) {}
    inline void f_poison_freed(void *const buf, const t_i32 buf_size) {}
    template <co_simple tp_type> inline void f_poison_freed_item(tp_type *const item) {}
#endif

    // Does not allocate any arena memory (blocks) upfront.
    inline t_arena f_mem_create_arena(const t_i32 block_min_size = f_megabytes_to_bytes(1)) {
        return {.block_min_size = block_min_size};
    }

    // Frees all arena memory. It is valid to call this even if no pushing was done.
    void f_mem_destroy_arena(t_arena *const arena);

    // Will lazily allocate memory as needed. Allocation failure is treated as fatal and causes an abort - you don't need to check for nullptr.
    void *f_mem_push(t_arena *const arena, const t_i32 size, const t_i32 alignment);

    template <c_simple tp_type>
    tp_type *f_mem_push_item(t_arena *const arena) {
        return static_cast<tp_type *>(f_mem_push(arena, ZF_SIZE_OF(tp_type), ZF_ALIGN_OF(tp_type)));
    }

    template <c_simple tp_type>
    tp_type *f_mem_push_item_zeroed(t_arena *const arena) {
        const auto result = static_cast<tp_type *>(f_mem_push(arena, ZF_SIZE_OF(tp_type), ZF_ALIGN_OF(tp_type)));
        f_mem_clear_item(result, 0);
        return result;
    }

    template <c_simple tp_type>
    t_array_mut<tp_type> f_mem_push_array(t_arena *const arena, const t_i32 len) {
        ZF_ASSERT(len >= 0);

        if (len == 0) {
            return {};
        }

        const auto raw = static_cast<tp_type *>(f_mem_push(arena, ZF_SIZE_OF(tp_type) * len, ZF_ALIGN_OF(tp_type)));
        return {raw, len};
    }

    template <c_simple tp_type>
    t_array_mut<tp_type> f_mem_push_array_zeroed(t_arena *const arena, const t_i32 len) {
        ZF_ASSERT(len >= 0);

        if (len == 0) {
            return {};
        }

        const t_i32 size = ZF_SIZE_OF(tp_type) * len;
        const auto raw = static_cast<tp_type *>(f_mem_push(arena, size, ZF_ALIGN_OF(tp_type)));
        f_mem_clear(raw, size, 0);
        return {raw, len};
    }

    // Takes the arena offset to the beginning of its allocated memory (if any) to overwrite from there.
    void f_mem_rewind_arena(t_arena *const arena);

    template <typename tp_type>
    t_array_rdonly<tp_type> f_mem_slice_array(const t_array_rdonly<tp_type> arr, const t_i32 beg, const t_i32 end) {
        ZF_ASSERT(beg >= 0 && beg <= arr.len);
        ZF_ASSERT(end >= beg && end <= arr.len);

        return {arr.raw + beg, end - beg};
    }

    template <typename tp_type>
    t_array_mut<tp_type> f_mem_slice_array(const t_array_mut<tp_type> arr, const t_i32 beg, const t_i32 end) {
        ZF_ASSERT(beg >= 0 && beg <= arr.len);
        ZF_ASSERT(end >= beg && end <= arr.len);

        return {arr.raw + beg, end - beg};
    }

    template <typename tp_type>
    t_array_rdonly<tp_type> f_mem_slice_array_from(const t_array_rdonly<tp_type> arr, const t_i32 beg) {
        ZF_ASSERT(beg >= 0 && beg <= arr.len);
        return {arr.raw + beg, arr.len - beg};
    }

    template <typename tp_type>
    t_array_mut<tp_type> f_mem_slice_array_from(const t_array_mut<tp_type> arr, const t_i32 beg) {
        ZF_ASSERT(beg >= 0 && beg <= arr.len);
        return {arr.raw + beg, arr.len - beg};
    }

    template <c_array tp_arr_type>
    t_i32 f_mem_array_size_in_bytes(const tp_arr_type arr) {
        return ZF_SIZE_OF(typename tp_arr_type::t_elem) * arr.len;
    }

    template <c_array tp_arr_type>
    auto f_mem_clone_array(const tp_arr_type arr_to_clone, t_arena *const arena) {
        const auto arr = f_mem_push_array<typename tp_arr_type::t_elem>(arena, arr_to_clone.len);
        CopyAll(arr, arr_to_clone);
        return arr;
    }

    template <typename tp_type, t_i32 tp_len>
    t_array_mut<tp_type> f_mem_as_nonstatic_array(t_static_array<tp_type, tp_len> &arr) {
        return {arr.raw, arr.g_len};
    }

    template <typename tp_type, t_i32 tp_len>
    t_array_rdonly<tp_type> f_mem_as_nonstatic_array(const t_static_array<tp_type, tp_len> &arr) {
        return {arr.raw, arr.g_len};
    }

    template <c_simple tp_type>
    t_array_mut<t_u8> f_mem_as_bytes(tp_type &val) {
        return {reinterpret_cast<t_u8 *>(&val), ZF_SIZE_OF(val)};
    }

    template <c_simple tp_type>
    t_array_rdonly<t_u8> f_mem_as_bytes(const tp_type &val) {
        return {reinterpret_cast<const t_u8 *>(&val), ZF_SIZE_OF(val)};
    }

    template <typename tp_type>
    t_array_mut<t_u8> f_mem_array_as_byte_array(const t_array_mut<tp_type> arr) {
        return {reinterpret_cast<t_u8 *>(arr.raw), f_mem_array_size_in_bytes(arr)};
    }

    template <typename tp_type>
    t_array_rdonly<t_u8> f_mem_array_as_byte_array(const t_array_rdonly<tp_type> arr) {
        return {reinterpret_cast<const t_u8 *>(arr.raw), f_mem_array_size_in_bytes(arr)};
    }

    // Creates a byte-sized bitmask with only a single bit set.
    inline t_u8 f_mem_byte_bitmask_single(const t_i32 bit_index) {
        ZF_ASSERT(bit_index >= 0 && bit_index < 8);
        return static_cast<t_u8>(1 << bit_index);
    }

    // Creates a byte-sized bitmask with only bits in the range [begin_bit_index, end_bit_index) set.
    inline t_u8 f_mem_byte_bitmask_range(const t_i32 begin_bit_index, const t_i32 end_bit_index = 8) {
        ZF_ASSERT(begin_bit_index >= 0 && begin_bit_index < 8);
        ZF_ASSERT(end_bit_index >= begin_bit_index && end_bit_index <= 8);

        if (end_bit_index - begin_bit_index == 8) {
            return 0xFF;
        }

        const auto bits_at_bottom = static_cast<t_u8>((1 << (end_bit_index - begin_bit_index)) - 1);
        return static_cast<t_u8>(bits_at_bottom << begin_bit_index);
    }

    inline t_bit_vec_mut f_mem_create_bit_vector(const t_array_mut<t_u8> bytes) {
        return {bytes.raw, f_bytes_to_bits(bytes.len)};
    }

    inline t_bit_vec_mut f_mem_create_bit_vector(const t_array_mut<t_u8> bytes, const t_i32 bit_cnt) {
        ZF_ASSERT(bit_cnt >= 0 && bit_cnt <= f_bytes_to_bits(bytes.len));
        return {bytes.raw, bit_cnt};
    }

    inline t_bit_vec_rdonly f_mem_create_bit_vector(const t_array_rdonly<t_u8> bytes) {
        return {bytes.raw, f_bytes_to_bits(bytes.len)};
    }

    inline t_bit_vec_rdonly f_mem_create_bit_vector(const t_array_rdonly<t_u8> bytes, const t_i32 bit_cnt) {
        ZF_ASSERT(bit_cnt >= 0 && bit_cnt <= f_bytes_to_bits(bytes.len));
        return {bytes.raw, bit_cnt};
    }

    inline t_bit_vec_mut f_mem_create_bit_vector(const t_i32 bit_cnt, t_arena *const arena) {
        ZF_ASSERT(bit_cnt >= 0);
        return {f_mem_push_array_zeroed<t_u8>(arena, f_bits_to_bytes(bit_cnt)).raw, bit_cnt};
    }

    inline t_array_mut<t_u8> f_mem_bit_vector_bytes(const t_bit_vec_mut bv) {
        return {bv.bytes_raw, f_bits_to_bytes(bv.bit_cnt)};
    }

    inline t_array_rdonly<t_u8> f_mem_bit_vector_bytes(const t_bit_vec_rdonly bv) {
        return {bv.bytes_raw, f_bits_to_bytes(bv.bit_cnt)};
    }

    inline t_i32 f_mem_bit_vector_last_byte_bit_cnt(const t_bit_vec_rdonly bv) {
        return ((bv.bit_cnt - 1) % 8) + 1;
    }

    // Gives a mask of the last byte in which only excess bits are unset.
    inline t_u8 f_mem_bit_vector_last_byte_mask(const t_bit_vec_rdonly bv) {
        return f_mem_byte_bitmask_range(0, f_mem_bit_vector_last_byte_bit_cnt(bv));
    }

    inline t_b8 f_mem_is_bit_set(const t_bit_vec_rdonly bv, const t_i32 index) {
        ZF_ASSERT(index >= 0 && index < bv.bit_cnt);
        return f_mem_bit_vector_bytes(bv)[index / 8] & f_mem_byte_bitmask_single(index % 8);
    }

    inline void f_mem_set_bit(const t_bit_vec_mut bv, const t_i32 index) {
        ZF_ASSERT(index >= 0 && index < bv.bit_cnt);
        f_mem_bit_vector_bytes(bv)[index / 8] |= f_mem_byte_bitmask_single(index % 8);
    }

    inline void f_mem_unset_bit(const t_bit_vec_mut bv, const t_i32 index) {
        ZF_ASSERT(index >= 0 && index < bv.bit_cnt);
        f_mem_bit_vector_bytes(bv)[index / 8] &= ~f_mem_byte_bitmask_single(index % 8);
    }

    t_b8 f_mem_is_any_bit_set(const t_bit_vec_rdonly bv);

    inline t_b8 f_mem_are_all_bits_unset(const t_bit_vec_rdonly bv) {
        return bv.bit_cnt > 0 && !f_mem_is_any_bit_set(bv);
    }

    t_b8 f_mem_are_all_bits_set(const t_bit_vec_rdonly bv);

    inline t_b8 f_mem_is_any_bit_unset(const t_bit_vec_rdonly bv) {
        return bv.bit_cnt > 0 && !f_mem_are_all_bits_set(bv);
    }

    void f_mem_set_all_bits(const t_bit_vec_mut bv);
    void f_mem_unset_all_bits(const t_bit_vec_mut bv);

    // Sets all bits in the range [begin_bit_index, end_bit_index).
    void f_mem_set_bits_in_range(const t_bit_vec_mut bv, const t_i32 begin_bit_index, const t_i32 end_bit_index);

    void f_mem_apply_mask_to_bits(const t_bit_vec_mut targ, const t_bit_vec_rdonly mask, const t_bitwise_mask_op op);

    // Shifts left only by 1. Returns the discarded bit as 0 or 1.
    t_u8 f_mem_shift_bits_left(const t_bit_vec_mut bv);

    void f_mem_shift_bits_left_by(const t_bit_vec_mut bv, const t_i32 amount);

    void f_mem_rot_bits_left_by(const t_bit_vec_mut bv, const t_i32 amount);

    // Shifts right only by 1. Returns the carry bit.
    t_u8 f_mem_shift_bits_right(const t_bit_vec_mut bv);

    void f_mem_shift_bits_right_by(const t_bit_vec_mut bv, const t_i32 amount);

    void f_mem_rot_bits_right_by(const t_bit_vec_mut bv, const t_i32 amount);

    // Returns -1 if all bits are unset.
    t_i32 f_mem_index_of_first_set_bit(const t_bit_vec_rdonly bv, const t_i32 from = 0);

    // Returns -1 if all bits are set.
    t_i32 f_mem_index_of_first_unset_bit(const t_bit_vec_rdonly bv, const t_i32 from = 0);

    t_i32 f_mem_count_set_bits(const t_bit_vec_rdonly bv);

    inline t_i32 f_mem_count_unset_bits(const t_bit_vec_rdonly bv) {
        return bv.bit_cnt - f_mem_count_set_bits(bv);
    }

    // pos is the walker state, initialise it to the bit index you want to start from.
    // o_index is assigned the index of the set bit to process.
    // Returns false iff the walk is complete.
    t_b8 f_mem_walk_set_bits(const t_bit_vec_rdonly bv, t_i32 *const pos, t_i32 *const o_index);

    // pos is the walker state, initialise it to the bit index you want to start from.
    // o_index is assigned the index of the unset bit to process.
    // Returns false iff the walk is complete.
    t_b8 f_mem_walk_unset_bits(const t_bit_vec_rdonly bv, t_i32 *const pos, t_i32 *const o_index);

#define ZF_WALK_SET_BITS(bv, index) for (t_i32 ZF_CONCAT(walk_pos_l, __LINE__) = 0, index; zf::f_mem_walk_set_bits(bv, &ZF_CONCAT(walk_pos_l, __LINE__), &index);)
#define ZF_WALK_UNSET_BITS(bv, index) for (t_i32 ZF_CONCAT(walk_pos_l, __LINE__) = 0, index; zf::f_mem_walk_unset_bits(bv, &ZF_CONCAT(walk_pos_l, __LINE__), &index);)

    // ============================================================
}
