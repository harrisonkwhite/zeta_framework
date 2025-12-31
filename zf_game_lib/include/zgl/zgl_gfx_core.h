#pragma once

#include <zcl.h>

namespace zf::gfx {
    struct s_rendering_basis;

    // This depends on the platform module being initialised beforehand.
    // Returns a pointer to a rendering basis, needed for all rendering operations.
    s_rendering_basis *startup(s_arena *const arena);

    void shutdown(const s_rendering_basis *const rendering_basis);


    // ============================================================
    // @section: Resources

    struct s_resource;

    struct s_resource_group {
        s_arena *arena;
        s_resource *head;
        s_resource *tail;
    };

    inline s_resource_group resource_group_create(s_arena *const arena) {
        return {.arena = arena};
    }

    void resource_group_destroy(s_resource_group *const group);

    s_resource *texture_create(const s_texture_data_rdonly texture_data, s_resource_group *const group);

    inline s_resource *texture_create_from_raw(const s_str_rdonly file_path, s_arena *const temp_arena, s_resource_group *const group) {
        s_texture_data texture_data;

        if (!LoadTextureDataFromRaw(file_path, temp_arena, temp_arena, &texture_data)) {
            ZF_FATAL();
        }

        return texture_create(texture_data, group);
    }

    inline s_resource *texture_create_from_packed(const s_str_rdonly file_path, s_arena *const temp_arena, s_resource_group *const group) {
        s_texture_data texture_data;

        if (!UnpackTexture(file_path, temp_arena, temp_arena, &texture_data)) {
            ZF_FATAL();
        }

        return texture_create(texture_data, group);
    }

    s_v2_i texture_get_size(const s_resource *const texture);

    s_resource *shader_prog_create(const s_array_rdonly<t_u8> vert_shader_compiled_bin, const s_array_rdonly<t_u8> frag_shader_compiled_bin, s_resource_group *const group);

    inline s_resource *shader_prog_create_from_packed(const s_str_rdonly vert_shader_file_path, const s_str_rdonly frag_shader_file_path, s_arena *const temp_arena, s_resource_group *const arena) {
        s_array_mut<t_u8> vert_shader_compiled_bin;

        if (!UnpackShader(vert_shader_file_path, temp_arena, temp_arena, &vert_shader_compiled_bin)) {
            ZF_FATAL();
        }

        s_array_mut<t_u8> frag_shader_compiled_bin;

        if (!UnpackShader(frag_shader_file_path, temp_arena, temp_arena, &frag_shader_compiled_bin)) {
            ZF_FATAL();
        }

        return shader_prog_create(vert_shader_compiled_bin, frag_shader_compiled_bin, arena);
    }

    // ============================================================


    // ============================================================
    // @section: Rendering

    struct s_rendering_context;

    s_rendering_context *rendering_begin(const s_rendering_basis *const rendering_basis, const s_color_rgb8 clear_col, s_arena *const rendering_context_arena);
    void rendering_end(s_rendering_context *const rendering_context);

    struct s_batch_vert {
        s_v2 pos;
        s_color_rgba32f blend;
        s_v2 uv;
    };

    struct s_batch_triangle {
        s_static_array<s_batch_vert, 3> verts;
    };

    // Leave texture as nullptr for no texture.
    void rendering_submit_triangles(s_rendering_context *const rc, const s_array_rdonly<s_batch_triangle> triangles, const s_resource *const texture);

    inline void rendering_submit_triangle(s_rendering_context *const rc, const s_static_array<s_v2, 3> &pts, const s_static_array<s_color_rgba32f, 3> &pt_colors) {
        const s_batch_triangle triangle = {
            .verts = {{
                {.pos = pts[0], .blend = pt_colors[0], .uv = {}},
                {.pos = pts[1], .blend = pt_colors[1], .uv = {}},
                {.pos = pts[2], .blend = pt_colors[2], .uv = {}},
            }},
        };

        rendering_submit_triangles(rc, {&triangle, 1}, nullptr);
    }

    inline void rendering_submit_triangle(s_rendering_context *const rc, const s_static_array<s_v2, 3> &pts, const s_color_rgba32f color) {
        rendering_submit_triangle(rc, pts, {{color, color, color}});
    }

    inline void rendering_submit_rect(s_rendering_context *const rc, const s_rect_f rect, const s_color_rgba32f color_topleft, const s_color_rgba32f color_topright, const s_color_rgba32f color_bottomright, const s_color_rgba32f color_bottomleft) {
        ZF_ASSERT(rect.width > 0.0f && rect.height > 0.0f);

        const s_static_array<s_batch_triangle, 2> triangles = {{
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

        rendering_submit_triangles(rc, triangles.AsNonstatic(), nullptr);
    }

    inline void rendering_submit_rect(s_rendering_context *const rc, const s_rect_f rect, const s_color_rgba32f color) {
        rendering_submit_rect(rc, rect, color, color, color, color);
    }

    // ============================================================
}
