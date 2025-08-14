#ifndef ZFWAP_H
#define ZFWAP_H

#include <zfws.h>
#include <cu.h>

bool PackTexture(const s_char_array_view file_path, const s_char_array_view output_file_path, s_mem_arena* const temp_mem_arena);
bool PackFont(const s_char_array_view file_path, const t_s32 height, const s_char_array_view output_file_path, s_mem_arena* const temp_mem_arena);
bool PackShaderProg(const s_char_array_view vert_file_path, const s_char_array_view frag_file_path, const s_char_array_view output_file_path, s_mem_arena* const temp_mem_arena);

#endif
