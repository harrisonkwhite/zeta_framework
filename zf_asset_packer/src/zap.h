#pragma once

#include <zcl.h>

[[nodiscard]] zcl::t_b8 PackAssets(const zcl::t_str_rdonly instrs_json_file_path);
[[nodiscard]] zcl::t_b8 CompileShader(const zcl::t_str_rdonly shader_file_path, const zcl::t_str_rdonly varying_def_file_path, const zcl::t_b8 is_frag, zcl::t_arena *const bin_arena, zcl::t_arena *const temp_arena, zcl::t_array_mut<zcl::t_u8> *const o_bin);
