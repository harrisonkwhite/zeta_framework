#pragma once

#include <zc.h>

namespace zf {
    bool PackTextureFromRawFile(c_file_writer& fw, const c_string_view file_path, c_mem_arena& temp_mem_arena);
    bool PackFontFromRawFile(c_file_writer& fw, const c_string_view file_path, const t_s32 height, c_mem_arena& temp_mem_arena);
    bool PackShaderProgFromRawFiles(c_file_writer& fw, const c_string_view vs_file_path, const c_string_view fs_file_path, const c_string_view varying_def_file_path);
}
