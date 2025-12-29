#pragma once

#include <zcl.h>
#include <zgl/zgl_gfx_core.h>

namespace zf {
    void RenderTexture(s_rendering_context &rendering_context, const s_gfx_resource &texture, const s_v2 pos, const s_rect_i src_rect = {});

    struct s_font {
        s_font_arrangement arrangement = {};
        c_array_mut<s_ptr<s_gfx_resource>> atlases = {};
    };

    s_font CreateFontFromRaw(const s_str_rdonly file_path, const t_i32 height, t_code_pt_bit_vec &code_pts, c_mem_arena &temp_mem_arena, s_gfx_resource_arena &resource_arena = PermGFXResourceArena());
    s_font CreateFontFromPacked(const s_str_rdonly file_path, c_mem_arena &temp_mem_arena, s_gfx_resource_arena &resource_arena = PermGFXResourceArena());

    c_array_mut<s_v2> CalcStrChrRenderPositions(const s_str_rdonly str, const s_font_arrangement &font_arrangement, const s_v2 pos, const s_v2 alignment, c_mem_arena &mem_arena);

    void RenderStr(s_rendering_context &rendering_context, const s_str_rdonly str, const s_font &font, const s_v2 pos, c_mem_arena &temp_mem_arena, const s_v2 alignment = g_alignment_topleft, const s_color_rgba32f blend = g_color_white);
}
