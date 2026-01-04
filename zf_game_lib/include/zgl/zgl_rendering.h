#pragma once

#include <zcl.h>

namespace zf {
    // ============================================================
    // @section: Types and Globals

    struct t_rendering_resource;

    struct t_rendering_resource_group {
        s_arena *arena;
        t_rendering_resource *head;
        t_rendering_resource *tail;
    };

    struct t_rendering_basis;

    struct t_rendering_context;

    struct t_batch_vertex {
        s_v2 pos;
        gfx::ColorRGBA32F blend;
        s_v2 uv;
    };

    struct t_batch_triangle {
        s_static_array<t_batch_vertex, 3> verts;
    };

    // ============================================================


    // ============================================================
    // @section: Functions

    // This depends on the platform module being initialised beforehand.
    // Returns a pointer to a rendering basis, needed for all rendering operations.
    t_rendering_basis *f_rendering_startup_module(s_arena *const arena, t_rendering_resource_group **const o_perm_resource_group);

    void f_rendering_shutdown_module(const t_rendering_basis *const basis);

    inline t_rendering_resource_group f_rendering_create_resource_group(s_arena *const arena, t_rendering_resource_group **const o_perm_group) {
        return {.arena = arena};
    }

    void f_rendering_destroy_resource_group(t_rendering_resource_group *const group);

    t_rendering_resource *f_rendering_create_texture(const gfx::TextureDataRdonly texture_data, t_rendering_resource_group *const group);

    inline t_rendering_resource *f_rendering_create_texture_from_raw(const strs::StrRdonly file_path, s_arena *const temp_arena, t_rendering_resource_group *const group) {
        gfx::TextureDataMut texture_data;

        if (!gfx::load_texture_from_raw(file_path, temp_arena, temp_arena, &texture_data)) {
            ZF_FATAL();
        }

        return f_rendering_create_texture(texture_data, group);
    }

    inline t_rendering_resource *f_rendering_create_texture_from_packed(const strs::StrRdonly file_path, s_arena *const temp_arena, t_rendering_resource_group *const group) {
        gfx::TextureDataMut texture_data;

        if (!unpack_texture(file_path, temp_arena, temp_arena, &texture_data)) {
            ZF_FATAL();
        }

        return f_rendering_create_texture(texture_data, group);
    }

    s_v2_i f_rendering_get_texture_size(const t_rendering_resource *const texture);

    t_rendering_resource *f_rendering_create_shader_prog(const s_array_rdonly<U8> vert_shader_compiled_bin, const s_array_rdonly<U8> frag_shader_compiled_bin, t_rendering_resource_group *const group);

    inline t_rendering_resource *f_rendering_create_shader_prog_from_packed(const strs::StrRdonly vert_shader_file_path, const strs::StrRdonly frag_shader_file_path, s_arena *const temp_arena, t_rendering_resource_group *const arena) {
        s_array_mut<U8> vert_shader_compiled_bin;

        if (!gfx::unpack_shader(vert_shader_file_path, temp_arena, temp_arena, &vert_shader_compiled_bin)) {
            ZF_FATAL();
        }

        s_array_mut<U8> frag_shader_compiled_bin;

        if (!gfx::unpack_shader(frag_shader_file_path, temp_arena, temp_arena, &frag_shader_compiled_bin)) {
            ZF_FATAL();
        }

        return f_rendering_create_shader_prog(vert_shader_compiled_bin, frag_shader_compiled_bin, arena);
    }

    t_rendering_context *f_rendering_begin_frame(const t_rendering_basis *const basis, const gfx::ColorRGB8 clear_col, s_arena *const context_arena);
    void f_rendering_end_frame(t_rendering_context *const context);

    // Leave texture as nullptr for no texture.
    void f_rendering_submit_triangle(t_rendering_context *const context, const s_array_rdonly<t_batch_triangle> triangles, const t_rendering_resource *const texture);

    inline void f_rendering_submit_triangle(t_rendering_context *const context, const s_static_array<s_v2, 3> &pts, const s_static_array<gfx::ColorRGBA32F, 3> &pt_colors) {
        const t_batch_triangle triangle = {
            .verts = {{
                {.pos = pts[0], .blend = pt_colors[0], .uv = {}},
                {.pos = pts[1], .blend = pt_colors[1], .uv = {}},
                {.pos = pts[2], .blend = pt_colors[2], .uv = {}},
            }},
        };

        f_rendering_submit_triangle(context, {&triangle, 1}, nullptr);
    }

    inline void f_rendering_submit_triangle(t_rendering_context *const context, const s_static_array<s_v2, 3> &pts, const gfx::ColorRGBA32F color) {
        f_rendering_submit_triangle(context, pts, {{color, color, color}});
    }

    inline void f_rendering_submit_rect(t_rendering_context *const context, const s_rect_f rect, const gfx::ColorRGBA32F color_topleft, const gfx::ColorRGBA32F color_topright, const gfx::ColorRGBA32F color_bottomright, const gfx::ColorRGBA32F color_bottomleft) {
        ZF_ASSERT(rect.width > 0.0f && rect.height > 0.0f);

        const s_static_array<t_batch_triangle, 2> triangles = {{
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

        f_rendering_submit_triangle(context, AsNonstatic(triangles), nullptr);
    }

    inline void f_rendering_submit_rect(t_rendering_context *const context, const s_rect_f rect, const gfx::ColorRGBA32F color) {
        f_rendering_submit_rect(context, rect, color, color, color, color);
    }

    void f_rendering_submit_texture(t_rendering_context *const context, const t_rendering_resource *const texture, const s_v2 pos, const s_rect_i src_rect = {});

    struct t_font {
        gfx::FontArrangement arrangement;
        s_array_mut<t_rendering_resource *> atlases;
    };

    t_font f_rendering_create_font_from_raw(const strs::StrRdonly file_path, const I32 height, strs::CodePointBitVector *const code_pts, s_arena *const temp_arena, t_rendering_resource_group *const resource_group);
    t_font f_rendering_create_font_from_packed(const strs::StrRdonly file_path, s_arena *const temp_arena, t_rendering_resource_group *const resource_group);

    s_array_mut<s_v2> f_rendering_get_str_chr_render_positions(const strs::StrRdonly str, const gfx::FontArrangement &font_arrangement, const s_v2 pos, const s_v2 alignment, s_arena *const arena);

    void f_rendering_submit_str(t_rendering_context *const context, const strs::StrRdonly str, const t_font &font, const s_v2 pos, s_arena *const temp_arena, const s_v2 alignment = gfx::g_alignment_topleft, const gfx::ColorRGBA32F blend = gfx::g_color_white);

    // ============================================================
}
