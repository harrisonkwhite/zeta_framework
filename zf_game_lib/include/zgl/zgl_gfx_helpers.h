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
}
