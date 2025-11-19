#pragma once

#include <initializer_list>
#include <zc/zc_mem.h>

namespace zf {
    template<typename tp_type>
    struct s_array {
        tp_type* buf_raw;
        t_size len;

        constexpr tp_type& operator[](const t_size index) const {
            ZF_ASSERT(index >= 0 && index < len);
            return buf_raw[index];
        }

        constexpr operator s_array<const tp_type>() const requires (!s_is_const<tp_type>::g_value) {
            return {buf_raw, len};
        }
    };

    template<typename tp_type, t_size tp_len>
    struct s_static_array {
        static_assert(!s_is_const<tp_type>::g_value);

        static constexpr t_size g_len = tp_len;

        tp_type buf_raw[tp_len];

        constexpr s_static_array() = default;

        constexpr s_static_array(const std::initializer_list<tp_type> init) {
            ZF_ASSERT(init.size() == tp_len);

            t_size i = 0;

            for (const tp_type& v : init) {
                buf_raw[i] = v;
                i++;
            }
        }

        constexpr tp_type& operator[](const t_size index) {
            ZF_ASSERT(index >= 0 && index < tp_len);
            return buf_raw[index];
        }

        constexpr const tp_type& operator[](const t_size index) const {
            ZF_ASSERT(index >= 0 && index < tp_len);
            return buf_raw[index];
        }

        constexpr operator s_array<tp_type>() {
            return {buf_raw, tp_len};
        }

        constexpr operator s_array<const tp_type>() const {
            return {buf_raw, tp_len};
        }
    };

    template<typename tp_type, t_size tp_len>
    constexpr s_array<tp_type> ToNonstatic(s_static_array<tp_type, tp_len>& arr) {
        return static_cast<s_array<tp_type>>(arr);
    }

    template<typename tp_type, t_size tp_len>
    constexpr s_array<const tp_type> ToNonstatic(const s_static_array<tp_type, tp_len>& arr) {
        return static_cast<s_array<const tp_type>>(arr);
    }

    template<typename tp_type>
    constexpr t_b8 IsArrayEmpty(const s_array<const tp_type> arr) {
        return arr.len == 0;
    }

    template<typename tp_type, t_size tp_len>
    constexpr t_b8 IsArrayEmpty(const s_static_array<tp_type, tp_len> arr) {
        return IsArrayEmpty(ToNonstatic(arr));
    }

    template<typename tp_type>
    constexpr t_size ArraySizeInBytes(const s_array<const tp_type> arr) {
        return static_cast<t_size>(sizeof(arr[0])) * arr.len;
    }

    template<typename tp_type, t_size tp_len>
    constexpr t_size ArraySizeInBytes(const s_static_array<tp_type, tp_len> arr) {
        return ArraySizeInBytes(ToNonstatic(arr));
    }

    template<typename tp_type>
    t_b8 MakeArray(s_mem_arena& mem_arena, const t_size len, s_array<tp_type>& o_arr) {
        ZF_ASSERT(len > 0);

        const auto buf_raw = PushToMemArena<tp_type>(mem_arena, len);

        if (!buf_raw) {
            return false;
        }

        o_arr = {buf_raw, len};

        return true;
    }

    template<typename tp_type>
    t_b8 CloneArray(s_mem_arena& mem_arena, const s_array<const tp_type> arr_to_clone, s_array<tp_type>& o_arr) {
        ZF_ASSERT(!IsArrayEmpty(arr_to_clone));

        if (!MakeArray(mem_arena, arr_to_clone.len, o_arr)) {
            return false;
        }

        Copy(o_arr, arr_to_clone);

        return true;
    }

    template<typename tp_type, t_size tp_len>
    t_b8 CloneArray(s_mem_arena& mem_arena, const s_static_array<const tp_type, tp_len> arr_to_clone, s_array<tp_type>& o_arr) {
        return CloneArray(mem_arena, ToNonstatic(arr_to_clone), o_arr);
    }

    template<typename tp_type>
    constexpr s_array<tp_type> Slice(const s_array<tp_type> arr, const t_size beg, const t_size end) {
        ZF_ASSERT(beg >= 0 && beg <= arr.len);
        ZF_ASSERT(end >= beg && end <= arr.len);

        return {arr.buf_raw + beg, end - beg};
    }

