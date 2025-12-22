#pragma once

#include <zcl.h>
#include <zgl/zgl_gfx_core.h>

namespace zf {
    void DrawTriangle(s_rendering_context &rc, const s_static_array<s_v2, 3> &pts, const s_static_array<s_color_rgba32f, 3> &pt_colors);

    inline void DrawTriangle(s_rendering_context &rc, const s_static_array<s_v2, 3> &pts, const s_color_rgba32f color) {
        DrawTriangle(rc, pts, {color, color, color});
    }

    void DrawRect(s_rendering_context &rc, const s_rect_f rect, const s_color_rgba32f color_topleft, const s_color_rgba32f color_topright, const s_color_rgba32f color_bottomright, const s_color_rgba32f color_bottomleft);

    inline void DrawRect(s_rendering_context &rc, const s_rect_f rect, const s_color_rgba32f color) {
        DrawRect(rc, rect, color, color, color, color);
    }

    void DrawTexture(s_rendering_context &rc, const s_gfx_resource &texture, const s_v2 pos, const s_rect_i src_rect = {});

#if 0
    struct s_font {
    };

    [[nodiscard]] t_b8 CreateFontFromRaw(const s_str_rdonly file_path, const t_i32 height, t_code_pt_bit_vec &code_pts, s_mem_arena &temp_mem_arena, s_ptr<s_gfx_resource> &o_resource, const s_ptr<s_gfx_resource_arena> resource_arena = nullptr);
    [[nodiscard]] t_b8 CreateFontFromPacked(const s_str_rdonly file_path, s_mem_arena &temp_mem_arena, s_ptr<s_gfx_resource> &o_resource, const s_ptr<s_gfx_resource_arena> resource_arena = nullptr);

    s_array<s_v2> LoadStrChrDrawPositions(const s_str_rdonly str, const s_font_arrangement &font_arrangement, const s_v2 pos, const s_v2 alignment, s_mem_arena &mem_arena);

    void DrawStr(s_rendering_context &rc, const s_str_rdonly str, const s_gfx_resource &font, const s_v2 pos, s_mem_arena &temp_mem_arena, const s_v2 alignment = alignments::g_topleft, const s_color_rgba32f blend = colors::g_white);
#endif
}
