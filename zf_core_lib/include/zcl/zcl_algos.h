#pragma once

#include <zcl/zcl_mem.h>

namespace zf {
    // O(n^2) time complexity, but O(1) space complexity. Can also be done at compile-time.
    // You're usually better off using a hash map and a linear search, or a bit vector if
    // values are numeric and the range is small.
    template <c_nonstatic_array tp_type>
    constexpr t_b8 HasDuplicatesSlow(const tp_type arr, const t_bin_comparator<typename tp_type::t_elem> comparator = DefaultBinComparator) {
        for (t_len i = 0; i < arr.len; i++) {
            for (t_len j = 0; j < arr.len; j++) {
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

    template <c_nonstatic_array tp_type>
    t_b8 BinarySearch(const tp_type arr, const typename tp_type::t_elem &elem, const t_ord_comparator<typename tp_type::t_elem> comparator = DefaultOrdComparator) {
        if (IsArrayEmpty(arr)) {
            return false;
        }

        const auto &mid = elem[arr.len / 2];
        const auto comp = comparator(elem, mid);

        if (comp == 0) {
            return true;
        } else if (comp < 0) {
            return BinarySearch(Slice(arr, 0, arr.len / 2), elem);
        } else {
            return BinarySearch(Slice(arr, (arr.len / 2) + 1, arr.len), elem);
        }
    }

    // ============================================================
    // @section: Sorting
    // ============================================================
    template <c_nonstatic_array tp_type>
    t_b8 IsSorted(const tp_type arr, const t_bin_comparator<typename tp_type::t_elem> comparator = DefaultOrdComparator) {
        ZF_ASSERT(comparator);

        for (t_len i = 0; i < arr.len - 1; i++) {
            if (comparator(arr[i], arr[i + 1]) > 0) {
                return false;
            }
        }

        return true;
    }

    // O(n) best-case if array is already sorted, O(n^2) worst-case.
    template <typename tp_type>
    void BubbleSort(const s_array<tp_type> arr, const t_bin_comparator<tp_type> comparator = DefaultOrdComparator) {
        ZF_ASSERT(comparator);

        t_b8 sorted;

        do {
            sorted = true;

            for (t_len i = 0; i < arr.len - 1; i++) {
                if (comparator(arr[i], arr[i + 1]) > 0) {
                    Swap(arr[i], arr[i + 1]);
                    sorted = false;
                }
            }
        } while (!sorted);
    }

    // O(n) best-case if array is already sorted, O(n^2) worst-case.
    template <typename tp_type>
    void InsertionSort(const s_array<tp_type> arr, const t_bin_comparator<tp_type> comparator = DefaultOrdComparator) {
        ZF_ASSERT(comparator);

        for (t_len i = 1; i < arr.len; i++) {
            const auto temp = arr[i];

            t_len j = i - 1;

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
    template <typename tp_type>
    void SelectionSort(const s_array<tp_type> arr, const t_bin_comparator<tp_type> comparator = DefaultOrdComparator) {
        ZF_ASSERT(comparator);

        for (t_len i = 0; i < arr.len - 1; i++) {
            auto &min = arr[i];

            for (t_len j = i + 1; j < arr.len; j++) {
                if (comparator(arr[j], min) < 0) {
                    min = arr[j];
                }
            }

            Swap(arr[i], min);
        }
    }

    // O(n log n) in both time complexity and space complexity in every case. Returns true iff
    // no error occurred.
    template <c_nonstatic_array tp_type>
    [[nodiscard]] t_b8 MergeSort(const tp_type arr, s_mem_arena *const temp_mem_arena, const t_bin_comparator<typename tp_type::t_elem> comparator = DefaultOrdComparator) {
        ZF_ASSERT(comparator);

        if (arr.len <= 1) {
            return true;
        }

        // Sort copies of the left and right partitions.
        const auto arr_left = Slice(arr, 0, arr.len / 2);
        s_array<typename tp_type::t_elem> arr_left_sorted;

        if (!AllocArrayClone(arr_left_sorted, arr_left, temp_mem_arena)) {
            return false;
        }

        const auto arr_right = Slice(arr, arr.len / 2, arr.len);
        s_array<typename tp_type::t_elem> arr_right_sorted;

        if (!AllocArrayClone(arr_right_sorted, arr_right, temp_mem_arena)) {
            return false;
        }

        if (!MergeSort(arr_left_sorted, temp_mem_arena, comparator) || !MergeSort(arr_right_sorted, temp_mem_arena, comparator)) {
            return false;
        }

        // Update this array.
        t_len i = 0;
        t_len j = 0;

        do {
            if (comparator(arr_left_sorted[i], arr_right_sorted[j]) <= 0) {
                arr[i + j] = arr_left_sorted[i];
                i++;

                if (i == arr_left_sorted.len) {
                    // Copy over the remainder of the right array.
                    Copy(Slice(arr, i + j, arr.len), Slice(arr_right_sorted, j, arr_right_sorted.len));
                    break;
                }
            } else {
                arr[i + j] = arr_right_sorted[j];
                j++;

                if (j == arr_right_sorted.len) {
                    // Copy over the remainder of the left array.
                    Copy(Slice(arr, i + j, arr.len), Slice(arr_left_sorted, i, arr_left_sorted.len));
                    break;
                }
            }
        } while (true);

        return true;
    }

    // Time complexity is O(n log n) best-case and O(n^2) worst-case depending on the pivot.
    // Space complexity is O(1) compared to merge sort.
    // In each recurse, the pivot is selected as the median of the first, middle, and last
    // elements.
    template <typename tp_type>
    void QuickSort(const s_array<tp_type> arr, const t_bin_comparator<tp_type> comparator = DefaultOrdComparator) {
        ZF_ASSERT(comparator);

        if (arr.len <= 1) {
            return;
        }

        if (arr.len == 2) {
            if (comparator(arr[0], arr[1]) > 0) {
                Swap(arr[0], arr[1]);
            }

            return;
        }

        // Determine the pivot - median of the first, middle, and last elements.
        const t_len pivot_index = [arr, comparator]() {
            const t_len ia = 0;
            const t_len ib = arr.len / 2;
            const t_len ic = arr.len - 1;

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
        Swap(arr[pivot_index], arr[arr.len - 1]);

        // Move smaller elements to the left, and decide the final pivot position.
        t_len left_sec_last_index = -1;

        for (t_len i = 0; i < arr.len; i++) {
            if (comparator(arr[i], arr[arr.len - 1]) <= 0) {
                // This element is not greater than the pivot, so swap it to the left section.
                left_sec_last_index++;
                Swap(arr[left_sec_last_index], arr[i]);
            }
        }

        // Sort for each subsection.
        const auto left = Slice(arr, 0, left_sec_last_index);
        QuickSort(left, comparator);

        const auto right = Slice(arr, left_sec_last_index + 1, arr.len);
        QuickSort(right, comparator);
    }
}
