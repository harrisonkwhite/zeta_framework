#pragma once

#include <zcl.h>

namespace zf::rendering {
    // ============================================================
    // @section: Types and Globals

    struct t_resource;

    struct t_resource_group {
        mem::t_arena *arena;
        t_resource *head;
        t_resource *tail;
    };

    struct t_basis;

    struct t_context;

    struct t_batch_vertex {
        math::t_v2 pos;
        gfx::t_color_rgba32f blend;
        math::t_v2 uv;
    };

    struct t_batch_triangle {
        t_static_array<t_batch_vertex, 3> verts;
    };

    // ============================================================


    // ============================================================
    // @section: Functions

    // This depends on the platform module being initialised beforehand.
    // Returns a pointer to a rendering basis, needed for all rendering operations.
    t_basis *f_startup_module(mem::t_arena *const arena, t_resource_group **const o_perm_resource_group);

    void f_shutdown_module(const t_basis *const basis);

    inline t_resource_group f_create_resource_group(mem::t_arena *const arena, t_resource_group **const o_perm_group) {
        return {.arena = arena};
    }

    void f_destroy_resource_group(t_resource_group *const group);

    t_resource *f_create_texture(const gfx::t_texture_data_rdonly texture_data, t_resource_group *const group);

    inline t_resource *f_create_texture_from_raw(const strs::t_str_rdonly file_path, mem::t_arena *const temp_arena, t_resource_group *const group) {
        gfx::t_texture_data_mut texture_data;

        if (!gfx::f_load_texture_from_raw(file_path, temp_arena, temp_arena, &texture_data)) {
            ZF_FATAL();
        }

        return f_create_texture(texture_data, group);
    }

    inline t_resource *f_create_texture_from_packed(const strs::t_str_rdonly file_path, mem::t_arena *const temp_arena, t_resource_group *const group) {
        gfx::t_texture_data_mut texture_data;

        if (!gfx::f_unpack_texture(file_path, temp_arena, temp_arena, &texture_data)) {
            ZF_FATAL();
        }

        return f_create_texture(texture_data, group);
    }

    math::t_v2_i f_get_texture_size(const t_resource *const texture);

    t_resource *f_create_shader_prog(const t_array_rdonly<t_u8> vert_shader_compiled_bin, const t_array_rdonly<t_u8> frag_shader_compiled_bin, t_resource_group *const group);

    inline t_resource *f_create_shader_prog_from_packed(const strs::t_str_rdonly vert_shader_file_path, const strs::t_str_rdonly frag_shader_file_path, mem::t_arena *const temp_arena, t_resource_group *const arena) {
        t_array_mut<t_u8> vert_shader_compiled_bin;

        if (!gfx::f_unpack_shader(vert_shader_file_path, temp_arena, temp_arena, &vert_shader_compiled_bin)) {
            ZF_FATAL();
        }

        t_array_mut<t_u8> frag_shader_compiled_bin;

        if (!gfx::f_unpack_shader(frag_shader_file_path, temp_arena, temp_arena, &frag_shader_compiled_bin)) {
            ZF_FATAL();
        }

        return f_create_shader_prog(vert_shader_compiled_bin, frag_shader_compiled_bin, arena);
    }

    t_context *f_begin_frame(const t_basis *const basis, const gfx::t_color_rgb8 clear_col, mem::t_arena *const context_arena);
    void f_end_frame(t_context *const context);

    // Leave texture as nullptr for no texture.
    void f_submit_triangle(t_context *const context, const t_array_rdonly<t_batch_triangle> triangles, const t_resource *const texture);

    inline void f_submit_triangle(t_context *const context, const t_static_array<math::t_v2, 3> &pts, const t_static_array<gfx::t_color_rgba32f, 3> &pt_colors) {
        const t_batch_triangle triangle = {
            .verts = {{
                {.pos = pts[0], .blend = pt_colors[0], .uv = {}},
                {.pos = pts[1], .blend = pt_colors[1], .uv = {}},
                {.pos = pts[2], .blend = pt_colors[2], .uv = {}},
            }},
        };

        f_submit_triangle(context, {&triangle, 1}, nullptr);
    }

    inline void f_submit_triangle(t_context *const context, const t_static_array<math::t_v2, 3> &pts, const gfx::t_color_rgba32f color) {
        f_submit_triangle(context, pts, {{color, color, color}});
    }

    inline void f_submit_rect(t_context *const context, const math::t_rect_f rect, const gfx::t_color_rgba32f color_topleft, const gfx::t_color_rgba32f color_topright, const gfx::t_color_rgba32f color_bottomright, const gfx::t_color_rgba32f color_bottomleft) {
        ZF_ASSERT(rect.width > 0.0f && rect.height > 0.0f);

        const t_static_array<t_batch_triangle, 2> triangles = {{
            {
                .verts = {{
                    {.pos = math::f_get_rect_topleft(rect), .blend = color_topleft, .uv = {0.0f, 0.0f}},
                    {.pos = math::f_get_rect_topright(rect), .blend = color_topright, .uv = {1.0f, 0.0f}},
                    {.pos = math::f_get_rect_bottomleft(rect), .blend = color_bottomright, .uv = {1.0f, 1.0f}},
                }},
            },
            {
                .verts = {{
                    {.pos = math::f_get_rect_bottomleft(rect), .blend = color_bottomright, .uv = {1.0f, 1.0f}},
                    {.pos = math::f_get_rect_bottomleft(rect), .blend = color_bottomleft, .uv = {0.0f, 1.0f}},
                    {.pos = math::f_get_rect_topleft(rect), .blend = color_topleft, .uv = {0.0f, 0.0f}},
                }},
            },
        }};

        f_submit_triangle(context, f_array_get_as_nonstatic(triangles), nullptr);
    }

    inline void f_submit_rect(t_context *const context, const math::t_rect_f rect, const gfx::t_color_rgba32f color) {
        f_submit_rect(context, rect, color, color, color, color);
    }

    void f_submit_texture(t_context *const context, const t_resource *const texture, const math::t_v2 pos, const math::t_rect_i src_rect = {});

    struct t_font {
        gfx::t_font_arrangement arrangement;
        t_array_mut<t_resource *> atlases;
    };

    t_font f_create_font_from_raw(const strs::t_str_rdonly file_path, const t_i32 height, strs::t_code_pt_bit_vec *const code_pts, mem::t_arena *const temp_arena, t_resource_group *const resource_group);
    t_font f_create_font_from_packed(const strs::t_str_rdonly file_path, mem::t_arena *const temp_arena, t_resource_group *const resource_group);

    t_array_mut<math::t_v2> f_get_str_chr_render_positions(const strs::t_str_rdonly str, const gfx::t_font_arrangement &font_arrangement, const math::t_v2 pos, const math::t_v2 alignment, mem::t_arena *const arena);

    void f_submit_str(t_context *const context, const strs::t_str_rdonly str, const t_font &font, const math::t_v2 pos, mem::t_arena *const temp_arena, const math::t_v2 alignment = gfx::g_alignment_topleft, const gfx::t_color_rgba32f blend = gfx::g_color_white);

    // ============================================================
}
