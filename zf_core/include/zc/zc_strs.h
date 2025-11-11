#pragma once

#include <zc/zc_essential.h>

namespace zf {
    constexpr t_size RawStrLen(const char* const raw_str) {
        t_size len = 0;
        for (; raw_str[len]; len++) {}
        return len;
    }

    // @todo: Probably don't use methods here.

    struct s_str_view {
        static constexpr s_str_view FromRawTerminated(const char* const raw) {
            return {{raw, RawStrLen(raw) + 1}};
        }

        static s_str_view FromRawTerminated(const char* const raw, const t_size len) {
            ZF_ASSERT(len == RawStrLen(raw));
            return {{raw, len + 1}};
        }

        c_array<const char> chrs; // The length of this IS NOT necessarily the string length!

        constexpr s_str_view() = default;
        constexpr s_str_view(const c_array<const char> chrs) : chrs(chrs) {}

        t_size CalcLen() const;
        t_b8 IsTerminated() const;

        constexpr const char* Raw() const {
            return chrs.Raw();
        }

        t_b8 IsEmpty() const {
            return chrs.IsEmpty() || !chrs[0];
        }
    };

    struct s_str {
        static s_str FromRawTerminated(char* const raw) {
            return {{raw, RawStrLen(raw) + 1}};
        }

        static s_str FromRawTerminated(char* const raw, const t_size len) {
            ZF_ASSERT(len == RawStrLen(raw));
            return {{raw, len + 1}};
        }

        c_array<char> chrs;

        constexpr s_str() = default;
        constexpr s_str(const c_array<char> chrs) : chrs(chrs) {}

        t_size CalcLen() const {
            return s_str_view(chrs).CalcLen();
        }

        t_b8 IsTerminated() const {
            return s_str_view(chrs).IsTerminated();
        }

        constexpr char* Raw() const {
            return chrs.Raw();
        }

        t_b8 IsEmpty() const {
            return s_str_view(chrs).IsEmpty();
        }

        constexpr operator s_str_view() const {
            return {static_cast<c_array<const char>>(chrs)};
        }
    };
}
