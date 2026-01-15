#pragma once

#include <zcl.h>

namespace zgl {
    struct t_platform;

    struct t_gfx;

    // @todo: Permanent resource group doesn't need to be exposed. Better to just have user create their own, more explicit and simple.
    t_gfx *GFXStartup(const t_platform *const platform, zcl::t_arena *const arena, zcl::t_arena *const temp_arena);

    void GFXShutdown(t_gfx *const gfx);

    // ============================================================
    // @section: Resources

    struct t_gfx_resource_group;

    zcl::t_arena *GFXResourceGroupGetArena(t_gfx_resource_group *const group); // @temp: Remove once fonts are reworked.

    t_gfx_resource_group *GFXResourceGroupCreate(t_gfx *const gfx, zcl::t_arena *const arena);
    void GFXResourceGroupDestroy(t_gfx *const gfx, t_gfx_resource_group *const group);

    struct t_gfx_resource;

    t_gfx_resource *TextureCreate(t_gfx *const gfx, const zcl::t_texture_data_rdonly texture_data, t_gfx_resource_group *const group);
    t_gfx_resource *TextureCreateFromExternal(t_gfx *const gfx, const zcl::t_str_rdonly file_path, t_gfx_resource_group *const group, zcl::t_arena *const temp_arena);
    t_gfx_resource *TextureCreateFromPacked(t_gfx *const gfx, const zcl::t_str_rdonly file_path, t_gfx_resource_group *const group, zcl::t_arena *const temp_arena);

    t_gfx_resource *TextureCreateTarget(t_gfx *const gfx, const zcl::t_v2_i size, t_gfx_resource_group *const group);

    void TextureResizeTarget(t_gfx *const gfx, t_gfx_resource *const texture, const zcl::t_v2_i size);

    zcl::t_v2_i TextureGetSize(const t_gfx *const gfx, const t_gfx_resource *const texture);

    // Resizes only if the given size is actually different to the current.
    inline void TextureResizeTargetIfNeeded(t_gfx *const gfx, t_gfx_resource *const texture, const zcl::t_v2_i size) {
        const zcl::t_v2_i size_cur = TextureGetSize(gfx, texture);

        if (size != size_cur) {
            TextureResizeTarget(gfx, texture, size);
        }
    }

    t_gfx_resource *ShaderProgCreate(t_gfx *const gfx, const zcl::t_array_rdonly<zcl::t_u8> vert_shader_compiled_bin, const zcl::t_array_rdonly<zcl::t_u8> frag_shader_compiled_bin, t_gfx_resource_group *const group);
    t_gfx_resource *ShaderProgCreateFromPacked(t_gfx *const gfx, const zcl::t_str_rdonly vert_shader_file_path, const zcl::t_str_rdonly frag_shader_file_path, t_gfx_resource_group *const group, zcl::t_arena *const temp_arena);

    enum t_uniform_type {
        ek_uniform_type_sampler,
        ek_uniform_type_v4,
        ek_uniform_type_mat4x4
    };

    t_gfx_resource *UniformCreate(t_gfx *const gfx, const zcl::t_str_rdonly name, const t_uniform_type type, t_gfx_resource_group *const group, zcl::t_arena *const temp_arena);
    t_uniform_type UniformGetType(const t_gfx *const gfx, const t_gfx_resource *const uniform);

    // @todo: This might be better off like every other resource, for API consistency.
    struct t_font {
        zcl::t_font_arrangement arrangement;
        zcl::t_array_mut<t_gfx_resource *> atlases;
    };

    t_font FontCreateFromExternal(t_gfx *const gfx, const zcl::t_str_rdonly file_path, const zcl::t_i32 height, zcl::t_code_point_bitset *const code_pts, t_gfx_resource_group *const resource_group, zcl::t_arena *const temp_arena);
    t_font FontCreateFromPacked(t_gfx *const gfx, const zcl::t_str_rdonly file_path, t_gfx_resource_group *const resource_group, zcl::t_arena *const temp_arena);

    // ============================================================


    // ============================================================
    // @section: Frame

    struct t_frame_state;

    struct t_frame_context {
        t_gfx *gfx;
        t_frame_state *state;
    };

    constexpr zcl::t_i16 k_frame_pass_limit = 256;

    struct t_frame_vertex {
        zcl::t_v2 pos;
        zcl::t_color_rgba32f blend;
        zcl::t_v2 uv;
    };

