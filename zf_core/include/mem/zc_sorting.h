#include "zc_mem.h"

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
}
