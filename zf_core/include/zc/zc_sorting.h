#pragma once

#include <zc/zc_seqs.h>
#include <zc/zc_bits.h>
#include <zc/zc_heaps.h>

namespace zf {
    template<typename tp_type>
    t_b8 IsSorted(const c_array<const tp_type> arr, const t_comparator<tp_type> comparator = DefaultComparator) {
        ZF_ASSERT(comparator);

        for (t_size i = 0; i < arr.Len() - 1; i++) {
            if (comparator(arr[i], arr[i + 1]) > 0) {
                return false;
            }
        }

        return true;
    }

    // O(n) best-case if array is already sorted, O(n^2) worst-case.
    template<typename tp_type>
    void BubbleSort(const c_array<tp_type> arr, const t_comparator<tp_type> comparator = DefaultComparator) {
        ZF_ASSERT(comparator);

        t_b8 sorted;

        do {
            sorted = true;

            for (t_size i = 0; i < arr.Len() - 1; i++) {
                if (comparator(arr[i], arr[i + 1]) > 0) {
                    Swap(arr[i], arr[i + 1]);
                    sorted = false;
                }
            }
        } while (!sorted);
    }

    // O(n) best-case if array is already sorted, O(n^2) worst-case.
    template<typename tp_type>
    void InsertionSort(const c_array<tp_type> arr, const t_comparator<tp_type> comparator = DefaultComparator) {
        ZF_ASSERT(comparator);

        for (t_size i = 1; i < arr.Len(); i++) {
            const tp_type temp = arr[i];

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
    template<typename tp_type>
    void SelectionSort(const c_array<tp_type> arr, const t_comparator<tp_type> comparator = DefaultComparator) {
        ZF_ASSERT(comparator);

        for (t_size i = 0; i < arr.Len() - 1; i++) {
            tp_type& min = arr[i];

            for (t_size j = i + 1; j < arr.Len(); j++) {
                if (comparator(arr[j], min) < 0) {
                    min = arr[j];
                }
            }

            Swap(arr[i], min);
        }
    }

    // O(n log n) in both time complexity and space complexity in every case.
    template<typename tp_type>
    t_b8 MergeSort(const c_array<tp_type> arr, c_mem_arena& temp_mem_arena, const t_comparator<tp_type> comparator = DefaultComparator) {
        ZF_ASSERT(comparator);

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

        if (!MergeSort(arr_left, comparator, temp_mem_arena) || !MergeSort(arr_right, comparator, temp_mem_arena)) {
            return false;
        }

        // Update this array.
        t_size i = 0;
        t_size j = 0;

        do {
            if (comparator(arr_left[i], arr_right[j]) <= 0) {
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
    void QuickSort(const c_array<const tp_type> arr, const t_comparator<tp_type> comparator = DefaultComparator, t_size (* const pivot_index_selection_func)(const c_array<const tp_type> arr) = QuickSortMedianOfThreePivotIndexSelection) {
        ZF_ASSERT(comparator);
        ZF_ASSERT(pivot_index_selection_func);

        if (arr.Len() <= 1) {
            return;
        }

        if (arr.Len() == 2) {
            if (comparator(arr[0], arr[1]) > 0) {
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
            if (comparator(arr[i], arr[arr.Len() - 1]) <= 0) {
                // This element is not greater than the pivot, so swap it to the left section.
                left_sec_last_index++;
                Swap(arr[left_sec_last_index], arr[i]);
            }
        }

        // Sort for each subsection.
        QuickSort(arr.Slice(0, left_sec_last_index), comparator, pivot_index_selection_func);
        QuickSort(arr.Slice(left_sec_last_index + 1), comparator, pivot_index_selection_func);
    }

    // Binary?
    template<co_integral tp_type>
    t_b8 RadixSort(const c_array<tp_type> arr, c_mem_arena& temp_mem_arena) {
        c_stack<tp_type> arr_0s;
        c_stack<tp_type> arr_1s;

        if (!arr_0s.Init(temp_mem_arena, arr.Len())) {
            return false;
        }

        if (!arr_1s.Init(temp_mem_arena, arr.Len())) {
            return false;
        }

        // Sort each bit from LSB to MSB.
        t_size bit_index = 0;

        while (bit_index < ZF_SIZE_IN_BITS(tp_type)) {
            for (t_size i = 0; i < arr.Len(); i++) {
                const c_bit_vector bytes(ToBytes(arr[0]));

                if (IsBitSet(bytes, bit_index)) {
                    arr_1s.Push(arr[i]);
                } else {
                    arr_0s.Push(arr[i]);
                }
            }

            t_size i = 0;

            for (t_size j = 0; j < arr_0s.Len(); j++) {
                arr[i] = arr_0s[j];
                i++;
            }

            arr_0s.Clear();

            for (t_size j = 0; j < arr_1s.Len(); j++) {
                arr[i] = arr_1s[j];
                i++;
            }

            arr_1s.Clear();

            bit_index++;
        }

        return true;
    }

    // O(n log n) in time complexity. The provided heap is modified in-place, a duplicate is not created for you.
    template<typename tp_key_type, typename tp_value_type>
    void HeapSort(const c_array<tp_value_type> dest, c_min_heap<tp_key_type, tp_value_type>& min_heap) {
        ZF_ASSERT(dest.Len() >= min_heap.GetElemCnt());

        t_size dest_index = 0;

        while (!min_heap.IsEmpty()) {
            dest[dest_index] = min_heap.GetMin().val;
            min_heap.RemoveMin();
            dest_index++;
        }
    }
}
