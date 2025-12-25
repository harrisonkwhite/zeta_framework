#pragma once

#include <zcl.h>

namespace zf {
    [[nodiscard]] t_b8 RunPacker(const s_str_rdonly instrs_json_file_path);
    [[nodiscard]] t_b8 CompileShader(const s_str_rdonly shader_file_path, const s_str_rdonly shaderc_file_path, const t_b8 is_frag, s_mem_arena &temp_mem_arena);
}
