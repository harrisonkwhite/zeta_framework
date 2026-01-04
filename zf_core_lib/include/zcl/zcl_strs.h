#pragma once

#include <zcl/zcl_algos.h>

namespace zf::strs {
    // ============================================================
    // @section: Types and Globals

    constexpr I32 g_code_pt_cnt = 1114112;

    using CodePoint = char32_t;
    using CodePointBitVector = s_static_bit_vec<g_code_pt_cnt>;

    constexpr I32 g_ascii_range_begin = 0;
    constexpr I32 g_ascii_range_end = 0x80;

    constexpr I32 g_printable_ascii_range_begin = 0x20;
    constexpr I32 g_printable_ascii_range_end = 0x7F;

    struct StrRdonly {
        s_array_rdonly<U8> bytes;
    };

    struct StrMut {
        s_array_mut<U8> bytes;

        operator StrRdonly() const {
            return {bytes};
        }
    };

    namespace detail {
        // Hidden object that can only be constructed with a valid string literal at compile time.
        // Implicit cast to ZF-style string has to be done at runtime due to reinterpret cast.
        struct CstrLiteral {
            CstrLiteral() = delete;

            template <I32 tp_buf_size>
            consteval CstrLiteral(const char (&buf)[tp_buf_size]) : buf(buf), buf_size(tp_buf_size) {
                if (buf[tp_buf_size - 1]) {
                    throw "Static char array not terminated at end!";
                }

                // Disabled because sometimes it might be useful to manually insert null characters before end.
#if 0
                for (I32 i = 0; i < tp_buf_size; i++) {
                    if (i < tp_buf_size - 1 && !buf[i]) {
                        throw "Terminator found in static char array before end!";
                    }
                }
#endif
            }

            operator StrRdonly() {
                return {{reinterpret_cast<const U8 *>(buf), buf_size - 1}};
            }

        private:
            const char *const buf;
            const I32 buf_size;
        };

#define ZF_STR_LITERAL(cstr_lit) zf::strs::StrRdonly(zf::strs::detail::CstrLiteral(cstr_lit))
    }

    inline const t_comparator_bin<StrRdonly> g_str_comparator_bin =
        [](const StrRdonly &a, const StrRdonly &b) {
            return g_array_comparator_bin<s_array_rdonly<U8>>(a.bytes, b.bytes);
        };

    struct StrWalkStep {
        CodePoint code_pt;
        I32 byte_index;
    };

    // ============================================================


    // ============================================================
    // @section: Functions

    constexpr B8 get_code_point_is_ascii(const CodePoint cp) {
        return cp >= g_ascii_range_begin && cp < g_ascii_range_end;
    }

    constexpr B8 get_code_point_is_printable_ascii(const CodePoint cp) {
        return cp >= g_printable_ascii_range_begin && cp < g_printable_ascii_range_end;
    }

    inline B8 determine_are_bytes_terminated_anywhere(const s_array_rdonly<U8> bytes) {
        for (I32 i = bytes.len - 1; i >= 0; i--) {
            if (!bytes[i]) {
                return true;
            }
        }

        return false;
    }

    inline B8 determine_are_bytes_terminated_only_at_end(const s_array_rdonly<U8> bytes) {
        if (bytes.len == 0 || bytes[bytes.len - 1]) {
            return false;
        }

        for (I32 i = bytes.len - 2; i >= 0; i--) {
            if (!bytes[i]) {
                return false;
            }
        }

        return true;
    }

    inline I32 calc_cstr_len(const char *const cstr) {
        I32 len = 0;
        for (; cstr[len]; len++) {}
        return len;
    }

    // Creates a NON-TERMINATED string object from the given TERMINATED C-string.
    // Does a conventional string walk to calculate length.
    inline StrMut convert_cstr(char *const cstr) {
        return {{reinterpret_cast<U8 *>(cstr), calc_cstr_len(cstr)}};
    }

    // Creates a read-only NON-TERMINATED string object from the given TERMINATED C-string.
    // Does a conventional string walk to calculate length.
    inline StrRdonly convert_cstr(const char *const cstr) {
        return {{reinterpret_cast<const U8 *>(cstr), calc_cstr_len(cstr)}};
    }

    inline char *get_as_cstr(const StrMut str) {
        ZF_ASSERT(determine_are_bytes_terminated_anywhere(str.bytes));
        return reinterpret_cast<char *>(str.bytes.raw);
    }

    inline const char *get_as_cstr(const StrRdonly str) {
        ZF_ASSERT(determine_are_bytes_terminated_anywhere(str.bytes));
        return reinterpret_cast<const char *>(str.bytes.raw);
    }

    // Allocates a clone of the given string using the memory arena, with a null byte added at the end (even if the string was already terminated).
    inline StrMut clone_str_but_add_terminator(const StrRdonly str, s_arena *const arena) {
        const StrMut clone = {ArenaPushArray<U8>(arena, str.bytes.len + 1)};
        CopyAll(str.bytes, clone.bytes);
        clone.bytes[clone.bytes.len - 1] = 0;
        return clone;
    }

    inline B8 get_is_empty(const StrRdonly str) {
        return str.bytes.len == 0;
    }

    inline B8 determine_are_equal(const StrRdonly a, const StrRdonly b) {
        return CompareAll(a.bytes, b.bytes) == 0;
    }

    B8 determine_is_valid_utf8(const StrRdonly str);

    // Calculates the string length in terms of code point count. Note that '\0' is treated just like any other ASCII character and does not terminate.
    I32 calc_len(const StrRdonly str);

    CodePoint find_code_point_at_byte(const StrRdonly str, const I32 byte_index);

    // Sets the bits associated with each unicode code point that appear in the string. No bits get unset.
    void mark_code_points(const StrRdonly str, CodePointBitVector *const code_pts);

    // byte_index should be initialised to the index of ANY byte in the code point to start walking from.
    // Returns false iff the walk has ended.
    B8 walk(const StrRdonly str, I32 *const byte_index, StrWalkStep *const o_step);

    // byte_index should be initialised to the index of ANY byte in the code point to start walking backwards from.
    // Returns false iff the walk has ended.
    B8 walk_reverse(const StrRdonly str, I32 *const byte_index, StrWalkStep *const o_step);

#define ZF_WALK_STR(str, step)                                                                                   \
    for (zf::I32 ZF_CONCAT(bi_l, __LINE__) = 0; ZF_CONCAT(bi_l, __LINE__) != -1; ZF_CONCAT(bi_l, __LINE__) = -1) \
        for (zf::strs::StrWalkStep step; zf::strs::walk(str, &ZF_CONCAT(bi_l, __LINE__), &step);)

#define ZF_WALK_STR_REVERSE(str, step)                                                                                                                       \
    for (zf::I32 ZF_CONCAT(bi_l, __LINE__) = (str).bytes.len - 1; ZF_CONCAT(bi_l, __LINE__) != (str).bytes.len; ZF_CONCAT(bi_l, __LINE__) = (str).bytes.len) \
        for (zf::strs::StrWalkStep step; zf::strs::walk_reverse(str, &ZF_CONCAT(bi_l, __LINE__), &step);)

    // ============================================================
}
