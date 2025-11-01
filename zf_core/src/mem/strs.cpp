#include <zc/mem/strs.h>

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

    // @todo: These should be made generic to support any array, not just strings.
    c_array<int> GenSuffixArray(const s_str str, c_mem_arena& mem_arena) {
        const auto arr = mem_arena.PushArray<int>(str.Len());

        if (!arr.IsEmpty()) {
            for (int i = 0; i < str.Len(); i++) {
                const s_str suffix = str.Suffix(i);

                int lesser_cnt = 0;

                for (int j = i + 1; j < str.Len(); j++) {
                    const s_str other_suffix = str.Suffix(j);

                    if (CompareStrs(suffix, other_suffix) == -1) {
                        lesser_cnt++;
                    }
                }

                arr[arr.Len() - lesser_cnt - 1] = i;
            }
        }

        return arr;
    }

    bool DoesSuffixExist(const s_str suffix_being_searched_for, const s_str str, const c_array<const int> suffix_arr) {
        assert(str.Len() == suffix_arr.Len());

        const int suffix_index = suffix_arr.Len() / 2;
        const auto suffix = str.Suffix(suffix_index);

        const int cmp = CompareStrs(suffix, suffix_being_searched_for);

        if (cmp == 0) {
            return true;
        }

        if (str.Len() == 0) {
            return false;
        }

        if (cmp == 1) {
            return DoesSuffixExist(suffix_being_searched_for, str.Suffix(suffix_index + 1), suffix_arr.Slice(suffix_index + 1, suffix_arr.Len()));
        }

        return DoesSuffixExist(suffix_being_searched_for, str.Prefix(suffix_index), suffix_arr.Slice(0, suffix_index));
    }
}
