#pragma once

#include <zcl.h>

namespace zf {
    [[nodiscard]] t_b8 PackAssets(const s_str_rdonly instrs_json_file_path);
    [[nodiscard]] t_b8 CompileShader(const s_str_rdonly shader_file_path, const s_str_rdonly varying_def_file_path, const t_b8 is_frag, s_mem_arena &bin_mem_arena, s_mem_arena &temp_mem_arena, s_array_mut<t_u8> &o_bin);
}
