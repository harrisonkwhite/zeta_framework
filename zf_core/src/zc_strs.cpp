#include <zc/zc_strs.h>

namespace zf {
    t_size CalcStrLen(const s_str_view str) {
        t_size len = 0;

        while (len < str.chrs.Len() && str.chrs[len]) {
            len++;
        }

        return len;
    }

    t_b8 IsStrTerminated(const s_str_view str) {
        // The terminator is most likely at the end, so we start there.
        for (t_size i = str.chrs.Len() - 1; i >= 0; i--) {
            if (!str.chrs[i]) {
                return true;
            }
        }

        return false;
    }
}
