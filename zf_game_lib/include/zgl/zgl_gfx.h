#pragma once

#include <zcl.h>

namespace zgl::gfx {
    struct t_resource;

    struct t_resource_group {
        zf::mem::t_arena *arena;
        t_resource *head;
        t_resource *tail;
    };

    struct t_rendering_basis;

    // This depends on the platform module being initialised beforehand.
    // Returns a pointer to a rendering basis, needed for all rendering operations.
    t_rendering_basis *module_startup(zf::mem::t_arena *const arena, zf::mem::t_arena *const temp_arena, t_resource_group **const o_perm_resource_group);

    void module_shutdown(const t_rendering_basis *const basis);

    inline t_resource_group resource_group_create(zf::mem::t_arena *const arena) {
        return {.arena = arena};
    }

    void resource_group_destroy(t_resource_group *const group);

    t_resource *texture_create(const zf::gfx::t_texture_data_rdonly texture_data, t_resource_group *const group);

    inline t_resource *texture_create_from_raw(const zf::strs::t_str_rdonly file_path, zf::mem::t_arena *const temp_arena, t_resource_group *const group) {
        zf::gfx::t_texture_data_mut texture_data;

        if (!zf::gfx::texture_load_from_raw(file_path, temp_arena, temp_arena, &texture_data)) {
            ZF_FATAL();
        }

        return texture_create(texture_data, group);
    }

    inline t_resource *texture_create_from_packed(const zf::strs::t_str_rdonly file_path, zf::mem::t_arena *const temp_arena, t_resource_group *const group) {
        zf::gfx::t_texture_data_mut texture_data;

        if (!zf::gfx::texture_unpack(file_path, temp_arena, temp_arena, &texture_data)) {
            ZF_FATAL();
        }

        return texture_create(texture_data, group);
    }

    t_resource *texture_create_target(const zf::math::t_v2_i size, t_resource_group *const group);

    void texture_resize_target(t_resource *const texture, const zf::math::t_v2_i size);

    zf::math::t_v2_i texture_get_size(const t_resource *const texture);

    // Resizes only if the given size is actually different to the current.
    inline void texture_resize_target_if_needed(t_resource *const texture, const zf::math::t_v2_i size) {
        const zf::math::t_v2_i size_cur = texture_get_size(texture);

        if (size != size_cur) {
            texture_resize_target(texture, size);
        }
    }

    t_resource *shader_prog_create(const zf::t_array_rdonly<zf::t_u8> vert_shader_compiled_bin, const zf::t_array_rdonly<zf::t_u8> frag_shader_compiled_bin, t_resource_group *const group);

    inline t_resource *shader_prog_create_from_packed(const zf::strs::t_str_rdonly vert_shader_file_path, const zf::strs::t_str_rdonly frag_shader_file_path, zf::mem::t_arena *const temp_arena, t_resource_group *const arena) {
        zf::t_array_mut<zf::t_u8> vert_shader_compiled_bin;

        if (!zf::gfx::shader_unpack(vert_shader_file_path, temp_arena, temp_arena, &vert_shader_compiled_bin)) {
            ZF_FATAL();
        }

        zf::t_array_mut<zf::t_u8> frag_shader_compiled_bin;

        if (!zf::gfx::shader_unpack(frag_shader_file_path, temp_arena, temp_arena, &frag_shader_compiled_bin)) {
            ZF_FATAL();
        }

        return shader_prog_create(vert_shader_compiled_bin, frag_shader_compiled_bin, arena);
    }

    enum t_uniform_type {
        ek_uniform_type_sampler,
        ek_uniform_type_v4,
        ek_uniform_type_mat4x4
    };

    t_resource *uniform_create(const zf::strs::t_str_rdonly name, const t_uniform_type type, t_resource_group *const group, zf::mem::t_arena *const temp_arena);

    t_uniform_type uniform_get_type(const t_resource *const uniform);

    struct t_font {
        zf::gfx::t_font_arrangement arrangement;
        zf::t_array_mut<t_resource *> atlases;
    };

    t_font font_create_from_raw(const zf::strs::t_str_rdonly file_path, const zf::t_i32 height, zf::strs::t_code_pt_bitset *const code_pts, zf::mem::t_arena *const temp_arena, t_resource_group *const resource_group);
    t_font font_create_from_packed(const zf::strs::t_str_rdonly file_path, zf::mem::t_arena *const temp_arena, t_resource_group *const resource_group);


    // ============================================================
    // @section: Frame

    constexpr zf::t_i16 k_frame_pass_limit = 256;

    struct t_frame_context;

    struct t_vertex {
        zf::math::t_v2 pos;
        zf::gfx::t_color_rgba32f blend;
        zf::math::t_v2 uv;
    };

