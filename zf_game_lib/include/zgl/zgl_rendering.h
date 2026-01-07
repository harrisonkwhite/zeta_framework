#pragma once

#include <zcl.h>

namespace zf::rendering {
    struct t_resource;

    struct t_resource_group {
        mem::t_arena *arena;
        t_resource *head;
        t_resource *tail;
    };

    struct t_basis;

    // This depends on the platform module being initialised beforehand.
    // Returns a pointer to a rendering basis, needed for all rendering operations.
    t_basis *module_startup(mem::t_arena *const arena, mem::t_arena *const temp_arena, t_resource_group **const o_perm_resource_group);

    void module_shutdown(const t_basis *const basis);

    inline t_resource_group resource_group_create(mem::t_arena *const arena) {
        return {.arena = arena};
    }

    void resource_group_destroy(t_resource_group *const group);

    t_resource *texture_create(const gfx::t_texture_data_rdonly texture_data, t_resource_group *const group);

    inline t_resource *texture_create_from_raw(const strs::t_str_rdonly file_path, mem::t_arena *const temp_arena, t_resource_group *const group) {
        gfx::t_texture_data_mut texture_data;

        if (!gfx::texture_load_from_raw(file_path, temp_arena, temp_arena, &texture_data)) {
            ZF_FATAL();
        }

        return texture_create(texture_data, group);
    }

    inline t_resource *texture_create_from_packed(const strs::t_str_rdonly file_path, mem::t_arena *const temp_arena, t_resource_group *const group) {
        gfx::t_texture_data_mut texture_data;

        if (!gfx::texture_unpack(file_path, temp_arena, temp_arena, &texture_data)) {
            ZF_FATAL();
        }

        return texture_create(texture_data, group);
    }

    math::t_v2_i texture_get_size(const t_resource *const texture);

    t_resource *shader_prog_create(const t_array_rdonly<t_u8> vert_shader_compiled_bin, const t_array_rdonly<t_u8> frag_shader_compiled_bin, t_resource_group *const group);

    inline t_resource *shader_prog_create_from_packed(const strs::t_str_rdonly vert_shader_file_path, const strs::t_str_rdonly frag_shader_file_path, mem::t_arena *const temp_arena, t_resource_group *const arena) {
        t_array_mut<t_u8> vert_shader_compiled_bin;

        if (!gfx::shader_unpack(vert_shader_file_path, temp_arena, temp_arena, &vert_shader_compiled_bin)) {
            ZF_FATAL();
        }

        t_array_mut<t_u8> frag_shader_compiled_bin;

        if (!gfx::shader_unpack(frag_shader_file_path, temp_arena, temp_arena, &frag_shader_compiled_bin)) {
            ZF_FATAL();
        }

        return shader_prog_create(vert_shader_compiled_bin, frag_shader_compiled_bin, arena);
    }

    enum t_uniform_type {
        ec_uniform_type_sampler,
        ec_uniform_type_v4,
        ec_uniform_type_mat4x4
    };

    t_resource *uniform_create(const strs::t_str_rdonly name, const t_uniform_type type, t_resource_group *const group, mem::t_arena *const temp_arena);

    t_uniform_type uniform_get_type(const t_resource *const uniform);

    struct t_font {
        gfx::t_font_arrangement arrangement;
        t_array_mut<t_resource *> atlases;
    };

    t_font font_create_from_raw(const strs::t_str_rdonly file_path, const t_i32 height, strs::t_code_pt_bitset *const code_pts, mem::t_arena *const temp_arena, t_resource_group *const resource_group);
    t_font font_create_from_packed(const strs::t_str_rdonly file_path, mem::t_arena *const temp_arena, t_resource_group *const resource_group);


    // ============================================================
    // @section: Frame

    inline const t_i16 g_frame_pass_limit = 16;

    struct t_frame_context;

    struct t_vertex {
        math::t_v2 pos;
        gfx::t_color_rgba32f blend;
        math::t_v2 uv;
    };

