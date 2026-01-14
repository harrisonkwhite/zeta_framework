#pragma once

#include <zcl.h>

namespace zgl::gfx {
    // ============================================================
    // @section: Types and Constants

    constexpr zcl::t_v2 k_origin_topleft = {0.0f, 0.0f};
    constexpr zcl::t_v2 k_origin_topcenter = {0.5f, 0.0f};
    constexpr zcl::t_v2 k_origin_topright = {1.0f, 0.0f};
    constexpr zcl::t_v2 k_origin_centerleft = {0.0f, 0.5f};
    constexpr zcl::t_v2 k_origin_center = {0.5f, 0.5f};
    constexpr zcl::t_v2 k_origin_centerright = {1.0f, 0.5f};
    constexpr zcl::t_v2 k_origin_bottomleft = {0.0f, 1.0f};
    constexpr zcl::t_v2 k_origin_bottomcenter = {0.5f, 1.0f};
    constexpr zcl::t_v2 k_origin_bottomright = {1.0f, 1.0f};

    constexpr zcl::t_v2 k_alignment_topleft = {0.0f, 0.0f};
    constexpr zcl::t_v2 k_alignment_topcenter = {0.5f, 0.0f};
    constexpr zcl::t_v2 k_alignment_topright = {1.0f, 0.0f};
    constexpr zcl::t_v2 k_alignment_centerleft = {0.0f, 0.5f};
    constexpr zcl::t_v2 k_alignment_center = {0.5f, 0.5f};
    constexpr zcl::t_v2 k_alignment_centerright = {1.0f, 0.5f};
    constexpr zcl::t_v2 k_alignment_bottomleft = {0.0f, 1.0f};
    constexpr zcl::t_v2 k_alignment_bottomcenter = {0.5f, 1.0f};
    constexpr zcl::t_v2 k_alignment_bottomright = {1.0f, 1.0f};

    struct t_resource;

    struct t_resource_group {
        zcl::t_arena *arena;
        t_resource *head;
        t_resource *tail;
    };

    enum t_uniform_type {
        ek_uniform_type_sampler,
        ek_uniform_type_v4,
        ek_uniform_type_mat4x4
    };

    struct t_frame_basis;
    struct t_frame_context;

    constexpr zcl::t_i16 k_frame_pass_limit = 256;

    struct t_vertex {
        zcl::t_v2 pos;
        zcl::t_color_rgba32f blend;
        zcl::t_v2 uv;
    };

    struct t_triangle {
        zcl::t_static_array<t_vertex, 3> verts;
    };

    // ============================================================


    // ============================================================
    // @section: Functions

    // This depends on the platform module being initialised beforehand.
    t_frame_basis *module_startup(zcl::t_arena *const arena, zcl::t_arena *const temp_arena, t_resource_group **const o_perm_resource_group);

    void module_shutdown(const t_frame_basis *const frame_basis);

    constexpr zcl::t_b8 origin_check_valid(const zcl::t_v2 origin) {
        return origin.x >= 0.0f && origin.x <= 1.0f && origin.y >= 0.0f && origin.y <= 1.0f;
    }

    constexpr zcl::t_b8 alignment_check_valid(const zcl::t_v2 alignment) {
        return alignment.x >= 0.0f && alignment.x <= 1.0f && alignment.y >= 0.0f && alignment.y <= 1.0f;
    }


    // ========================================
    // @subsection: Resources

    inline t_resource_group resource_group_create(zcl::t_arena *const arena) {
        return {.arena = arena};
    }

    void resource_group_destroy(t_resource_group *const group);

    t_resource *texture_create(const zcl::t_texture_data_rdonly texture_data, t_resource_group *const group);

    inline t_resource *texture_create_from_raw(const zcl::t_str_rdonly file_path, zcl::t_arena *const temp_arena, t_resource_group *const group) {
        zcl::t_texture_data_mut texture_data;

        if (!zcl::texture_load_from_raw(file_path, temp_arena, temp_arena, &texture_data)) {
            ZCL_FATAL();
        }

        return texture_create(texture_data, group);
    }

