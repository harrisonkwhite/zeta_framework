#pragma once

#include <zcl/zcl_mem.h>

namespace zf {
    using t_unicode_code_pt = char32_t;

    constexpr t_len g_unicode_code_pt_cnt = 1114112;

    using t_unicode_code_pt_bit_vec = s_static_bit_vec<g_unicode_code_pt_cnt>;

    constexpr t_len UnicodeCodePointToByteCnt(const t_unicode_code_pt cp) {
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

    constexpr t_len g_ascii_range_begin = 0;
    constexpr t_len g_ascii_range_end = 0x80;

    constexpr t_b8 IsASCII(const t_unicode_code_pt cp) {
        return cp >= g_ascii_range_begin && cp < g_ascii_range_end;
    }

    constexpr t_len g_printable_ascii_range_begin = 0x20;
    constexpr t_len g_printable_ascii_range_end = 0x7F;

    constexpr t_b8 IsPrintableASCII(const t_unicode_code_pt cp) {
        return cp >= g_printable_ascii_range_begin && cp < g_printable_ascii_range_end;
    }

    struct s_str_rdonly {
        s_array_rdonly<char> bytes;

        constexpr s_str_rdonly() = default;
        constexpr s_str_rdonly(const s_array_rdonly<char> bytes) : bytes(bytes) {}

        template <t_len tp_len>
        constexpr s_str_rdonly(const char (&raw)[tp_len]) : bytes({raw, tp_len}) {}
    };

    struct s_str {
        s_array<char> bytes;

        constexpr operator s_str_rdonly() const {
            return {bytes};
        }
    };

    constexpr t_len CountRawChrsBeforeNull(const char *const raw) {
        t_len cnt = 0;
        for (; raw[cnt]; cnt++) {}
        return cnt;
    }

    constexpr s_str StrFromRaw(char *const raw) {
        return {{raw, CountRawChrsBeforeNull(raw) + 1}};
    }

    constexpr s_str_rdonly StrFromRaw(const char *const raw) {
        return {{raw, CountRawChrsBeforeNull(raw) + 1}};
    }

    constexpr s_str StrFromRaw(char *const raw, const t_len len) {
        ZF_ASSERT(len == CountRawChrsBeforeNull(raw));
        return {{raw, len + 1}};
    }

    constexpr s_str_rdonly StrFromRaw(const char *const raw, const t_len len) {
        ZF_ASSERT(len == CountRawChrsBeforeNull(raw));
        return {{raw, len + 1}};
    }

    constexpr t_b8 IsStrTerminated(const s_str_rdonly str) {
        for (t_len i = str.bytes.len - 1; i >= 0; i--) {
            if (str.bytes[i] == '\0') {
                return true;
            }
        }

        return false;
    }

    constexpr t_b8 IsStrEmpty(const s_str_rdonly str) {
        return str.bytes.len == 0 || !str.bytes[0];
    }

    constexpr char *StrRaw(const s_str str) {
        ZF_ASSERT(IsStrTerminated(str));
        return str.bytes.buf;
    }

    constexpr const char *StrRaw(const s_str_rdonly str) {
        ZF_ASSERT(IsStrTerminated(str));
        return str.bytes.buf_raw;
    }

    t_b8 IsValidUTF8Str(const s_str_rdonly str);
    t_len CalcStrLen(const s_str_rdonly str);
    t_unicode_code_pt StrChrAtByte(const s_str_rdonly str, const t_len byte_index);

    struct s_str_chr_info {
        t_unicode_code_pt code_pt;
        t_len byte_index;
    };

    t_b8 WalkStr(const s_str_rdonly str, t_len *const byte_index, s_str_chr_info *const o_chr_info);        // Returns false iff the walk has ended.
    t_b8 WalkStrReverse(const s_str_rdonly str, t_len *const byte_index, s_str_chr_info *const o_chr_info); // Returns false iff the walk has ended. For a full walk, the byte index has to be initialised to the index of ANY BYTE in the last character.

#define ZF_WALK_STR(str, chr_info)                                                                             \
    for (t_len ZF_CONCAT(bi_l, __LINE__) = 0; ZF_CONCAT(bi_l, __LINE__) != -1; ZF_CONCAT(bi_l, __LINE__) = -1) \
        for (s_str_chr_info chr_info; WalkStr(str, &ZF_CONCAT(bi_l, __LINE__), &chr_info);)

#define ZF_WALK_STR_REVERSE(str, chr_info)                                                                                                           \
    for (t_len ZF_CONCAT(bi_l, __LINE__) = str.bytes.len - 1; ZF_CONCAT(bi_l, __LINE__) != str.bytes.len; ZF_CONCAT(bi_l, __LINE__) = str.bytes.len) \
        for (s_str_chr_info chr_info; WalkStrReverse(str, &ZF_CONCAT(bi_l, __LINE__), &chr_info);)

    void MarkStrCodePoints(const s_str_rdonly str, t_unicode_code_pt_bit_vec &code_pts); // Sets the bits associated with each unicode code point that appear in the input string.
}