    constexpr zcl::t_b8 FrameVertexCheckValid(const t_frame_vertex vert) {
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

    t_frame_context FrameBegin(t_gfx *const gfx, const t_platform *const platform, zcl::t_arena *const context_arena);
    void FrameEnd(const t_frame_context context);

    void FramePassBegin(const t_frame_context context, const zcl::t_v2_i size, const zcl::t_mat4x4 &view_mat = zcl::MatrixCreateIdentity(), const zcl::t_b8 clear = false, const zcl::t_color_rgba32f clear_col = zcl::k_color_black);
    void FramePassBeginOffscreen(const t_frame_context context, const t_gfx_resource *const texture_target, const zcl::t_mat4x4 &view_mat = zcl::MatrixCreateIdentity(), const zcl::t_b8 clear = false, const zcl::t_color_rgba32f clear_col = zcl::k_color_black);

    void FramePassEnd(const t_frame_context context);

    zcl::t_b8 FramePassCheckActive(const t_frame_context context);
    zcl::t_i32 FramePassGetIndex(const t_frame_context context);

    // Set prog as nullptr to just assign the default shader program.
    void FrameSetShaderProg(const t_frame_context context, const t_gfx_resource *const prog);

    // @todo: These functoins are awkward!
    const t_gfx_resource *FrameGetBuiltinShaderProgDefault(const t_frame_context context);
    const t_gfx_resource *FrameGetBuiltinShaderProgBlend(const t_frame_context context);
    const t_gfx_resource *FrameGetBuiltinUniformBlend(const t_frame_context context);
    // -----------------------------------

    void FrameSetUniformSampler(const t_frame_context context, const t_gfx_resource *const uniform, const t_gfx_resource *const sampler_texture);
    void FrameSetUniformV4(const t_frame_context context, const t_gfx_resource *const uniform, const zcl::t_v4 v4);
    void FrameSetUniformMat4x4(const t_frame_context context, const t_gfx_resource *const uniform, const zcl::t_mat4x4 &mat4x4);

    // Leave texture as nullptr for no texture.
    void FrameSubmitTriangles(const t_frame_context context, const zcl::t_array_rdonly<t_frame_triangle> triangles, const t_gfx_resource *const texture = nullptr);

    inline void FrameSubmitTriangle(const t_frame_context context, const zcl::t_static_array<zcl::t_v2, 3> &pts, const zcl::t_static_array<zcl::t_color_rgba32f, 3> &pt_colors) {
        const t_frame_triangle triangle = {
            .verts = {{
                {.pos = pts[0], .blend = pt_colors[0], .uv = {}},
                {.pos = pts[1], .blend = pt_colors[1], .uv = {}},
                {.pos = pts[2], .blend = pt_colors[2], .uv = {}},
            }},
        };

        FrameSubmitTriangles(context, {&triangle, 1}, nullptr);
    }

    inline void FrameSubmitTriangle(const t_frame_context context, const zcl::t_static_array<zcl::t_v2, 3> &pts, const zcl::t_color_rgba32f color) {
        FrameSubmitTriangle(context, pts, {{color, color, color}});
    }

    inline void FrameSubmitRect(const t_frame_context context, const zcl::t_rect_f rect, const zcl::t_color_rgba32f color_topleft, const zcl::t_color_rgba32f color_topright, const zcl::t_color_rgba32f color_bottomright, const zcl::t_color_rgba32f color_bottomleft) {
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

        FrameSubmitTriangles(context, zcl::ArrayToNonstatic(&triangles), nullptr);
    }

    inline void FrameSubmitRect(const t_frame_context context, const zcl::t_rect_f rect, const zcl::t_color_rgba32f color) {
        FrameSubmitRect(context, rect, color, color, color, color);
    }

    void FrameSubmitRectRotated(const t_frame_context context, const zcl::t_v2 pos, const zcl::t_v2 size, const zcl::t_v2 origin, const zcl::t_f32 rot, const zcl::t_color_rgba32f color_topleft, const zcl::t_color_rgba32f color_topright, const zcl::t_color_rgba32f color_bottomright, const zcl::t_color_rgba32f color_bottomleft);

    inline void FrameSubmitRectRotated(const t_frame_context context, const zcl::t_v2 pos, const zcl::t_v2 size, const zcl::t_v2 origin, const zcl::t_f32 rot, const zcl::t_color_rgba32f color) {
        FrameSubmitRectRotated(context, pos, size, origin, rot, color, color, color, color);
    }

    void FrameSubmitTexture(const t_frame_context context, const t_gfx_resource *const texture, const zcl::t_v2 pos, const zcl::t_rect_i src_rect = {}, const zcl::t_v2 origin = zcl::k_origin_top_left, const zcl::t_f32 rot = 0.0f);

    void FrameSubmitStr(const t_frame_context context, const zcl::t_str_rdonly str, const t_font &font, const zcl::t_v2 pos, zcl::t_arena *const temp_arena, const zcl::t_v2 origin = zcl::k_origin_top_left, const zcl::t_color_rgba32f blend = zcl::k_color_white);

    // ============================================================
}
