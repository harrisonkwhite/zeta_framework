#pragma once

#include <zcl/zcl_algos.h>

namespace zf {
    // ============================================================
    // @section: Types and Globals

    constexpr t_i32 g_unicode_code_pt_cnt = 1114112;

    using t_code_pt = char32_t;
    using t_code_pt_bit_vec = s_static_bit_vec<g_unicode_code_pt_cnt>;

    constexpr t_i32 g_ascii_range_begin = 0;
    constexpr t_i32 g_ascii_range_end = 0x80;

    constexpr t_i32 g_printable_ascii_range_begin = 0x20;
    constexpr t_i32 g_printable_ascii_range_end = 0x7F;

    struct s_str_rdonly {
        s_array_rdonly<t_u8> bytes;
    };

    struct s_str_mut {
        s_array_mut<t_u8> bytes;

        operator s_str_rdonly() const {
            return {bytes};
        }
    };

    namespace detail {
        // Hidden object that can only be constructed with a valid string literal at compile time.
        // Implicit cast to ZF-style string has to be done at runtime due to reinterpret cast.
        struct s_cstr_literal {
            s_cstr_literal() = delete;

            template <t_i32 tp_buf_size>
            consteval s_cstr_literal(const char (&buf)[tp_buf_size]) : buf(buf), buf_size(tp_buf_size) {
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

            operator s_str_rdonly() {
                return {{reinterpret_cast<const t_u8 *>(buf), buf_size - 1}};
            }

        private:
            const char *const buf;
            const t_i32 buf_size;
        };

#define ZF_STR_LITERAL(cstr_lit) zf::s_str_rdonly(zf::detail::s_cstr_literal(cstr_lit))
    }

    inline const t_comparator_bin<s_str_rdonly> g_str_comparator_bin =
        [](const s_str_rdonly &a, const s_str_rdonly &b) {
            return g_array_comparator_bin<s_array_rdonly<t_u8>>(a.bytes, b.bytes);
        };

    struct s_str_walk_step {
        t_code_pt code_pt;
        t_i32 byte_index;
    };

    // ============================================================


    // ============================================================
    // @section: Functions

    constexpr t_b8 IsCodePointASCII(const t_code_pt cp) {
        return cp >= g_ascii_range_begin && cp < g_ascii_range_end;
    }

    constexpr t_b8 IsCodePointPrintableASCII(const t_code_pt cp) {
        return cp >= g_printable_ascii_range_begin && cp < g_printable_ascii_range_end;
    }

    inline t_b8 AreBytesTerminatedAnywhere(const s_array_rdonly<t_u8> bytes) {
        for (t_i32 i = bytes.len - 1; i >= 0; i--) {
            if (!bytes[i]) {
                return true;
            }
        }

        return false;
    }

    inline t_b8 AreBytesTerminatedOnlyAtEnd(const s_array_rdonly<t_u8> bytes) {
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

    inline t_i32 CalcCstrLen(const char *const cstr) {
        t_i32 len = 0;
        for (; cstr[len]; len++) {}
        return len;
    }

    // Creates a NON-TERMINATED string object from the given TERMINATED C-string.
    // Does a conventional string walk to calculate length.
    inline s_str_mut ConvertCstr(char *const cstr) {
        return {{reinterpret_cast<t_u8 *>(cstr), CalcCstrLen(cstr)}};
    }

    // Creates a read-only NON-TERMINATED string object from the given TERMINATED C-string.
    // Does a conventional string walk to calculate length.
    inline s_str_rdonly ConvertCstr(const char *const cstr) {
        return {{reinterpret_cast<const t_u8 *>(cstr), CalcCstrLen(cstr)}};
    }

    inline char *AsCstr(const s_str_mut str) {
        ZF_ASSERT(AreBytesTerminatedAnywhere(str.bytes));
        return reinterpret_cast<char *>(str.bytes.raw);
    }

    inline const char *AsCstr(const s_str_rdonly str) {
        ZF_ASSERT(AreBytesTerminatedAnywhere(str.bytes));
        return reinterpret_cast<const char *>(str.bytes.raw);
    }

    // Allocates a clone of the given string using the memory arena, with a null byte added at the end (even if the string was already terminated).
    inline s_str_mut CloneStrButAddTerminator(const s_str_rdonly str, s_arena *const arena) {
        const s_str_mut clone = {PushArray<t_u8>(arena, str.bytes.len + 1)};
        CopyAll(str.bytes, clone.bytes);
        clone.bytes[clone.bytes.len - 1] = 0;
        return clone;
    }

    inline t_b8 IsStrEmpty(const s_str_rdonly str) {
        return str.bytes.len == 0;
    }

    inline t_b8 AreStrsEqual(const s_str_rdonly a, const s_str_rdonly b) {
        return CompareAll(a.bytes, b.bytes) == 0;
    }

    t_b8 IsValidUTF8(const s_str_rdonly str);

    // Calculates the length in terms of code point count. Note that '\0' is treated just like any other ASCII character and does not terminate.
    t_i32 CalcStrLen(const s_str_rdonly str);

    t_code_pt FindCodePointAtByte(const s_str_rdonly str, const t_i32 byte_index);

    // Sets the bits associated with each unicode code point that appear in the string. No bits get unset.
    void MarkCodePoints(const s_str_rdonly str, t_code_pt_bit_vec *const code_pts);

    // byte_index should be initialised to the index of ANY byte in the code point to start walking from.
    // Returns false iff the walk has ended.
    t_b8 Walk(const s_str_rdonly str, t_i32 *const byte_index, s_str_walk_step *const o_step);

    // byte_index should be initialised to the index of ANY byte in the code point to start walking backwards from.
    // Returns false iff the walk has ended.
    t_b8 WalkReverse(const s_str_rdonly str, t_i32 *const byte_index, s_str_walk_step *const o_step);

#define ZF_WALK_STR(str, step)                                                                                     \
    for (zf::t_i32 ZF_CONCAT(bi_l, __LINE__) = 0; ZF_CONCAT(bi_l, __LINE__) != -1; ZF_CONCAT(bi_l, __LINE__) = -1) \
        for (zf::s_str_walk_step step; zf::Walk(str, &ZF_CONCAT(bi_l, __LINE__), &step);)

#define ZF_WALK_STR_REVERSE(str, step)                                                                                                                         \
    for (zf::t_i32 ZF_CONCAT(bi_l, __LINE__) = (str).bytes.len - 1; ZF_CONCAT(bi_l, __LINE__) != (str).bytes.len; ZF_CONCAT(bi_l, __LINE__) = (str).bytes.len) \
        for (zf::s_str_walk_step step; zf::WalkReverse(str, &ZF_CONCAT(bi_l, __LINE__), &step);)

    // ============================================================
}
