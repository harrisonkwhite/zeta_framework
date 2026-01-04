#pragma once

#include <zcl/zcl_mem.h>

namespace zf {
    template <co_array tp_src_arr_type, co_array_mut tp_dest_arr_type>
        requires co_same<typename tp_src_arr_type::t_elem, typename tp_dest_arr_type::t_elem>
    void CopyAll(const tp_src_arr_type src, const tp_dest_arr_type dest, const B8 allow_truncation = false) {
        if (!allow_truncation) {
            ZF_ASSERT(dest.len >= src.len);

            for (I32 i = 0; i < src.len; i++) {
                dest[i] = src[i];
            }
        } else {
            const auto min_len = ZF_MIN(src.len, dest.len);

            for (I32 i = 0; i < min_len; i++) {
                dest[i] = src[i];
            }
        }
    }

    template <co_array tp_arr_a_type, co_array tp_arr_b_type>
        requires co_same<typename tp_arr_a_type::t_elem, typename tp_arr_b_type::t_elem>
    I32 CompareAll(const tp_arr_a_type a, const tp_arr_b_type b, const t_comparator_ord<typename tp_arr_a_type::t_elem> comparator = g_comparator_ord_default<typename tp_arr_a_type::t_elem>) {
        const auto min_len = ZF_MIN(a.len, b.len);

        for (I32 i = 0; i < min_len; i++) {
            const I32 comp = comparator(a[i], b[i]);

            if (comp != 0) {
                return comp;
            }
        }

        return 0;
    }

    template <co_array tp_arr_type>
    B8 DoAllEqual(const tp_arr_type arr, const typename tp_arr_type::t_elem &val, const t_comparator_bin<typename tp_arr_type::t_elem> comparator = g_comparator_bin_default<typename tp_arr_type::t_elem>) {
        if (arr.len == 0) {
            return false;
        }

        for (I32 i = 0; i < arr.len; i++) {
            if (!comparator(arr[i], val)) {
                return false;
            }
        }

        return true;
    }

    template <co_array tp_arr_type>
    B8 DoAnyEqual(const tp_arr_type arr, const typename tp_arr_type::t_elem &val, const t_comparator_bin<typename tp_arr_type::t_elem> comparator = g_comparator_bin_default<typename tp_arr_type::t_elem>) {
        for (I32 i = 0; i < arr.len; i++) {
            if (comparator(arr[i], val)) {
                return true;
            }
        }

        return false;
    }

    template <co_array_mut tp_arr_type>
    void SetAllTo(const tp_arr_type arr, const typename tp_arr_type::t_elem &val) {
        for (I32 i = 0; i < arr.len; i++) {
            arr[i] = val;
        }
    }

    template <co_array_mut tp_arr_type>
    void Reverse(const tp_arr_type arr) {
        for (I32 i = 0; i < arr.len / 2; i++) {
            swap(&arr[i], &arr[arr.len - 1 - i]);
        }
    }

