#pragma once

#include <zcl/zcl_basic.h>

namespace zcl {
    template <c_array_mut tp_arr_type>
    constexpr void SetAllTo(const tp_arr_type arr, const typename tp_arr_type::t_elem &val) {
        for (t_i32 i = 0; i < arr.len; i++) {
            arr[i] = val;
        }
    }

    template <c_array tp_arr_type>
    t_b8 CheckAllEqual(const tp_arr_type arr, const typename tp_arr_type::t_elem &val, const t_comparator_bin<typename tp_arr_type::t_elem> comparator = k_comparator_bin_default<typename tp_arr_type::t_elem>) {
        if (arr.len == 0) {
            return false;
        }

        for (t_i32 i = 0; i < arr.len; i++) {
            if (!comparator(arr[i], val)) {
                return false;
            }
        }

        return true;
    }

    template <c_array tp_arr_type>
    t_b8 CheckAnyEqual(const tp_arr_type arr, const typename tp_arr_type::t_elem &val, const t_comparator_bin<typename tp_arr_type::t_elem> comparator = k_comparator_bin_default<typename tp_arr_type::t_elem>) {
        for (t_i32 i = 0; i < arr.len; i++) {
            if (comparator(arr[i], val)) {
                return true;
            }
        }

        return false;
    }

    template <c_array tp_arr_a_type, c_array tp_arr_b_type>
        requires c_same<typename tp_arr_a_type::t_elem, typename tp_arr_b_type::t_elem>
    t_b8 CompareAllBin(const tp_arr_a_type a, const tp_arr_b_type b, const t_comparator_bin<typename tp_arr_a_type::t_elem> comparator = k_comparator_bin_default<typename tp_arr_a_type::t_elem>) {
        if (a.len != b.len) {
            return false;
        }

        for (t_i32 i = 0; i < a.len; i++) {
            if (!comparator(a[i], b[i])) {
                return false;
            }
        }

        return true;
    }

    template <c_array tp_arr_a_type, c_array tp_arr_b_type>
        requires c_same<typename tp_arr_a_type::t_elem, typename tp_arr_b_type::t_elem>
    t_i32 CompareAllOrd(const tp_arr_a_type a, const tp_arr_b_type b, const t_comparator_ord<typename tp_arr_a_type::t_elem> comparator = k_comparator_ord_default<typename tp_arr_a_type::t_elem>) {
        if (a.len != b.len) {
            return a.len < b.len ? -1 : 1;
        }

        for (t_i32 i = 0; i < a.len; i++) {
            const t_i32 comp = comparator(a[i], b[i]);

            if (comp != 0) {
                return comp;
            }
        }

        return 0;
    }

    template <c_array_mut tp_arr_type>
    constexpr void Reverse(const tp_arr_type arr) {
        for (t_i32 i = 0; i < arr.len / 2; i++) {
            Swap(&arr[i], &arr[arr.len - 1 - i]);
        }
    }

