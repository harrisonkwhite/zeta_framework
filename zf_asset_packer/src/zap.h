#pragma once

#include <zcl.h>

namespace zf {
    [[nodiscard]] B8 PackAssets(const s_str_rdonly instrs_json_file_path);
    [[nodiscard]] B8 CompileShader(const s_str_rdonly shader_file_path, const s_str_rdonly varying_def_file_path, const B8 is_frag, s_arena *const bin_arena, s_arena *const temp_arena, s_array_mut<t_u8> *const o_bin);
}
