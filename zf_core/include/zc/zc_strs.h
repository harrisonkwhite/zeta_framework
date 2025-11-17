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
        s_array<const char> chrs; // The length of this IS NOT necessarily the string length!

        constexpr s_str_rdonly() = default;
        constexpr s_str_rdonly(const s_array<const char> chrs) : chrs(chrs) {}
        consteval s_str_rdonly(const char* const raw);

        constexpr const char* Raw() const {
            return chrs.Raw();
        }
    };

    struct s_str {
        s_array<char> chrs;

        constexpr s_str() = default;
        constexpr s_str(const s_array<char> chrs) : chrs(chrs) {}

        constexpr char* Raw() const {
            return chrs.Raw();
        }

        constexpr operator s_str_rdonly() const {
            return {static_cast<s_array<const char>>(chrs)};
        }
    };

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

    // Compile-time only - this is too big of an operation to occur implicitly at runtime.
    consteval s_str_rdonly::s_str_rdonly(const char* const raw) : chrs(StrFromRawTerminated(raw).chrs) {}

    constexpr t_b8 IsStrEmpty(const s_str_rdonly str) {
        return str.chrs.IsEmpty() || !str.chrs[0];
    }

    t_size CalcStrLen(const s_str_rdonly str);
    t_b8 IsStrTerminated(const s_str_rdonly str);
}