    inline t_resource *texture_create_from_packed(const zcl::t_str_rdonly file_path, zcl::t_arena *const temp_arena, t_resource_group *const group) {
        zcl::t_texture_data_mut texture_data;

        if (!zcl::texture_unpack(file_path, temp_arena, temp_arena, &texture_data)) {
            ZCL_FATAL();
        }

        return texture_create(texture_data, group);
    }

    t_resource *texture_create_target(const zcl::t_v2_i size, t_resource_group *const group);

    void texture_resize_target(t_resource *const texture, const zcl::t_v2_i size);

    zcl::t_v2_i texture_get_size(const t_resource *const texture);

    // Resizes only if the given size is actually different to the current.
    inline void texture_resize_target_if_needed(t_resource *const texture, const zcl::t_v2_i size) {
        const zcl::t_v2_i size_cur = texture_get_size(texture);

        if (size != size_cur) {
            texture_resize_target(texture, size);
        }
    }

    t_resource *shader_prog_create(const zcl::t_array_rdonly<zcl::t_u8> vert_shader_compiled_bin, const zcl::t_array_rdonly<zcl::t_u8> frag_shader_compiled_bin, t_resource_group *const group);

    inline t_resource *shader_prog_create_from_packed(const zcl::t_str_rdonly vert_shader_file_path, const zcl::t_str_rdonly frag_shader_file_path, zcl::t_arena *const temp_arena, t_resource_group *const arena) {
        zcl::t_array_mut<zcl::t_u8> vert_shader_compiled_bin;

        if (!zcl::shader_unpack(vert_shader_file_path, temp_arena, temp_arena, &vert_shader_compiled_bin)) {
            ZCL_FATAL();
        }

        zcl::t_array_mut<zcl::t_u8> frag_shader_compiled_bin;

        if (!zcl::shader_unpack(frag_shader_file_path, temp_arena, temp_arena, &frag_shader_compiled_bin)) {
            ZCL_FATAL();
        }

        return shader_prog_create(vert_shader_compiled_bin, frag_shader_compiled_bin, arena);
    }


    t_resource *uniform_create(const zcl::t_str_rdonly name, const t_uniform_type type, t_resource_group *const group, zcl::t_arena *const temp_arena);

    t_uniform_type uniform_get_type(const t_resource *const uniform);

    struct t_font {
        zcl::t_font_arrangement arrangement;
        zcl::t_array_mut<t_resource *> atlases;
    };

    t_font font_create_from_raw(const zcl::t_str_rdonly file_path, const zcl::t_i32 height, zcl::t_code_point_bitset *const code_pts, zcl::t_arena *const temp_arena, t_resource_group *const resource_group);
    t_font font_create_from_packed(const zcl::t_str_rdonly file_path, zcl::t_arena *const temp_arena, t_resource_group *const resource_group);

    // ========================================


    // ========================================
    // @subsection: Frame

    constexpr zcl::t_b8 vertex_check_valid(const t_vertex vert) {
        return zcl::color_check_normalized(vert.blend)
            && vert.uv.x >= 0.0f && vert.uv.y >= 0.0f && vert.uv.x <= 1.0f && vert.uv.y <= 1.0f;
    }

    constexpr zcl::t_b8 triangle_check_valid(const t_triangle tri) {
        return vertex_check_valid(tri.verts[0])
            && vertex_check_valid(tri.verts[1])
            && vertex_check_valid(tri.verts[2]);
    }

    t_frame_context *frame_begin(const t_frame_basis *const basis, zcl::t_arena *const context_arena);
    void frame_end(t_frame_context *const context);

    void frame_pass_begin(t_frame_context *const context, const zcl::t_v2_i size, const zcl::t_mat4x4 &view_mat = zcl::MatrixCreateIdentity(), const zcl::t_b8 clear = false, const zcl::t_color_rgba32f clear_col = zcl::k_color_black);
    void frame_pass_begin_offscreen(t_frame_context *const context, const t_resource *const texture_target, const zcl::t_mat4x4 &view_mat = zcl::MatrixCreateIdentity(), const zcl::t_b8 clear = false, const zcl::t_color_rgba32f clear_col = zcl::k_color_black);

