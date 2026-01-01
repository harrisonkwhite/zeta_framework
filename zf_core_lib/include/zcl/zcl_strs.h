#pragma once

#include <zcl/zcl_mem.h>

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

    struct s_cstr_literal {
        s_array_rdonly<char> buf;

        s_cstr_literal() = default;

        template <t_i32 tp_raw_size>
        consteval s_cstr_literal(const char (&raw)[tp_raw_size]) : buf({raw, tp_raw_size}) {
            if (raw[tp_raw_size - 1]) {
                throw "Static char array not terminated at end!";
            }

            for (t_i32 i = 0; i < tp_raw_size; i++) {
                if (i < tp_raw_size - 1 && !raw[i]) {
                    throw "Terminator found in static char array before end!";
                }
            }
        }
    };

    struct s_str_rdonly {
        s_array_rdonly<char> bytes;

        s_str_rdonly() = default;
        s_str_rdonly(const s_array_rdonly<char> bytes) : bytes(bytes) {}

        template <t_i32 tp_raw_size>
        consteval s_str_rdonly(const char (&raw)[tp_raw_size]) : bytes({raw, tp_raw_size - 1}) {
            if (raw[tp_raw_size - 1]) {
                throw "Static char array not terminated at end!";
            }

            for (t_i32 i = 0; i < tp_raw_size; i++) {
                if (i < tp_raw_size - 1 && !raw[i]) {
                    throw "Terminator found in static char array before end!";
                }
            }
        }
    };

    struct s_str_mut {
        s_array_mut<char> bytes;

        s_str_mut() = default;
        s_str_mut(const s_array_mut<char> bytes) : bytes(bytes) {}

        operator s_str_rdonly() const {
            return {bytes};
        }
    };

#if 0
    inline t_bin_comparator<s_str_rdonly> g_str_bin_comparator =
        [](const s_str_rdonly &a, const s_str_rdonly &b) {
            return g_array_bin_comparator<s_array_rdonly<t_u8>>(a.bytes, b.bytes);
        };
#endif

    struct s_str_walk_step {
        t_code_pt code_pt;
        t_i32 byte_index;
    };

    // ============================================================


    // ============================================================
    // @section: Functions

    constexpr t_b8 CodePointIsASCII(const t_code_pt cp) {
        return cp >= g_ascii_range_begin && cp < g_ascii_range_end;
    }

    constexpr t_b8 CodePointIsPrintableASCII(const t_code_pt cp) {
        return cp >= g_printable_ascii_range_begin && cp < g_printable_ascii_range_end;
    }

    // Does a 0 appear anywhere in the array?
    inline t_b8 AreBytesTerminated(const s_array_rdonly<char> bytes) {
        for (t_i32 i = bytes.len - 1; i >= 0; i--) {
            if (!bytes[i]) {
                return true;
            }
        }

        return false;
    }

    constexpr t_i32 CstrCalcLen(const char *const cstr) {
        t_i32 len = 0;
        for (; cstr[len]; len++) {}
        return len;
    }

#if 0
    // Creates a NON-TERMINATED string object from the given TERMINATED C-string.
    // Does a conventional string walk to calculate length.
    constexpr s_str_mut CstrToStr(char *const cstr) {
        return {{cstr, CstrCalcLen(cstr)}};
    }

    // Creates a read-only NON-TERMINATED string object from the given TERMINATED C-string.
    // Does a conventional string walk to calculate length.
    inline s_str_rdonly CstrToStr(const char *const cstr) {
        return {{reinterpret_cast<const t_u8 *>(cstr), CstrCalcLen(cstr)}};
    }
#endif

    // REQUIRES that the string is terminated somewhere. Use very sparingly!
    inline char *StrAsCstr(const s_str_mut str) {
        ZF_REQUIRE(AreBytesTerminated(str.bytes));
        return reinterpret_cast<char *>(str.bytes.raw);
    }

    // REQUIRES that the string is terminated somewhere. Use very sparingly!
    inline const char *StrAsCstr(const s_str_rdonly str) {
        ZF_REQUIRE(AreBytesTerminated(str.bytes));
        return reinterpret_cast<const char *>(str.bytes.raw);
    }

    // Allocates a clone of the given string using the memory arena, with a null byte added at the end (even if the string was already terminated).
    inline s_str_mut StrCloneButAddTerminator(const s_str_rdonly str, s_arena *const arena) {
        const s_str_mut clone = {ArenaPushArray<char>(arena, str.bytes.len + 1)};
        CopyAll(str.bytes, clone.bytes);
        clone.bytes[clone.bytes.len - 1] = 0;
        return clone;
    }

    inline t_b8 StrIsEmpty(const s_str_rdonly str) {
        return str.bytes.len == 0;
    }

    inline t_b8 StrEquals(const s_str_rdonly str, const s_str_rdonly other) {
        return CompareAll(str.bytes, other.bytes) == 0;
    }

    t_b8 StrIsValidUTF8(const s_str_rdonly str);

    // Calculates the length in terms of code point count. Note that '\0' is treated just like any other ASCII character and does not terminate.
    t_i32 StrCalcLen(const s_str_rdonly str);

    t_code_pt StrFindCodePointAtByte(const s_str_rdonly str, const t_i32 byte_index);

    // Sets the bits associated with each unicode code point that appear in the string. No bits get unset.
    void StrMarkCodePoints(const s_str_rdonly str, t_code_pt_bit_vec *const code_pts);

    // byte_index should be initialised to the index of ANY byte in the code point to start walking from.
    // Returns false iff the walk has ended.
    t_b8 StrWalk(const s_str_rdonly str, t_i32 *const byte_index, s_str_walk_step *const o_step);

    // byte_index should be initialised to the index of ANY byte in the code point to start walking backwards from.
    // Returns false iff the walk has ended.
    t_b8 StrWalkReverse(const s_str_rdonly str, t_i32 *const byte_index, s_str_walk_step *const o_step);

#define ZF_STR_WALK(str, step)                                                                                     \
    for (zf::t_i32 ZF_CONCAT(bi_l, __LINE__) = 0; ZF_CONCAT(bi_l, __LINE__) != -1; ZF_CONCAT(bi_l, __LINE__) = -1) \
        for (zf::s_str_walk_step step; zf::StrWalk(str, &ZF_CONCAT(bi_l, __LINE__), &step);)

#define ZF_STR_WALK_REVERSE(str, step)                                                                                                                         \
    for (zf::t_i32 ZF_CONCAT(bi_l, __LINE__) = (str).bytes.len - 1; ZF_CONCAT(bi_l, __LINE__) != (str).bytes.len; ZF_CONCAT(bi_l, __LINE__) = (str).bytes.len) \
        for (zf::s_str_walk_step step; zf::StrWalkReverse(str, &ZF_CONCAT(bi_l, __LINE__), &step);)

    // ============================================================
}