    constexpr zf::t_b8 vertex_check_valid(const t_vertex vert) {
        return zf::gfx::color_check_normalized(vert.blend)
            && vert.uv.x >= 0.0f && vert.uv.y >= 0.0f && vert.uv.x <= 1.0f && vert.uv.y <= 1.0f;
    }

    struct t_triangle {
        zf::t_static_array<t_vertex, 3> verts;
    };

    constexpr zf::t_b8 triangle_check_valid(const t_triangle tri) {
        return vertex_check_valid(tri.verts[0])
            && vertex_check_valid(tri.verts[1])
            && vertex_check_valid(tri.verts[2]);
    }

    t_frame_context *frame_begin(const t_rendering_basis *const basis, zf::mem::t_arena *const context_arena);
    void frame_end(t_frame_context *const context);

    void frame_pass_begin(t_frame_context *const context, const zf::math::t_v2_i size, const zf::math::t_mat4x4 &view_mat = zf::math::matrix_create_identity(), const zf::t_b8 clear = false, const zf::gfx::t_color_rgba32f clear_col = zf::gfx::k_color_black);
    void frame_pass_begin_offscreen(t_frame_context *const context, const t_resource *const texture_target, const zf::math::t_mat4x4 &view_mat = zf::math::matrix_create_identity(), const zf::t_b8 clear = false, const zf::gfx::t_color_rgba32f clear_col = zf::gfx::k_color_black);

    void frame_pass_end(t_frame_context *const context); // @todo: Maybe not necessary, though it does make the user code easier to read.

    zf::t_b8 frame_pass_check_active(const t_frame_context *const context);
    zf::t_i32 frame_pass_get_index(const t_frame_context *const context);

#if 0
    void frame_pass_configure(t_frame_context *const context, const zf::t_i32 pass_index, const zf::math::t_v2_i size, const zf::math::t_mat4x4 &view_mat = zf::math::matrix_create_identity(), const zf::t_b8 clear = false, const zf::gfx::t_color_rgba32f clear_col = zf::gfx::k_color_black);
    void frame_pass_configure_texture_target(t_frame_context *const context, const zf::t_i32 pass_index, const t_resource *const texture_target, const zf::math::t_mat4x4 &view_mat = zf::math::matrix_create_identity(), const zf::t_b8 clear = false, const zf::gfx::t_color_rgba32f clear_col = zf::gfx::k_color_black);

    void frame_pass_set(t_frame_context *const context, const zf::t_i32 pass_index);
#endif

    // Set prog as nullptr to just assign the default shader program.
    void frame_set_shader_prog(t_frame_context *const context, const t_resource *const prog);

    const t_resource *frame_get_builtin_shader_prog_default(t_frame_context *const context);
    const t_resource *frame_get_builtin_shader_prog_blend(t_frame_context *const context);

    const t_resource *frame_get_builtin_uniform_blend(t_frame_context *const context);

    void frame_set_uniform_sampler(t_frame_context *const context, const t_resource *const uniform, const t_resource *const sampler_texture);
    void frame_set_uniform_v4(t_frame_context *const context, const t_resource *const uniform, const zf::math::t_v4 v4);
    void frame_set_uniform_mat4x4(t_frame_context *const context, const t_resource *const uniform, const zf::math::t_mat4x4 &mat4x4);

    // Leave texture as nullptr for no texture.
    void frame_submit_triangles(t_frame_context *const context, const zf::t_array_rdonly<t_triangle> triangles, const t_resource *const texture = nullptr);

    inline void frame_submit_triangle(t_frame_context *const context, const zf::t_static_array<zf::math::t_v2, 3> &pts, const zf::t_static_array<zf::gfx::t_color_rgba32f, 3> &pt_colors) {
        const t_triangle triangle = {
            .verts = {{
                {.pos = pts[0], .blend = pt_colors[0], .uv = {}},
                {.pos = pts[1], .blend = pt_colors[1], .uv = {}},
                {.pos = pts[2], .blend = pt_colors[2], .uv = {}},
            }},
        };

        frame_submit_triangles(context, {&triangle, 1}, nullptr);
    }

    inline void frame_submit_triangle(t_frame_context *const context, const zf::t_static_array<zf::math::t_v2, 3> &pts, const zf::gfx::t_color_rgba32f color) {
        frame_submit_triangle(context, pts, {{color, color, color}});
    }

