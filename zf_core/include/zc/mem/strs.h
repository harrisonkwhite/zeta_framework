#pragma once

#include <zc/mem/mem.h>
#include <zc/math.h>

namespace zf {
    // @todo: Also need to support other languages (i.e. characters more than a byte in length).

    struct s_str_view {
        static s_str_view FromRaw(const char* const raw) {
            return {{raw, static_cast<int>(strlen(raw))}};
        }

        c_array<const char> chrs; // The length of this IS NOT necessarily the string length!

        s_str_view() = default;
        s_str_view(const c_array<const char> chrs) : chrs(chrs) {}

        int CalcLen() const;
        bool IsTerminated() const;
    };

    struct s_str {
        static s_str FromRaw(char* const raw) {
            return {{raw, static_cast<int>(strlen(raw))}};
        }

        c_array<char> chrs;

        s_str() = default;
        s_str(const c_array<char> chrs) : chrs(chrs) {}

        operator s_str_view() const {
            return {chrs};
        }

        int CalcLen() const {
            return s_str_view(chrs).CalcLen();
        }

        bool IsTerminated(const c_array<const char> str) const {
            return s_str_view(chrs).IsTerminated();
        }
    };
}
