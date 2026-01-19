#pragma once

#include <zcl.h>
#include <zgl/zgl_platform_public.h>

namespace zgl {
    struct t_gfx;

    struct t_vertex {
        zcl::t_v2 pos;
        zcl::t_color_rgba32f blend;
        zcl::t_v2 uv;
    };

    constexpr zcl::t_b8 VertexCheckValid(const t_vertex vert) {
        return zcl::ColorCheckNormalized(vert.blend)
            && vert.uv.x >= 0.0f && vert.uv.y >= 0.0f && vert.uv.x <= 1.0f && vert.uv.y <= 1.0f;
    }

    struct t_triangle {
        zcl::t_static_array<t_vertex, 3> vertices;
    };

    constexpr zcl::t_b8 TriangleCheckValid(const t_triangle tri) {
        return VertexCheckValid(tri.vertices[0])
            && VertexCheckValid(tri.vertices[1])
            && VertexCheckValid(tri.vertices[2]);
    }

    zcl::t_v2_i BackbufferGetSize(const t_gfx *const gfx);


    // ============================================================
    // @section: Resources

    struct t_gfx_resource_group;

    t_gfx_resource_group *GFXResourceGroupCreate(t_gfx *const gfx, zcl::t_arena *const arena);

    void GFXResourceGroupDestroy(t_gfx *const gfx, t_gfx_resource_group *const group);

    zcl::t_arena *GFXResourceGroupGetArena(const t_gfx *const gfx, const t_gfx_resource_group *const group);

    struct t_gfx_resource;

    t_gfx_resource *TextureCreate(t_gfx *const gfx, const zcl::t_texture_data_rdonly texture_data, t_gfx_resource_group *const resource_group);

    t_gfx_resource *TextureCreateFromExternal(t_gfx *const gfx, const zcl::t_str_rdonly file_path, t_gfx_resource_group *const group, zcl::t_arena *const temp_arena);

    t_gfx_resource *TextureCreateFromPacked(t_gfx *const gfx, const zcl::t_str_rdonly file_path, t_gfx_resource_group *const group, zcl::t_arena *const temp_arena);

    t_gfx_resource *TextureCreateTarget(t_gfx *const gfx, const zcl::t_v2_i size, t_gfx_resource_group *const resource_group);

    void TextureResizeTarget(t_gfx *const gfx, t_gfx_resource *const texture, const zcl::t_v2_i size);

    zcl::t_v2_i TextureGetSize(const t_gfx *const gfx, const t_gfx_resource *const texture);

    // Resizes only if the given size is actually different to the current.
    inline void TextureResizeTargetIfNeeded(t_gfx *const gfx, t_gfx_resource *const texture, const zcl::t_v2_i size) {
        const zcl::t_v2_i size_cur = TextureGetSize(gfx, texture);

        if (size != size_cur) {
            TextureResizeTarget(gfx, texture, size);
        }
    }

    t_gfx_resource *ShaderProgCreate(t_gfx *const gfx, const zcl::t_array_rdonly<zcl::t_u8> vertex_shader_compiled_bin, const zcl::t_array_rdonly<zcl::t_u8> frag_shader_compiled_bin, t_gfx_resource_group *const resource_group);

    t_gfx_resource *ShaderProgCreateFromPacked(t_gfx *const gfx, const zcl::t_str_rdonly vert_shader_file_path, const zcl::t_str_rdonly frag_shader_file_path, t_gfx_resource_group *const group, zcl::t_arena *const temp_arena);

    enum t_uniform_type : zcl::t_i32 {
        ek_uniform_type_sampler,
        ek_uniform_type_v4,
        ek_uniform_type_mat4x4
    };

    t_gfx_resource *UniformCreate(t_gfx *const gfx, const zcl::t_str_rdonly name, const t_uniform_type type, t_gfx_resource_group *const resource_group, zcl::t_arena *const temp_arena);

    t_uniform_type UniformGetType(const t_gfx *const gfx, const t_gfx_resource *const uniform);

    void UniformSetSampler(t_gfx *const gfx, const t_gfx_resource *const uniform, const t_gfx_resource *const sampler_texture);

    void UniformSetV4(t_gfx *const gfx, const t_gfx_resource *const uniform, const zcl::t_v4 v4);

    void UniformSetMatrix4x4(t_gfx *const gfx, const t_gfx_resource *const uniform, const zcl::t_mat4x4 &mat4x4);

    struct t_font {
        zcl::t_font_arrangement arrangement;
        zcl::t_array_mut<t_gfx_resource *> atlases;
    };

    t_font FontCreateFromExternal(t_gfx *const gfx, const zcl::t_str_rdonly file_path, const zcl::t_i32 height, zcl::t_code_point_bitset *const code_pts, t_gfx_resource_group *const resource_group, zcl::t_arena *const temp_arena);

    t_font FontCreateFromPacked(t_gfx *const gfx, const zcl::t_str_rdonly file_path, t_gfx_resource_group *const resource_group, zcl::t_arena *const temp_arena);

    // ============================================================


    // ============================================================
    // @section: Frame

