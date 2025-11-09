#pragma once

#include <zc/mem/mem.h>
#include <zc/math.h>

namespace zf {
    constexpr t_s32 RawStrLen(const char* const raw_str) {
        t_s32 len = 0;
        for (; raw_str[len]; len++) {}
        return len;
    }

    struct s_str_view {
        static constexpr s_str_view FromRawTerminated(const char* const raw) {
            return {{raw, static_cast<t_s32>(RawStrLen(raw)) + 1}};
        }

        static s_str_view FromRawTerminated(const char* const raw, const t_s32 len) {
            ZF_ASSERT(len == RawStrLen(raw));
            return {{raw, len + 1}};
        }

        c_array<const char> chrs; // The length of this IS NOT necessarily the string length!

        constexpr s_str_view() = default;
        constexpr s_str_view(const c_array<const char> chrs) : chrs(chrs) {}

        t_s32 CalcLen() const;
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
            return {{raw, static_cast<t_s32>(strlen(raw)) + 1}};
        }

        static s_str FromRawTerminated(char* const raw, const t_s32 len) {
            ZF_ASSERT(len == strlen(raw));
            return {{raw, len + 1}};
        }

        c_array<char> chrs;

        constexpr s_str() = default;
        constexpr s_str(const c_array<char> chrs) : chrs(chrs) {}

        t_s32 CalcLen() const {
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