    // O(n^2) time complexity, but O(1) space complexity.
    // You're usually better off using a hash map and a linear search, or a bitset if values are numeric and the range is small.
    template <c_array tp_arr_type>
    t_b8 CheckDuplicatesSlow(const tp_arr_type arr, const t_comparator_bin<typename tp_arr_type::t_elem> comparator = k_comparator_bin_default<typename tp_arr_type::t_elem>) {
        for (t_i32 i = 0; i < arr.len; i++) {
            for (t_i32 j = 0; j < arr.len; j++) {
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

    template <c_array tp_arr_type>
    t_b8 RunBinarySearch(const tp_arr_type arr, const typename tp_arr_type::t_elem &elem, const t_comparator_ord<typename tp_arr_type::t_elem> comparator = k_comparator_ord_default<typename tp_arr_type::t_elem>) {
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

    template <c_array tp_arr_type>
    t_b8 CheckSorted(const tp_arr_type arr, const t_comparator_ord<typename tp_arr_type::t_elem> comparator = k_comparator_ord_default<typename tp_arr_type::t_elem>) {
        for (t_i32 i = 0; i < arr.len - 1; i++) {
            if (comparator(arr[i], arr[i + 1]) > 0) {
                return false;
            }
        }

        return true;
    }

    // O(n) best-case if array is already sorted, O(n^2) worst-case.
    template <c_array tp_arr_type>
    void RunBubbleSort(const tp_arr_type arr, const t_comparator_ord<typename tp_arr_type::t_elem> comparator = k_comparator_ord_default<typename tp_arr_type::t_elem>) {
        t_b8 sorted;

        do {
            sorted = true;

            for (t_i32 i = 0; i < arr.len - 1; i++) {
                if (comparator(arr[i], arr[i + 1]) > 0) {
                    Swap(&arr[i], &arr[i + 1]);
                    sorted = false;
                }
            }
        } while (!sorted);
    }

    // O(n) best-case if array is already sorted, O(n^2) worst-case.
    template <c_array tp_arr_type>
    void RunInsertionSort(const tp_arr_type arr, const t_comparator_ord<typename tp_arr_type::t_elem> comparator = k_comparator_ord_default<typename tp_arr_type::t_elem>) {
        for (t_i32 i = 1; i < arr.len; i++) {
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
    template <c_array tp_arr_type>
    void RunSelectionSort(const tp_arr_type arr, const t_comparator_ord<typename tp_arr_type::t_elem> comparator = k_comparator_ord_default<typename tp_arr_type::t_elem>) {
        for (t_i32 i = 0; i < arr.len - 1; i++) {
            const auto min = &arr[i];

            for (t_i32 j = i + 1; j < arr.len; j++) {
                if (comparator(arr[j], *min) < 0) {
                    *min = arr[j];
                }
            }

            Swap(&arr[i], &min);
        }
    }

    // O(n log n) in both time complexity and space complexity in every case.
    template <typename tp_arr_type>
    void RunMergeSort(const tp_arr_type arr, t_arena *const temp_arena, const t_comparator_ord<typename tp_arr_type::t_elem> comparator = k_comparator_ord_default<typename tp_arr_type::t_elem>) {
        if (arr.len <= 1) {
            return;
        }

        // Sort copies of the left and right partitions.
        const auto arr_left_sorted = ArenaPushArrayClone(ArraySlice(arr, 0, arr.len / 2), temp_arena);
        RunMergeSort(arr_left_sorted, temp_arena, comparator);

        const auto arr_right_sorted = ArenaPushArrayClone(ArraySliceFrom(arr, arr.len / 2), temp_arena);
        RunMergeSort(arr_right_sorted, temp_arena, comparator);

        // Update this array.
        t_i32 i = 0;
        t_i32 j = 0;

        do {
            if (comparator(arr_left_sorted[i], arr_right_sorted[j]) <= 0) {
                arr[i + j] = arr_left_sorted[i];
                i++;

                if (i == arr_left_sorted.len) {
                    // Copy over the remainder of the right array.
                    f_copy_all(ArraySliceFrom(arr_right_sorted, j), ArraySliceFrom(arr, i + j));
                    break;
                }
            } else {
                arr[i + j] = arr_right_sorted[j];
                j++;

                if (j == arr_right_sorted.len) {
                    // Copy over the remainder of the left array.
                    f_copy_all(ArraySliceFrom(arr_left_sorted, i), ArraySliceFrom(arr, i + j));
                    break;
                }
            }
        } while (true);
    }

    // Time complexity is O(n log n) best-case and O(n^2) worst-case depending on the pivot.
    // Space complexity is O(1) compared to merge sort.
    // In each recurse, the pivot is selected as the median of the first, middle, and last elements.
    template <c_array tp_arr_type>
    void RunQuickSort(const tp_arr_type arr, const t_comparator_ord<typename tp_arr_type::t_elem> comparator = k_comparator_ord_default<typename tp_arr_type::t_elem>) {
        if (arr.len <= 1) {
            return;
        }

        if (arr.len == 2) {
            if (comparator(arr[0], arr[1]) > 0) {
                Swap(&arr[0], &arr[1]);
            }

            return;
        }

        // Determine the pivot - median of the first, middle, and last elements.
        const t_i32 pivot_index = [arr, comparator]() {
            const t_i32 ia = 0;
            const t_i32 ib = arr.len / 2;
            const t_i32 ic = arr.len - 1;

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
        Swap(&arr[pivot_index], &arr[arr.len - 1]);

        // Move smaller elements to the left, and decide the final pivot position.
        t_i32 left_sec_last_index = -1;

        for (t_i32 i = 0; i < arr.len; i++) {
            if (comparator(arr[i], arr[arr.len - 1]) <= 0) {
                // This element is not greater than the pivot, so swap it to the left section.
                left_sec_last_index++;
                Swap(&arr[left_sec_last_index], &arr[i]);
            }
        }

        // Sort for each subsection.
        RunQuickSort(ArraySlice(arr, 0, left_sec_last_index), comparator);
        RunQuickSort(ArraySliceFrom(arr, left_sec_last_index + 1), comparator);
    }
}