    constexpr zcl::t_i16 k_frame_pass_limit = 256;

    void FramePassBegin(t_gfx *const gfx, const zcl::t_v2_i size, const zcl::t_mat4x4 &view_mat = zcl::MatrixCreateIdentity(), const zcl::t_b8 clear = false, const zcl::t_color_rgba32f clear_col = zcl::k_color_black);
    void FramePassBeginOffscreen(t_gfx *const gfx, const t_gfx_resource *const texture_target, const zcl::t_mat4x4 &view_mat = zcl::MatrixCreateIdentity(), const zcl::t_b8 clear = false, const zcl::t_color_rgba32f clear_col = zcl::k_color_black);

    void FramePassEnd(t_gfx *const gfx);

    zcl::t_b8 FramePassCheckActive(const t_gfx *const gfx);

    // Set prog as nullptr to just assign the default shader program.
    void FrameSetShaderProg(t_gfx *const gfx, const t_gfx_resource *const prog);

    void FrameSetUniformSampler(t_gfx *const gfx, const t_gfx_resource *const uniform, const t_gfx_resource *const sampler_texture);
    void FrameSetUniformV4(t_gfx *const gfx, const t_gfx_resource *const uniform, const zcl::t_v4 v4);
    void FrameSetUniformMat4x4(t_gfx *const gfx, const t_gfx_resource *const uniform, const zcl::t_mat4x4 &mat4x4);

    // Leave texture as nullptr for no texture.
    void FrameSubmitTriangles(t_gfx *const gfx, const zcl::t_array_rdonly<t_triangle> triangles, const t_gfx_resource *const texture = nullptr);

    inline void FrameSubmitTriangle(t_gfx *const gfx, const zcl::t_static_array<zcl::t_v2, 3> &pts, const zcl::t_static_array<zcl::t_color_rgba32f, 3> &pt_colors) {
        const t_triangle triangle = {
            .vertices = {{
                {.pos = pts[0], .blend = pt_colors[0], .uv = {}},
                {.pos = pts[1], .blend = pt_colors[1], .uv = {}},
                {.pos = pts[2], .blend = pt_colors[2], .uv = {}},
            }},
        };

        FrameSubmitTriangles(gfx, {&triangle, 1}, nullptr);
    }

    inline void FrameSubmitTriangle(t_gfx *const gfx, const zcl::t_static_array<zcl::t_v2, 3> &pts, const zcl::t_color_rgba32f color) {
        FrameSubmitTriangle(gfx, pts, {{color, color, color}});
    }

    inline void FrameSubmitRect(t_gfx *const gfx, const zcl::t_rect_f rect, const zcl::t_color_rgba32f color_topleft, const zcl::t_color_rgba32f color_topright, const zcl::t_color_rgba32f color_bottomright, const zcl::t_color_rgba32f color_bottomleft) {
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

        FrameSubmitTriangles(gfx, zcl::ArrayToNonstatic(&triangles), nullptr);
    }

    inline void FrameSubmitRect(t_gfx *const gfx, const zcl::t_rect_f rect, const zcl::t_color_rgba32f color) {
        FrameSubmitRect(gfx, rect, color, color, color, color);
    }

    void FrameSubmitRectRotated(t_gfx *const gfx, const zcl::t_v2 pos, const zcl::t_v2 size, const zcl::t_v2 origin, const zcl::t_f32 rot, const zcl::t_color_rgba32f color_topleft, const zcl::t_color_rgba32f color_topright, const zcl::t_color_rgba32f color_bottomright, const zcl::t_color_rgba32f color_bottomleft);

    inline void FrameSubmitRectRotated(t_gfx *const gfx, const zcl::t_v2 pos, const zcl::t_v2 size, const zcl::t_v2 origin, const zcl::t_f32 rot, const zcl::t_color_rgba32f color) {
        FrameSubmitRectRotated(gfx, pos, size, origin, rot, color, color, color, color);
    }

    void FrameSubmitTexture(t_gfx *const gfx, const t_gfx_resource *const texture, const zcl::t_v2 pos, const zcl::t_rect_i src_rect = {}, const zcl::t_v2 origin = zcl::k_origin_top_left, const zcl::t_f32 rot = 0.0f);

    void FrameSubmitStr(t_gfx *const gfx, const zcl::t_str_rdonly str, const t_font &font, const zcl::t_v2 pos, zcl::t_arena *const temp_arena, const zcl::t_v2 origin = zcl::k_origin_top_left, const zcl::t_color_rgba32f blend = zcl::k_color_white);

    // ============================================================


    namespace internal {
        t_gfx *GFXStartup(const t_platform_ticket_rdonly platform_ticket, zcl::t_arena *const arena, zcl::t_arena *const temp_arena);
        void GFXShutdown(t_gfx *const gfx);

        void FrameBegin(t_gfx *const gfx, const t_platform_ticket_rdonly platform_ticket);
        void FrameEnd(t_gfx *const gfx);

        void BackbufferResizeIfNeeded(t_gfx *const gfx, const zcl::t_v2_i size);
    }
}
