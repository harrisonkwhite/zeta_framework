#include <stdlib.h>
#include <stdio.h>
#include "zfw_utils.h"

bool ZFWIsZero(const void* const mem, const int size) {
    assert(mem);
    assert(size > 0);

    const zfw_t_byte* const mem_bytes = mem;

    for (int i = 0; i < size; i++) {
        if (mem_bytes[i]) {
            return false;
        }
    }

    return true;
}

bool ZFWIsNullTerminated(const char* const buf, const int buf_size) {
    for (int i = 0; i < buf_size; i++) {
        if (buf[i] == '\0') {
            return true;
        }
    }

    return false;
}

bool ZFWInitMemArena(zfw_s_mem_arena* const arena, const int size) {
    assert(arena);
    assert(ZFW_IS_ZERO(*arena));
    assert(size > 0);

    arena->buf = malloc(size);

    if (!arena->buf) {
        return false;
    }

    ZFWZeroOut(arena->buf, size);

    arena->size = size;

    return true;
}

void ZFWCleanMemArena(zfw_s_mem_arena* const arena) {
    assert(arena);
    ZFWAssertMemArenaValidity(arena);

    if (arena->buf) {
        free(arena->buf);
    }

    ZFW_ZERO_OUT(*arena);
}

void* ZFWPushToMemArena(zfw_s_mem_arena* const arena, const int size, const int alignment) {
    assert(arena);
    assert(ZFWIsMemArenaValid(arena));
    assert(size > 0);
    assert(ZFWIsValidAlignment(alignment));

    const int offs_aligned = ZFWAlignForward(arena->offs, alignment);
    const int offs_next = offs_aligned + size;

    if (offs_next > arena->size) {
        fprintf(stderr, "Failed to push to memory arena!");
        return NULL;
    }

    arena->offs = offs_next;

    return arena->buf + offs_aligned;
}

void ZFWResetMemArena(zfw_s_mem_arena* const arena) {
    assert(arena);
    ZFWAssertMemArenaValidity(arena);
    assert(!ZFW_IS_ZERO(*arena));

    if (arena->offs > 0) {
        ZFWZeroOut(arena->buf, arena->offs);
        arena->offs = 0;
    }
}

void ZFWAssertMemArenaValidity(const zfw_s_mem_arena* const arena) {
    assert(arena);

    if (!ZFW_IS_ZERO(*arena)) {
        assert(arena->buf);
        assert(arena->size > 0);
        assert(arena->offs >= 0 && arena->offs <= arena->size);
    }
}

int ZFWFirstActiveBitIndex(const zfw_t_byte* const bytes, const int bit_cnt) {
    assert(bytes);
    assert(bit_cnt > 0);

    for (int i = 0; i < (bit_cnt / 8); i++) {
        if (bytes[i] == 0x00) {
            continue;
        }

        for (int j = 0; j < 8; j++) {
            if (bytes[i] & (1 << j)) {
                return (i * 8) + j;
            }
        }
    }

    const int excess_bits = bit_cnt % 8;

    if (excess_bits > 0) {
        // Get the last byte, masking out any bits we don't care about.
        const zfw_t_byte last_byte = ZFWKeepFirstNBitsOfByte(bytes[bit_cnt / 8], excess_bits);

        if (last_byte != 0x00) {
            for (int i = 0; i < 8; i++) {
                if (last_byte & (1 << i)) {
                    return bit_cnt - excess_bits + i;
                }
            }
        }
    }
    
    return -1;
}

int ZFWFirstInactiveBitIndex(const zfw_t_byte* const bytes, const int bit_cnt) {
    assert(bytes);
    assert(bit_cnt > 0);

    for (int i = 0; i < (bit_cnt / 8); i++) {
        if (bytes[i] == 0xFF) {
            continue;
        }

        for (int j = 0; j < 8; j++) {
            if (!(bytes[i] & (1 << j))) {
                return (i * 8) + j;
            }
        }
    }

    const int excess_bits = bit_cnt % 8;

    if (excess_bits > 0) {
        // Get the last byte, masking out any bits we don't care about.
        const zfw_t_byte last_byte = ZFWKeepFirstNBitsOfByte(bytes[bit_cnt / 8], excess_bits);

        if (last_byte != 0xFF) {
            for (int i = 0; i < 8; i++) {
                if (!(last_byte & (1 << i))) {
                    return bit_cnt - excess_bits + i;
                }
            }
        }
    }
    
    return -1;
}

bool ZFWDoesFilenameHaveExt(const char* const filename, const char* const ext) {
    assert(filename);
    assert(ext);

    const char* const ext_actual = strrchr(filename, '.');

    if (!ext_actual) {
        return false;
    }

    return strcmp(ext_actual, ext) == 0;
}

zfw_t_byte* ZFWPushEntireFileContents(const char* const file_path, zfw_s_mem_arena* const mem_arena, const bool incl_term_byte) {
    assert(file_path);
    assert(mem_arena);
    assert(ZFWIsMemArenaValid(mem_arena));

    FILE* const fs = fopen(file_path, "rb");

    if (!fs) {
        fprintf(stderr, "Failed to open \"%s\"!\n", file_path);
        return NULL;
    }

    fseek(fs, 0, SEEK_END);
    const int file_size = ftell(fs);
    fseek(fs, 0, SEEK_SET);

    zfw_t_byte* const contents = ZFW_MEM_ARENA_PUSH_TYPE_MANY(mem_arena, zfw_t_byte, incl_term_byte ? (file_size + 1) : file_size);

    if (!contents) {
        return NULL;
    }

    const int read_cnt = fread(contents, 1, file_size, fs);
    
    if (read_cnt != file_size) {
        fprintf(stderr, "Failed to read the contents of \"%s\"!\n", file_path);
        return NULL;
    }

    return contents;
}
