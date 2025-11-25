#pragma once

#include <zc/zc_mem.h>
#include <zc/ds/zc_bit_vector.h>

namespace zf {
    // @todo: Clean up all this.

    using t_code_pt = char32_t;

    constexpr t_size g_unicode_code_pt_cnt = 1114112;

    using t_unicode_code_pt_bit_vector = s_static_bit_vector<g_unicode_code_pt_cnt>;

    constexpr t_unicode_code_pt_bit_vector g_asciis = []() constexpr {
        // @todo: Should be able to use masking for this!

        t_unicode_code_pt_bit_vector bv = {};

        for (t_size i = 32; i < 127; i++) {
            SetBit(bv, i);
        }

        return bv;
    }();

    constexpr t_size CountCharsUntilNull(const char* const chrs_raw) {
        t_size len = 0;
        for (; chrs_raw[len]; len++) {}
        return len;
    }

    struct s_str_utf8_rdonly {
        s_array_rdonly<t_u8> bytes;
    };

    struct s_str_utf8 {
        s_array<t_u8> bytes;

        constexpr operator s_str_utf8_rdonly() const {
            return {bytes};
        }
    };

    struct s_str_ascii_rdonly {
        s_array_rdonly<char> chrs; // The length of this IS NOT necessarily the string length!

        constexpr s_str_ascii_rdonly() = default;
        constexpr s_str_ascii_rdonly(const s_array_rdonly<char> chrs) : chrs(chrs) {}
        consteval s_str_ascii_rdonly(const char* const raw_term);

        constexpr operator s_str_utf8_rdonly() const {
            return {ToByteArray(chrs)};
        }
    };

    struct s_str_ascii {
        s_array<char> chrs;

        constexpr operator s_str_ascii_rdonly() const {
            return {chrs};
        }

        constexpr operator s_str_utf8() const {
            return {ToByteArray(chrs)};
        }

        constexpr operator s_str_utf8_rdonly() const {
            return {ToByteArray(chrs)};
        }
    };

    constexpr char* StrRaw(const s_str_ascii str) {
        return str.chrs.buf_raw;
    }

    constexpr const char* StrRaw(const s_str_ascii_rdonly str) {
        return str.chrs.buf_raw;
    }

    constexpr t_size CalcStrLen(const s_str_ascii_rdonly str) {
        t_size len = 0;
        for (; str.chrs[len] && len < str.chrs.len; len++) {}
        return len;
    }

    constexpr s_str_ascii StrFromRawTerminated(char* const raw) {
        return {{raw, CountCharsUntilNull(raw) + 1}};
    }

    constexpr s_str_ascii_rdonly StrFromRawTerminated(const char* const raw) {
        return {{raw, CountCharsUntilNull(raw) + 1}};
    }

    constexpr s_str_ascii StrFromRawTerminated(char* const raw, const t_size len) {
        ZF_ASSERT(len == CountCharsUntilNull(raw));
        return {{raw, len + 1}};
    }

    constexpr s_str_ascii_rdonly StrFromRawTerminated(const char* const raw, const t_size len) {
        ZF_ASSERT(len == CountCharsUntilNull(raw));
        return {{raw, len + 1}};
    }

    inline s_str_utf8_rdonly StrFromRawTerminatedTest(const char* const raw) {
        return {{reinterpret_cast<const t_u8*>(raw), CountCharsUntilNull(raw) + 1}};
    }

    consteval s_str_ascii_rdonly::s_str_ascii_rdonly(const char* const raw_term)
        : chrs({raw_term, CountCharsUntilNull(raw_term) + 1}) {}

    constexpr t_b8 IsStrEmpty(const s_str_ascii_rdonly str) {
        return IsArrayEmpty(str.chrs) || !str.chrs[0];
    }

    constexpr t_b8 IsStrTerminated(const s_str_ascii_rdonly str) {
        // The terminator is most likely at the end, so we start there.
        for (t_size i = str.chrs.len - 1; i >= 0; i--) {
            if (!str.chrs[i]) {
                return true;
            }
        }

        return false;
    }

    t_b8 IsValidUTF8Str(const s_str_utf8_rdonly str);
    [[nodiscard]] t_b8 CalcUTF8StrLen(const s_str_utf8_rdonly str, t_size& o_len); // Returns false iff the provided string is not valid UTF-8 form. Works with either terminated or non-terminated strings.
    t_size CalcUTF8StrLenFastButUnsafe(const s_str_utf8_rdonly str); // This (in release mode) DOES NOT check whether the string is in valid UTF-8 form!
    t_b8 WalkUTF8Str(const s_str_utf8_rdonly str, t_size& pos, t_code_pt& o_code_pt); // Returns false iff the walk is complete.
    t_code_pt UTF8ChrBytesToCodePoint(const s_array_rdonly<t_u8> bytes); // Only accepts bytes representing a unicode code point in an array with a length in [1, 4].

    void MarkCodePoints(const s_str_utf8_rdonly str, t_unicode_code_pt_bit_vector& code_pt_bv); // Sets the bits associated with each unicode code point that appear in the input string.

#define ZF_ITER_UTF8_STR(str, code_pt) \
    for (t_size ZF_CONCAT(p_pos_l, __LINE__) = 0; ZF_CONCAT(p_pos_l, __LINE__) != -1; ZF_CONCAT(p_pos_l, __LINE__) = -1) \
        for (t_code_pt code_pt; WalkUTF8Str(str, ZF_CONCAT(p_pos_l, __LINE__), code_pt); )
}
