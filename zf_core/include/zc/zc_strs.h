// Strings in ZF are all UTF-8 and are NON-TERMINATED.
// '\0' is now treated just like any other non-printable ASCII character.
// This adds a huge benefit in terms of simplifying all string-related code.
// The cost however is that interop with C standard library functions and most other libraries is more awkward, though these are insignificant portions of the code. The approach generally is to make a terminated clone of the string on a temporary memory arena. Since this is just an arena push as opposed to a heap allocation, the cost is trivial.

#pragma once

#include <zc/zc_mem.h>

namespace zf {
    using t_unicode_code_pt = char32_t;

    constexpr t_size g_unicode_code_pt_cnt = 1114112;

    using t_unicode_code_pt_bits = s_static_array<t_u8, BitsToBytes(g_unicode_code_pt_cnt)>;

    constexpr t_size UnicodeCodePointToByteCnt(const t_unicode_code_pt cp) {
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

    constexpr t_size g_ascii_printable_range_begin = 0x20;
    constexpr t_size g_ascii_printable_range_end = 0x7F;

    constexpr t_b8 IsASCII(const t_unicode_code_pt cp) {
        return cp <= 0x7F;
    }

    struct s_str_rdonly {
        s_array_rdonly<t_u8> bytes;

        constexpr s_str_rdonly() = default;
        constexpr s_str_rdonly(const s_array_rdonly<t_u8> bytes) : bytes(bytes) {}

        template <t_size tp_len>
        constexpr s_str_rdonly(const char (&raw)[tp_len]) : bytes({reinterpret_cast<const t_u8*>(raw), tp_len - 1}) {}
    };

    struct s_str {
        s_array<t_u8> bytes;

        constexpr operator s_str_rdonly() const {
            return {bytes};
        }
    };

    constexpr t_size CountRawChrsBeforeNull(const char* const raw) {
        t_size cnt = 0;
        for (; raw[cnt]; cnt++) {}
        return cnt;
    }

    constexpr t_b8 IsStrTerminatedOnlyAtEnd(const s_str_rdonly str) {
        // Make sure no terminators exist before end.
        for (t_size i = 0; i < str.bytes.len - 1; i++) {
            if (!str.bytes[i]) {
                return false;
            }
        }

        return !str.bytes[str.bytes.len - 1];
    }

    inline char* StrRaw(const s_str str) {
        ZF_ASSERT(IsStrTerminatedOnlyAtEnd(str));
        return reinterpret_cast<char*>(str.bytes.buf_raw);
    }

    inline const char* StrRaw(const s_str_rdonly str) {
        ZF_ASSERT(IsStrTerminatedOnlyAtEnd(str));
        return reinterpret_cast<const char*>(str.bytes.buf_raw);
    }

    inline s_str StrFromRaw(char* const raw) {
        return {{reinterpret_cast<t_u8*>(raw), CountRawChrsBeforeNull(raw)}};
    }

    inline s_str_rdonly StrFromRaw(const char* const raw) {
        return {{reinterpret_cast<const t_u8*>(raw), CountRawChrsBeforeNull(raw)}};
    }

    inline s_str StrFromRaw(char* const raw, const t_size len) {
        ZF_ASSERT(len == CountRawChrsBeforeNull(raw));
        return {{reinterpret_cast<t_u8*>(raw), len}};
    }

    inline s_str_rdonly StrFromRaw(const char* const raw, const t_size len) {
        ZF_ASSERT(len == CountRawChrsBeforeNull(raw));
        return {{reinterpret_cast<const t_u8*>(raw), len}};
    }

    [[nodiscard]] inline t_b8 CloneStrButAddTerminator(const s_str_rdonly src, s_mem_arena& mem_arena, s_str& o_clone) {
        o_clone = {};

        if (!MakeArray(mem_arena, src.bytes.len + 1, o_clone.bytes)) {
            return false;
        }

        Copy(o_clone.bytes, src.bytes);
        ZF_ASSERT(!o_clone.bytes[o_clone.bytes.len - 1]);

        return true;
    }

    constexpr t_b8 IsStrEmpty(const s_str_rdonly str) {
        return IsArrayEmpty(str.bytes);
    }

    t_b8 IsValidUTF8Str(const s_str_rdonly str);
    t_size CalcStrLen(const s_str_rdonly str);
    t_unicode_code_pt StrChrAtByte(const s_str_rdonly str, const t_size byte_index);

    struct s_str_chr_info {
        t_unicode_code_pt code_pt;
        t_size byte_index;
    };

    t_b8 WalkStr(const s_str_rdonly str, t_size& byte_index, s_str_chr_info& o_chr_info); // Returns false iff the walk has ended.
    t_b8 WalkStrReverse(const s_str_rdonly str, t_size& byte_index, s_str_chr_info& o_chr_info); // Returns false iff the walk has ended. For a full walk, the byte index has to be initialised to the index of ANY BYTE in the last character.

    t_unicode_code_pt UTF8ChrBytesToCodePoint(const s_array_rdonly<t_u8> bytes); // Only accepts bytes representing a unicode code point in an array with a length in [1, 4].
    void MarkStrCodePoints(const s_str_rdonly str, t_unicode_code_pt_bits& code_pt_bits); // Sets the bits associated with each unicode code point that appear in the input string.

#define ZF_WALK_STR(str, chr_info) \
    for (t_size ZF_CONCAT(p_bi_l, __LINE__) = 0; ZF_CONCAT(p_bi_l, __LINE__) != -1; ZF_CONCAT(p_bi_l, __LINE__) = -1) \
        for (s_str_chr_info chr_info; WalkStr(str, ZF_CONCAT(p_bi_l, __LINE__), chr_info); )

#define ZF_WALK_STR_REVERSE(str, chr_info) \
    for (t_size ZF_CONCAT(p_bi_l, __LINE__) = str.bytes.len - 1; ZF_CONCAT(p_bi_l, __LINE__) != str.bytes.len; ZF_CONCAT(p_bi_l, __LINE__) = str.bytes.len) \
        for (s_str_chr_info chr_info; WalkStrReverse(str, ZF_CONCAT(p_bi_l, __LINE__), chr_info); )
}
