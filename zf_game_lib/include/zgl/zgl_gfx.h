#pragma once

#include <zcl.h>

namespace zgl::gfx {
    struct t_frame_basis;
    struct t_resource_group;

    // This depends on the platform module being initialized beforehand.
    t_frame_basis *ModuleStartup(zcl::t_arena *const arena, zcl::t_arena *const temp_arena, t_resource_group **const o_perm_resource_group);

    void ModuleShutdown(const t_frame_basis *const frame_basis);

    // ============================================================
    // @section: Resources

    struct t_resource;

    struct t_resource_group {
        zcl::t_arena *arena;
        t_resource *head;
        t_resource *tail;
    };

    inline t_resource_group ResourceGroupCreate(zcl::t_arena *const arena) {
        return {.arena = arena};
    }

    void ResourceGroupDestroy(t_resource_group *const group);

    t_resource *TextureCreate(const zcl::t_texture_data_rdonly texture_data, t_resource_group *const group);
    t_resource *TextureCreateFromExternal(const zcl::t_str_rdonly file_path, t_resource_group *const group, zcl::t_arena *const temp_arena);
    t_resource *TextureCreateFromPacked(const zcl::t_str_rdonly file_path, t_resource_group *const group, zcl::t_arena *const temp_arena);

    t_resource *TextureCreateTarget(const zcl::t_v2_i size, t_resource_group *const group);

    void TextureResizeTarget(t_resource *const texture, const zcl::t_v2_i size);

    zcl::t_v2_i TextureGetSize(const t_resource *const texture);

    // Resizes only if the given size is actually different to the current.
    inline void TextureResizeTargetIfNeeded(t_resource *const texture, const zcl::t_v2_i size) {
        const zcl::t_v2_i size_cur = TextureGetSize(texture);

        if (size != size_cur) {
            TextureResizeTarget(texture, size);
        }
    }

    t_resource *ShaderProgCreate(const zcl::t_array_rdonly<zcl::t_u8> vert_shader_compiled_bin, const zcl::t_array_rdonly<zcl::t_u8> frag_shader_compiled_bin, t_resource_group *const group);
    t_resource *ShaderProgCreateFromPacked(const zcl::t_str_rdonly vert_shader_file_path, const zcl::t_str_rdonly frag_shader_file_path, t_resource_group *const group, zcl::t_arena *const temp_arena);

    enum t_uniform_type {
        ek_uniform_type_sampler,
        ek_uniform_type_v4,
        ek_uniform_type_mat4x4
    };

    t_resource *UniformCreate(const zcl::t_str_rdonly name, const t_uniform_type type, t_resource_group *const group, zcl::t_arena *const temp_arena);
    t_uniform_type UniformGetType(const t_resource *const uniform);

    // @todo: This might be better off like every other resource, for API consistency.
    struct t_font {
        zcl::t_font_arrangement arrangement;
        zcl::t_array_mut<t_resource *> atlases;
    };

    t_font FontCreateFromExternal(const zcl::t_str_rdonly file_path, const zcl::t_i32 height, zcl::t_code_point_bitset *const code_pts, t_resource_group *const resource_group, zcl::t_arena *const temp_arena);
    t_font FontCreateFromPacked(const zcl::t_str_rdonly file_path, t_resource_group *const resource_group, zcl::t_arena *const temp_arena);

    // ============================================================


    // ============================================================
    // @section: Frame

    struct t_frame_context;

    constexpr zcl::t_i16 k_frame_pass_limit = 256;

    constexpr zcl::t_v2 k_origin_top_left = {0.0f, 0.0f};
    constexpr zcl::t_v2 k_origin_top_center = {0.5f, 0.0f};
    constexpr zcl::t_v2 k_origin_top_right = {1.0f, 0.0f};
    constexpr zcl::t_v2 k_origin_center_left = {0.0f, 0.5f};
    constexpr zcl::t_v2 k_origin_center = {0.5f, 0.5f};
    constexpr zcl::t_v2 k_origin_center_right = {1.0f, 0.5f};
    constexpr zcl::t_v2 k_origin_bottom_left = {0.0f, 1.0f};
    constexpr zcl::t_v2 k_origin_bottom_center = {0.5f, 1.0f};
    constexpr zcl::t_v2 k_origin_bottom_right = {1.0f, 1.0f};

    constexpr zcl::t_b8 OriginCheckValid(const zcl::t_v2 origin) {
        return origin.x >= 0.0f && origin.x <= 1.0f && origin.y >= 0.0f && origin.y <= 1.0f;
    }

    constexpr zcl::t_v2 k_alignment_top_left = {0.0f, 0.0f};
    constexpr zcl::t_v2 k_alignment_top_center = {0.5f, 0.0f};
    constexpr zcl::t_v2 k_alignment_top_right = {1.0f, 0.0f};
    constexpr zcl::t_v2 k_alignment_center_left = {0.0f, 0.5f};
    constexpr zcl::t_v2 k_alignment_center = {0.5f, 0.5f};
    constexpr zcl::t_v2 k_alignment_center_right = {1.0f, 0.5f};
    constexpr zcl::t_v2 k_alignment_bottom_left = {0.0f, 1.0f};
    constexpr zcl::t_v2 k_alignment_bottom_center = {0.5f, 1.0f};
    constexpr zcl::t_v2 k_alignment_bottom_right = {1.0f, 1.0f};

