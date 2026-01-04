#pragma once

#include <zcl.h>

namespace zf {
    // ============================================================
    // @section: Types and Globals

    struct s_gfx_resource;

    struct zf_rendering_resource_group {
        s_arena *arena;
        s_gfx_resource *head;
        s_gfx_resource *tail;
    };

    struct s_rendering_basis;

    struct s_rendering_context;

    struct s_batch_vert {
        s_v2 pos;
        s_color_rgba32f blend;
        s_v2 uv;
    };

    struct s_batch_triangle {
        s_static_array<s_batch_vert, 3> verts;
    };

    // ============================================================


    // ============================================================
    // @section: Functions

    // This depends on the platform module being initialised beforehand.
    // Returns a pointer to a rendering basis, needed for all rendering operations.
    s_rendering_basis *StartupGFX(s_arena *const arena, zf_rendering_resource_group **const o_perm_resource_group);

    void ShutdownGFX(const s_rendering_basis *const rendering_basis);

    inline zf_rendering_resource_group CreateGFXResourceGroup(s_arena *const arena, zf_rendering_resource_group **const o_perm_group) {
        return {.arena = arena};
    }

    void DestroyGFXResourceGroup(zf_rendering_resource_group *const group);

    s_gfx_resource *CreateTexture(const s_texture_data_rdonly texture_data, zf_rendering_resource_group *const group);

    inline s_gfx_resource *CreateTextureFromRaw(const strs::StrRdonly file_path, s_arena *const temp_arena, zf_rendering_resource_group *const group) {
        s_texture_data texture_data;

        if (!LoadTextureDataFromRaw(file_path, temp_arena, temp_arena, &texture_data)) {
            ZF_FATAL();
        }

        return CreateTexture(texture_data, group);
    }

    inline s_gfx_resource *CreateTextureFromPacked(const strs::StrRdonly file_path, s_arena *const temp_arena, zf_rendering_resource_group *const group) {
        s_texture_data texture_data;

        if (!UnpackTexture(file_path, temp_arena, temp_arena, &texture_data)) {
            ZF_FATAL();
        }

        return CreateTexture(texture_data, group);
    }

    s_v2_i TextureSize(const s_gfx_resource *const texture);

    s_gfx_resource *CreateShaderProg(const s_array_rdonly<U8> vert_shader_compiled_bin, const s_array_rdonly<U8> frag_shader_compiled_bin, zf_rendering_resource_group *const group);

    inline s_gfx_resource *CreateShaderProgFromPacked(const strs::StrRdonly vert_shader_file_path, const strs::StrRdonly frag_shader_file_path, s_arena *const temp_arena, zf_rendering_resource_group *const arena) {
        s_array_mut<U8> vert_shader_compiled_bin;

        if (!UnpackShader(vert_shader_file_path, temp_arena, temp_arena, &vert_shader_compiled_bin)) {
            ZF_FATAL();
        }

        s_array_mut<U8> frag_shader_compiled_bin;

        if (!UnpackShader(frag_shader_file_path, temp_arena, temp_arena, &frag_shader_compiled_bin)) {
            ZF_FATAL();
        }

        return CreateShaderProg(vert_shader_compiled_bin, frag_shader_compiled_bin, arena);
    }

    s_rendering_context *zf_rendering_begin_frame(const s_rendering_basis *const rendering_basis, const s_color_rgb8 clear_col, s_arena *const rendering_context_arena);
    void zf_rendering_end_frame(s_rendering_context *const rendering_context);

    // Leave texture as nullptr for no texture.
    void SubmitTrianglesToBatch(s_rendering_context *const rc, const s_array_rdonly<s_batch_triangle> triangles, const s_gfx_resource *const texture);

    inline void RenderTriangle(s_rendering_context *const rc, const s_static_array<s_v2, 3> &pts, const s_static_array<s_color_rgba32f, 3> &pt_colors) {
        const s_batch_triangle triangle = {
            .verts = {{
                {.pos = pts[0], .blend = pt_colors[0], .uv = {}},
                {.pos = pts[1], .blend = pt_colors[1], .uv = {}},
                {.pos = pts[2], .blend = pt_colors[2], .uv = {}},
            }},
        };

        SubmitTrianglesToBatch(rc, {&triangle, 1}, nullptr);
    }

    inline void RenderTriangle(s_rendering_context *const rc, const s_static_array<s_v2, 3> &pts, const s_color_rgba32f color) {
        RenderTriangle(rc, pts, {{color, color, color}});
    }

    inline void RenderRect(s_rendering_context *const rc, const s_rect_f rect, const s_color_rgba32f color_topleft, const s_color_rgba32f color_topright, const s_color_rgba32f color_bottomright, const s_color_rgba32f color_bottomleft) {
        ZF_ASSERT(rect.width > 0.0f && rect.height > 0.0f);

        const s_static_array<s_batch_triangle, 2> triangles = {{
            {
                .verts = {{
                    {.pos = TopLeft(rect), .blend = color_topleft, .uv = {0.0f, 0.0f}},
                    {.pos = TopRight(rect), .blend = color_topright, .uv = {1.0f, 0.0f}},
                    {.pos = BottomRight(rect), .blend = color_bottomright, .uv = {1.0f, 1.0f}},
                }},
            },
            {
                .verts = {{
                    {.pos = BottomRight(rect), .blend = color_bottomright, .uv = {1.0f, 1.0f}},
                    {.pos = BottomLeft(rect), .blend = color_bottomleft, .uv = {0.0f, 1.0f}},
                    {.pos = TopLeft(rect), .blend = color_topleft, .uv = {0.0f, 0.0f}},
                }},
            },
        }};

        SubmitTrianglesToBatch(rc, AsNonstatic(triangles), nullptr);
    }

    inline void RenderRect(s_rendering_context *const rc, const s_rect_f rect, const s_color_rgba32f color) {
        RenderRect(rc, rect, color, color, color, color);
    }

    // ============================================================
}
