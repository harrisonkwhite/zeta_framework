#pragma once

#include <zcl/zcl_algos.h>

namespace zf {
    // ============================================================
    // @section: Types and Globals

    constexpr t_i32 g_code_pt_cnt = 1114112;

    using t_code_pt = char32_t;
    using t_code_pt_bit_vec = mem::t_static_bitset<g_code_pt_cnt>;

    constexpr t_i32 g_ascii_range_begin = 0;
    constexpr t_i32 g_ascii_range_end = 0x80;

    constexpr t_i32 g_printable_ascii_range_begin = 0x20;
    constexpr t_i32 g_printable_ascii_range_end = 0x7F;

    struct t_str_rdonly {
        t_array_rdonly<t_u8> bytes;
    };

    struct t_str_mut {
        t_array_mut<t_u8> bytes;

        operator t_str_rdonly() const {
            return {bytes};
        }
    };

    namespace detail {
        // Hidden object that can only be constructed with a valid string literal at compile time.
        // Implicit cast to ZF-style string has to be done at runtime due to reinterpret cast.
        struct t_cstr_literal {
            t_cstr_literal() = delete;

            template <t_i32 tp_buf_size>
            consteval t_cstr_literal(const char (&buf)[tp_buf_size]) : buf(buf), buf_size(tp_buf_size) {
                if (buf[tp_buf_size - 1]) {
                    throw "Static char array not terminated at end!";
                }

                // Disabled because sometimes it might be useful to manually insert null characters before end.
#if 0
                for (t_i32 i = 0; i < tp_buf_size; i++) {
                    if (i < tp_buf_size - 1 && !buf[i]) {
                        throw "Terminator found in static char array before end!";
                    }
                }
#endif
            }

            operator t_str_rdonly() {
                return {{reinterpret_cast<const t_u8 *>(buf), buf_size - 1}};
            }

        private:
            const char *const buf;
            const t_i32 buf_size;
        };

#define ZF_STR_LITERAL(cstr_lit) zf::t_str_rdonly(zf::detail::t_cstr_literal(cstr_lit))
    }

    inline const t_comparator_bin<t_str_rdonly> g_str_comparator_bin =
        [](const t_str_rdonly &a, const t_str_rdonly &b) {
            return g_array_comparator_bin<t_array_rdonly<t_u8>>(a.bytes, b.bytes);
        };

    struct t_str_walk_step {
        t_code_pt code_pt;
        t_i32 byte_index;
    };

    // ============================================================


    // ============================================================
    // @section: Functions

    constexpr t_b8 f_strs_is_code_pt_ascii(const t_code_pt cp) {
        return cp >= g_ascii_range_begin && cp < g_ascii_range_end;
    }

    constexpr t_b8 f_strs_is_code_pt_printable_ascii(const t_code_pt cp) {
        return cp >= g_printable_ascii_range_begin && cp < g_printable_ascii_range_end;
    }

    inline t_b8 f_strs_are_bytes_terminated_anywhere(const t_array_rdonly<t_u8> bytes) {
        for (t_i32 i = bytes.len - 1; i >= 0; i--) {
            if (!bytes[i]) {
                return true;
            }
        }

        return false;
    }

    inline t_b8 f_strs_are_bytes_terminated_only_at_end(const t_array_rdonly<t_u8> bytes) {
        if (bytes.len == 0 || bytes[bytes.len - 1]) {
            return false;
        }

        for (t_i32 i = bytes.len - 2; i >= 0; i--) {
            if (!bytes[i]) {
                return false;
            }
        }

        return true;
    }

    inline t_i32 f_strs_get_cstr_len(const char *const cstr) {
        t_i32 len = 0;
        for (; cstr[len]; len++) {}
        return len;
    }

    // Creates a NON-TERMINATED string object from the given TERMINATED C-string.
    // Does a conventional string walk to calculate length.
    inline t_str_mut f_strs_convert_cstr(char *const cstr) {
        return {{reinterpret_cast<t_u8 *>(cstr), f_strs_get_cstr_len(cstr)}};
    }

    // Creates a read-only NON-TERMINATED string object from the given TERMINATED C-string.
    // Does a conventional string walk to calculate length.
    inline t_str_rdonly f_strs_convert_cstr(const char *const cstr) {
        return {{reinterpret_cast<const t_u8 *>(cstr), f_strs_get_cstr_len(cstr)}};
    }

    inline char *f_strs_get_as_cstr(const t_str_mut str) {
        ZF_ASSERT(f_strs_are_bytes_terminated_anywhere(str.bytes));
        return reinterpret_cast<char *>(str.bytes.raw);
    }

    inline const char *f_strs_get_as_cstr(const t_str_rdonly str) {
        ZF_ASSERT(f_strs_are_bytes_terminated_anywhere(str.bytes));
        return reinterpret_cast<const char *>(str.bytes.raw);
    }

    // Allocates a clone of the given string using the memory arena, with a null byte added at the end (even if the string was already terminated).
    inline t_str_mut f_strs_clone_but_add_terminator(const t_str_rdonly str, mem::t_arena *const arena) {
        const t_str_mut clone = {mem::f_arena_push_array<t_u8>(arena, str.bytes.len + 1)};
        f_algos_copy_all(str.bytes, clone.bytes);
        clone.bytes[clone.bytes.len - 1] = 0;
        return clone;
    }

    inline t_b8 f_strs_is_empty(const t_str_rdonly str) {
        return str.bytes.len == 0;
    }

    inline t_b8 f_strs_are_equal(const t_str_rdonly a, const t_str_rdonly b) {
        return f_algos_compare_all(a.bytes, b.bytes) == 0;
    }

    t_b8 f_strs_is_valid_utf8(const t_str_rdonly str);

    // Calculates the string length in terms of code point count. Note that '\0' is treated just like any other ASCII character and does not terminate.
    t_i32 f_strs_get_len(const t_str_rdonly str);

    t_code_pt f_strs_find_code_pt_at_byte(const t_str_rdonly str, const t_i32 byte_index);

    // Sets the bits associated with each unicode code point that appear in the string. No bits get unset.
    void f_strs_mark_code_points(const t_str_rdonly str, t_code_pt_bit_vec *const code_pts);

    // byte_index should be initialised to the index of ANY byte in the code point to start walking from.
    // Returns false iff the walk has ended.
    t_b8 f_strs_walk(const t_str_rdonly str, t_i32 *const byte_index, t_str_walk_step *const o_step);

    // byte_index should be initialised to the index of ANY byte in the code point to start walking backwards from.
    // Returns false iff the walk has ended.
    t_b8 f_strs_walk_reverse(const t_str_rdonly str, t_i32 *const byte_index, t_str_walk_step *const o_step);

#define ZF_WALK_STR(str, step)                                                                                     \
    for (zf::t_i32 ZF_CONCAT(bi_l, __LINE__) = 0; ZF_CONCAT(bi_l, __LINE__) != -1; ZF_CONCAT(bi_l, __LINE__) = -1) \
        for (zf::t_str_walk_step step; zf::f_strs_walk(str, &ZF_CONCAT(bi_l, __LINE__), &step);)

#define ZF_WALK_STR_REVERSE(str, step)                                                                                                                         \
    for (zf::t_i32 ZF_CONCAT(bi_l, __LINE__) = (str).bytes.len - 1; ZF_CONCAT(bi_l, __LINE__) != (str).bytes.len; ZF_CONCAT(bi_l, __LINE__) = (str).bytes.len) \
        for (zf::t_str_walk_step step; zf::f_strs_walk_reverse(str, &ZF_CONCAT(bi_l, __LINE__), &step);)

    // ============================================================
}
