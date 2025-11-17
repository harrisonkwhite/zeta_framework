#pragma once

#include <zc/zc_allocators.h>

namespace zf {
    template<typename tp_type>
    struct s_array {
    public:
        constexpr s_array() = default;

        constexpr s_array(tp_type* const buf_raw, const t_size len) : m_buf_raw(buf_raw), m_len(len) {
            ZF_ASSERT((!buf_raw && len == 0) || (buf_raw && len >= 0));
        }

        constexpr tp_type* Raw() const {
            return m_buf_raw;
        }

        constexpr t_size Len() const {
            return m_len;
        }

        constexpr t_size SizeInBytes() const {
            return ZF_SIZE_OF(tp_type) * m_len;
        }

        constexpr t_b8 IsEmpty() const {
            return m_len == 0;
        }

        constexpr tp_type& operator[](const t_size index) const {
            ZF_ASSERT(index >= 0 && index < m_len);
            return m_buf_raw[index];
        }

        constexpr s_array<const tp_type> ToReadonly() const requires (!s_is_const<tp_type>::sm_value) {
            return {m_buf_raw, m_len};
        }

        constexpr operator s_array<const tp_type>() const requires (!s_is_const<tp_type>::sm_value) {
            return ToReadonly();
        }

    private:
        tp_type* m_buf_raw;
        t_size m_len;
    };

    template<typename tp_type, t_size tp_len>
    struct s_static_array {
        static_assert(!s_is_const<tp_type>::sm_value);
        static_assert(tp_len > 0);

        tp_type buf_raw[tp_len] = {};

        constexpr s_static_array() = default;

        constexpr s_static_array(const tp_type (&buf_raw)[tp_len]) {
            for (t_size i = 0; i < tp_len; i++) {
                this->buf_raw[i] = buf_raw[i];
            }
        }

        constexpr t_size Len() const {
            return tp_len;
        }

        constexpr tp_type& operator[](const t_size index) {
            ZF_ASSERT(index >= 0 && index < tp_len);
            return buf_raw[index];
        }

        constexpr const tp_type& operator[](const t_size index) const {
            ZF_ASSERT(index >= 0 && index < tp_len);
            return buf_raw[index];
        }

        constexpr s_array<tp_type> ToNonstatic() {
            return {buf_raw, tp_len};
        }

        constexpr operator s_array<tp_type>() {
            return ToNonstatic();
        }

        constexpr s_array<const tp_type> ToNonstatic() const {
            return {buf_raw, tp_len};
        }

        constexpr operator s_array<const tp_type>() const {
            return ToNonstatic();
        }
    };

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
        ZF_ASSERT(!arr_to_clone.IsEmpty());

        if (!MakeArray(mem_arena, arr_to_clone.Len(), o_arr)) {
            return false;
        }

        Copy(o_arr, arr_to_clone);

        return true;
    }

    template<typename tp_type>
    constexpr s_array<tp_type> Slice(const s_array<tp_type> arr, const t_size beg, const t_size end) {
        ZF_ASSERT(beg >= 0 && beg <= arr.Len());
        ZF_ASSERT(end >= beg && end <= arr.Len());
        return {arr.Raw() + beg, end - beg};
    }

    template<typename tp_type>
    void Copy(const s_array<tp_type> dest, const s_array<const tp_type> src) {
        ZF_ASSERT(dest.Len() >= src.Len());

        for (t_size i = 0; i < src.Len(); i++) {
            dest[i] = src[i];
        }
    }

    template<typename tp_type>
    void CopyReverse(const s_array<tp_type> dest, const s_array<const tp_type> src) {
        ZF_ASSERT(dest.Len() >= src.Len());

        for (t_size i = src.Len() - 1; i >= 0; i--) {
            dest[i] = src[i];
        }
    }

    template<typename tp_type>
    t_b8 AreAllEqualTo(const s_array<const tp_type> arr, const tp_type& val, const t_comparator<tp_type> comparator = DefaultComparator) {
        ZF_ASSERT(comparator);

        for (t_size i = 0; i < arr.Len(); i++) {
            if (comparator(arr[i], val) != 0) {
                return false;
            }
        }

        return true;
    }

    template<typename tp_type>
    t_b8 AreAnyEqualTo(const s_array<const tp_type> arr, const tp_type& val, const t_comparator<tp_type> comparator = DefaultComparator) {
        ZF_ASSERT(comparator);

        for (t_size i = 0; i < arr.Len(); i++) {
            if (comparator(arr[i], val) == 0) {
                return true;
            }
        }

        return false;
    }

    template<typename tp_type>
    void SetAllTo(const s_array<tp_type> arr, const tp_type& val) {
        for (t_size i = 0; i < arr.Len(); i++) {
            arr[i] = val;
        }
    }

    template<typename tp_type>
    t_b8 BinarySearch(const s_array<const tp_type> arr, const tp_type& elem, const t_comparator<tp_type> comparator = DefaultComparator) {
        ZF_ASSERT(IsSorted(arr));

        if (arr.Len() == 0) {
            return false;
        }

        const tp_type& mid = elem[arr.Len() / 2];

        if (elem == mid) {
            return true;
        } else if (elem < mid) {
            return BinarySearch(arr.Slice(0, arr.Len() / 2), elem);
        } else {
            return BinarySearch(arr.Slice((arr.Len() / 2) + 1, arr.Len()), elem);
        }
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
