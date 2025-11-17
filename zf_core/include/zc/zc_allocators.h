#pragma once

#include <new>
#include <zc/zc_debug.h>

namespace zf {
    struct s_mem_arena {
        [[nodiscard]] t_b8 Init(const t_size size);
        void Release();

        void* PushRaw(const t_size size, const t_size alignment);
        template<typename tp_type> tp_type* Push(const t_size cnt = 1);

        t_size Size() const {
            return size;
        }

        t_size Offs() const {
            return offs;
        }

        void Rewind(const t_size offs) {
            ZF_ASSERT(offs <= size);
            this->offs = offs;
        }

    private:
        void* buf = nullptr;
        t_size size = 0;
        t_size offs = 0;
    };

    template<typename tp_type>
    tp_type* s_mem_arena::Push(const t_size cnt) {
        ZF_ASSERT(buf);
        ZF_ASSERT(cnt >= 1);

        void* const buf_generic = PushRaw(ZF_SIZE_OF(tp_type) * cnt, alignof(tp_type));

        if (!buf_generic) {
            return nullptr;
        }

        const auto buf = static_cast<tp_type*>(buf_generic);

        for (t_size i = 0; i < cnt; i++) {
            new (&buf[i]) tp_type();
        }

        return buf;
    }
}
