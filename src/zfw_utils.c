#include <stdlib.h>
#include <stdio.h>
#include "zfw_utils.h"

bool ZFW_IsZero(const void* const mem, const size_t size) {
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

bool ZFW_IsNullTerminated(const char* const buf, const size_t buf_size) {
    assert(buf);
    assert(buf_size > 0);

    for (int i = 0; i < buf_size; i++) {
        if (buf[i] == '\0') {
            return true;
        }
    }

    return false;
}

bool ZFW_InitMemArena(zfw_s_mem_arena* const arena, const size_t size) {
    assert(arena && ZFW_IS_ZERO(*arena));
    assert(size > 0);

    arena->buf = malloc(size);

    if (!arena->buf) {
        return false;
    }

    ZFW_ZeroOut(arena->buf, size);

    arena->size = size;

    return true;
}

void ZFW_CleanMemArena(zfw_s_mem_arena* const arena) {
    assert(arena && ZFW_IsMemArenaValid(arena));
    free(arena->buf);
    ZFW_ZERO_OUT(*arena);
}

void* ZFW_PushToMemArena(zfw_s_mem_arena* const arena, const size_t size, const size_t alignment) {
    assert(arena && ZFW_IsMemArenaValid(arena));
    assert(size > 0);
    assert(ZFW_IsValidAlignment(alignment));

    const size_t offs_aligned = ZFW_AlignForward(arena->offs, alignment);
    const size_t offs_next = offs_aligned + size;

    if (offs_next > arena->size) {
        fprintf(stderr, "Failed to push to memory arena!");
        return NULL;
    }

    arena->offs = offs_next;

    return arena->buf + offs_aligned;
}

void ZFW_RewindMemArena(zfw_s_mem_arena* const arena, const size_t rewind_offs) {
    assert(arena && ZFW_IsMemArenaValid(arena));
    assert(rewind_offs <= arena->offs);

    if (rewind_offs != arena->offs) {
        ZFW_ZeroOut(arena->buf + rewind_offs, arena->offs - rewind_offs);
        arena->offs = rewind_offs;
    }
}

int ZFW_FirstActiveBitIndex(const zfw_t_byte* const bytes, const int bit_cnt) {
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
        const zfw_t_byte last_byte = ZFW_KeepFirstNBitsOfByte(bytes[bit_cnt / 8], excess_bits);

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

int ZFW_FirstInactiveBitIndex(const zfw_t_byte* const bytes, const int bit_cnt) {
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
        const zfw_t_byte last_byte = ZFW_KeepFirstNBitsOfByte(bytes[bit_cnt / 8], excess_bits);

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

bool ZFW_DoesFilenameHaveExt(const char* const filename, const char* const ext) {
    assert(filename);
    assert(ext);

    const char* const ext_actual = strrchr(filename, '.');

    if (!ext_actual) {
        return false;
    }

    return strcmp(ext_actual, ext) == 0;
}

zfw_t_byte* ZFW_PushEntireFileContents(const char* const file_path, zfw_s_mem_arena* const mem_arena, const bool incl_terminating_byte) {
    assert(file_path);
    assert(mem_arena);
    assert(ZFW_IsMemArenaValid(mem_arena));

    FILE* const fs = fopen(file_path, "rb");

    if (!fs) {
        fprintf(stderr, "Failed to open \"%s\"!\n", file_path);
        return NULL;
    }

    fseek(fs, 0, SEEK_END);
    const size_t file_size = ftell(fs);
    fseek(fs, 0, SEEK_SET);

    zfw_t_byte* const contents = ZFW_MEM_ARENA_PUSH_TYPE_MANY(mem_arena, zfw_t_byte, incl_terminating_byte ? (file_size + 1) : file_size);

    if (!contents) {
        return NULL;
    }

    const size_t read_cnt = fread(contents, 1, file_size, fs);
    
    if (read_cnt != file_size) {
        fprintf(stderr, "Failed to read the contents of \"%s\"!\n", file_path);
        return NULL;
    }

    return contents;
}
