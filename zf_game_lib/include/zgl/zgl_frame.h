#pragma once

namespace zgl {
    struct t_vertex {
        zcl::t_v2 pos;
        zcl::t_color_rgba32f blend;
        zcl::t_v2 uv;
    };

    constexpr zcl::t_b8 FrameVertexCheckValid(const t_vertex vert) {
        return zcl::ColorCheckNormalized(vert.blend)
            && vert.uv.x >= 0.0f && vert.uv.y >= 0.0f && vert.uv.x <= 1.0f && vert.uv.y <= 1.0f;
    }

    struct t_frame_triangle {
        zcl::t_static_array<t_frame_vertex, 3> verts;
    };

    constexpr zcl::t_b8 FrameTriangleCheckValid(const t_frame_triangle tri) {
        return FrameVertexCheckValid(tri.verts[0])
            && FrameVertexCheckValid(tri.verts[1])
            && FrameVertexCheckValid(tri.verts[2]);
    }

    constexpr zcl::t_i16 k_frame_pass_limit = 256;

    zcl::t_v2_i FrameGetSize();
    void FrameSetSize(const zcl::t_v2_i size);

    void FrameBegin(const t_platform_ticket_rdonly platform_ticket);
    void FrameEnd();

    void FramePassBegin(const zcl::t_v2_i size, const zcl::t_mat4x4 &view_mat = zcl::MatrixCreateIdentity(), const zcl::t_b8 clear = false, const zcl::t_color_rgba32f clear_col = zcl::k_color_black);
    void FramePassBeginOffscreen(const t_gfx_resource *const texture_target, const zcl::t_mat4x4 &view_mat = zcl::MatrixCreateIdentity(), const zcl::t_b8 clear = false, const zcl::t_color_rgba32f clear_col = zcl::k_color_black);

    void FramePassEnd();

    zcl::t_b8 FramePassCheckActive();
    zcl::t_i32 FramePassGetIndex();

    // Set prog as nullptr to just assign the default shader program.
    void FrameSetShaderProg(const t_gfx_resource *const prog);

    void FrameSetUniformSampler(const t_gfx_resource *const uniform, const t_gfx_resource *const sampler_texture);
    void FrameSetUniformV4(const t_gfx_resource *const uniform, const zcl::t_v4 v4);
    void FrameSetUniformMat4x4(const t_gfx_resource *const uniform, const zcl::t_mat4x4 &mat4x4);

    // Leave texture as nullptr for no texture.
    void FrameSubmitTriangles(const zcl::t_array_rdonly<t_frame_triangle> triangles, const t_gfx_resource *const texture = nullptr);

    inline void FrameSubmitTriangle(const zcl::t_static_array<zcl::t_v2, 3> &pts, const zcl::t_static_array<zcl::t_color_rgba32f, 3> &pt_colors) {
        const t_frame_triangle triangle = {
            .verts = {{
                {.pos = pts[0], .blend = pt_colors[0], .uv = {}},
                {.pos = pts[1], .blend = pt_colors[1], .uv = {}},
                {.pos = pts[2], .blend = pt_colors[2], .uv = {}},
            }},
        };

        FrameSubmitTriangles({&triangle, 1}, nullptr);
    }

    inline void FrameSubmitTriangle(const zcl::t_static_array<zcl::t_v2, 3> &pts, const zcl::t_color_rgba32f color) {
        FrameSubmitTriangle(pts, {{color, color, color}});
    }

    inline void FrameSubmitRect(const zcl::t_rect_f rect, const zcl::t_color_rgba32f color_topleft, const zcl::t_color_rgba32f color_topright, const zcl::t_color_rgba32f color_bottomright, const zcl::t_color_rgba32f color_bottomleft) {
        ZCL_ASSERT(rect.width > 0.0f && rect.height > 0.0f);

        const zcl::t_static_array<t_frame_triangle, 2> triangles = {{
            {
                .verts = {{
                    {.pos = zcl::RectGetTopLeft(rect), .blend = color_topleft, .uv = {0.0f, 0.0f}},
                    {.pos = zcl::RectGetTopRight(rect), .blend = color_topright, .uv = {1.0f, 0.0f}},
                    {.pos = zcl::RectGetBottomLeft(rect), .blend = color_bottomleft, .uv = {1.0f, 1.0f}},
                }},
            },
            {
                .verts = {{
                    {.pos = zcl::RectGetBottomLeft(rect), .blend = color_bottomleft, .uv = {1.0f, 1.0f}},
                    {.pos = zcl::RectGetTopRight(rect), .blend = color_topright, .uv = {0.0f, 1.0f}},
                    {.pos = zcl::RectGetBottomRight(rect), .blend = color_bottomright, .uv = {0.0f, 0.0f}},
                }},
            },
        }};

        FrameSubmitTriangles(zcl::ArrayToNonstatic(&triangles), nullptr);
    }

    inline void FrameSubmitRect(const zcl::t_rect_f rect, const zcl::t_color_rgba32f color) {
        FrameSubmitRect(rect, color, color, color, color);
    }

    void FrameSubmitRectRotated(const zcl::t_v2 pos, const zcl::t_v2 size, const zcl::t_v2 origin, const zcl::t_f32 rot, const zcl::t_color_rgba32f color_topleft, const zcl::t_color_rgba32f color_topright, const zcl::t_color_rgba32f color_bottomright, const zcl::t_color_rgba32f color_bottomleft);

    inline void FrameSubmitRectRotated(const zcl::t_v2 pos, const zcl::t_v2 size, const zcl::t_v2 origin, const zcl::t_f32 rot, const zcl::t_color_rgba32f color) {
        FrameSubmitRectRotated(pos, size, origin, rot, color, color, color, color);
    }

    void FrameSubmitTexture(const t_gfx_resource *const texture, const zcl::t_v2 pos, const zcl::t_rect_i src_rect = {}, const zcl::t_v2 origin = zcl::k_origin_top_left, const zcl::t_f32 rot = 0.0f);

    // ============================================================
}
