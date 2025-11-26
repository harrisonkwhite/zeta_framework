#pragma once

#include <zc/zc_mem.h>
#include <zc/ds/zc_bit_vector.h>

namespace zf {
    using t_unicode_code_pt = char32_t;

    constexpr t_size g_unicode_code_pt_cnt = 1114112;

    using t_unicode_code_pt_bit_vector = s_static_bit_vector<g_unicode_code_pt_cnt>;

    constexpr t_unicode_code_pt_bit_vector g_unicode_printable_ascii_code_pts = []() constexpr {
        // @todo: Should be able to use masking for this!

        t_unicode_code_pt_bit_vector bv = {};

        for (t_size i = 32; i < 127; i++) {
            SetBit(bv, i);
        }

        return bv;
    }();

    struct s_str_rdonly {
        s_array_rdonly<t_u8> bytes;
    };

    struct s_str {
        s_array<t_u8> bytes;

        constexpr operator s_str_rdonly() const {
            return {bytes};
        }
    };

    constexpr t_size CountRawChrsBeforeNull(const char* const chrs_raw) {
        t_size cnt = 0;
        for (; chrs_raw[cnt]; cnt++) {}
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

    struct s_str_chr_info {
        t_unicode_code_pt code_pt;
        t_size byte_index;
    };

    t_b8 WalkStr(const s_str_rdonly str, t_size& byte_index, s_str_chr_info& o_chr_info); // Returns false iff the walk has ended.
    t_b8 WalkStrReverse(const s_str_rdonly str, t_size& byte_index, s_str_chr_info& o_chr_info); // Returns false iff the walk has ended. For a full walk, the byte index has to be initialised to the index of ANY BYTE in the last character.

    t_unicode_code_pt UTF8ChrBytesToCodePoint(const s_array_rdonly<t_u8> bytes); // Only accepts bytes representing a unicode code point in an array with a length in [1, 4].
    void MarkStrCodePoints(const s_str_rdonly str, t_unicode_code_pt_bit_vector& code_pt_bv); // Sets the bits associated with each unicode code point that appear in the input string.

#define ZF_WALK_STR(str, chr_info) \
    for (t_size ZF_CONCAT(p_bi_l, __LINE__) = 0; ZF_CONCAT(p_bi_l, __LINE__) != -1; ZF_CONCAT(p_bi_l, __LINE__) = -1) \
        for (s_str_chr_info chr_info; WalkStr(str, ZF_CONCAT(p_bi_l, __LINE__), chr_info); )

#define ZF_WALK_STR_REVERSE(str, chr_info) \
    for (t_size ZF_CONCAT(p_bi_l, __LINE__) = str.bytes.len - 1; ZF_CONCAT(p_bi_l, __LINE__) != str.bytes.len; ZF_CONCAT(p_bi_l, __LINE__) = str.bytes.len) \
        for (s_str_chr_info chr_info; WalkStrReverse(str, ZF_CONCAT(p_bi_l, __LINE__), chr_info); )
}