    void frame_pass_end(t_frame_context *const context);

    zcl::t_b8 frame_pass_check_active(const t_frame_context *const context);
    zcl::t_i32 frame_pass_get_index(const t_frame_context *const context);

    // Set prog as nullptr to just assign the default shader program.
    void frame_set_shader_prog(t_frame_context *const context, const t_resource *const prog);

    const t_resource *frame_get_builtin_shader_prog_default(t_frame_context *const context);
    const t_resource *frame_get_builtin_shader_prog_blend(t_frame_context *const context);

    const t_resource *frame_get_builtin_uniform_blend(t_frame_context *const context);

    void frame_set_uniform_sampler(t_frame_context *const context, const t_resource *const uniform, const t_resource *const sampler_texture);
    void frame_set_uniform_v4(t_frame_context *const context, const t_resource *const uniform, const zcl::t_v4 v4);
    void frame_set_uniform_mat4x4(t_frame_context *const context, const t_resource *const uniform, const zcl::t_mat4x4 &mat4x4);

    // Leave texture as nullptr for no texture.
    void frame_submit_triangles(t_frame_context *const context, const zcl::t_array_rdonly<t_triangle> triangles, const t_resource *const texture = nullptr);

    inline void frame_submit_triangle(t_frame_context *const context, const zcl::t_static_array<zcl::t_v2, 3> &pts, const zcl::t_static_array<zcl::t_color_rgba32f, 3> &pt_colors) {
        const t_triangle triangle = {
            .verts = {{
                {.pos = pts[0], .blend = pt_colors[0], .uv = {}},
                {.pos = pts[1], .blend = pt_colors[1], .uv = {}},
                {.pos = pts[2], .blend = pt_colors[2], .uv = {}},
            }},
        };

        frame_submit_triangles(context, {&triangle, 1}, nullptr);
    }

    inline void frame_submit_triangle(t_frame_context *const context, const zcl::t_static_array<zcl::t_v2, 3> &pts, const zcl::t_color_rgba32f color) {
        frame_submit_triangle(context, pts, {{color, color, color}});
    }

    inline void frame_submit_rect(t_frame_context *const context, const zcl::t_rect_f rect, const zcl::t_color_rgba32f color_topleft, const zcl::t_color_rgba32f color_topright, const zcl::t_color_rgba32f color_bottomright, const zcl::t_color_rgba32f color_bottomleft) {
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

        frame_submit_triangles(context, zcl::array_to_nonstatic(&triangles), nullptr);
    }

    inline void frame_submit_rect(t_frame_context *const context, const zcl::t_rect_f rect, const zcl::t_color_rgba32f color) {
        frame_submit_rect(context, rect, color, color, color, color);
    }

    void frame_submit_rect_rotated(t_frame_context *const context, const zcl::t_v2 pos, const zcl::t_v2 size, const zcl::t_v2 origin, const zcl::t_f32 rot, const zcl::t_color_rgba32f color_topleft, const zcl::t_color_rgba32f color_topright, const zcl::t_color_rgba32f color_bottomright, const zcl::t_color_rgba32f color_bottomleft);

    inline void frame_submit_rect_rotated(t_frame_context *const context, const zcl::t_v2 pos, const zcl::t_v2 size, const zcl::t_v2 origin, const zcl::t_f32 rot, const zcl::t_color_rgba32f color) {
        frame_submit_rect_rotated(context, pos, size, origin, rot, color, color, color, color);
    }

    void frame_submit_texture(t_frame_context *const context, const t_resource *const texture, const zcl::t_v2 pos, const zcl::t_rect_i src_rect = {}, const zcl::t_v2 origin = k_origin_topleft, const zcl::t_f32 rot = 0.0f);

    void frame_submit_str(t_frame_context *const context, const zcl::t_str_rdonly str, const t_font &font, const zcl::t_v2 pos, zcl::t_arena *const temp_arena, const zcl::t_v2 alignment = k_alignment_topleft, const zcl::t_color_rgba32f blend = zcl::k_color_white);

    // ========================================

    // ============================================================
}
