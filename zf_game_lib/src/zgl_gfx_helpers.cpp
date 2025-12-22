#include <zgl/zgl_gfx_helpers.h>

namespace zf {
    void DrawTriangle(s_rendering_context &rc, const s_static_array<s_v2, 3> &pts, const s_static_array<s_color_rgba32f, 3> &pt_colors) {
        const s_batch_triangle tri = {
            .verts = {
                {.pos = pts[0], .blend = pt_colors[0], .uv = {}},
                {.pos = pts[1], .blend = pt_colors[1], .uv = {}},
                {.pos = pts[2], .blend = pt_colors[2], .uv = {}},
            },
        };

        SubmitTriangle(rc, tri, nullptr);
    }

    void DrawRect(s_rendering_context &rc, const s_rect_f rect, const s_color_rgba32f color_topleft, const s_color_rgba32f color_topright, const s_color_rgba32f color_bottomright, const s_color_rgba32f color_bottomleft) {
        const s_static_array<s_batch_triangle, 2> triangles = {
            {
                .verts = {
                    {.pos = rect.TopLeft(), .blend = color_topleft, .uv = {0.0f, 0.0f}},
                    {.pos = rect.TopRight(), .blend = color_topright, .uv = {1.0f, 0.0f}},
                    {.pos = rect.BottomRight(), .blend = color_bottomright, .uv = {1.0f, 1.0f}},
                },
            },
            {
                .verts = {
                    {.pos = rect.BottomRight(), .blend = color_bottomright, .uv = {1.0f, 1.0f}},
                    {.pos = rect.BottomLeft(), .blend = color_bottomleft, .uv = {0.0f, 1.0f}},
                    {.pos = rect.TopLeft(), .blend = color_topleft, .uv = {0.0f, 0.0f}},
                },
            },
        };

        SubmitTriangles(rc, triangles.ToNonstatic(), nullptr);
    }
}
