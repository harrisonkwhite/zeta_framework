#pragma once

#include <zc/zc_mem.h>
#include <zc/ds/zc_bit_vector.h>

namespace zf {
    using t_unicode_code_pt = char32_t;

    constexpr t_size g_unicode_code_pt_cnt = 1114112;

    using t_unicode_code_pt_bit_vector = s_static_bit_vector<g_unicode_code_pt_cnt>;

    constexpr t_unicode_code_pt_bit_vector g_unicode_ascii_code_pts = []() constexpr {
        // @todo: Should be able to use masking for this!

        t_unicode_code_pt_bit_vector bv = {};

        for (t_size i = 32; i < 127; i++) {
            SetBit(bv, i);
        }

        return bv;
    }();

    struct s_str_utf8_rdonly {
        s_array_rdonly<t_u8> bytes;
    };

    struct s_str_utf8 {
        s_array<t_u8> bytes;

        constexpr operator s_str_utf8_rdonly() const {
            return {bytes};
        }
    };

    inline char* StrRaw(const s_str_utf8 str) {
        return reinterpret_cast<char*>(str.bytes.buf_raw);
    }

    inline const char* StrRaw(const s_str_utf8_rdonly str) {
        return reinterpret_cast<const char*>(str.bytes.buf_raw);
    }

    constexpr t_size CountRawChrsBeforeNull(const char* const chrs_raw) {
        t_size cnt = 0;
        for (; chrs_raw[cnt]; cnt++) {}
        return cnt;
    }

    inline s_str_utf8 StrFromRawTerminated(char* const raw) {
        return {{reinterpret_cast<t_u8*>(raw), CountRawChrsBeforeNull(raw) + 1}};
    }

    inline s_str_utf8_rdonly StrFromRawTerminated(const char* const raw) {
        return {{reinterpret_cast<const t_u8*>(raw), CountRawChrsBeforeNull(raw) + 1}};
    }

    inline s_str_utf8 StrFromRawTerminated(char* const raw, const t_size len) {
        ZF_ASSERT(len == CountRawChrsBeforeNull(raw));
        return {{reinterpret_cast<t_u8*>(raw), len + 1}};
    }

    inline s_str_utf8_rdonly StrFromRawTerminated(const char* const raw, const t_size len) {
        ZF_ASSERT(len == CountRawChrsBeforeNull(raw));
        return {{reinterpret_cast<const t_u8*>(raw), len + 1}};
    }

    constexpr t_b8 IsStrEmpty(const s_str_utf8_rdonly str) {
        return IsArrayEmpty(str.bytes) || !str.bytes[0];
    }

    constexpr t_b8 IsStrTerminatedAnywhere(const s_str_utf8_rdonly str) {
        // The terminator is most likely at the end, so we start there.
        for (t_size i = str.bytes.len - 1; i >= 0; i--) {
            if (!str.bytes[i]) {
                return true;
            }
        }

        return false;
    }

    constexpr t_b8 IsStrTerminatedOnlyAtEnd(const s_str_utf8_rdonly str) {
        if (IsArrayEmpty(str.bytes)) {
            return false;
        }

        for (t_size i = 0; i < str.bytes.len - 1; i++) {
            if (!str.bytes[i]) {
                return false;
            }
        }

        return str.bytes[str.bytes.len - 1] == '\0';
    }

    t_b8 IsValidUTF8Str(const s_str_utf8_rdonly str);
    t_size CalcStrLen(const s_str_utf8_rdonly str);
    [[nodiscard]] t_b8 CalcStrLenAndCheckValidity(const s_str_utf8_rdonly str, t_size& o_len); // Returns false iff the provided string is not valid UTF-8 form.
    t_b8 WalkStr(const s_str_utf8_rdonly str, t_size& byte_index, t_unicode_code_pt& o_code_pt); // Returns false iff the walk has ended.
    t_b8 WalkStrReverse(const s_str_utf8_rdonly str, t_size& byte_index, t_unicode_code_pt& o_code_pt); // Returns false iff the walk has ended. For a full walk, byte index has to be initialised to the index of ANY BYTE in the last character.
    t_unicode_code_pt UTF8ChrBytesToCodePoint(const s_array_rdonly<t_u8> bytes); // Only accepts bytes representing a unicode code point in an array with a length in [1, 4].
    void MarkStrCodePoints(const s_str_utf8_rdonly str, t_unicode_code_pt_bit_vector& code_pt_bv); // Sets the bits associated with each unicode code point that appear in the input string.

#define ZF_ITER_STR(str, byte_index, code_pt) \
    for (t_size byte_index = 0; byte_index != -1; byte_index = -1) \
        for (t_unicode_code_pt code_pt; WalkStr(str, byte_index, code_pt); )

#define ZF_ITER_STR_REVERSE(str, byte_index, code_pt) \
    for (t_size byte_index = str.bytes.len - 1; byte_index != str.bytes.len; byte_index = str.bytes.len) \
        for (t_unicode_code_pt code_pt; WalkStrReverse(str, byte_index, code_pt); )
}
