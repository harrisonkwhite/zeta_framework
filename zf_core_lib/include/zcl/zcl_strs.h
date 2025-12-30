#pragma once

#include <zcl/zcl_mem.h>

namespace zf {
    using t_code_pt = char32_t;

    constexpr t_i32 g_unicode_code_pt_cnt = 1114112;

    using t_code_pt_bit_vec = s_static_bit_vec<g_unicode_code_pt_cnt>;

    constexpr t_i32 UnicodeCodePointToByteCnt(const t_code_pt cp) {
        if (cp <= 0x7F) {
            return 1;
        }

        if (cp <= 0x7FF) {
            return 2;
        }

        if (cp <= 0xFFFF) {
            return 3;
        }

        if (cp <= 0x10FFFF) {
            return 4;
        }

        ZF_ASSERT(false);

        return 0;
    }

    constexpr t_i32 g_ascii_range_begin = 0;
    constexpr t_i32 g_ascii_range_end = 0x80;

    constexpr t_b8 IsASCII(const t_code_pt cp) {
        return cp >= g_ascii_range_begin && cp < g_ascii_range_end;
    }

    constexpr t_i32 g_printable_ascii_range_begin = 0x20;
    constexpr t_i32 g_printable_ascii_range_end = 0x7F;

    constexpr t_b8 IsPrintableASCII(const t_code_pt cp) {
        return cp >= g_printable_ascii_range_begin && cp < g_printable_ascii_range_end;
    }

    constexpr t_i32 CalcCstrLen(const char *const cstr) {
        t_i32 len = 0;
        for (; cstr[len]; len++) {}
        return len;
    }

    // Does a 0 appear anywhere in the array?
    constexpr t_b8 AreBytesTerminated(const c_array_rdonly<t_u8> bytes) {
        for (t_i32 i = bytes.Len() - 1; i >= 0; i--) {
            if (!bytes[i]) {
                return true;
            }
        }

        return false;
    }

    class c_cstr_literal {
    public:
        const c_array_rdonly<char> buf;

        c_cstr_literal() = delete;

        template <t_i32 tp_raw_size>
        consteval c_cstr_literal(const char (&raw)[tp_raw_size]) : buf({raw, tp_raw_size}) {
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
        c_array_rdonly<t_u8> bytes;

        s_str_rdonly() = default;
        s_str_rdonly(const c_array_rdonly<t_u8> bytes) : bytes(bytes) {}

        // This very intentionally drops the terminator.
        s_str_rdonly(const c_cstr_literal lit) : bytes({reinterpret_cast<const t_u8 *>(lit.buf.Raw()), lit.buf.Len() - 1}) {}

        // Requires that there is a terminating byte somewhere.
        const char *Cstr() const {
            ZF_REQUIRE(AreBytesTerminated(bytes));
            return reinterpret_cast<const char *>(bytes.Raw());
        }
    };

    struct s_str {
        c_array_mut<t_u8> bytes;

        s_str() = default;
        s_str(const c_array_mut<t_u8> bytes) : bytes(bytes) {}

        operator s_str_rdonly() const {
            return {bytes};
        }

        // Requires that there is a terminating byte somewhere.
        char *Cstr() const {
            ZF_REQUIRE(AreBytesTerminated(bytes));
            return reinterpret_cast<char *>(bytes.Raw());
        }
    };

    inline t_bin_comparator<s_str_rdonly> g_str_bin_comparator =
        [](const s_str_rdonly &a, const s_str_rdonly &b) {
            return g_array_bin_comparator<c_array_rdonly<t_u8>>(a.bytes, b.bytes);
        };

    // Creates a NON-TERMINATED string object from the given TERMINATED C-string.
    // Does a conventional string walk to calculate length.
    inline s_str ConvertCstr(char *const cstr) {
        return {{reinterpret_cast<t_u8 *>(cstr), CalcCstrLen(cstr)}};
    }

    // Creates a read-only NON-TERMINATED string object from the given TERMINATED C-string.
    // Does a conventional string walk to calculate length.
    inline s_str_rdonly ConvertCstr(const char *const cstr) {
        return {{reinterpret_cast<const t_u8 *>(cstr), CalcCstrLen(cstr)}};
    }

    // Allocates a clone of the given string using the memory arena, with a null byte added at the end (even if the string was already terminated).
    inline s_str AllocStrCloneButAddTerminator(const s_str_rdonly str, c_arena *const arena) {
        const s_str clone = {AllocArray<t_u8>(str.bytes.Len() + 1, arena)};
        Copy(clone.bytes, str.bytes);
        clone.bytes[clone.bytes.Len() - 1] = 0;
        return clone;
    }

    inline t_b8 IsStrEmpty(const s_str_rdonly str) {
        return str.bytes.Len() == 0;
    }

    inline t_b8 AreStrsEqual(const s_str_rdonly a, const s_str_rdonly b) {
        return Compare(a.bytes, b.bytes) == 0;
    }

    t_b8 IsStrValidUTF8(const s_str_rdonly str);

    // Calculates the length in terms of code point count. Note that '\0' is treated just like any other ASCII character.
    t_i32 CalcStrLen(const s_str_rdonly str);

    t_code_pt FindStrCodePointAtByte(const s_str_rdonly str, const t_i32 byte_index);

    // Sets the bits associated with each unicode code point that appear in the string. No bits get unset.
    void MarkStrCodePoints(const s_str_rdonly str, t_code_pt_bit_vec *const code_pts);

    struct s_str_walk_info {
        t_code_pt code_pt;
        t_i32 byte_index;
    };

    // byte_index should be initialised to the index of ANY byte in the code point to start walking from.
    // Returns false iff the walk has ended.
    t_b8 WalkStr(const s_str_rdonly str, t_i32 *const byte_index, s_str_walk_info *const o_info);

    // byte_index should be initialised to the index of ANY byte in the code point to start walking backwards from.
    // Returns false iff the walk has ended.
    t_b8 WalkStrReverse(const s_str_rdonly str, t_i32 *const byte_index, s_str_walk_info *const o_info);

#define ZF_WALK_STR(str, info)                                                                                 \
    for (t_i32 ZF_CONCAT(bi_l, __LINE__) = 0; ZF_CONCAT(bi_l, __LINE__) != -1; ZF_CONCAT(bi_l, __LINE__) = -1) \
        for (s_str_walk_info info; WalkStr(str, &ZF_CONCAT(bi_l, __LINE__), &info);)

#define ZF_WALK_STR_REVERSE(str, info)                                                                                                                           \
    for (t_i32 ZF_CONCAT(bi_l, __LINE__) = (str).bytes.Len() - 1; ZF_CONCAT(bi_l, __LINE__) != (str).bytes.Len(); ZF_CONCAT(bi_l, __LINE__) = (str).bytes.Len()) \
        for (s_str_walk_info info; WalkStrReverse(str, &ZF_CONCAT(bi_l, __LINE__), &info);)
}
