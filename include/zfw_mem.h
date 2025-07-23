#ifndef ZFW_MEM_H
#define ZFW_MEM_H

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#ifdef _MSC_VER
#define ZFW_ALIGN_OF(x) __alignof(x)
#else
#include <stdalign.h>
#define ZFW_ALIGN_OF(x) alignof(x)
#endif

#define ZFW_STATIC_ARRAY_LEN(x) (sizeof(x) / sizeof((x)[0]))

#define ZFW_CHECK_STATIC_ARRAY_LEN(a, l) static_assert(ZFW_STATIC_ARRAY_LEN(a) == (l), "Invalid array length!")

#define ZFW_KILOBYTES(x) (((size_t)1 << 10) * (x))
#define ZFW_MEGABYTES(x) (((size_t)1 << 20) * (x))
#define ZFW_GIGABYTES(x) (((size_t)1 << 30) * (x))

#define ZFW_BITS_TO_BYTES(x) (((x) + 7) / 8)
#define ZFW_BYTES_TO_BITS(x) ((x) * 8)

#define ZFW_SIZE_IN_BITS(x) ZFW_BYTES_TO_BITS(sizeof(x))

typedef uint8_t zfw_t_byte;

bool ZFW_IsZero(const void* const mem, const size_t size);
bool ZFW_IsNullTerminated(const char* const buf, const size_t buf_size);

static inline void ZFW_ZeroOut(void* const mem, const size_t size) {
    assert(mem);
    assert(size > 0);

    memset(mem, 0, size);
}

#define ZFW_IS_ZERO(x) ZFW_IsZero(&x, sizeof(x))
#define ZFW_ZERO_OUT(x) ZFW_ZeroOut(&x, sizeof(x))

static inline bool ZFW_IsPowerOfTwo(const size_t n) {
    return n > 0 && (n & (n - 1)) == 0;
}

static inline bool ZFW_IsValidAlignment(const size_t n) {
    return n > 0 && ZFW_IsPowerOfTwo(n);
}

static inline size_t ZFW_AlignForward(const size_t n, const size_t alignment) {
    assert(ZFW_IsValidAlignment(alignment));
    return (n + alignment - 1) & ~(alignment - 1);
}

/*

A memory arena in this context is effectively just a single buffer that is allocated upfront of an arbitrary size. It has an offset indicating how many bytes in the arena have been used. When you "push" to the memory arena, you are effectively just increasing this offset and retrieving a pointer to the buffer but from the original offset. You cannot "free" any specific thing from a memory arena unless it is at the top, and you do it via a rewind (i.e. decrease the offset so that you can subsequently overwrite the data there). You can alternatively (actually) free the entire buffer at once; if you have a number of things with a shared lifetime, it would be reasonable to put them in the same arena, as opposed to having to manually deallocate them each (and risk forgetting to deallocate one of them).

*/

typedef struct {
    zfw_t_byte* buf;
    size_t size;
    size_t offs;
} zfw_s_mem_arena;

static inline bool ZFW_IsMemArenaValid(const zfw_s_mem_arena* const arena) {
    return arena->buf && arena->size > 0 && arena->offs <= arena->size;
}

bool ZFW_InitMemArena(zfw_s_mem_arena* const arena, const size_t size);
void ZFW_CleanMemArena(zfw_s_mem_arena* const arena);
void* ZFW_PushToMemArena(zfw_s_mem_arena* const arena, const size_t size, const size_t alignment);
void ZFW_RewindMemArena(zfw_s_mem_arena* const arena, const size_t rewind_offs);

#define ZFW_MEM_ARENA_PUSH_TYPE(arena, type) (type*)ZFW_PushToMemArena(arena, sizeof(type), ZFW_ALIGN_OF(type))
#define ZFW_MEM_ARENA_PUSH_TYPE_MANY(arena, type, cnt) (type*)ZFW_PushToMemArena(arena, sizeof(type) * (cnt), ZFW_ALIGN_OF(type))

int ZFW_FirstActiveBitIndex(const zfw_t_byte* const bytes, const int bit_cnt); // Returns -1 if an active bit is not found.
int ZFW_FirstInactiveBitIndex(const zfw_t_byte* const bytes, const int bit_cnt); // Returns -1 if an inactive bit is not found.

static inline void ZFW_ActivateBit(const size_t bit_index, zfw_t_byte* const bytes, const size_t bit_cnt) {
    assert(bit_index < bit_cnt);
    assert(bytes);
    assert(bit_cnt > 0);

    bytes[bit_index / 8] |= 1 << (bit_index % 8);
}

static inline void ZFW_DeactivateBit(const size_t bit_index, zfw_t_byte* const bytes, const size_t bit_cnt) {
    assert(bit_index < bit_cnt);
    assert(bytes);
    assert(bit_cnt > 0);

    bytes[bit_index / 8] &= ~(1 << (bit_index % 8));
}

static inline bool ZFW_IsBitActive(const size_t bit_index, const zfw_t_byte* const bytes, const size_t bit_cnt) {
    assert(bit_index < bit_cnt);
    assert(bytes);
    assert(bit_cnt > 0);

    return bytes[bit_index / 8] & (1 << (bit_index % 8));
}

static inline zfw_t_byte ZFW_KeepFirstNBitsOfByte(const zfw_t_byte byte, const size_t n) {
    assert(n <= 8);
    return byte & ((1 << n) - 1);
}

#endif