    template<typename tp_type, t_size tp_len>
    constexpr s_array<tp_type> Slice(const s_static_array<tp_type, tp_len> arr, const t_size beg, const t_size end) {
        return Slice(ToNonstatic(arr), beg, end);
    }

    template<typename tp_type>
    void Copy(const s_array<tp_type> dest, const s_array<const tp_type> src) {
        ZF_ASSERT(dest.len >= src.len);

        for (t_size i = 0; i < src.len; i++) {
            dest[i] = src[i];
        }
    }

    template<typename tp_type, t_size tp_len>
    void Copy(const s_static_array<tp_type, tp_len> dest, const s_array<const tp_type> src) {
        Copy(ToNonstatic(dest), src);
    }

    template<typename tp_type>
    void CopyReverse(const s_array<tp_type> dest, const s_array<const tp_type> src) {
        ZF_ASSERT(dest.len >= src.len);

        for (t_size i = src.len - 1; i >= 0; i--) {
            dest[i] = src[i];
        }
    }

    template<typename tp_type, t_size tp_len>
    void CopyReverse(const s_static_array<tp_type, tp_len> dest, const s_array<const tp_type> src) {
        CopyReverse(ToNonstatic(dest), src);
    }

    template<typename tp_type>
    t_b8 AreAllEqualTo(const s_array<const tp_type> arr, const tp_type& val, const t_comparator<tp_type> comparator = DefaultComparator) {
        ZF_ASSERT(comparator);

        for (t_size i = 0; i < arr.len; i++) {
            if (comparator(arr[i], val) != 0) {
                return false;
            }
        }

        return true;
    }

    template<typename tp_type, t_size tp_len>
    t_b8 AreAllEqualTo(const s_static_array<const tp_type, tp_len> arr, const tp_type& val, const t_comparator<tp_type> comparator = DefaultComparator) {
        return AreAllEqualTo(ToNonstatic(arr), val, comparator);
    }

    template<typename tp_type>
    t_b8 AreAnyEqualTo(const s_array<const tp_type> arr, const tp_type& val, const t_comparator<tp_type> comparator = DefaultComparator) {
        ZF_ASSERT(comparator);

        for (t_size i = 0; i < arr.len; i++) {
            if (comparator(arr[i], val) == 0) {
                return true;
            }
        }

        return false;
    }

    template<typename tp_type, t_size tp_len>
    t_b8 AreAnyEqualTo(const s_static_array<const tp_type, tp_len> arr, const tp_type& val, const t_comparator<tp_type> comparator = DefaultComparator) {
        return AreAnyEqualTo(ToNonstatic(arr), val, comparator);
    }

    template<typename tp_type>
    void SetAllTo(const s_array<tp_type> arr, const tp_type& val) {
        for (t_size i = 0; i < arr.len; i++) {
            arr[i] = val;
        }
    }

    template<typename tp_type, t_size tp_len>
    void SetAllTo(const s_static_array<tp_type, tp_len> arr, const tp_type& val) {
        SetAllTo(ToNonstatic(arr), val);
    }

    template<typename tp_type>
    t_b8 BinarySearch(const s_array<const tp_type> arr, const tp_type& elem, const t_comparator<tp_type> comparator = DefaultComparator) {
        ZF_ASSERT(IsSorted(arr));

        if (arr.len == 0) {
            return false;
        }

        const tp_type& mid = elem[arr.len / 2];

        if (elem == mid) {
            return true;
        } else if (elem < mid) {
            return BinarySearch(arr.Slice(0, arr.len / 2), elem);
        } else {
            return BinarySearch(arr.Slice((arr.len / 2) + 1, arr.len), elem);
        }
    }

    template<typename tp_type, t_size tp_len>
    t_b8 BinarySearch(const s_static_array<const tp_type, tp_len> arr, const tp_type& elem, const t_comparator<tp_type> comparator = DefaultComparator) {
        return BinarySearch(ToNonstatic(arr), elem, comparator);
    }

    template<typename tp_type>
    constexpr s_array<const t_u8> ToBytes(const tp_type& item) {
        return {reinterpret_cast<const t_u8*>(item), ZF_SIZE_OF(item)};
    }

    template<typename tp_type>
    constexpr s_array<t_u8> ToBytes(tp_type& item) {
        return {reinterpret_cast<t_u8*>(item), ZF_SIZE_OF(item)};
    }
}
