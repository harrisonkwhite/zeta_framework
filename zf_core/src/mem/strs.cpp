#include <zc/mem/strs.h>

namespace zf {
    t_s32 s_str_view::CalcLen() const {
        t_s32 len = 0;

        while (len < chrs.Len() && chrs[len]) {
            len++;
        }

        return len;
    }

    t_b8 s_str_view::IsTerminated() const {
        // The terminator is most likely at the end, so we start there.
        for (t_s32 i = chrs.Len() - 1; i >= 0; i--) {
            if (!chrs[i]) {
                return true;
            }
        }

        return false;
    }
}
