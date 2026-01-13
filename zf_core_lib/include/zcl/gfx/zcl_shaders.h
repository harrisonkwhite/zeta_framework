#pragma once

#include <zcl/zcl_math.h>
#include <zcl/ds/zcl_hash_maps.h>
#include <zcl/zcl_strs.h>

namespace zcl::gfx {
    [[nodiscard]] t_b8 shader_pack(const strs::t_str_rdonly file_path, const t_array_rdonly<t_u8> compiled_shader_bin, t_arena *const temp_arena);
    [[nodiscard]] t_b8 shader_unpack(const strs::t_str_rdonly file_path, t_arena *const shader_bin_arena, t_arena *const temp_arena, t_array_mut<t_u8> *const o_shader_bin);
}
