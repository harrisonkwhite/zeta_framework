#pragma once

#include <new>
#include <zc/debug.h>

#define ZF_SIZE_OF(x) static_cast<zf::t_size>(sizeof(x))
#define ZF_SIZE_IN_BITS(x) (8 * ZF_SIZE_OF(x))

namespace zf {
    constexpr t_size Kilobytes(const t_size x) { return (static_cast<t_size>(1) << 10) * x; }
    constexpr t_size Megabytes(const t_size x) { return (static_cast<t_size>(1) << 20) * x; }
    constexpr t_size Gigabytes(const t_size x) { return (static_cast<t_size>(1) << 30) * x; }
    constexpr t_size Terabytes(const t_size x) { return (static_cast<t_size>(1) << 40) * x; }

    constexpr t_size BitsToBytes(const t_size x) { return (x + 7) / 8; }
    constexpr t_size BytesToBits(const t_size x) { return x * 8; }

    constexpr t_u8 BitMask(const t_size index) {
        ZF_ASSERT(index >= 0 && index < 8);
        return static_cast<t_u8>(1 << (index % 8));
    }

    constexpr t_u8 BitRangeMask(const t_size begin_index, const t_size end_index = 8) {
        ZF_ASSERT(end_index >= 0 && end_index <= 8);
        ZF_ASSERT(begin_index >= 0 && begin_index <= end_index);

        const t_size range_len = end_index - begin_index;
        const t_u8 mask_at_bottom = static_cast<t_u8>((1 << range_len) - 1);
        return static_cast<t_u8>(mask_at_bottom << begin_index);
    }

    constexpr t_b8 IsPowerOfTwo(const t_size n) {
        return n > 0 && (n & (n - 1)) == 0;
    }

    constexpr t_b8 IsAlignmentValid(const t_size n) {
        return IsPowerOfTwo(n);
    }

    constexpr t_size AlignForward(const t_size n, const t_size alignment) {
        ZF_ASSERT(n >= 0);
        ZF_ASSERT(IsAlignmentValid(alignment));

        return (n + alignment - 1) & ~(alignment - 1);
    }

    constexpr t_size IndexFrom2D(const t_size x, const t_size y, const t_size width) {
        ZF_ASSERT(x >= 0 && x < width);
        ZF_ASSERT(y >= 0);
        ZF_ASSERT(width > 0);

        return (width * y) + x;
    }

    class c_mem_arena {
    public:
        [[nodiscard]] t_b8 Init(const t_size size);
        void Release();
        void* Push(const t_size size, const t_size alignment);
        template<typename tp_type> tp_type* PushType(const t_size cnt = 1);

        t_b8 IsInitted() const {
            return m_buf;
        }

        t_size Size() const {
            ZF_ASSERT(IsInitted());
            return m_size;
        }

        t_size Offs() const {
            ZF_ASSERT(IsInitted());
            return m_offs;
        }

        t_b8 IsEmpty() const {
            ZF_ASSERT(IsInitted());
            return m_offs == 0;
        }

        void Rewind(const t_size offs) {
            ZF_ASSERT(IsInitted());
            ZF_ASSERT(offs <= m_size);
            m_offs = offs;
        }

    private:
        void* m_buf = nullptr;
        t_size m_size = 0;
        t_size m_offs = 0;
    };

    template<typename tp_type>
    tp_type* c_mem_arena::PushType(const t_size cnt) {
        ZF_ASSERT(m_buf);
        ZF_ASSERT(cnt > 0);

        void* const buf_generic = Push(ZF_SIZE_OF(tp_type) * cnt, alignof(tp_type));

        if (!buf_generic) {
            return nullptr;
        }

        tp_type* const buf = reinterpret_cast<tp_type*>(buf_generic);

        for (t_size i = 0; i < cnt; i++) {
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
}
