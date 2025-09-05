#pragma once

#include <cstdint>
#include <span>

typedef int8_t t_s8;
typedef uint8_t t_u8;
typedef int16_t t_s16;
typedef uint16_t t_u16;
typedef int32_t t_s32;
typedef uint32_t t_u32;
typedef int64_t t_s64;
typedef uint64_t t_u64;

class c_mem_arena {
public:
    ~c_mem_arena() {
        assert(!m_buf && "Arena buffer not freed!");
    }

    bool Init(const size_t size);
    void Clean();
    void Rewind(const size_t offs);

    void* PushRaw(const size_t size, const size_t alignment);

    template <typename tp_type>
    tp_type* Push(size_t cnt) {
        void* mem = PushRaw(sizeof(tp_type), alignof(tp_type));

        if (mem) {
            new (mem) tp_type();
        }

        return mem;
    }

    template <typename tp_type>
    std::span<tp_type> PushArray(size_t cnt) {
        void* mem = Push(sizeof(tp_type) * cnt, alignof(T));

        if (!mem) return {};

        tp_type* arr = reinterpret_cast<tp_type*>(mem);

        for (size_t i = 0; i < cnt; ++i) {
            new (&arr[i]) tp_type();
        }

        return {arr, cnt};
    }

private:
    t_u8* m_buf = nullptr;
    size_t m_size = 0;
    size_t m_offs = 0;
};