    // O(n^2) time complexity, but O(1) space complexity.
    // You're usually better off using a hash map and a linear search, or a bit vector if values are numeric and the range is small.
    template <co_array tp_arr_type>
    B8 HasDuplicatesSlow(const tp_arr_type arr, const t_comparator_bin<typename tp_arr_type::t_elem> comparator = g_comparator_bin_default<typename tp_arr_type::t_elem>) {
        for (I32 i = 0; i < arr.len; i++) {
            for (I32 j = 0; j < arr.len; j++) {
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

    template <co_array tp_arr_type>
    B8 RunBinarySearch(const tp_arr_type arr, const typename tp_arr_type::t_elem &elem, const t_comparator_ord<typename tp_arr_type::t_elem> comparator = g_comparator_ord_default<typename tp_arr_type::t_elem>) {
        if (arr.len == 0) {
            return false;
        }

        const auto &mid = arr[arr.len / 2];
        const auto comp_res = comparator(elem, mid);

        if (comp_res == 0) {
            return true;
        } else if (comp_res < 0) {
            return RunBinarySearch(ArraySlice(arr, 0, arr.len / 2), elem);
        } else {
            return RunBinarySearch(ArraySliceFrom(arr, (arr.len / 2) + 1), elem);
        }
    }

    template <co_array tp_arr_type>
    B8 IsSorted(const tp_arr_type arr, const t_comparator_ord<typename tp_arr_type::t_elem> comparator = g_comparator_ord_default<typename tp_arr_type::t_elem>) {
        for (I32 i = 0; i < arr.len - 1; i++) {
            if (comparator(arr[i], arr[i + 1]) > 0) {
                return false;
            }
        }

        return true;
    }

    // O(n) best-case if array is already sorted, O(n^2) worst-case.
    template <co_array tp_arr_type>
    void RunBubbleSort(const tp_arr_type arr, const t_comparator_ord<typename tp_arr_type::t_elem> comparator = g_comparator_ord_default<typename tp_arr_type::t_elem>) {
        B8 sorted;

        do {
            sorted = true;

            for (I32 i = 0; i < arr.len - 1; i++) {
                if (comparator(arr[i], arr[i + 1]) > 0) {
                    swap(&arr[i], &arr[i + 1]);
                    sorted = false;
                }
            }
        } while (!sorted);
    }

    // O(n) best-case if array is already sorted, O(n^2) worst-case.
    template <co_array tp_arr_type>
    void RunInsertionSort(const tp_arr_type arr, const t_comparator_ord<typename tp_arr_type::t_elem> comparator = g_comparator_ord_default<typename tp_arr_type::t_elem>) {
        for (I32 i = 1; i < arr.len; i++) {
            const auto temp = arr[i];

            I32 j = i - 1;

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
    template <co_array tp_arr_type>
    void RunSelectionSort(const tp_arr_type arr, const t_comparator_ord<typename tp_arr_type::t_elem> comparator = g_comparator_ord_default<typename tp_arr_type::t_elem>) {
        for (I32 i = 0; i < arr.len - 1; i++) {
            const auto min = &arr[i];

            for (I32 j = i + 1; j < arr.len; j++) {
                if (comparator(arr[j], *min) < 0) {
                    *min = arr[j];
                }
            }

            swap(&arr[i], &min);
        }
    }

    // O(n log n) in both time complexity and space complexity in every case.
    template <typename tp_arr_type>
    void RunMergeSort(const tp_arr_type arr, s_arena *const temp_arena, const t_comparator_ord<typename tp_arr_type::t_elem> comparator = g_comparator_ord_default<typename tp_arr_type::t_elem>) {
        if (arr.len <= 1) {
            return;
        }

        // Sort copies of the left and right partitions.
        const auto arr_left_sorted = CloneArray(ArraySlice(arr, 0, arr.len / 2), temp_arena);
        RunMergeSort(arr_left_sorted, temp_arena, comparator);

        const auto arr_right_sorted = CloneArray(ArraySliceFrom(arr, arr.len / 2), temp_arena);
        RunMergeSort(arr_right_sorted, temp_arena, comparator);

        // Update this array.
        I32 i = 0;
        I32 j = 0;

        do {
            if (comparator(arr_left_sorted[i], arr_right_sorted[j]) <= 0) {
                arr[i + j] = arr_left_sorted[i];
                i++;

                if (i == arr_left_sorted.len) {
                    // Copy over the remainder of the right array.
                    CopyAll(ArraySliceFrom(arr_right_sorted, j), ArraySliceFrom(arr, i + j));
                    break;
                }
            } else {
                arr[i + j] = arr_right_sorted[j];
                j++;

                if (j == arr_right_sorted.len) {
                    // Copy over the remainder of the left array.
                    CopyAll(ArraySliceFrom(arr_left_sorted, i), ArraySliceFrom(arr, i + j));
                    break;
                }
            }
        } while (true);
    }

    // Time complexity is O(n log n) best-case and O(n^2) worst-case depending on the pivot.
    // Space complexity is O(1) compared to merge sort.
    // In each recurse, the pivot is selected as the median of the first, middle, and last elements.
    template <co_array tp_arr_type>
    void RunQuickSort(const tp_arr_type arr, const t_comparator_ord<typename tp_arr_type::t_elem> comparator = g_comparator_ord_default<typename tp_arr_type::t_elem>) {
        if (arr.len <= 1) {
            return;
        }

        if (arr.len == 2) {
            if (comparator(arr[0], arr[1]) > 0) {
                swap(&arr[0], &arr[1]);
            }

            return;
        }

        // Determine the pivot - median of the first, middle, and last elements.
        const I32 pivot_index = [arr, comparator]() {
            const I32 ia = 0;
            const I32 ib = arr.len / 2;
            const I32 ic = arr.len - 1;

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
        swap(&arr[pivot_index], &arr[arr.len - 1]);

        // Move smaller elements to the left, and decide the final pivot position.
        I32 left_sec_last_index = -1;

        for (I32 i = 0; i < arr.len; i++) {
            if (comparator(arr[i], arr[arr.len - 1]) <= 0) {
                // This element is not greater than the pivot, so swap it to the left section.
                left_sec_last_index++;
                swap(&arr[left_sec_last_index], &arr[i]);
            }
        }

        // Sort for each subsection.
        RunQuickSort(ArraySlice(arr, 0, left_sec_last_index), comparator);
        RunQuickSort(ArraySliceFrom(arr, left_sec_last_index + 1), comparator);
    }
}
