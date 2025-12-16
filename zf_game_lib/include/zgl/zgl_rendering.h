#pragma once

#include <zgl/zgl_gfx.h>

namespace zf {
    struct s_rendering_basis {
        s_gfx_resource_arena gfx_res_arena;
        s_ptr<s_gfx_resource> batch_mesh;
        s_ptr<s_gfx_resource> batch_shader_prog;
        s_ptr<s_gfx_resource> white_px_texture;
    };

    s_rendering_basis CreateRenderingBasis(s_mem_arena &mem_arena, s_mem_arena &temp_mem_arena);

    struct s_rendering_state;

    void DrawClear(s_rendering_state &rs, const s_color_rgb24f col);

    void DrawTriangle(s_rendering_state &rs, const s_static_array<s_v2, 3> &pts, const s_static_array<s_color_rgba32f, 3> &pt_colors);

    inline void DrawTriangle(s_rendering_state &rs, const s_static_array<s_v2, 3> &pts, const s_color_rgba32f color) {
        DrawTriangle(rs, pts, {color, color, color});
    }

    void DrawRect(s_rendering_state &rs, const s_rect_f rect, const s_color_rgba32f color_topleft, const s_color_rgba32f color_topright, const s_color_rgba32f color_bottomright, const s_color_rgba32f color_bottomleft);

    inline void DrawRect(s_rendering_state &rs, const s_rect_f rect, const s_color_rgba32f color) {
        DrawRect(rs, rect, color, color, color, color);
    }

    namespace internal {
        s_rendering_state &BeginRendering(const s_rendering_basis &basis, s_mem_arena &mem_arena);
        void EndRendering(s_rendering_state &rs, s_mem_arena &temp_mem_arena);
    }
}
