#pragma once

#include <zcl.h>

namespace zf {
    // ============================================================
    // @section: Types and Globals

    struct t_rendering_resource;

    struct t_rendering_resource_group {
        mem::t_arena *arena;
        t_rendering_resource *head;
        t_rendering_resource *tail;
    };

    struct t_rendering_basis;

    struct t_rendering_context;

    struct t_batch_vertex {
        t_v2 pos;
        gfx::t_color_rgba32f blend;
        t_v2 uv;
    };

    struct t_batch_triangle {
        t_static_array<t_batch_vertex, 3> verts;
    };

    // ============================================================


    // ============================================================
    // @section: Functions

    // This depends on the platform module being initialised beforehand.
    // Returns a pointer to a rendering basis, needed for all rendering operations.
    t_rendering_basis *f_rendering_startup_module(mem::t_arena *const arena, t_rendering_resource_group **const o_perm_resource_group);

    void f_rendering_shutdown_module(const t_rendering_basis *const basis);

    inline t_rendering_resource_group f_rendering_create_resource_group(mem::t_arena *const arena, t_rendering_resource_group **const o_perm_group) {
        return {.arena = arena};
    }

    void f_rendering_destroy_resource_group(t_rendering_resource_group *const group);

    t_rendering_resource *f_rendering_create_texture(const gfx::t_texture_data_rdonly texture_data, t_rendering_resource_group *const group);

    inline t_rendering_resource *f_rendering_create_texture_from_raw(const t_str_rdonly file_path, mem::t_arena *const temp_arena, t_rendering_resource_group *const group) {
        gfx::t_texture_data_mut texture_data;

        if (!gfx::f_load_texture_from_raw(file_path, temp_arena, temp_arena, &texture_data)) {
            ZF_FATAL();
        }

        return f_rendering_create_texture(texture_data, group);
    }

    inline t_rendering_resource *f_rendering_create_texture_from_packed(const t_str_rdonly file_path, mem::t_arena *const temp_arena, t_rendering_resource_group *const group) {
        gfx::t_texture_data_mut texture_data;

        if (!gfx::f_unpack_texture(file_path, temp_arena, temp_arena, &texture_data)) {
            ZF_FATAL();
        }

        return f_rendering_create_texture(texture_data, group);
    }

    t_v2_i f_rendering_get_texture_size(const t_rendering_resource *const texture);

    t_rendering_resource *f_rendering_create_shader_prog(const t_array_rdonly<t_u8> vert_shader_compiled_bin, const t_array_rdonly<t_u8> frag_shader_compiled_bin, t_rendering_resource_group *const group);

    inline t_rendering_resource *f_rendering_create_shader_prog_from_packed(const t_str_rdonly vert_shader_file_path, const t_str_rdonly frag_shader_file_path, mem::t_arena *const temp_arena, t_rendering_resource_group *const arena) {
        t_array_mut<t_u8> vert_shader_compiled_bin;

        if (!gfx::f_unpack_shader(vert_shader_file_path, temp_arena, temp_arena, &vert_shader_compiled_bin)) {
            ZF_FATAL();
        }

        t_array_mut<t_u8> frag_shader_compiled_bin;

        if (!gfx::f_unpack_shader(frag_shader_file_path, temp_arena, temp_arena, &frag_shader_compiled_bin)) {
            ZF_FATAL();
        }

        return f_rendering_create_shader_prog(vert_shader_compiled_bin, frag_shader_compiled_bin, arena);
    }

    t_rendering_context *f_rendering_begin_frame(const t_rendering_basis *const basis, const gfx::t_color_rgb8 clear_col, mem::t_arena *const context_arena);
    void f_rendering_end_frame(t_rendering_context *const context);

    // Leave texture as nullptr for no texture.
    void f_rendering_submit_triangle(t_rendering_context *const context, const t_array_rdonly<t_batch_triangle> triangles, const t_rendering_resource *const texture);

    inline void f_rendering_submit_triangle(t_rendering_context *const context, const t_static_array<t_v2, 3> &pts, const t_static_array<gfx::t_color_rgba32f, 3> &pt_colors) {
        const t_batch_triangle triangle = {
            .verts = {{
                {.pos = pts[0], .blend = pt_colors[0], .uv = {}},
                {.pos = pts[1], .blend = pt_colors[1], .uv = {}},
                {.pos = pts[2], .blend = pt_colors[2], .uv = {}},
            }},
        };

        f_rendering_submit_triangle(context, {&triangle, 1}, nullptr);
    }

    inline void f_rendering_submit_triangle(t_rendering_context *const context, const t_static_array<t_v2, 3> &pts, const gfx::t_color_rgba32f color) {
        f_rendering_submit_triangle(context, pts, {{color, color, color}});
    }

    inline void f_rendering_submit_rect(t_rendering_context *const context, const t_rect_f rect, const gfx::t_color_rgba32f color_topleft, const gfx::t_color_rgba32f color_topright, const gfx::t_color_rgba32f color_bottomright, const gfx::t_color_rgba32f color_bottomleft) {
        ZF_ASSERT(rect.width > 0.0f && rect.height > 0.0f);

        const t_static_array<t_batch_triangle, 2> triangles = {{
            {
                .verts = {{
                    {.pos = f_math_get_rect_topleft(rect), .blend = color_topleft, .uv = {0.0f, 0.0f}},
                    {.pos = f_math_get_rect_topright(rect), .blend = color_topright, .uv = {1.0f, 0.0f}},
                    {.pos = f_math_get_rect_bottomleft(rect), .blend = color_bottomright, .uv = {1.0f, 1.0f}},
                }},
            },
            {
                .verts = {{
                    {.pos = f_math_get_rect_bottomleft(rect), .blend = color_bottomright, .uv = {1.0f, 1.0f}},
                    {.pos = f_math_get_rect_bottomleft(rect), .blend = color_bottomleft, .uv = {0.0f, 1.0f}},
                    {.pos = f_math_get_rect_topleft(rect), .blend = color_topleft, .uv = {0.0f, 0.0f}},
                }},
            },
        }};

        f_rendering_submit_triangle(context, f_array_get_as_nonstatic(triangles), nullptr);
    }

    inline void f_rendering_submit_rect(t_rendering_context *const context, const t_rect_f rect, const gfx::t_color_rgba32f color) {
        f_rendering_submit_rect(context, rect, color, color, color, color);
    }

    void f_rendering_submit_texture(t_rendering_context *const context, const t_rendering_resource *const texture, const t_v2 pos, const t_rect_i src_rect = {});

    struct t_font {
        gfx::t_font_arrangement arrangement;
        t_array_mut<t_rendering_resource *> atlases;
    };

    t_font f_rendering_create_font_from_raw(const t_str_rdonly file_path, const t_i32 height, t_code_pt_bit_vec *const code_pts, mem::t_arena *const temp_arena, t_rendering_resource_group *const resource_group);
    t_font f_rendering_create_font_from_packed(const t_str_rdonly file_path, mem::t_arena *const temp_arena, t_rendering_resource_group *const resource_group);

    t_array_mut<t_v2> f_rendering_get_str_chr_render_positions(const t_str_rdonly str, const gfx::t_font_arrangement &font_arrangement, const t_v2 pos, const t_v2 alignment, mem::t_arena *const arena);

    void f_rendering_submit_str(t_rendering_context *const context, const t_str_rdonly str, const t_font &font, const t_v2 pos, mem::t_arena *const temp_arena, const t_v2 alignment = gfx::g_gfx_alignment_topleft, const gfx::t_color_rgba32f blend = gfx::g_gfx_color_white);

    // ============================================================
}
