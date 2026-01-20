#pragma once

#include <zcl.h>
#include <zgl/zgl_gfx.h>

namespace zgl {
    struct t_rendering_basis;
    struct t_rendering_state;

    struct t_rendering_context {
        const t_rendering_basis *basis;
        t_rendering_state *state;

        t_gfx *gfx;
    };

    void RendererPassBegin(const t_rendering_context rc, const zcl::t_v2_i size, const zcl::t_mat4x4 &view_mat = zcl::MatrixCreateIdentity(), const zcl::t_b8 clear = false, const zcl::t_color_rgba32f clear_col = zcl::k_color_black);

    void RendererPassBeginOffscreen(const t_rendering_context rc, const t_gfx_resource *const texture_target, const zcl::t_mat4x4 &view_mat = zcl::MatrixCreateIdentity(), const zcl::t_b8 clear = false, const zcl::t_color_rgba32f clear_col = zcl::k_color_black);

    void RendererPassEnd(const t_rendering_context rc);

    // Set prog as nullptr to just assign the default shader program.
    void RendererSetShaderProg(const t_rendering_context rc, const t_gfx_resource *const prog);

    // Leave texture as nullptr for no texture.
    void RendererSubmit(const t_rendering_context rc, const zcl::t_array_rdonly<t_triangle> triangles, const t_gfx_resource *const texture = nullptr);

    inline void RendererSubmitTriangle(const t_rendering_context rc, const zcl::t_static_array<zcl::t_v2, 3> &pts, const zcl::t_static_array<zcl::t_color_rgba32f, 3> &pt_colors) {
        const t_triangle triangle = {
            .vertices = {{
                {.pos = pts[0], .blend = pt_colors[0], .uv = {}},
                {.pos = pts[1], .blend = pt_colors[1], .uv = {}},
                {.pos = pts[2], .blend = pt_colors[2], .uv = {}},
            }},
        };

        RendererSubmit(rc, {&triangle, 1}, nullptr);
    }

    inline void RendererSubmitTriangle(const t_rendering_context rc, const zcl::t_static_array<zcl::t_v2, 3> &pts, const zcl::t_color_rgba32f color) {
        RendererSubmitTriangle(rc, pts, {{color, color, color}});
    }

    void RendererSubmitRect(const t_rendering_context rc, const zcl::t_rect_f rect, const zcl::t_color_rgba32f color_topleft, const zcl::t_color_rgba32f color_topright, const zcl::t_color_rgba32f color_bottomright, const zcl::t_color_rgba32f color_bottomleft);

    inline void RendererSubmitRect(const t_rendering_context rc, const zcl::t_rect_f rect, const zcl::t_color_rgba32f color) {
        RendererSubmitRect(rc, rect, color, color, color, color);
    }

    void RendererSubmitRectRotated(const t_rendering_context rc, const zcl::t_v2 pos, const zcl::t_v2 size, const zcl::t_v2 origin, const zcl::t_f32 rot, const zcl::t_color_rgba32f color_topleft, const zcl::t_color_rgba32f color_topright, const zcl::t_color_rgba32f color_bottomright, const zcl::t_color_rgba32f color_bottomleft);

    inline void RendererSubmitRectRotated(const t_rendering_context rc, const zcl::t_v2 pos, const zcl::t_v2 size, const zcl::t_v2 origin, const zcl::t_f32 rot, const zcl::t_color_rgba32f color) {
        RendererSubmitRectRotated(rc, pos, size, origin, rot, color, color, color, color);
    }

    void RendererSubmitTexture(const t_rendering_context rc, const t_gfx_resource *const texture, const zcl::t_v2 pos, const zcl::t_rect_i src_rect = {}, const zcl::t_v2 origin = zcl::k_origin_top_left, const zcl::t_f32 rot = 0.0f);

    void RendererSubmitStr(const t_rendering_context rc, const zcl::t_str_rdonly str, const t_font &font, const zcl::t_v2 pos, zcl::t_arena *const temp_arena, const zcl::t_v2 origin = zcl::k_origin_top_left, const zcl::t_color_rgba32f blend = zcl::k_color_white);

    namespace internal {
        t_rendering_basis *RenderingBasisCreate(t_gfx *const gfx, zcl::t_arena *const arena, zcl::t_arena *const temp_arena);

        void RenderingBasisDestroy(t_rendering_basis *const basis, t_gfx *const gfx);

        t_rendering_context RendererBegin(const t_rendering_basis *const rendering_basis, t_gfx *const gfx, zcl::t_arena *const rendering_state_arena);

        void RendererEnd(const t_rendering_context rc);
    }
}
