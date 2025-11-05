#pragma once

#include <cstring>
#include <cassert>
#include <cstdlib>
#include <new>

#define ZF_SIZE_IN_BITS(x) (8 * sizeof(x))

namespace zf {
    // @todo: This is used very inconsistently! Fix!
    using t_s8 = char;
    using t_u8 = unsigned char;
    using t_s16 = short;
    using t_u16 = unsigned short;
    using t_s32 = int;
    using t_u32 = unsigned int;
    using t_s64 = long long;
    using t_u64 = unsigned long long;
    // =============================================

    constexpr size_t Kilobytes(const size_t x) { return (static_cast<size_t>(1) << 10) * x; }
    constexpr size_t Megabytes(const size_t x) { return (static_cast<size_t>(1) << 20) * x; }
    constexpr size_t Gigabytes(const size_t x) { return (static_cast<size_t>(1) << 30) * x; }
    constexpr size_t Terabytes(const size_t x) { return (static_cast<size_t>(1) << 40) * x; }

    constexpr size_t BitsToBytes(const size_t x) { return (x + 7) / 8; }
    constexpr size_t BytesToBits(const size_t x) { return x * 8; }

    constexpr t_u8 BitMask(const size_t index) {
        return 1 << (index % 8);
    }

    constexpr bool IsPowerOfTwo(const size_t n) {
        return n > 0 && (n & (n - 1)) == 0;
    }

    constexpr bool IsAlignmentValid(const size_t n) {
        return n > 0 && IsPowerOfTwo(n);
    }

    constexpr size_t AlignForward(const size_t n, const size_t alignment) {
        return (n + alignment - 1) & ~(alignment - 1);
    }

    class c_mem_arena {
    public:
        [[nodiscard]] bool Init(const size_t size);
        void Release();
        void* Push(const size_t size, const size_t alignment);
        template<typename tp_type> tp_type* PushType(const int cnt = 1);

        bool IsInitted() const {
            return m_buf;
        }

        size_t Size() const {
            assert(IsInitted());
            return m_size;
        }

        size_t Offs() const {
            assert(IsInitted());
            return m_offs;
        }

        bool IsEmpty() const {
            assert(IsInitted());
            return m_offs == 0;
        }

        void Rewind(const size_t offs) {
            assert(IsInitted());
            assert(offs <= m_size);
            m_offs = offs;
        }

    private:
        t_u8* m_buf = nullptr;
        size_t m_size = 0;
        size_t m_offs = 0;
    };

    template<typename tp_type>
    tp_type* c_mem_arena::PushType(const int cnt) {
        assert(m_buf);
        assert(cnt > 0);

        void* const buf_generic = Push(sizeof(tp_type) * cnt, alignof(tp_type));

        if (!buf_generic) {
            return nullptr;
        }

        tp_type* const buf = reinterpret_cast<tp_type*>(buf_generic);

        for (int i = 0; i < cnt; i++) {
            new (&buf[i]) tp_type();
        }

        return buf;
    }

    template<typename tp_type>
    void Swap(tp_type& a, tp_type& b) {
        const tp_type temp = a;
        a = b;
        b = temp;
    }

    static inline size_t IndexFrom2D(const size_t x, const size_t y, const size_t width) {
        assert(x < width);
        return (width * y) + x;
    }

    static inline t_u8 BitRangeMask(const size_t begin_index, const size_t end_index = 8) {
        assert(end_index <= 8);
        assert(begin_index <= end_index);

        const size_t range_len = end_index - begin_index;
        const t_u8 mask_at_bottom = (1 << range_len) - 1;
        return mask_at_bottom << begin_index;
    }
}
