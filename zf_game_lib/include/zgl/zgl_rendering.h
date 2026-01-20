#pragma once

#include <zcl.h>
#include <zgl/zgl_gfx.h>

namespace zgl {
    struct t_rendering_basis;

    t_rendering_basis *RenderingBasisCreate(t_gfx *const gfx);

    void RenderingBasisDestroy(t_rendering_basis *const basis);

    struct t_rendering_state;

    t_rendering_state *RendererBegin(const t_rendering_basis *const rendering_basis, t_gfx *const gfx, zcl::t_arena *const rendering_state_arena);

    void RendererEnd(t_rendering_state *const rs);

    void RendererPassBegin(t_rendering_state *const rs, const zcl::t_v2_i size, const zcl::t_mat4x4 &view_mat = zcl::MatrixCreateIdentity(), const zcl::t_b8 clear = false, const zcl::t_color_rgba32f clear_col = zcl::k_color_black);

    void RendererPassBeginOffscreen(t_rendering_state *const rs, const t_gfx_resource *const texture_target, const zcl::t_mat4x4 &view_mat = zcl::MatrixCreateIdentity(), const zcl::t_b8 clear = false, const zcl::t_color_rgba32f clear_col = zcl::k_color_black);

    void RendererPassEnd(t_rendering_state *const rs);

    // Set prog as nullptr to just assign the default shader program.
    void RendererSetShaderProg(t_rendering_state *const rs, const t_gfx_resource *const prog);

    // Leave texture as nullptr for no texture.
    void RendererSubmitTriangles(t_rendering_state *const rs, const zcl::t_array_rdonly<t_triangle> triangles, const t_gfx_resource *const texture = nullptr);

    inline void RendererSubmitTriangle(t_rendering_state *const rs, const zcl::t_static_array<zcl::t_v2, 3> &pts, const zcl::t_static_array<zcl::t_color_rgba32f, 3> &pt_colors) {
        const t_triangle triangle = {
            .vertices = {{
                {.pos = pts[0], .blend = pt_colors[0], .uv = {}},
                {.pos = pts[1], .blend = pt_colors[1], .uv = {}},
                {.pos = pts[2], .blend = pt_colors[2], .uv = {}},
            }},
        };

        RendererSubmitTriangles(rs, {&triangle, 1}, nullptr);
    }

    inline void RendererSubmitTriangle(t_rendering_state *const rs, const zcl::t_static_array<zcl::t_v2, 3> &pts, const zcl::t_color_rgba32f color) {
        RendererSubmitTriangle(rs, pts, {{color, color, color}});
    }

    inline void RendererSubmitRect(t_rendering_state *const rs, const zcl::t_rect_f rect, const zcl::t_color_rgba32f color_topleft, const zcl::t_color_rgba32f color_topright, const zcl::t_color_rgba32f color_bottomright, const zcl::t_color_rgba32f color_bottomleft) {
        ZCL_ASSERT(rect.width > 0.0f && rect.height > 0.0f);

        const zcl::t_static_array<t_triangle, 2> triangles = {{
            {
                .vertices = {{
                    {.pos = zcl::RectGetTopLeft(rect), .blend = color_topleft, .uv = {0.0f, 0.0f}},
                    {.pos = zcl::RectGetTopRight(rect), .blend = color_topright, .uv = {1.0f, 0.0f}},
                    {.pos = zcl::RectGetBottomLeft(rect), .blend = color_bottomleft, .uv = {1.0f, 1.0f}},
                }},
            },
            {
                .vertices = {{
                    {.pos = zcl::RectGetBottomLeft(rect), .blend = color_bottomleft, .uv = {1.0f, 1.0f}},
                    {.pos = zcl::RectGetTopRight(rect), .blend = color_topright, .uv = {0.0f, 1.0f}},
                    {.pos = zcl::RectGetBottomRight(rect), .blend = color_bottomright, .uv = {0.0f, 0.0f}},
                }},
            },
        }};

        RendererSubmitTriangles(rs, zcl::ArrayToNonstatic(&triangles), nullptr);
    }

    inline void RendererSubmitRect(t_rendering_state *const rs, const zcl::t_rect_f rect, const zcl::t_color_rgba32f color) {
        RendererSubmitRect(rs, rect, color, color, color, color);
    }

    void RendererSubmitRectRotated(t_rendering_state *const rs, const zcl::t_v2 pos, const zcl::t_v2 size, const zcl::t_v2 origin, const zcl::t_f32 rot, const zcl::t_color_rgba32f color_topleft, const zcl::t_color_rgba32f color_topright, const zcl::t_color_rgba32f color_bottomright, const zcl::t_color_rgba32f color_bottomleft);

    inline void RendererSubmitRectRotated(t_rendering_state *const rs, const zcl::t_v2 pos, const zcl::t_v2 size, const zcl::t_v2 origin, const zcl::t_f32 rot, const zcl::t_color_rgba32f color) {
        RendererSubmitRectRotated(rs, pos, size, origin, rot, color, color, color, color);
    }

    void RendererSubmitTexture(t_rendering_state *const rs, const t_gfx_resource *const texture, const zcl::t_v2 pos, const zcl::t_rect_i src_rect = {}, const zcl::t_v2 origin = zcl::k_origin_top_left, const zcl::t_f32 rot = 0.0f);

    void RendererSubmitStr(t_rendering_state *const rs, const zcl::t_str_rdonly str, const t_font &font, const zcl::t_v2 pos, zcl::t_arena *const temp_arena, const zcl::t_v2 origin = zcl::k_origin_top_left, const zcl::t_color_rgba32f blend = zcl::k_color_white);
}
