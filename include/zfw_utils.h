#ifndef GCE_UTILS_H
#define GCE_UTILS_H

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdalign.h>
#include <assert.h>

#define BITS_TO_BYTES(x) ((x + 7) & ~7)
#define BYTES_TO_BITS(x) (x * 8)

typedef uint8_t t_byte;

bool IsZero(const void* const mem, const int size);

static inline void ZeroOut(void* const mem, const int size) {
    assert(mem);
    assert(size > 0);

    memset(mem, 0, size);
}

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

typedef struct {
    t_byte* buf;
    int size;
    int offs;
} s_mem_arena;

static inline bool IsMemArenaValid(const s_mem_arena* const arena) {
    assert(arena);
    return IsZero(arena, sizeof(*arena))
        || (arena->buf && arena->size > 0 && arena->offs >= 0 && arena->offs <= arena->size);
}

bool InitMemArena(s_mem_arena* const arena, const int size);
void CleanMemArena(s_mem_arena* const arena);
void* PushToMemArena(s_mem_arena* const arena, const int size, const int alignment);
void ResetMemArena(s_mem_arena* const arena);
void AssertMemArenaValidity(const s_mem_arena* const arena);

#define MEM_ARENA_PUSH_TYPE(arena, type) (type*)PushToMemArena(arena, sizeof(type), alignof(type))
#define MEM_ARENA_PUSH_TYPE_MANY(arena, type, cnt) (type*)PushToMemArena(arena, sizeof(type) * (cnt), alignof(type))

int FirstActiveBitIndex(const t_byte* const bytes, const int byte_cnt); // Returns -1 if an active bit is not found.
int FirstInactiveBitIndex(const t_byte* const bytes, const int byte_cnt); // Returns -1 if an inactive bit is not found.

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

t_byte* PushEntireFileContents(const char* const file_path, s_mem_arena* const mem_arena, const bool incl_term_byte);

#endif
