#pragma once

#include <zc/zc_mem.h>

namespace zf {
    // O(n^2) time complexity, but O(1) space complexity. Can also be done at compile-time.
    // You're usually better off using a hash map and a linear search, or a bit vector if values are numeric and the range is small.
    template<c_nonstatic_array tp_type>
    constexpr t_b8 HasDuplicatesSlow(const tp_type arr, const t_bin_comparator<typename tp_type::t_elem> comparator = DefaultBinComparator) {
        for (t_size i = 0; i < ArrayLen(arr); i++) {
            for (t_size j = 0; j < ArrayLen(arr); j++) {
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

    template<typename tp_type, t_size tp_len>
    constexpr t_b8 HasDuplicatesSlow(const s_static_array<tp_type, tp_len>& arr, const t_bin_comparator<tp_type> comparator = DefaultBinComparator) {
        return HasDuplicatesSlow(static_cast<s_array_rdonly<tp_type>>(arr), comparator);
    }

    template<c_nonstatic_array tp_type>
    t_b8 BinarySearch(const tp_type arr, const typename tp_type::t_elem& elem, const t_ord_comparator<typename tp_type::t_elem> comparator = DefaultOrdComparator) {
        if (IsArrayEmpty(arr)) {
            return false;
        }

        const auto& mid = elem[ArrayLen(arr) / 2];
        const auto comp = comparator(elem, mid);

        if (comp == 0) {
            return true;
        } else if (comp < 0) {
            return BinarySearch(Slice(arr, 0, ArrayLen(arr) / 2), elem);
        } else {
            return BinarySearch(Slice(arr, (ArrayLen(arr) / 2) + 1, ArrayLen(arr)), elem);
        }
    }

    template<typename tp_type, t_size tp_len>
    t_b8 BinarySearch(const s_static_array<tp_type, tp_len>& arr, const tp_type& elem, const t_ord_comparator<tp_type> comparator = DefaultOrdComparator) {
        return BinarySearch(static_cast<s_array_rdonly<tp_type>>(arr), elem, comparator);
    }

    // ============================================================
    // @section: Sorting
    // ============================================================
    template<c_nonstatic_array tp_type>
    t_b8 IsSorted(const tp_type arr, const t_bin_comparator<typename tp_type::t_elem> comparator = DefaultOrdComparator) {
        ZF_ASSERT(comparator);

        for (t_size i = 0; i < ArrayLen(arr) - 1; i++) {
            if (comparator(arr[i], arr[i + 1]) > 0) {
                return false;
            }
        }

        return true;
    }

    template<typename tp_type, t_size tp_len>
    t_b8 IsSorted(const s_static_array<tp_type, tp_len>& arr, const t_bin_comparator<tp_type> comparator = DefaultOrdComparator) {
        return IsSorted(static_cast<s_array_rdonly<tp_type>>(arr), comparator);
    }

    // O(n) best-case if array is already sorted, O(n^2) worst-case.
    template<c_nonstatic_array_mut tp_type>
    void BubbleSort(const tp_type arr, const t_bin_comparator<typename tp_type::t_elem> comparator = DefaultOrdComparator) {
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

    template<typename tp_type, t_size tp_len>
    void BubbleSort(s_static_array<tp_type, tp_len>& arr, const t_bin_comparator<tp_type> comparator = DefaultOrdComparator) {
        BubbleSort(static_cast<s_array<tp_type>>(arr), comparator);
    }

    // O(n) best-case if array is already sorted, O(n^2) worst-case.
    template<c_nonstatic_array_mut tp_type>
    void InsertionSort(const tp_type arr, const t_bin_comparator<typename tp_type::t_elem> comparator = DefaultOrdComparator) {
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

    template<typename tp_type, t_size tp_len>
    void InsertionSort(s_static_array<tp_type, tp_len>& arr, const t_bin_comparator<tp_type> comparator = DefaultOrdComparator) {
        InsertionSort(static_cast<s_array<tp_type>>(arr), comparator);
    }

    // O(n^2) in every case.
    template<c_nonstatic_array_mut tp_type>
    void SelectionSort(const tp_type arr, const t_bin_comparator<typename tp_type::t_elem> comparator = DefaultOrdComparator) {
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

    template<typename tp_type, t_size tp_len>
    void SelectionSort(s_static_array<tp_type, tp_len>& arr, const t_bin_comparator<tp_type> comparator = DefaultOrdComparator) {
        SelectionSort(static_cast<s_array<tp_type>>(arr), comparator);
    }

    // O(n log n) in both time complexity and space complexity in every case. Returns true iff no error occurred.
    template<c_nonstatic_array_mut tp_type>
    [[nodiscard]] t_b8 MergeSort(const tp_type arr, s_mem_arena& temp_mem_arena, const t_bin_comparator<typename tp_type::t_elem> comparator = DefaultOrdComparator) {
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

    template<typename tp_type, t_size tp_len>
    [[nodiscard]] t_b8 MergeSort(s_static_array<tp_type, tp_len>& arr, s_mem_arena& temp_mem_arena, const t_bin_comparator<tp_type> comparator = DefaultOrdComparator) {
        return MergeSort(static_cast<s_array<tp_type>>(arr), temp_mem_arena, comparator);
    }

    // Time complexity is O(n log n) best-case and O(n^2) worst-case depending on the pivot.
    // Space complexity is O(1) compared to merge sort.
    // In each recurse, the pivot is selected as the median of the first, middle, and last elements.
    template<c_nonstatic_array_mut tp_type>
    void QuickSort(const tp_type arr, const t_bin_comparator<typename tp_type::t_elem> comparator = DefaultOrdComparator) {
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

    template<typename tp_type, t_size tp_len>
    void QuickSort(s_static_array<tp_type, tp_len>& arr, const t_bin_comparator<tp_type> comparator = DefaultOrdComparator) {
        QuickSort(static_cast<s_array<tp_type>>(arr), comparator);
    }
}
