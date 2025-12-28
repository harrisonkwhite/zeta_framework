#pragma once

#include <zcl.h>
#include <zgl/zgl_gfx_core.h>

namespace zf {
    void RenderTriangle(s_rendering_context &rendering_context, const s_static_array<s_v2, 3> &pts, const s_static_array<s_color_rgba32f, 3> &pt_colors);

    inline void RenderTriangle(s_rendering_context &rendering_context, const s_static_array<s_v2, 3> &pts, const s_color_rgba32f color) {
        RenderTriangle(rendering_context, pts, {{color, color, color}});
    }

    void RenderRect(s_rendering_context &rendering_context, const s_rect_f rect, const s_color_rgba32f color_topleft, const s_color_rgba32f color_topright, const s_color_rgba32f color_bottomright, const s_color_rgba32f color_bottomleft);

    inline void RenderRect(s_rendering_context &rendering_context, const s_rect_f rect, const s_color_rgba32f color) {
        RenderRect(rendering_context, rect, color, color, color, color);
    }

    void RenderTexture(s_rendering_context &rendering_context, const s_gfx_resource &texture, const s_v2 pos, const s_rect_i src_rect = {});

    struct s_font {
        s_font_arrangement arrangement = {};
        s_array<s_ptr<s_gfx_resource>> atlases = {};
    };

    [[nodiscard]] t_b8 CreateFontFromRaw(const s_str_rdonly file_path, const t_i32 height, t_code_pt_bit_vec &code_pts, s_mem_arena &temp_mem_arena, s_font &o_font, const s_ptr<s_gfx_resource_arena> resource_arena = nullptr);
    [[nodiscard]] t_b8 CreateFontFromPacked(const s_str_rdonly file_path, s_mem_arena &temp_mem_arena, s_font &o_font, const s_ptr<s_gfx_resource_arena> resource_arena = nullptr);

    s_array<s_v2> LoadStrChrDrawPositions(const s_str_rdonly str, const s_font_arrangement &font_arrangement, const s_v2 pos, const s_v2 alignment, s_mem_arena &mem_arena);

    void RenderStr(s_rendering_context &rendering_context, const s_str_rdonly str, const s_font &font, const s_v2 pos, s_mem_arena &temp_mem_arena, const s_v2 alignment = g_alignment_topleft, const s_color_rgba32f blend = g_color_white);
}
