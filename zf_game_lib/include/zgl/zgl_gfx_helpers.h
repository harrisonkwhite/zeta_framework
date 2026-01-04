#pragma once

#include <zcl.h>
#include <zgl/zgl_gfx_core.h>

namespace zf {
    void RenderTexture(s_rendering_context *const rc, const s_gfx_resource *const texture, const s_v2 pos, const s_rect_i src_rect = {});

    struct s_font {
        s_font_arrangement arrangement;
        s_array_mut<s_gfx_resource *> atlases;
    };

    s_font CreateFontFromRaw(const s_str_rdonly file_path, const t_i32 height, t_code_pt_bit_vec *const code_pts, s_arena *const temp_arena, zf_rendering_resource_group *const resource_group);
    s_font CreateFontFromPacked(const s_str_rdonly file_path, s_arena *const temp_arena, zf_rendering_resource_group *const resource_group);

    s_array_mut<s_v2> CalcStrChrRenderPositions(const s_str_rdonly str, const s_font_arrangement &font_arrangement, const s_v2 pos, const s_v2 alignment, s_arena *const arena);

    void RenderStr(s_rendering_context *const rc, const s_str_rdonly str, const s_font &font, const s_v2 pos, s_arena *const temp_arena, const s_v2 alignment = g_str_alignment_topleft, const s_color_rgba32f blend = g_color_white);
}