    inline t_b8 vertex_check_valid(const t_vertex vert) {
        return gfx::color_check_normalized(vert.blend)
            && vert.uv.x >= 0.0f && vert.uv.y >= 0.0f && vert.uv.x <= 1.0f && vert.uv.y <= 1.0f;
    }

    struct t_triangle {
        t_static_array<t_vertex, 3> verts;
    };

    inline t_b8 triangle_check_valid(const t_triangle tri) {
        return vertex_check_valid(tri.verts[0])
            && vertex_check_valid(tri.verts[1])
            && vertex_check_valid(tri.verts[2]);
    }

    t_frame_context *frame_begin(const t_basis *const basis, mem::t_arena *const context_arena);
    void frame_end(t_frame_context *const context);

    void frame_pass_configure(t_frame_context *const context, const t_i32 pass_index, const math::t_v2_i size, const math::t_mat4x4 &view_mat = math::g_mat4x4_identity, const gfx::t_color_rgba32f clear_col = {});
    void frame_pass_set(t_frame_context *const context, const t_i32 pass_index);

    // Set prog as nullptr to just assign the default shader program.
    void frame_set_shader_prog(t_frame_context *const context, const t_resource *const prog);

    const t_resource *frame_get_builtin_shader_prog_default(t_frame_context *const context);
    const t_resource *frame_get_builtin_shader_prog_blend(t_frame_context *const context);

    const t_resource *frame_get_builtin_uniform_blend(t_frame_context *const context);

    void frame_set_uniform_sampler(t_frame_context *const context, const t_resource *const uniform, const t_resource *const sampler_texture);
    void frame_set_uniform_v4(t_frame_context *const context, const t_resource *const uniform, const math::t_v4 v4);
    void frame_set_uniform_mat4x4(t_frame_context *const context, const t_resource *const uniform, const math::t_mat4x4 &mat4x4);

    // Leave texture as nullptr for no texture.
    void frame_submit_triangles(t_frame_context *const context, const t_array_rdonly<t_triangle> triangles, const t_resource *const texture = nullptr);

    inline void frame_submit_triangle(t_frame_context *const context, const t_static_array<math::t_v2, 3> &pts, const t_static_array<gfx::t_color_rgba32f, 3> &pt_colors) {
        const t_triangle triangle = {
            .verts = {{
                {.pos = pts[0], .blend = pt_colors[0], .uv = {}},
                {.pos = pts[1], .blend = pt_colors[1], .uv = {}},
                {.pos = pts[2], .blend = pt_colors[2], .uv = {}},
            }},
        };

        frame_submit_triangles(context, {&triangle, 1}, nullptr);
    }

    inline void frame_submit_triangle(t_frame_context *const context, const t_static_array<math::t_v2, 3> &pts, const gfx::t_color_rgba32f color) {
        frame_submit_triangle(context, pts, {{color, color, color}});
    }

    inline void frame_submit_rect(t_frame_context *const context, const math::t_rect_f rect, const gfx::t_color_rgba32f color_topleft, const gfx::t_color_rgba32f color_topright, const gfx::t_color_rgba32f color_bottomright, const gfx::t_color_rgba32f color_bottomleft) {
        ZF_ASSERT(rect.width > 0.0f && rect.height > 0.0f);

        const t_static_array<t_triangle, 2> triangles = {{
            {
                .verts = {{
                    {.pos = math::rect_get_topleft(rect), .blend = color_topleft, .uv = {0.0f, 0.0f}},
                    {.pos = math::rect_get_topright(rect), .blend = color_topright, .uv = {1.0f, 0.0f}},
                    {.pos = math::rect_get_bottomleft(rect), .blend = color_bottomleft, .uv = {1.0f, 1.0f}},
                }},
            },
            {
                .verts = {{
                    {.pos = math::rect_get_bottomleft(rect), .blend = color_bottomleft, .uv = {1.0f, 1.0f}},
                    {.pos = math::rect_get_topright(rect), .blend = color_topright, .uv = {0.0f, 1.0f}},
                    {.pos = math::rect_get_bottomright(rect), .blend = color_bottomright, .uv = {0.0f, 0.0f}},
                }},
            },
        }};

        frame_submit_triangles(context, array_to_nonstatic(triangles), nullptr);
    }

