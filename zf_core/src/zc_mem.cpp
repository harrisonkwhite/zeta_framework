#include "mem/zc_strs.h"

namespace zf {
    int CompareStrs(const s_str a, const s_str b) {
        const int len_of_shorter = Min(a.Len(), b.Len());

        for (int i = 0; i < len_of_shorter; i++) {
            if (a[i] < b[i]) {
                return -1;
            } else if (a[i] > b[i]) {
                return 1;
            }
        }

        return Sign(b.Len() - a.Len());
    }

#if 0
    bool SuffixArray(const s_str str) {
        for (int i = 0; i < str.Len(); i++) {
            const auto suffix = str.Suffix(i);

            for (int j = 0; j < str.Len(); j++) {
            }
        }

        return true;
    }
#endif
}
