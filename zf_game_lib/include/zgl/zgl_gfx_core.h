#pragma once

#include <zcl.h>

namespace zf {
    // ============================================================
    // @section: General
    // ============================================================
    struct s_rendering_basis;

    // This depends on the platform module being initialised beforehand.
    s_rendering_basis &StartupGFXModule(c_mem_arena &mem_arena);

    void ShutdownGFXModule(s_rendering_basis &rendering_basis);

    // ============================================================
    // @section: Resources
    // ============================================================
    struct s_gfx_resource;

    s_v2_i TextureSize(const s_gfx_resource &texture);

    struct s_gfx_resource_arena {
        s_ptr<c_mem_arena> mem_arena = nullptr;
        s_ptr<s_gfx_resource> head = nullptr;
        s_ptr<s_gfx_resource> tail = nullptr;
    };

    s_gfx_resource_arena &PermGFXResourceArena();

    void ReleaseGFXResources(s_gfx_resource_arena &arena);

    s_ptr<s_gfx_resource> CreateTextureResource(const s_texture_data_rdonly texture_data, s_gfx_resource_arena &arena = PermGFXResourceArena());

    inline s_ptr<s_gfx_resource> CreateTextureResourceFromRaw(const s_str_rdonly file_path, c_mem_arena &temp_mem_arena, s_gfx_resource_arena &arena = PermGFXResourceArena()) {
        s_texture_data texture_data;

        if (!LoadTextureDataFromRaw(file_path, temp_mem_arena, temp_mem_arena, texture_data)) {
            ZF_FATAL();
        }

        return CreateTextureResource(texture_data, arena);
    }

    inline s_ptr<s_gfx_resource> CreateTextureResourceFromPacked(const s_str_rdonly file_path, c_mem_arena &temp_mem_arena, s_gfx_resource_arena &arena = PermGFXResourceArena()) {
        s_texture_data texture_data;

        if (!UnpackTexture(file_path, temp_mem_arena, temp_mem_arena, texture_data)) {
            ZF_FATAL();
        }

        return CreateTextureResource(texture_data, arena);
    }

    s_ptr<s_gfx_resource> CreateShaderProgResource(const c_array_rdonly<t_u8> vert_shader_compiled_bin, const c_array_rdonly<t_u8> frag_shader_compiled_bin, s_gfx_resource_arena &arena = PermGFXResourceArena());

    inline s_ptr<s_gfx_resource> CreateShaderProgResourceFromPacked(const s_str_rdonly vert_shader_file_path, const s_str_rdonly frag_shader_file_path, c_mem_arena &temp_mem_arena, s_gfx_resource_arena &arena = PermGFXResourceArena()) {
        c_array_mut<t_u8> vert_shader_compiled_bin;

        if (!UnpackShader(vert_shader_file_path, temp_mem_arena, temp_mem_arena, vert_shader_compiled_bin)) {
            ZF_FATAL();
        }

        c_array_mut<t_u8> frag_shader_compiled_bin;

        if (!UnpackShader(frag_shader_file_path, temp_mem_arena, temp_mem_arena, frag_shader_compiled_bin)) {
            ZF_FATAL();
        }

        return CreateShaderProgResource(vert_shader_compiled_bin, frag_shader_compiled_bin, arena);
    }

    // ============================================================
    // @section: Rendering
    // ============================================================
    struct s_rendering_context;

    s_rendering_context &BeginRendering(const s_rendering_basis &rendering_basis, const s_color_rgb24f clear_col, c_mem_arena &rendering_context_mem_arena);
    void EndRendering(s_rendering_context &rendering_context);

    struct s_rendering_vert {
        s_v2 pos = {};
        s_color_rgba32f blend = {};
        s_v2 uv = {};
    };

    struct s_render_triangle {
        s_static_array<s_rendering_vert, 3> verts = {};
    };

    void RenderTriangles(s_rendering_context &rendering_context, const c_array_rdonly<s_render_triangle> triangles, const s_ptr<const s_gfx_resource> texture);

    inline void RenderTriangle(s_rendering_context &rendering_context, const s_static_array<s_v2, 3> &pts, const s_static_array<s_color_rgba32f, 3> &pt_colors) {
        const s_render_triangle triangle = {
            .verts = {{
                {.pos = pts[0], .blend = pt_colors[0], .uv = {}},
                {.pos = pts[1], .blend = pt_colors[1], .uv = {}},
                {.pos = pts[2], .blend = pt_colors[2], .uv = {}},
            }},
        };

        RenderTriangles(rendering_context, {&triangle, 1}, nullptr);
    }

    inline void RenderTriangle(s_rendering_context &rendering_context, const s_static_array<s_v2, 3> &pts, const s_color_rgba32f color) {
        RenderTriangle(rendering_context, pts, {{color, color, color}});
    }

    inline void RenderRect(s_rendering_context &rendering_context, const s_rect_f rect, const s_color_rgba32f color_topleft, const s_color_rgba32f color_topright, const s_color_rgba32f color_bottomright, const s_color_rgba32f color_bottomleft) {
        const s_static_array<s_render_triangle, 2> triangles = {{
            {
                .verts = {{
                    {.pos = rect.TopLeft(), .blend = color_topleft, .uv = {0.0f, 0.0f}},
                    {.pos = rect.TopRight(), .blend = color_topright, .uv = {1.0f, 0.0f}},
                    {.pos = rect.BottomRight(), .blend = color_bottomright, .uv = {1.0f, 1.0f}},
                }},
            },
            {
                .verts = {{
                    {.pos = rect.BottomRight(), .blend = color_bottomright, .uv = {1.0f, 1.0f}},
                    {.pos = rect.BottomLeft(), .blend = color_bottomleft, .uv = {0.0f, 1.0f}},
                    {.pos = rect.TopLeft(), .blend = color_topleft, .uv = {0.0f, 0.0f}},
                }},
            },
        }};

        RenderTriangles(rendering_context, triangles.ToNonstatic(), nullptr);
    }

    inline void RenderRect(s_rendering_context &rendering_context, const s_rect_f rect, const s_color_rgba32f color) {
        RenderRect(rendering_context, rect, color, color, color, color);
    }
}
