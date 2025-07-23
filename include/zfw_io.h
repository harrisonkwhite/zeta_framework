#ifndef ZFW_IO_H
#define ZFW_IO_H

#include "zfw_mem.h"

bool ZFW_DoesFilenameHaveExt(const char* const filename, const char* const ext);
zfw_t_byte* ZFW_PushEntireFileContents(const char* const file_path, zfw_s_mem_arena* const mem_arena, const bool incl_terminating_byte);

#endif
