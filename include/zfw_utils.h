#ifndef ZFW_UTILS_H
#define ZFW_UTILS_H

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

#define ZFW_BITS_TO_BYTES(x) (((x) + 7) / 8)
#define ZFW_BYTES_TO_BITS(x) (x * 8)

#define ZFW_SIZE_IN_BITS(x) ZFW_BYTES_TO_BITS(sizeof(x))

typedef uint8_t zfw_t_byte;

bool ZFWIsZero(const void* const mem, const int size);
bool ZFWIsNullTerminated(const char* const buf, const int buf_size);

static inline void ZFWZeroOut(void* const mem, const int size) {
    assert(mem);
    assert(size > 0);

    memset(mem, 0, size);
}

#define ZFW_IS_ZERO(x) ZFWIsZero(&x, sizeof(x))
#define ZFW_ZERO_OUT(x) ZFWZeroOut(&x, sizeof(x))

static inline bool ZFWIsPowerOfTwo(const int n) {
    return n > 0 && (n & (n - 1)) == 0;
}

static inline bool ZFWIsValidAlignment(const int n) {
    return n > 0 && ZFWIsPowerOfTwo(n);
}

static inline int ZFWAlignForward(const int n, const int alignment) {
    assert(n >= 0);
    assert(ZFWIsValidAlignment(alignment));
    return (n + alignment - 1) & ~(alignment - 1);
}

/*

A memory arena in this context is effectively just a single buffer that is allocated upfront of an arbitrary size. It has an offset indicating how many bytes in the arena have been used. When you "push" to the memory arena, you are effectively just increasing this offset and retrieving a pointer to the buffer but from the original offset. You cannot free any specific thing from a memory arena, you can only reset it (i.e. reset the offset) or clean it (i.e. free the allocated memory) in its entirety. Therefore, if you have a number of things with a shared lifetime, it would be reasonable to put them in the same arena, as opposed to having to manually deallocate them each (and risk forgetting to deallocate one of them).

*/

typedef struct {
    zfw_t_byte* buf;
    int size;
    int offs;
} zfw_s_mem_arena;

static inline bool ZFWIsMemArenaValid(const zfw_s_mem_arena* const arena) {
    assert(arena);
    return ZFWIsZero(arena, sizeof(*arena)) || (arena->buf && arena->size > 0 && arena->offs >= 0 && arena->offs <= arena->size);
}

bool ZFWInitMemArena(zfw_s_mem_arena* const arena, const int size);
void ZFWCleanMemArena(zfw_s_mem_arena* const arena);
void* ZFWPushToMemArena(zfw_s_mem_arena* const arena, const int size, const int alignment);
void ZFWResetMemArena(zfw_s_mem_arena* const arena);
void ZFWAssertMemArenaValidity(const zfw_s_mem_arena* const arena);

#define ZFW_MEM_ARENA_PUSH_TYPE(arena, type) (type*)ZFWPushToMemArena(arena, sizeof(type), ZFW_ALIGN_OF(type))
#define ZFW_MEM_ARENA_PUSH_TYPE_MANY(arena, type, cnt) (type*)ZFWPushToMemArena(arena, sizeof(type) * (cnt), ZFW_ALIGN_OF(type))

int ZFWFirstActiveBitIndex(const zfw_t_byte* const bytes, const int bit_cnt); // Returns -1 if an active bit is not found.
int ZFWFirstInactiveBitIndex(const zfw_t_byte* const bytes, const int bit_cnt); // Returns -1 if an inactive bit is not found.

static inline void ZFWActivateBit(const int bit_index, zfw_t_byte* const bytes, const int bit_cnt) {
    assert(bit_index >= 0 && bit_index < bit_cnt);
    assert(bytes);
    assert(bit_cnt > 0);

    bytes[bit_index / 8] |= 1 << (bit_index % 8);
}

static inline void ZFWDeactivateBit(const int bit_index, zfw_t_byte* const bytes, const int bit_cnt) {
    assert(bit_index >= 0 && bit_index < bit_cnt);
    assert(bytes);
    assert(bit_cnt > 0);

    bytes[bit_index / 8] &= ~(1 << (bit_index % 8));
}

static inline bool ZFWIsBitActive(const int bit_index, const zfw_t_byte* const bytes, const int bit_cnt) {
    assert(bit_index >= 0 && bit_index < bit_cnt);
    assert(bytes);
    assert(bit_cnt > 0);

    return bytes[bit_index / 8] & (1 << (bit_index % 8));
}

static inline zfw_t_byte ZFWKeepFirstNBitsOfByte(const zfw_t_byte byte, const int n) {
    assert(n >= 0 && n <= 8);
    return byte & ((1 << n) - 1);
}

bool ZFWDoesFilenameHaveExt(const char* const filename, const char* const ext);
zfw_t_byte* ZFWPushEntireFileContents(const char* const file_path, zfw_s_mem_arena* const mem_arena, const bool incl_term_byte);

#endif
