#pragma once

#include <zcl.h>

namespace zf {
    [[nodiscard]] B8 PackAssets(const strs::StrRdonly instrs_json_file_path);
    [[nodiscard]] B8 CompileShader(const strs::StrRdonly shader_file_path, const strs::StrRdonly varying_def_file_path, const B8 is_frag, s_arena *const bin_arena, s_arena *const temp_arena, s_array_mut<U8> *const o_bin);
}
