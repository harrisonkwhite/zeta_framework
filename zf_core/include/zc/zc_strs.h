#pragma once

#include <zc/zc_basic.h>
#include <zc/ds/zc_array.h>

namespace zf {
    constexpr t_size CalcRawStrLen(const char* const raw_str) {
        t_size len = 0;
        for (; raw_str[len]; len++) {}
        return len;
    }

    struct s_str_rdonly {
        s_array_rdonly<char> chrs; // The length of this IS NOT necessarily the string length!

        constexpr s_str_rdonly() = default;
        constexpr s_str_rdonly(const s_array_rdonly<char> chrs) : chrs(chrs) {}
        consteval s_str_rdonly(const char* const raw_term);
    };

    struct s_str {
        s_array<char> chrs;

        constexpr operator s_str_rdonly() const {
            return {static_cast<s_array_rdonly<char>>(chrs)};
        }
    };

    constexpr char* StrRaw(const s_str str) {
        return str.chrs.buf_raw;
    }

    constexpr const char* StrRaw(const s_str_rdonly str) {
        return str.chrs.buf_raw;
    }

    constexpr s_str_rdonly StrFromRawTerminated(const char* const raw) {
        return {{raw, CalcRawStrLen(raw) + 1}};
    }

    constexpr s_str StrFromRawTerminated(char* const raw) {
        return {{raw, CalcRawStrLen(raw) + 1}};
    }

    constexpr s_str_rdonly StrFromRawTerminated(const char* const raw, const t_size len) {
        ZF_ASSERT(len == CalcRawStrLen(raw));
        return {{raw, len + 1}};
    }

    constexpr s_str StrFromRawTerminated(char* const raw, const t_size len) {
        ZF_ASSERT(len == CalcRawStrLen(raw));
        return {{raw, len + 1}};
    }

    consteval s_str_rdonly::s_str_rdonly(const char* const raw_term)
        : chrs({raw_term, CalcRawStrLen(raw_term) + 1}) {}

    constexpr t_b8 IsStrEmpty(const s_str_rdonly str) {
        return IsArrayEmpty(str.chrs) || !str.chrs[0];
    }

    t_size CalcASCIIStrLen(const s_str_rdonly str);
    [[nodiscard]] t_b8 CalcUTF8StrLen(const s_str_rdonly str, t_size& o_len); // Returns false iff the provided string is not valid UTF-8 form. Works with either terminated or non-terminated strings.
    t_size CalcUTF8StrLenFastButUnsafe(const s_str_rdonly str); // This (in release mode) DOES NOT check whether the string is in valid UTF-8 form!
    t_b8 IsStrTerminated(const s_str_rdonly str);
}
