#pragma once

#include <zcl.h>

namespace zf::rendering {
    // ============================================================
    // @section: Types and Globals

    struct Resource;

    struct ResourceGroup {
        s_arena *arena;
        Resource *head;
        Resource *tail;
    };

    struct Basis;

    struct Context;

    struct BatchVertex {
        s_v2 pos;
        gfx::ColorRGBA32F blend;
        s_v2 uv;
    };

    struct BatchTriangle {
        s_static_array<BatchVertex, 3> verts;
    };

    // ============================================================


    // ============================================================
    // @section: Functions

    // This depends on the platform module being initialised beforehand.
    // Returns a pointer to a rendering basis, needed for all rendering operations.
    Basis *startup_module(s_arena *const arena, ResourceGroup **const o_perm_resource_group);

    void shutdown_module(const Basis *const basis);

    inline ResourceGroup create_resource_group(s_arena *const arena, ResourceGroup **const o_perm_group) {
        return {.arena = arena};
    }

    void destroy_resource_group(ResourceGroup *const group);

    Resource *create_texture(const gfx::TextureDataRdonly texture_data, ResourceGroup *const group);

    inline Resource *create_texture_from_raw(const strs::StrRdonly file_path, s_arena *const temp_arena, ResourceGroup *const group) {
        gfx::TextureDataMut texture_data;

        if (!gfx::load_texture_from_raw(file_path, temp_arena, temp_arena, &texture_data)) {
            ZF_FATAL();
        }

        return create_texture(texture_data, group);
    }

    inline Resource *create_texture_from_packed(const strs::StrRdonly file_path, s_arena *const temp_arena, ResourceGroup *const group) {
        gfx::TextureDataMut texture_data;

        if (!unpack_texture(file_path, temp_arena, temp_arena, &texture_data)) {
            ZF_FATAL();
        }

        return create_texture(texture_data, group);
    }

    s_v2_i get_texture_size(const Resource *const texture);

    Resource *create_shader_prog(const s_array_rdonly<U8> vert_shader_compiled_bin, const s_array_rdonly<U8> frag_shader_compiled_bin, ResourceGroup *const group);

    inline Resource *create_shader_prog_from_packed(const strs::StrRdonly vert_shader_file_path, const strs::StrRdonly frag_shader_file_path, s_arena *const temp_arena, ResourceGroup *const arena) {
        s_array_mut<U8> vert_shader_compiled_bin;

        if (!gfx::unpack_shader(vert_shader_file_path, temp_arena, temp_arena, &vert_shader_compiled_bin)) {
            ZF_FATAL();
        }

        s_array_mut<U8> frag_shader_compiled_bin;

        if (!gfx::unpack_shader(frag_shader_file_path, temp_arena, temp_arena, &frag_shader_compiled_bin)) {
            ZF_FATAL();
        }

        return create_shader_prog(vert_shader_compiled_bin, frag_shader_compiled_bin, arena);
    }

    Context *begin_frame(const Basis *const basis, const gfx::ColorRGB8 clear_col, s_arena *const context_arena);
    void end_frame(Context *const context);

    // Leave texture as nullptr for no texture.
    void submit_triangle(Context *const context, const s_array_rdonly<BatchTriangle> triangles, const Resource *const texture);

    inline void submit_triangle(Context *const context, const s_static_array<s_v2, 3> &pts, const s_static_array<gfx::ColorRGBA32F, 3> &pt_colors) {
        const BatchTriangle triangle = {
            .verts = {{
                {.pos = pts[0], .blend = pt_colors[0], .uv = {}},
                {.pos = pts[1], .blend = pt_colors[1], .uv = {}},
                {.pos = pts[2], .blend = pt_colors[2], .uv = {}},
            }},
        };

        submit_triangle(context, {&triangle, 1}, nullptr);
    }

    inline void submit_triangle(Context *const context, const s_static_array<s_v2, 3> &pts, const gfx::ColorRGBA32F color) {
        submit_triangle(context, pts, {{color, color, color}});
    }

    inline void submit_rect(Context *const context, const s_rect_f rect, const gfx::ColorRGBA32F color_topleft, const gfx::ColorRGBA32F color_topright, const gfx::ColorRGBA32F color_bottomright, const gfx::ColorRGBA32F color_bottomleft) {
        ZF_ASSERT(rect.width > 0.0f && rect.height > 0.0f);

        const s_static_array<BatchTriangle, 2> triangles = {{
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

        submit_triangle(context, AsNonstatic(triangles), nullptr);
    }

    inline void submit_rect(Context *const context, const s_rect_f rect, const gfx::ColorRGBA32F color) {
        submit_rect(context, rect, color, color, color, color);
    }

    void submit_texture(Context *const context, const Resource *const texture, const s_v2 pos, const s_rect_i src_rect = {});

    struct Font {
        gfx::FontArrangement arrangement;
        s_array_mut<Resource *> atlases;
    };

    Font create_font_from_raw(const strs::StrRdonly file_path, const I32 height, strs::CodePointBitVector *const code_pts, s_arena *const temp_arena, ResourceGroup *const resource_group);
    Font create_font_from_packed(const strs::StrRdonly file_path, s_arena *const temp_arena, ResourceGroup *const resource_group);

    s_array_mut<s_v2> calc_str_chr_render_positions(const strs::StrRdonly str, const gfx::FontArrangement &font_arrangement, const s_v2 pos, const s_v2 alignment, s_arena *const arena);

    void submit_str(Context *const context, const strs::StrRdonly str, const Font &font, const s_v2 pos, s_arena *const temp_arena, const s_v2 alignment = gfx::g_alignment_topleft, const gfx::ColorRGBA32F blend = gfx::g_color_white);

    // ============================================================
}