    constexpr zcl::t_b8 AlignmentCheckValid(const zcl::t_v2 alignment) {
        return alignment.x >= 0.0f && alignment.x <= 1.0f && alignment.y >= 0.0f && alignment.y <= 1.0f;
    }

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
        zcl::t_static_array<t_vertex, 3> verts;
    };

    constexpr zcl::t_b8 TriangleValid(const t_triangle tri) {
        return VertexCheckValid(tri.verts[0])
            && VertexCheckValid(tri.verts[1])
            && VertexCheckValid(tri.verts[2]);
    }

    t_frame_context *FrameBegin(const t_frame_basis *const basis, zcl::t_arena *const context_arena);
    void FrameEnd(t_frame_context *const context);

    void FramePassBegin(t_frame_context *const context, const zcl::t_v2_i size, const zcl::t_mat4x4 &view_mat = zcl::MatrixCreateIdentity(), const zcl::t_b8 clear = false, const zcl::t_color_rgba32f clear_col = zcl::k_color_black);
    void FramePassBeginOffscreen(t_frame_context *const context, const t_resource *const texture_target, const zcl::t_mat4x4 &view_mat = zcl::MatrixCreateIdentity(), const zcl::t_b8 clear = false, const zcl::t_color_rgba32f clear_col = zcl::k_color_black);

    void FramePassEnd(t_frame_context *const context);

    zcl::t_b8 FramePassCheckActive(const t_frame_context *const context);
    zcl::t_i32 FramePassGetIndex(const t_frame_context *const context);

    // Set prog as nullptr to just assign the default shader program.
    void FrameSetShaderProg(t_frame_context *const context, const t_resource *const prog);

    const t_resource *FrameGetBuiltinShaderProgDefault(t_frame_context *const context);
    const t_resource *FrameGetBuiltinShaderProgBlend(t_frame_context *const context);

    const t_resource *FrameGetBuiltinUniformBlend(t_frame_context *const context);

    void FrameSetUniformSampler(t_frame_context *const context, const t_resource *const uniform, const t_resource *const sampler_texture);
    void FrameSetUniformV4(t_frame_context *const context, const t_resource *const uniform, const zcl::t_v4 v4);
    void FrameSetUniformMat4x4(t_frame_context *const context, const t_resource *const uniform, const zcl::t_mat4x4 &mat4x4);

    // Leave texture as nullptr for no texture.
    void FrameSubmitTriangles(t_frame_context *const context, const zcl::t_array_rdonly<t_triangle> triangles, const t_resource *const texture = nullptr);

    inline void FrameSubmitTriangle(t_frame_context *const context, const zcl::t_static_array<zcl::t_v2, 3> &pts, const zcl::t_static_array<zcl::t_color_rgba32f, 3> &pt_colors) {
        const t_triangle triangle = {
            .verts = {{
                {.pos = pts[0], .blend = pt_colors[0], .uv = {}},
                {.pos = pts[1], .blend = pt_colors[1], .uv = {}},
                {.pos = pts[2], .blend = pt_colors[2], .uv = {}},
            }},
        };

        FrameSubmitTriangles(context, {&triangle, 1}, nullptr);
    }

    inline void FrameSubmitTriangle(t_frame_context *const context, const zcl::t_static_array<zcl::t_v2, 3> &pts, const zcl::t_color_rgba32f color) {
        FrameSubmitTriangle(context, pts, {{color, color, color}});
    }

    inline void FrameSubmitRect(t_frame_context *const context, const zcl::t_rect_f rect, const zcl::t_color_rgba32f color_topleft, const zcl::t_color_rgba32f color_topright, const zcl::t_color_rgba32f color_bottomright, const zcl::t_color_rgba32f color_bottomleft) {
        ZCL_ASSERT(rect.width > 0.0f && rect.height > 0.0f);

        const zcl::t_static_array<t_triangle, 2> triangles = {{
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

    inline void FrameSubmitRect(t_frame_context *const context, const zcl::t_rect_f rect, const zcl::t_color_rgba32f color) {
        FrameSubmitRect(context, rect, color, color, color, color);
    }

    void FrameSubmitRectRotated(t_frame_context *const context, const zcl::t_v2 pos, const zcl::t_v2 size, const zcl::t_v2 origin, const zcl::t_f32 rot, const zcl::t_color_rgba32f color_topleft, const zcl::t_color_rgba32f color_topright, const zcl::t_color_rgba32f color_bottomright, const zcl::t_color_rgba32f color_bottomleft);

    inline void FrameSubmitRectRotated(t_frame_context *const context, const zcl::t_v2 pos, const zcl::t_v2 size, const zcl::t_v2 origin, const zcl::t_f32 rot, const zcl::t_color_rgba32f color) {
        FrameSubmitRectRotated(context, pos, size, origin, rot, color, color, color, color);
    }

    void FrameSubmitTexture(t_frame_context *const context, const t_resource *const texture, const zcl::t_v2 pos, const zcl::t_rect_i src_rect = {}, const zcl::t_v2 origin = k_origin_top_left, const zcl::t_f32 rot = 0.0f);

    void FrameSubmitStr(t_frame_context *const context, const zcl::t_str_rdonly str, const t_font &font, const zcl::t_v2 pos, zcl::t_arena *const temp_arena, const zcl::t_v2 alignment = k_alignment_top_left, const zcl::t_color_rgba32f blend = zcl::k_color_white);

    // ============================================================
}