    inline void frame_submit_rect(t_frame_context *const context, const math::t_rect_f rect, const gfx::t_color_rgba32f color) {
        frame_submit_rect(context, rect, color, color, color, color);
    }

    void frame_submit_rect_rotated(t_frame_context *const context, const math::t_v2 pos, const math::t_v2 size, const math::t_v2 origin, const t_f32 rot, const gfx::t_color_rgba32f color_topleft, const gfx::t_color_rgba32f color_topright, const gfx::t_color_rgba32f color_bottomright, const gfx::t_color_rgba32f color_bottomleft);

    inline void frame_submit_rect_rotated(t_frame_context *const context, const math::t_v2 pos, const math::t_v2 size, const math::t_v2 origin, const t_f32 rot, const gfx::t_color_rgba32f color) {
        frame_submit_rect_rotated(context, pos, size, origin, rot, color, color, color, color);
    }

    constexpr math::t_v2 g_origin_topleft = {0.0f, 0.0f};
    constexpr math::t_v2 g_origin_topcenter = {0.5f, 0.0f};
    constexpr math::t_v2 g_origin_topright = {1.0f, 0.0f};
    constexpr math::t_v2 g_origin_centerleft = {0.0f, 0.5f};
    constexpr math::t_v2 g_origin_center = {0.5f, 0.5f};
    constexpr math::t_v2 g_origin_centerright = {1.0f, 0.5f};
    constexpr math::t_v2 g_origin_bottomleft = {0.0f, 1.0f};
    constexpr math::t_v2 g_origin_bottomcenter = {0.5f, 1.0f};
    constexpr math::t_v2 g_origin_bottomright = {1.0f, 1.0f};

    inline t_b8 origin_check_valid(const math::t_v2 origin) {
        return origin.x >= 0.0f && origin.x <= 1.0f && origin.y >= 0.0f && origin.y <= 1.0f;
    }

    void frame_submit_texture(t_frame_context *const context, const t_resource *const texture, const math::t_v2 pos, const math::t_rect_i src_rect = {}, const math::t_v2 origin = g_origin_topleft, const t_f32 rot = 0.0f);

    constexpr math::t_v2 g_str_alignment_topleft = {0.0f, 0.0f};
    constexpr math::t_v2 g_str_alignment_topcenter = {0.5f, 0.0f};
    constexpr math::t_v2 g_str_alignment_topright = {1.0f, 0.0f};
    constexpr math::t_v2 g_str_alignment_centerleft = {0.0f, 0.5f};
    constexpr math::t_v2 g_str_alignment_center = {0.5f, 0.5f};
    constexpr math::t_v2 g_str_alignment_centerright = {1.0f, 0.5f};
    constexpr math::t_v2 g_str_alignment_bottomleft = {0.0f, 1.0f};
    constexpr math::t_v2 g_str_alignment_bottomcenter = {0.5f, 1.0f};
    constexpr math::t_v2 g_str_alignment_bottomright = {1.0f, 1.0f};

    inline t_b8 str_alignment_check_valid(const math::t_v2 alignment) {
        return alignment.x >= 0.0f && alignment.x <= 1.0f && alignment.y >= 0.0f && alignment.y <= 1.0f;
    }

    void frame_submit_str(t_frame_context *const context, const strs::t_str_rdonly str, const t_font &font, const math::t_v2 pos, mem::t_arena *const temp_arena, const math::t_v2 alignment = g_str_alignment_topleft, const gfx::t_color_rgba32f blend = gfx::g_color_white);

    // ============================================================
}
