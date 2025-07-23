#ifndef ZFW_IO_H
#define ZFW_IO_H

#include "zfw_mem.h"

#define ZFW_FILENAME_BUF_SIZE 256

typedef char zfw_t_filename_buf[ZFW_FILENAME_BUF_SIZE];

typedef struct {
    const zfw_t_filename_buf* buf;
    int cnt;
} zfw_s_filenames;

void ZFW_Log(const char* const format, ...);
void ZFW_LogError(const char* const format, ...);

bool ZFW_DoesFilenameHaveExt(const char* const filename, const char* const ext);
zfw_t_byte* ZFW_PushEntireFileContents(const char* const file_path, zfw_s_mem_arena* const mem_arena, const bool incl_terminating_byte);
bool ZFW_LoadDirectoryFilenames(zfw_s_filenames* const filenames, zfw_s_mem_arena* const mem_arena, const char* const dir_param);

#endif
