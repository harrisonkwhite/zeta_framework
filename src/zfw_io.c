#include "zfw_io.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#if defined(_WIN32)
#include <windows.h>
#else
#include <dirent.h>
#endif

void ZFW_Log(const char* const format, ...) {
    va_list args;
    va_start(args, format);

    printf("ZFW: ");
    vprintf(format, args);
    printf("\n");

    va_end(args);
}

void ZFW_LogError(const char* const format, ...) {
    va_list args;
    va_start(args, format);

    fprintf(stderr, "ZFW Error: ");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");

    va_end(args);
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
    assert(mem_arena && ZFW_IsMemArenaValid(mem_arena));

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

bool ZFW_LoadDirectoryFilenames(zfw_s_filenames* const filenames, zfw_s_mem_arena* const mem_arena, const char* const dir_param) {
    assert(filenames && ZFW_IS_ZERO(*filenames));
    assert(mem_arena && ZFW_IsMemArenaValid(mem_arena));
    assert(dir_param);

    const size_t mem_arena_init_offs = mem_arena->offs;

#if defined(_WIN32)
    char search_path[MAX_PATH];
    snprintf(search_path, MAX_PATH, "%s\\*", dir_param);

    WIN32_FIND_DATAA find_data;
    HANDLE find = FindFirstFileA(search_path, &find_data);

    if (find == INVALID_HANDLE_VALUE) {
        return false;
    }

    do {
        zfw_t_filename_buf* const buf = ZFW_MEM_ARENA_PUSH_TYPE(mem_arena, zfw_t_filename_buf);

        if (!buf) {
            FindClose(ha);
            ZFW_ZERO_OUT(*filenames);
            ZFW_RewindMemArena(mem_arena, mem_arena_init_offs);
            return false;
        }

        strncpy((char*)buf, find_data.cFileName, sizeof(*buf));

        if (!filenames->buf) {
            filenames->buf = buf;
        }

        filenames->cnt++;
    } while (FindNextFileA(find, &find_data));

    FindClose(find);
#else
    DIR* const dir = opendir(dir_param);

    if (!dir) {
        return false;
    }

    const struct dirent* dir_entry;

    while ((dir_entry = readdir(dir)) != NULL) {
        zfw_t_filename_buf* const buf = ZFW_MEM_ARENA_PUSH_TYPE(mem_arena, zfw_t_filename_buf);

        if (!buf) {
            closedir(dir);
            ZFW_ZERO_OUT(*filenames);
            ZFW_RewindMemArena(mem_arena, mem_arena_init_offs);
            return false;
        }

        strncpy((char*)buf, dir_entry->d_name, sizeof(*buf));

        if (!filenames->buf)
            filenames->buf = buf;

        filenames->cnt++;
    }

    closedir(dir);
#endif

    return true;
}
