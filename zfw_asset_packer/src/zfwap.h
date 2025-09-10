#pragma once

#include <zfws.h>
#include <cu.h>
#include <cstdio>

bool PackTexture(const c_string_view file_path, const c_string_view output_file_path, c_mem_arena& temp_mem_arena);
bool PackFont(const c_string_view file_path, const t_s32 height, const c_string_view output_file_path, c_mem_arena& temp_mem_arena);
bool PackShaderProg(const c_string_view vert_file_path, const c_string_view frag_file_path, const c_string_view output_file_path, c_mem_arena& temp_mem_arena);
