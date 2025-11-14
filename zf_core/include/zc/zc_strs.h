#pragma once

#include <zc/zc_essential.h>

namespace zf {
    constexpr t_size CalcRawStrLen(const char* const raw_str) {
        t_size len = 0;
        for (; raw_str[len]; len++) {}
        return len;
    }

    struct s_str_ro {
        c_array<const char> chrs; // The length of this IS NOT necessarily the string length!

        constexpr s_str_ro() = default;
        constexpr s_str_ro(const c_array<const char> chrs) : chrs(chrs) {}
        consteval s_str_ro(const char* const raw);

        constexpr const char* Raw() const {
            return chrs.Buf();
        }
    };

    struct s_str_mut {
        c_array<char> chrs;

        constexpr s_str_mut() = default;
        constexpr s_str_mut(const c_array<char> chrs) : chrs(chrs) {}

        constexpr char* Raw() const {
            return chrs.Buf();
        }

        constexpr operator s_str_ro() const {
            return {static_cast<c_array<const char>>(chrs)};
        }
    };

    constexpr s_str_ro StrFromRawTerminated(const char* const raw) {
        return {{raw, CalcRawStrLen(raw) + 1}};
    }

    constexpr s_str_mut StrFromRawTerminated(char* const raw) {
        return {{raw, CalcRawStrLen(raw) + 1}};
    }

    constexpr s_str_ro StrFromRawTerminated(const char* const raw, const t_size len) {
        ZF_ASSERT(len == CalcRawStrLen(raw));
        return {{raw, len + 1}};
    }

    constexpr s_str_mut StrFromRawTerminated(char* const raw, const t_size len) {
        ZF_ASSERT(len == CalcRawStrLen(raw));
        return {{raw, len + 1}};
    }

    consteval s_str_ro::s_str_ro(const char* const raw) : chrs(StrFromRawTerminated(raw).chrs) {}

    constexpr t_b8 IsStrEmpty(const s_str_ro str) {
        return str.chrs.IsEmpty() || !str.chrs[0];
    }

    t_size CalcStrLen(const s_str_ro str);
    t_b8 IsStrTerminated(const s_str_ro str);
}
