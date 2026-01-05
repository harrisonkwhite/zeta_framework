#pragma once

#include <zcl.h>

namespace zf {
    [[nodiscard]] t_b8 PackAssets(const strs::StrRdonly instrs_json_file_path);
    [[nodiscard]] t_b8 compile_shader(const strs::StrRdonly shader_file_path, const strs::StrRdonly varying_def_file_path, const t_b8 is_frag, t_arena *const bin_arena, t_arena *const temp_arena, t_array_mut<t_u8> *const o_bin);
}
