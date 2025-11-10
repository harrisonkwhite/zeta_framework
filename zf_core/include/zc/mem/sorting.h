#pragma once

#include <zc/mem/min_heap.h>

namespace zf {
    // Like the qsort comparison function, should return negative if a < b, 0 if a == b, and positive if b > a.
    template<typename tp_type>
    using t_comparison_func = t_s32 (*)(const tp_type& a, const tp_type& b);

    template<typename tp_type>
    t_b8 IsSorted(const c_array<const tp_type> arr, const t_comparison_func<tp_type> comparison_func) {
        ZF_ASSERT(comparison_func);

        for (t_size i = 0; i < arr.Len() - 1; i++) {
            if (comparison_func(arr[i], arr[i + 1]) > 0) {
                return false;
            }
        }

        return true;
    }

    // O(n) best-case if array is already sorted, O(n^2) worst-case.
    template<typename tp_type>
    void BubbleSort(const c_array<tp_type> arr, const t_comparison_func<tp_type> comparison_func) {
        ZF_ASSERT(comparison_func);

        t_b8 sorted;

        do {
            sorted = true;

            for (t_size i = 0; i < arr.Len() - 1; i++) {
                if (comparison_func(arr[i], arr[i + 1]) > 0) {
                    Swap(arr[i], arr[i + 1]);
                    sorted = false;
                }
            }
        } while (!sorted);
    }

    // O(n) best-case if array is already sorted, O(n^2) worst-case.
    template<typename tp_type>
    void InsertionSort(const c_array<tp_type> arr, const t_comparison_func<tp_type> comparison_func) {
        ZF_ASSERT(comparison_func);

        for (t_size i = 1; i < arr.Len(); i++) {
            const tp_type temp = arr[i];

            t_size j = i - 1;

            for (; j >= 0; j--) {
                if (comparison_func(arr[j], temp) <= 0) {
                    break;
                }

                arr[j + 1] = arr[j];
            }

            arr[j + 1] = temp;
        }
    }

    // O(n^2) in every case.
    template<typename tp_type>
    void SelectionSort(const c_array<tp_type> arr, const t_comparison_func<tp_type> comparison_func) {
        ZF_ASSERT(comparison_func);

        for (t_size i = 0; i < arr.Len() - 1; i++) {
            tp_type& min = arr[i];

            for (t_size j = i + 1; j < arr.Len(); j++) {
                if (comparison_func(arr[j], min) < 0) {
                    min = arr[j];
                }
            }

            Swap(arr[i], min);
        }
    }

    // O(n log n) in both time complexity and space complexity in every case.
    template<typename tp_type>
    t_b8 MergeSort(const c_array<tp_type> arr, const t_comparison_func<tp_type> comparison_func, c_mem_arena& temp_mem_arena) {
        ZF_ASSERT(comparison_func);

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

        if (!MergeSort(arr_left, comparison_func, temp_mem_arena) || !MergeSort(arr_right, comparison_func, temp_mem_arena)) {
            return false;
        }

        // Update this array.
        t_size i = 0;
        t_size j = 0;

        do {
            if (comparison_func(arr_left[i], arr_right[j]) <= 0) {
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
    t_size QuickSortMedianOfThreePivotIndexSelection(const c_array<const tp_type> arr) {
        const t_size ia = 0;
        const t_size ib = arr.Len() / 2;
        const t_size ic = arr.Len() - 1;

        if (arr[ia] <= arr[ib]) {
            if (arr[ib] <= arr[ic]) {
                return ib;
            } else {
                if (arr[ia] <= arr[ic]) {
                    return ic;
                } else {
                    return ia;
                }
            }
        } else {
            if (arr[ia] <= arr[ic]) {
                return ia;
            } else {
                if (arr[ib] <= arr[ic]) {
                    return ic;
                } else {
                    return ib;
                }
            }
        }
    }

    // Time complexity is O(n log n) best-case and O(n^2) worst-case depending on the pivot.
    // Space complexity is O(1) compared to merge sort.
    // Custom pivot index selection function is only for arrays of length 3 or greater.
    template<typename tp_type>
    void QuickSort(const c_array<const tp_type> arr, const t_comparison_func<const tp_type> comparison_func, t_size (* const pivot_index_selection_func)(const c_array<const tp_type> arr) = QuickSortMedianOfThreePivotIndexSelection) {
        ZF_ASSERT(comparison_func);
        ZF_ASSERT(pivot_index_selection_func);

        if (arr.Len() <= 1) {
            return;
        }

        if (arr.Len() == 2) {
            if (comparison_func(arr[0], arr[1]) > 0) {
                Swap(arr[0], arr[1]);
            }

            return;
        }

        // Get the pivot index from the given function and swap the pivot out to the end.
        const t_size pivot_index = pivot_index_selection_func(arr);
        ZF_ASSERT(pivot_index < arr.Len());

        Swap(arr[pivot_index], arr[arr.Len() - 1]);

        // Move smaller elements to the left, and decide the final pivot position.
        t_size left_sec_last_index = -1;

        for (t_size i = 0; i < arr.Len(); i++) {
            if (comparison_func(arr[i], arr[arr.Len() - 1]) <= 0) {
                // This element is not greater than the pivot, so swap it to the left section.
                left_sec_last_index++;
                Swap(arr[left_sec_last_index], arr[i]);
            }
        }

        // Sort for each subsection.
        QuickSort(arr.Slice(0, left_sec_last_index), comparison_func, pivot_index_selection_func);
        QuickSort(arr.Slice(left_sec_last_index + 1), comparison_func, pivot_index_selection_func);
    }

    // O(n log n) in time complexity. The provided heap is modified in-place, a duplicate is not created for you.
    template<typename tp_type>
    void HeapSort(const c_array<tp_type> dest, c_min_heap<tp_type>& min_heap) {
        ZF_ASSERT(dest.Len() >= min_heap.GetElemCnt());

        t_size dest_index = 0;

        while (!min_heap.IsEmpty()) {
            dest[dest_index] = min_heap.GetMin();
            min_heap.RemoveMin();
            dest_index++;
        }
    }
}
