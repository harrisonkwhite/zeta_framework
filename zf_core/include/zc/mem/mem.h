#pragma once

#include <new>
#include <zc/debug.h>

#define ZF_SIZE_IN_BITS(x) (8 * sizeof(x))

namespace zf {
    constexpr t_u64 Kilobytes(const t_u64 x) { return (static_cast<t_u64>(1) << 10) * x; }
    constexpr t_u64 Megabytes(const t_u64 x) { return (static_cast<t_u64>(1) << 20) * x; }
    constexpr t_u64 Gigabytes(const t_u64 x) { return (static_cast<t_u64>(1) << 30) * x; }
    constexpr t_u64 Terabytes(const t_u64 x) { return (static_cast<t_u64>(1) << 40) * x; }

    constexpr t_u64 BitsToBytes(const t_u64 x) { return (x + 7) / 8; }
    constexpr t_u64 BytesToBits(const t_u64 x) { return x * 8; }

    constexpr t_u8 BitMask(const t_u64 index) {
        return static_cast<t_u8>(1 << (index % 8));
    }

    constexpr t_b8 IsPowerOfTwo(const t_u64 n) {
        return n > 0 && (n & (n - 1)) == 0;
    }

    constexpr t_b8 IsAlignmentValid(const t_u64 n) {
        return n > 0 && IsPowerOfTwo(n);
    }

    constexpr t_u64 AlignForward(const t_u64 n, const t_u64 alignment) {
        return (n + alignment - 1) & ~(alignment - 1);
    }

    class c_mem_arena {
    public:
        [[nodiscard]] t_b8 Init(const t_u64 size);
        void Release();
        void* Push(const t_u64 size, const t_u64 alignment);
        template<typename tp_type> tp_type* PushType(const t_s32 cnt = 1);

        t_b8 IsInitted() const {
            return m_buf;
        }

        t_u64 Size() const {
            ZF_ASSERT(IsInitted());
            return m_size;
        }

        t_u64 Offs() const {
            ZF_ASSERT(IsInitted());
            return m_offs;
        }

        t_b8 IsEmpty() const {
            ZF_ASSERT(IsInitted());
            return m_offs == 0;
        }

        void Rewind(const t_u64 offs) {
            ZF_ASSERT(IsInitted());
            ZF_ASSERT(offs <= m_size);
            m_offs = offs;
        }

    private:
        t_u8* m_buf = nullptr;
        t_u64 m_size = 0;
        t_u64 m_offs = 0;
    };

    template<typename tp_type>
    tp_type* c_mem_arena::PushType(const t_s32 cnt) {
        ZF_ASSERT(m_buf);
        ZF_ASSERT(cnt > 0);

        void* const buf_generic = Push(sizeof(tp_type) * cnt, alignof(tp_type));

        if (!buf_generic) {
            return nullptr;
        }

        tp_type* const buf = reinterpret_cast<tp_type*>(buf_generic);

        for (t_s32 i = 0; i < cnt; i++) {
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

    inline t_u64 IndexFrom2D(const t_u64 x, const t_u64 y, const t_u64 width) {
        ZF_ASSERT(x < width);
        return (width * y) + x;
    }

    inline t_u8 BitRangeMask(const t_u64 begin_index, const t_u64 end_index = 8) {
        ZF_ASSERT(end_index <= 8);
        ZF_ASSERT(begin_index <= end_index);

        const t_u64 range_len = end_index - begin_index;
        const t_u8 mask_at_bottom = (1 << range_len) - 1;
        return mask_at_bottom << begin_index;
    }
}
