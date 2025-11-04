#include <zc/mem/strs.h>

namespace zf {
    int s_str_view::CalcLen() const {
        int len = 0;

        while (len < chrs.Len() && chrs[len]) {
            len++;
        }

        return len;
    }

    bool s_str_view::IsTerminated() const {
        // The terminator is most likely at the end, so we start there.
        for (int i = chrs.Len() - 1; i >= 0; i--) {
            if (!chrs[i]) {
                return true;
            }
        }

        return false;
    }
}
