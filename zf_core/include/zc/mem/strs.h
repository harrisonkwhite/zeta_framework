#pragma once

#include <zc/mem/mem.h>
#include <zc/math.h>

namespace zf {
    // @todo: Also need to support other languages (i.e. characters more than a byte in length).

    struct s_str_view {
        static s_str_view FromRawTerminated(const char* const raw) {
            return {{raw, static_cast<int>(strlen(raw)) + 1}};
        }

        static s_str_view FromRawTerminated(const char* const raw, const int len) {
            assert(len == strlen(raw));
            return {{raw, len + 1}};
        }

        c_array<const char> chrs; // The length of this IS NOT necessarily the string length!

        s_str_view() = default;
        s_str_view(const c_array<const char> chrs) : chrs(chrs) {}

        int CalcLen() const;
        bool IsTerminated() const;

        const char* Raw() const {
            return chrs.Raw();
        }

        bool IsEmpty() const {
            return chrs.IsEmpty() || !chrs[0];
        }
    };

    struct s_str {
        static s_str FromRawTerminated(char* const raw) {
            return {{raw, static_cast<int>(strlen(raw)) + 1}};
        }

        static s_str FromRawTerminated(char* const raw, const int len) {
            assert(len == strlen(raw));
            return {{raw, len + 1}};
        }

        c_array<char> chrs;

        s_str() = default;
        s_str(const c_array<char> chrs) : chrs(chrs) {}

        int CalcLen() const {
            return s_str_view(chrs).CalcLen();
        }

        bool IsTerminated(const c_array<const char> str) const {
            return s_str_view(chrs).IsTerminated();
        }

        char* Raw() const {
            return chrs.Raw();
        }

        bool IsEmpty() const {
            return s_str_view(chrs).IsEmpty();
        }

        operator s_str_view() const {
            return {static_cast<c_array<const char>>(chrs)};
        }
    };
}
