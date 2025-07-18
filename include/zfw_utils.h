#ifndef ZFW_UTILS_H
#define ZFW_UTILS_H

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#ifdef _MSC_VER
#define ALIGN_OF(x) __alignof(x)
#else
#include <stdalign.h>
#define ALIGN_OF(x) alignof(x)
#endif

#define STATIC_ARRAY_LEN(x) (sizeof(x) / sizeof((x)[0]))

#define BITS_TO_BYTES(x) ((x + 7) & ~7)
#define BYTES_TO_BITS(x) (x * 8)

typedef uint8_t t_byte;

bool IsZero(const void* const mem, const int size);
bool IsNullTerminated(const char* const buf, const int buf_size);

static inline void ZeroOut(void* const mem, const int size) {
    assert(mem);
    assert(size > 0);

    memset(mem, 0, size);
}

#define IS_ZERO(x) IsZero(&x, sizeof(x))
#define ZERO_OUT(x) ZeroOut(&x, sizeof(x))

static inline bool IsPowerOfTwo(const int n) {
    return n > 0 && (n & (n - 1)) == 0;
}

static inline bool IsValidAlignment(const int n) {
    return n > 0 && IsPowerOfTwo(n);
}

static inline int AlignForward(const int n, const int alignment) {
    assert(n >= 0);
    assert(IsValidAlignment(alignment));
    return (n + alignment - 1) & ~(alignment - 1);
}

/*

A memory arena in this context is effectively just a single buffer that is allocated upfront of an arbitrary size. It has an offset indicating how many bytes in the arena have been used. When you "push" to the memory arena, you are effectively just increasing this offset and retrieving a pointer to the buffer but from the original offset. You cannot free any specific thing from a memory arena, you can only reset it (i.e. reset the offset) or clean it (i.e. free the allocated memory) in its entirety. Therefore, if you have a number of things with a shared lifetime, it would be reasonable to put them in the same arena, as opposed to having to manually deallocate them each (and risk forgetting to deallocate one of them).

*/

typedef struct {
    t_byte* buf;
    int size;
    int offs;
} s_mem_arena;

static inline bool IsMemArenaValid(const s_mem_arena* const arena) {
    assert(arena);
    return IsZero(arena, sizeof(*arena)) || (arena->buf && arena->size > 0 && arena->offs >= 0 && arena->offs <= arena->size);
}

bool InitMemArena(s_mem_arena* const arena, const int size);
void CleanMemArena(s_mem_arena* const arena);
void* PushToMemArena(s_mem_arena* const arena, const int size, const int alignment);
void ResetMemArena(s_mem_arena* const arena);
void AssertMemArenaValidity(const s_mem_arena* const arena);

#define MEM_ARENA_PUSH_TYPE(arena, type) (type*)PushToMemArena(arena, sizeof(type), ALIGN_OF(type))
#define MEM_ARENA_PUSH_TYPE_MANY(arena, type, cnt) (type*)PushToMemArena(arena, sizeof(type) * (cnt), ALIGN_OF(type))

int FirstActiveBitIndex(const t_byte* const bytes, const int bit_cnt); // Returns -1 if an active bit is not found.
int FirstInactiveBitIndex(const t_byte* const bytes, const int bit_cnt); // Returns -1 if an inactive bit is not found.

static inline void ActivateBit(const int bit_index, t_byte* const bytes, const int bit_cnt) {
    assert(bit_index >= 0 && bit_index < bit_cnt);
    assert(bytes);
    assert(bit_cnt > 0);

    bytes[bit_index / 8] |= 1 << (bit_index % 8);
}

static inline void DeactivateBit(const int bit_index, t_byte* const bytes, const int bit_cnt) {
    assert(bit_index >= 0 && bit_index < bit_cnt);
    assert(bytes);
    assert(bit_cnt > 0);

    bytes[bit_index / 8] &= ~(1 << (bit_index % 8));
}

static inline bool IsBitActive(const int bit_index, const t_byte* const bytes, const int bit_cnt) {
    assert(bit_index >= 0 && bit_index < bit_cnt);
    assert(bytes);
    assert(bit_cnt > 0);

    return bytes[bit_index / 8] & (1 << (bit_index % 8));
}

static inline t_byte KeepFirstNBitsOfByte(const t_byte byte, const int n) {
    assert(n >= 0 && n <= 8);
    return byte & ((1 << n) - 1);
}

bool DoesFilenameHaveExt(const char* const filename, const char* const ext);
t_byte* PushEntireFileContents(const char* const file_path, s_mem_arena* const mem_arena, const bool incl_term_byte);

#endif
