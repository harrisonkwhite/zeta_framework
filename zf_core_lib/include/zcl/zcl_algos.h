#pragma once

#include <zcl/zcl_mem.h>

namespace zf {
    template <co_array_nonstatic_mut tp_arr_type>
    constexpr void Reverse(const tp_arr_type arr) {
        for (t_i32 i = 0; i < arr.Len() / 2; i++) {
            Swap(&arr[i], &arr[arr.Len() - 1 - i]);
        }
    }

    // O(n^2) time complexity, but O(1) space complexity. Can also be done at compile time.
    // You're usually better off using a hash map and a linear search, or a bit vector if values are numeric and the range is small.
    template <co_array_nonstatic tp_arr_type>
    constexpr t_b8 HasDuplicatesSlow(const tp_arr_type arr, const t_bin_comparator<typename tp_arr_type::t_elem> comparator = DefaultBinComparator) {
        for (t_i32 i = 0; i < arr.Len(); i++) {
            for (t_i32 j = 0; j < arr.Len(); j++) {
                if (i == j) {
                    continue;
                }

                if (comparator(arr[i], arr[j])) {
                    return true;
                }
            }
        }

        return false;
    }

    template <co_array_nonstatic tp_arr_type>
    t_b8 RunBinarySearch(const tp_arr_type arr, const typename tp_arr_type::t_elem &elem, const t_ord_comparator<typename tp_arr_type::t_elem> comparator = DefaultOrdComparator) {
        if (arr.Len() == 0) {
            return false;
        }

        const auto &mid = arr[arr.Len() / 2];
        const auto comp_res = comparator(elem, mid);

        if (comp_res == 0) {
            return true;
        } else if (comp_res < 0) {
            return RunBinarySearch(arr.Slice(0, arr.Len() / 2), elem);
        } else {
            return RunBinarySearch(arr.SliceFrom((arr.Len() / 2) + 1), elem);
        }
    }


    // ============================================================
    // @section: Sorting

    template <co_array_nonstatic tp_arr_type>
    t_b8 IsSorted(const tp_arr_type arr, const t_ord_comparator<typename tp_arr_type::t_elem> comparator = DefaultOrdComparator) {
        for (t_i32 i = 0; i < arr.Len() - 1; i++) {
            if (comparator(arr[i], arr[i + 1]) > 0) {
                return false;
            }
        }

        return true;
    }

    // O(n) best-case if array is already sorted, O(n^2) worst-case.
    template <co_array_nonstatic tp_arr_type>
    void RunBubbleSort(const tp_arr_type arr, const t_ord_comparator<typename tp_arr_type::t_elem> comparator = DefaultOrdComparator) {
        t_b8 sorted;

        do {
            sorted = true;

            for (t_i32 i = 0; i < arr.Len() - 1; i++) {
                if (comparator(arr[i], arr[i + 1]) > 0) {
                    Swap(&arr[i], &arr[i + 1]);
                    sorted = false;
                }
            }
        } while (!sorted);
    }

    // O(n) best-case if array is already sorted, O(n^2) worst-case.
    template <co_array_nonstatic tp_arr_type>
    void RunInsertionSort(const tp_arr_type arr, const t_ord_comparator<typename tp_arr_type::t_elem> comparator = DefaultOrdComparator) {
        for (t_i32 i = 1; i < arr.Len(); i++) {
            const auto temp = arr[i];

            t_i32 j = i - 1;

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
    template <co_array_nonstatic tp_arr_type>
    void RunSelectionSort(const tp_arr_type arr, const t_ord_comparator<typename tp_arr_type::t_elem> comparator = DefaultOrdComparator) {
        for (t_i32 i = 0; i < arr.Len() - 1; i++) {
            const auto min = &arr[i];

            for (t_i32 j = i + 1; j < arr.Len(); j++) {
                if (comparator(arr[j], *min) < 0) {
                    *min = arr[j];
                }
            }

            Swap(&arr[i], &min);
        }
    }

    // O(n log n) in both time complexity and space complexity in every case.
    template <typename tp_arr_type>
    void RunMergeSort(const tp_arr_type arr, s_arena *const temp_mem_arena, const t_ord_comparator<typename tp_arr_type::t_elem> comparator = DefaultOrdComparator) {
        if (arr.Len() <= 1) {
            return;
        }

        // Sort copies of the left and right partitions.
        const auto arr_left_sorted = AllocArrayClone(arr.Slice(0, arr.Len() / 2), temp_mem_arena);
        RunMergeSort(arr_left_sorted, temp_mem_arena, comparator);

        const auto arr_right_sorted = AllocArrayClone(arr.SliceFrom(arr.Len() / 2), temp_mem_arena);
        RunMergeSort(arr_right_sorted, temp_mem_arena, comparator);

        // Update this array.
        t_i32 i = 0;
        t_i32 j = 0;

        do {
            if (comparator(arr_left_sorted[i], arr_right_sorted[j]) <= 0) {
                arr[i + j] = arr_left_sorted[i];
                i++;

                if (i == arr_left_sorted.Len()) {
                    // Copy over the remainder of the right array.
                    Copy(arr.SliceFrom(i + j), arr_right_sorted.SliceFrom(j));
                    break;
                }
            } else {
                arr[i + j] = arr_right_sorted[j];
                j++;

                if (j == arr_right_sorted.Len()) {
                    // Copy over the remainder of the left array.
                    Copy(arr.SliceFrom(i + j), arr_left_sorted.SliceFrom(i));
                    break;
                }
            }
        } while (true);
    }

    // Time complexity is O(n log n) best-case and O(n^2) worst-case depending on the pivot.
    // Space complexity is O(1) compared to merge sort.
    // In each recurse, the pivot is selected as the median of the first, middle, and last elements.
    template <co_array_nonstatic tp_arr_type>
    void RunQuickSort(const tp_arr_type arr, const t_ord_comparator<typename tp_arr_type::t_elem> comparator = DefaultOrdComparator) {
        if (arr.Len() <= 1) {
            return;
        }

        if (arr.Len() == 2) {
            if (comparator(arr[0], arr[1]) > 0) {
                Swap(&arr[0], &arr[1]);
            }

            return;
        }

        // Determine the pivot - median of the first, middle, and last elements.
        const t_i32 pivot_index = [arr, comparator]() {
            const t_i32 ia = 0;
            const t_i32 ib = arr.Len() / 2;
            const t_i32 ic = arr.Len() - 1;

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
        Swap(&arr[pivot_index], &arr[arr.Len() - 1]);

        // Move smaller elements to the left, and decide the final pivot position.
        t_i32 left_sec_last_index = -1;

        for (t_i32 i = 0; i < arr.Len(); i++) {
            if (comparator(arr[i], arr[arr.Len() - 1]) <= 0) {
                // This element is not greater than the pivot, so swap it to the left section.
                left_sec_last_index++;
                Swap(&arr[left_sec_last_index], &arr[i]);
            }
        }

        // Sort for each subsection.
        RunQuickSort(arr.Slice(0, left_sec_last_index), comparator);
        RunQuickSort(arr.SliceFrom(left_sec_last_index + 1), comparator);
    }

    // ============================================================
}
