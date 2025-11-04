#pragma once

#include <concepts>
#include <new>
#include <type_traits>

namespace zf {
    template<typename tp_type>
    concept co_allocator = requires(tp_type a, size_t size, size_t align, void* ptr) {
        { a.Alloc(size, align) } -> std::same_as<void*>;
        { a.Free(ptr) } -> std::same_as<void>;
    };

    class c_heap_allocator {
    };

    class c_arena_allocator {
    };
}
