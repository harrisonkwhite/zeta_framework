#pragma once

#include <zcl.h>

namespace zf {
    [[nodiscard]] t_b8 PackAssets(const strs::t_str_rdonly instrs_json_file_path);
    [[nodiscard]] t_b8 compile_shader(const strs::t_str_rdonly shader_file_path, const strs::t_str_rdonly varying_def_file_path, const t_b8 is_frag, mem::t_arena *const bin_arena, mem::t_arena *const temp_arena, t_array_mut<t_u8> *const o_bin);
}
