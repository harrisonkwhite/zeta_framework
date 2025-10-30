#include "zc_mem.h"

#include "zc_static_array.h"
#include "zc_dynamic_array.h"
#include "zc_math.h"

namespace zf {
    // @todo: Need to figure out some way to allow for custom sorting comparison approaches.

    template<typename tp_type>
    bool IsSorted(const c_array<const tp_type> arr) {
        for (int i = 0; i < arr.Len() - 1; i++) {
            if (arr[i] >= arr[i + 1]) {
                return false;
            }
        }

        return true;
    }

    template<typename tp_type>
    void BubbleSort(const c_array<tp_type> arr) {
        bool sorted;

        do {
            sorted = true;

            for (int i = 0; i < arr.Len() - 1; i++) {
                if (arr[i] >= arr[i + 1]) {
                    Swap(arr[i], arr[i + 1]);
                    sorted = false;
                    break;
                }
            }
        } while (!sorted);
    }

    template<typename tp_type>
    void InsertionSort(const c_array<tp_type> arr) {
        for (int i = 0; i < arr.Len(); i++) {
            const tp_type temp = arr[i];

            int j = i - 1;

            for (; j >= 0; j--) {
                if (arr[j] <= arr[i]) {
                    break;
                }

                arr[j + 1] = arr[j];
            }

            arr[j + 1] = arr[i];
        }
    }

    template<typename tp_type>
    void SelectionSort(const c_array<tp_type> arr) {
        for (int i = 0; i < arr.Len() - 1; i++) {
            tp_type& min = arr[i];

            for (int j = i + 1; j < arr.Len(); j++) {
                if (arr[j] < min) {
                    min = arr[j];
                }
            }

            Swap(arr[i], min);
        }
    }

    template<typename tp_type>
    bool MergeSort(const c_array<tp_type> arr, c_mem_arena& temp_mem_arena) {
        if (arr.Len() <= 1) {
            return true;
        }

        // Sort copies of the left and right partitions.
        const c_array<tp_type> arr_left = MemClone(temp_mem_arena, arr.Slice(0, arr.Len() / 2));

        if (arr_left.IsEmpty()) {
            return false;
        }

        const c_array<tp_type> arr_right = MemClone(temp_mem_arena, arr.Slice(arr.Len() / 2));

        if (arr_right.IsEmpty()) {
            return false;
        }

        if (!MergeSort(arr_left, temp_mem_arena) || !MergeSort(arr_right, temp_mem_arena)) {
            return false;
        }

        // Update this array.
        int i = 0;
        int j = 0;

        do {
            if (arr_left[i] <= arr_right[j]) {
                arr[i + j] = arr_left[i];
                i++;

                if (i == arr_left.Len()) {
                    // Copy over the remainder of the right array.
                    MemCopy(arr.Slice(i + j), arr_right.Slice(j));
                    break;
                }
            } else {
                arr[i + j] = arr_right[j];
                j++;

                if (j == arr_right.Len()) {
                    // Copy over the remainder of the left array.
                    MemCopy(arr.Slice(i + j), arr_left.Slice(i));
                    break;
                }
            }
        } while (true);

        return true;
    }

    template<typename tp_type>
    void QuickSort(const c_array<const tp_type> arr) {
        if (arr.Len() <= 1) {
            return;
        }

        if (arr.Len() == 2) {
            if (arr[0] > arr[1]) {
                Swap(arr[0], arr[1]);
            }

            return;
        }

        // Select the pivot as being the median of the leftmost, middle, and rightmost elements. We obviously need at least 3 elements at this point.
        const auto pivot_index = [arr]() {
            const s_static_array<int, 3> pivot_index_opts = {
                {0, arr.Len() / 2, arr.Len() - 1}
            };

            for (int i = 0; i < pivot_index_opts.Len() - 1; i++) {
                const int next = pivot_index_opts[Wrap(i + 1, 0, pivot_index_opts.Len())];
                const int prev = pivot_index_opts[Wrap(i - 1, 0, pivot_index_opts.Len())];

                if ((prev <= pivot_index_opts[i] && pivot_index_opts[i] <= next) || (next <= pivot_index_opts[i] && pivot_index_opts[i] <= prev)) {
                    return pivot_index_opts[i];
                }
            }

            return pivot_index_opts[pivot_index_opts.Len() - 1];
        }();

        // Swap the pivot out to the end.
        Swap(arr[pivot_index], arr[arr.Len() - 1]);

        // Move smaller elements to the left, and decide the final pivot position.
        int left_sec_last_index = -1;

        for (int i = 0; i < arr.Len(); i++) {
            if (arr[i] <= arr[arr.Len() - 1]) {
                // This element is not greater than the pivot, so swap it to the left section.
                left_sec_last_index++;
                Swap(arr[left_sec_last_index], arr[i]);
            }
        }

        // Sort for each subsection.
        QuickSort(arr.Slice(0, left_sec_last_index));
        QuickSort(arr.Slice(left_sec_last_index + 1));
    }

    static bool RadixSort(const zf::c_array<int> arr, zf::c_mem_arena& temp_mem_arena) {
        int digit_cnt_max = 1;

        for (int i = 0; i < arr.Len(); i++) {
            arr[i] = 10 + arr.Len() - 1 - i;
            const int digit_cnt = DigitCnt(arr[i]);
            digit_cnt_max = zf::Max(digit_cnt_max, digit_cnt);
        }

        // Set up 10 buckets, one per digit.
        zf::s_static_array<zf::c_dynamic_array<int>, 10> buckets;

        for (int i = 0; i < buckets.Len(); i++) {
            if (!buckets[i].Init(temp_mem_arena, (arr.Len() / buckets.Len()) + 2)) {
                return false;
            }
        }

        for (int d = 0; d < digit_cnt_max; d++) {
            for (int i = 0; i < arr.Len(); i++) {
                const int digit = DigitAt(arr[i], d);

                if (!buckets[digit].Append(arr[i])) {
                    return false;
                }
            }

            // Repopulate the array.
            int arr_index = 0;

            for (int i = 0; i < buckets.Len(); i++) {
                for (int j = 0; j < buckets[i].Len(); j++) {
                    arr[arr_index] = buckets[i][j];
                    arr_index++;
                }

                buckets[i].Clear();
            }
        }

        return true;
    }
}
