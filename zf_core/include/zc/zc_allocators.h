#pragma once

#include <new>
#include <zc/zc_essential.h>

namespace zf {
    class c_mem_arena {
    public:
        [[nodiscard]] t_b8 Init(const t_size size);
        void Release();

        void* PushRaw(const t_size size, const t_size alignment);
        template<typename tp_type> tp_type* Push();
        template<typename tp_type> [[nodiscard]] t_b8 PushArray(const t_size cnt, c_array<tp_type>& o_arr);
        template<typename tp_type> [[nodiscard]] t_b8 CloneArray(const c_array<const tp_type> src_arr, c_array<tp_type>& o_arr);

        t_size Size() const {
            return m_size;
        }

        t_size Offs() const {
            return m_offs;
        }

        void Rewind(const t_size offs) {
            ZF_ASSERT(offs <= m_size);
            m_offs = offs;
        }

    private:
        void* m_buf = nullptr;
        t_size m_size = 0;
        t_size m_offs = 0;
    };

    template<typename tp_type>
    tp_type* c_mem_arena::Push() {
        void* const ptr_generic = PushRaw(ZF_SIZE_OF(tp_type), alignof(tp_type));

        if (!ptr_generic) {
            return nullptr;
        }

        const auto ptr = reinterpret_cast<tp_type*>(ptr_generic);
        new (ptr) tp_type();

        return ptr;
    }

    template<typename tp_type>
    t_b8 c_mem_arena::PushArray(const t_size cnt, c_array<tp_type>& o_arr) {
        void* const buf_generic = PushRaw(ZF_SIZE_OF(tp_type) * cnt, alignof(tp_type));

        if (!buf_generic) {
            return false;
        }

        const auto buf = reinterpret_cast<tp_type*>(buf_generic);

        for (t_size i = 0; i < cnt; i++) {
            new (&buf[i]) tp_type();
        }

        o_arr = {buf, cnt};

        return true;
    }

    template<typename tp_type>
    t_b8 c_mem_arena::CloneArray(const c_array<const tp_type> src_arr, c_array<tp_type>& o_arr) {
        ZF_ASSERT(!src_arr.IsEmpty());

        if (!PushArray(src_arr.Len(), o_arr)) {
            return false;
        }

        Copy(o_arr, src_arr);

        return true;
    }
}