    inline void frame_submit_rect(t_frame_context *const context, const zf::math::t_rect_f rect, const zf::gfx::t_color_rgba32f color_topleft, const zf::gfx::t_color_rgba32f color_topright, const zf::gfx::t_color_rgba32f color_bottomright, const zf::gfx::t_color_rgba32f color_bottomleft) {
        ZF_ASSERT(rect.width > 0.0f && rect.height > 0.0f);

        const zf::t_static_array<t_triangle, 2> triangles = {{
            {
                .verts = {{
                    {.pos = zf::math::rect_get_topleft(rect), .blend = color_topleft, .uv = {0.0f, 0.0f}},
                    {.pos = zf::math::rect_get_topright(rect), .blend = color_topright, .uv = {1.0f, 0.0f}},
                    {.pos = zf::math::rect_get_bottomleft(rect), .blend = color_bottomleft, .uv = {1.0f, 1.0f}},
                }},
            },
            {
                .verts = {{
                    {.pos = zf::math::rect_get_bottomleft(rect), .blend = color_bottomleft, .uv = {1.0f, 1.0f}},
                    {.pos = zf::math::rect_get_topright(rect), .blend = color_topright, .uv = {0.0f, 1.0f}},
                    {.pos = zf::math::rect_get_bottomright(rect), .blend = color_bottomright, .uv = {0.0f, 0.0f}},
                }},
            },
        }};

        frame_submit_triangles(context, zf::array_to_nonstatic(triangles), nullptr);
    }

    inline void frame_submit_rect(t_frame_context *const context, const zf::math::t_rect_f rect, const zf::gfx::t_color_rgba32f color) {
        frame_submit_rect(context, rect, color, color, color, color);
    }

    void frame_submit_rect_rotated(t_frame_context *const context, const zf::math::t_v2 pos, const zf::math::t_v2 size, const zf::math::t_v2 origin, const zf::t_f32 rot, const zf::gfx::t_color_rgba32f color_topleft, const zf::gfx::t_color_rgba32f color_topright, const zf::gfx::t_color_rgba32f color_bottomright, const zf::gfx::t_color_rgba32f color_bottomleft);

    inline void frame_submit_rect_rotated(t_frame_context *const context, const zf::math::t_v2 pos, const zf::math::t_v2 size, const zf::math::t_v2 origin, const zf::t_f32 rot, const zf::gfx::t_color_rgba32f color) {
        frame_submit_rect_rotated(context, pos, size, origin, rot, color, color, color, color);
    }

    constexpr zf::math::t_v2 k_origin_topleft = {0.0f, 0.0f};
    constexpr zf::math::t_v2 k_origin_topcenter = {0.5f, 0.0f};
    constexpr zf::math::t_v2 k_origin_topright = {1.0f, 0.0f};
    constexpr zf::math::t_v2 k_origin_centerleft = {0.0f, 0.5f};
    constexpr zf::math::t_v2 k_origin_center = {0.5f, 0.5f};
    constexpr zf::math::t_v2 k_origin_centerright = {1.0f, 0.5f};
    constexpr zf::math::t_v2 k_origin_bottomleft = {0.0f, 1.0f};
    constexpr zf::math::t_v2 k_origin_bottomcenter = {0.5f, 1.0f};
    constexpr zf::math::t_v2 k_origin_bottomright = {1.0f, 1.0f};

    constexpr zf::t_b8 origin_check_valid(const zf::math::t_v2 origin) {
        return origin.x >= 0.0f && origin.x <= 1.0f && origin.y >= 0.0f && origin.y <= 1.0f;
    }

    void frame_submit_texture(t_frame_context *const context, const t_resource *const texture, const zf::math::t_v2 pos, const zf::math::t_rect_i src_rect = {}, const zf::math::t_v2 origin = k_origin_topleft, const zf::t_f32 rot = 0.0f);

    constexpr zf::math::t_v2 k_str_alignment_topleft = {0.0f, 0.0f};
    constexpr zf::math::t_v2 k_str_alignment_topcenter = {0.5f, 0.0f};
    constexpr zf::math::t_v2 k_str_alignment_topright = {1.0f, 0.0f};
    constexpr zf::math::t_v2 k_str_alignment_centerleft = {0.0f, 0.5f};
    constexpr zf::math::t_v2 k_str_alignment_center = {0.5f, 0.5f};
    constexpr zf::math::t_v2 k_str_alignment_centerright = {1.0f, 0.5f};
    constexpr zf::math::t_v2 k_str_alignment_bottomleft = {0.0f, 1.0f};
    constexpr zf::math::t_v2 k_str_alignment_bottomcenter = {0.5f, 1.0f};
    constexpr zf::math::t_v2 k_str_alignment_bottomright = {1.0f, 1.0f};

    constexpr zf::t_b8 str_alignment_check_valid(const zf::math::t_v2 alignment) {
        return alignment.x >= 0.0f && alignment.x <= 1.0f && alignment.y >= 0.0f && alignment.y <= 1.0f;
    }

    void frame_submit_str(t_frame_context *const context, const zf::strs::t_str_rdonly str, const t_font &font, const zf::math::t_v2 pos, zf::mem::t_arena *const temp_arena, const zf::math::t_v2 alignment = k_str_alignment_topleft, const zf::gfx::t_color_rgba32f blend = zf::gfx::k_color_white);

    // ============================================================
}
