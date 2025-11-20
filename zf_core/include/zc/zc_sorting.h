#pragma once

#include <zc/ds/zc_array.h>

namespace zf {
    template<c_array tp_type>
    t_b8 IsSorted(tp_type& arr, const t_comparator<typename tp_type::t_elem> comparator = DefaultComparator) {
        ZF_ASSERT(comparator);

        for (t_size i = 0; i < ArrayLen(arr) - 1; i++) {
            if (comparator(arr[i], arr[i + 1]) > 0) {
                return false;
            }
        }

        return true;
    }

    // O(n) best-case if array is already sorted, O(n^2) worst-case.
    template<c_array_mut tp_type>
    void BubbleSort(tp_type& arr, const t_comparator<typename tp_type::t_elem> comparator = DefaultComparator) {
        ZF_ASSERT(comparator);

        t_b8 sorted;

        do {
            sorted = true;

            for (t_size i = 0; i < ArrayLen(arr) - 1; i++) {
                if (comparator(arr[i], arr[i + 1]) > 0) {
                    Swap(arr[i], arr[i + 1]);
                    sorted = false;
                }
            }
        } while (!sorted);
    }

    // O(n) best-case if array is already sorted, O(n^2) worst-case.
    template<c_array_mut tp_type>
    void InsertionSort(tp_type& arr, const t_comparator<typename tp_type::t_elem> comparator = DefaultComparator) {
        ZF_ASSERT(comparator);

        for (t_size i = 1; i < ArrayLen(arr); i++) {
            const auto temp = arr[i];

            t_size j = i - 1;

            for (; j >= 0; j--) {
                if (comparator(arr[j], temp) <= 0) {
                    break;
                }

                arr[j + 1] = arr[j];
            }

            arr[j + 1] = temp;
        }
    }

    // O(n^2) in every case.
    template<c_array_mut tp_type>
    void SelectionSort(tp_type& arr, const t_comparator<typename tp_type::t_elem> comparator = DefaultComparator) {
        ZF_ASSERT(comparator);

        for (t_size i = 0; i < ArrayLen(arr) - 1; i++) {
            auto& min = arr[i];

            for (t_size j = i + 1; j < ArrayLen(arr); j++) {
                if (comparator(arr[j], min) < 0) {
                    min = arr[j];
                }
            }

            Swap(arr[i], min);
        }
    }

    // O(n log n) in both time complexity and space complexity in every case.
    template<c_array_mut tp_type>
    [[nodiscard]] t_b8 MergeSort(tp_type& arr, s_mem_arena& temp_mem_arena, const t_comparator<typename tp_type::t_elem> comparator = DefaultComparator) {
        ZF_ASSERT(comparator);

        if (ArrayLen(arr) <= 1) {
            return true;
        }

        // Sort copies of the left and right partitions.
        const auto arr_left = Slice(arr, 0, ArrayLen(arr) / 2);
        s_array<typename tp_type::t_elem> arr_left_sorted;

        if (!MakeArrayClone(temp_mem_arena, arr_left, arr_left_sorted)) {
            return false;
        }

        const auto arr_right = Slice(arr, ArrayLen(arr) / 2, ArrayLen(arr));
        s_array<typename tp_type::t_elem> arr_right_sorted;

        if (!MakeArrayClone(temp_mem_arena, arr_right, arr_right_sorted)) {
            return false;
        }

        if (!MergeSort(arr_left_sorted, temp_mem_arena, comparator) || !MergeSort(arr_right_sorted, temp_mem_arena, comparator)) {
            return false;
        }

        // Update this array.
        t_size i = 0;
        t_size j = 0;

        do {
            if (comparator(arr_left_sorted[i], arr_right_sorted[j]) <= 0) {
                arr[i + j] = arr_left_sorted[i];
                i++;

                if (i == ArrayLen(arr_left_sorted)) {
                    // Copy over the remainder of the right array.
                    const auto dest = Slice(arr, i + j, ArrayLen(arr));
                    const auto src = Slice(arr_right_sorted, j, ArrayLen(arr_right_sorted));
                    Copy(dest, src);

                    break;
                }
            } else {
                arr[i + j] = arr_right_sorted[j];
                j++;

                if (j == ArrayLen(arr_right_sorted)) {
                    // Copy over the remainder of the left array.
                    const auto dest = Slice(arr, i + j, ArrayLen(arr));
                    const auto src = Slice(arr_left_sorted, i, ArrayLen(arr_left_sorted));
                    Copy(dest, src);

                    break;
                }
            }
        } while (true);

        return true;
    }

    // Time complexity is O(n log n) best-case and O(n^2) worst-case depending on the pivot.
    // Space complexity is O(1) compared to merge sort.
    // In each recurse, the pivot is selected as the median of the first, middle, and last elements.
    template<c_array_mut tp_type>
    void QuickSort(tp_type& arr, const t_comparator<typename tp_type::t_elem> comparator = DefaultComparator) {
        ZF_ASSERT(comparator);

        if (ArrayLen(arr) <= 1) {
            return;
        }

        if (ArrayLen(arr) == 2) {
            if (comparator(arr[0], arr[1]) > 0) {
                Swap(arr[0], arr[1]);
            }

            return;
        }

        // Determine the pivot - median of the first, middle, and last elements.
        const t_size pivot_index = [arr, comparator]() {
            const t_size ia = 0;
            const t_size ib = ArrayLen(arr) / 2;
            const t_size ic = ArrayLen(arr) - 1;

            if (comparator(arr[ia], arr[ib]) <= 0) {
                if (comparator(arr[ib], arr[ic]) <= 0) {
                    return ib;
                } else {
                    if (comparator(arr[ia], arr[ic]) <= 0) {
                        return ic;
                    } else {
                        return ia;
                    }
                }
            } else {
                if (comparator(arr[ia], arr[ic]) <= 0) {
                    return ia;
                } else {
                    if (comparator(arr[ib], arr[ic]) <= 0) {
                        return ic;
                    } else {
                        return ib;
                    }
                }
            }
        }();

        // Swap it out to the end to get it out of the way.
        Swap(arr[pivot_index], arr[ArrayLen(arr) - 1]);

        // Move smaller elements to the left, and decide the final pivot position.
        t_size left_sec_last_index = -1;

        for (t_size i = 0; i < ArrayLen(arr); i++) {
            if (comparator(arr[i], arr[ArrayLen(arr) - 1]) <= 0) {
                // This element is not greater than the pivot, so swap it to the left section.
                left_sec_last_index++;
                Swap(arr[left_sec_last_index], arr[i]);
            }
        }

        // Sort for each subsection.
        const auto left = Slice(arr, 0, left_sec_last_index);
        QuickSort(left, comparator);

        const auto right = Slice(arr, left_sec_last_index + 1, ArrayLen(arr));
        QuickSort(right, comparator);
    }
}
