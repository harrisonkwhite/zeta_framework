#pragma once

#include <cstring>
#include <cassert>
#include <cstdlib>
#include <type_traits>
#include <new>
#include <zc/mem/arrays.h>

#define ZF_SIZE_IN_BITS(x) (8 * sizeof(x))

namespace zf {
    using t_s8 = char;
    using t_u8 = unsigned char;
    using t_s16 = short;
    using t_u16 = unsigned short;
    using t_s32 = int;
    using t_u32 = unsigned int;
    using t_s64 = long long;
    using t_u64 = unsigned long long;

    template<typename tp_type>
    static inline void ZeroOut(tp_type& item) {
        const auto item_bytes = reinterpret_cast<t_u8*>(&item);

        for (size_t i = 0; i < sizeof(item); i++) {
            item_bytes[i] = 0;
        }
    }

    template<typename tp_type>
    static constexpr tp_type Min(const tp_type& a, const tp_type& b) {
        return a <= b ? a : b;
    }

    template<typename tp_type>
    static constexpr tp_type Max(const tp_type& a, const tp_type& b) {
        return a >= b ? a : b;
    }

    constexpr size_t Kilobytes(const size_t x) { return (static_cast<size_t>(1) << 10) * x; }
    constexpr size_t Megabytes(const size_t x) { return (static_cast<size_t>(1) << 20) * x; }
    constexpr size_t Gigabytes(const size_t x) { return (static_cast<size_t>(1) << 30) * x; }
    constexpr size_t Terabytes(const size_t x) { return (static_cast<size_t>(1) << 40) * x; }

    constexpr size_t BitsToBytes(const size_t x) { return (x + 7) / 8; }
    constexpr size_t BytesToBits(const size_t x) { return x * 8; }

    constexpr bool IsPowerOfTwo(const size_t n) {
        return n > 0 && (n & (n - 1)) == 0;
    }

    constexpr bool IsAlignmentValid(const size_t n) {
        return n > 0 && IsPowerOfTwo(n);
    }

    constexpr size_t AlignForward(const size_t n, const size_t alignment) {
        return (n + alignment - 1) & ~(alignment - 1);
    }

    static inline size_t IndexFrom2D(const size_t x, const size_t y, const size_t width) {
        assert(x < width);
        return (width * y) + x;
    }

    template<typename tp_type>
    void MemCopy(const c_array<tp_type> dest, const c_array<const tp_type> src) {
        assert(dest.Len() >= src.Len());
        memcpy(dest.Raw(), src.Raw(), src.SizeInBytes());
    }

    template<typename tp_type>
    void MemCopy(const c_array<tp_type> dest, const c_array<tp_type> src) {
        MemCopy(dest, src.View());
    }

    class c_mem_arena {
    public:
        bool Init(const size_t size) {
            assert(!m_buf);

            m_buf = static_cast<t_u8*>(calloc(size, 1));

            if (!m_buf) {
                return false;
            }

            m_size = size;

            return true;
        }

        void Release() {
            assert(m_buf);

            free(m_buf);
            m_buf = nullptr;
            m_size = 0;
            m_offs = 0;
        }

        void* PushRaw(const size_t size, const size_t alignment) {
            const size_t offs_aligned = AlignForward(m_offs, alignment);
            const size_t offs_next = offs_aligned + size;

            if (offs_next > m_size) {
                return nullptr;
            }

            m_offs = offs_next;

            return m_buf + offs_aligned;
        }

        template<typename tp_type>
        tp_type* Push() {
            static_assert(std::is_trivially_destructible_v<tp_type>);

            tp_type* const ptr = static_cast<tp_type*>(PushRaw(sizeof(tp_type), alignof(tp_type)));

            if (ptr) {
                new (ptr) tp_type();
            }

            return ptr;
        }

        template<typename tp_type>
        c_array<tp_type> PushArray(const int len) {
            void* const mem = PushRaw(sizeof(tp_type) * len, alignof(tp_type));

            if (!mem) {
                return {};
            }

            tp_type* const arr_raw = reinterpret_cast<tp_type*>(mem);

            for (int i = 0; i < len; i++) {
                new (&arr_raw[i]) tp_type();
            }

            return {arr_raw, len};
        }

        size_t Size() const {
            return m_size;
        }

        size_t Offs() const {
            return m_offs;
        }

        bool IsEmpty() const {
            return m_offs == 0;
        }

        void Rewind(const size_t offs) {
            assert(offs <= m_size);
            m_offs = offs;
        }

    private:
        t_u8* m_buf = nullptr;
        size_t m_size = 0;
        size_t m_offs = 0;
    };

    template<typename tp_type>
    c_array<tp_type> MemClone(c_mem_arena& mem_arena, const c_array<const tp_type> src) {
        const c_array<tp_type> clone = mem_arena.PushArray<tp_type>(src.Len());

        if (!clone.IsEmpty()) {
            MemCopy(clone, src);
        }

        return clone;
    }

    template<typename tp_type>
    bool BinarySearch(const c_array<const tp_type> arr, const tp_type& elem) {
        assert(IsSorted(arr));

        if (arr.Len() == 0) {
            return false;
        }

        const tp_type& mid = elem[arr.Len() / 2];

        if (mid == elem) {
            return true;
        }

        return elem < mid ? BinarySearch(arr.Slice(0, arr.Len() / 2), elem) : BinarySearch(arr.Slice((arr.Len() / 2) + 1, arr.Len()), elem);
    }

    template<typename tp_type>
    void Swap(tp_type& a, tp_type& b) {
        const tp_type temp = a;
        a = b;
        b = temp;
    }
}
