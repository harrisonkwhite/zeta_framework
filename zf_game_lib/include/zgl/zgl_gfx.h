#pragma once

#include <zcl.h>
#include <zgl/zgl_platform_public.h>

namespace zgl {
    struct t_gfx;

    t_gfx *GFXStartup(const t_platform_ticket_rdonly platform_ticket);
    void GFXShutdown(t_gfx *const gfx);


    // ============================================================
    // @section: Resources

    struct t_gfx_resource_group;

    t_gfx_resource_group *GFXResourceGroupCreate(t_gfx *const gfx, zcl::t_arena *const arena);

    void GFXResourceGroupDestroy(t_gfx *const gfx, t_gfx_resource_group *const group);

    struct t_gfx_resource;

    t_gfx_resource *VertexBufCreate(t_gfx *const gfx, const zcl::t_i32 vertex_cnt, t_gfx_resource_group *const resource_group);

    void VertexBufWrite(t_gfx *const gfx, t_gfx_resource *const dest_vertex_buf, const zcl::t_i32 dest_vertices_index_begin, const zcl::t_i32 dest_vertices_index_end, const zcl::t_array_rdonly<zcl::t_f32> src_vertices);

    t_gfx_resource *TextureCreate(t_gfx *const gfx, const zcl::t_texture_data_rdonly texture_data, t_gfx_resource_group *const resource_group);

    t_gfx_resource *TextureCreateTarget(t_gfx *const gfx, const zcl::t_v2_i size, t_gfx_resource_group *const resource_group);

    void TextureResizeTarget(t_gfx *const gfx, t_gfx_resource *const texture, const zcl::t_v2_i size);

    zcl::t_v2_i TextureGetSize(t_gfx *const gfx, const t_gfx_resource *const texture);

    // Resizes only if the given size is actually different to the current.
    inline void TextureResizeTargetIfNeeded(t_gfx *const gfx, t_gfx_resource *const texture, const zcl::t_v2_i size) {
        const zcl::t_v2_i size_cur = TextureGetSize(gfx, texture);

        if (size != size_cur) {
            TextureResizeTarget(gfx, texture, size);
        }
    }

    t_gfx_resource *ShaderProgCreate(t_gfx *const gfx, const zcl::t_array_rdonly<zcl::t_u8> vertex_shader_compiled_bin, const zcl::t_array_rdonly<zcl::t_u8> frag_shader_compiled_bin, t_gfx_resource_group *const resource_group);

    enum t_uniform_type : zcl::t_i32 {
        ek_uniform_type_sampler,
        ek_uniform_type_v4,
        ek_uniform_type_mat4x4
    };

    t_gfx_resource *UniformCreate(t_gfx *const gfx, const zcl::t_str_rdonly name, const t_uniform_type type, t_gfx_resource_group *const resource_group, zcl::t_arena *const temp_arena);

    t_uniform_type UniformGetType(t_gfx *const gfx, const t_gfx_resource *const uniform);

    void UniformSetSampler(t_gfx *const gfx, const t_gfx_resource *const uniform, const t_gfx_resource *const sampler_texture);

    void UniformSetV4(t_gfx *const gfx, const t_gfx_resource *const uniform, const zcl::t_v4 v4);

    void UniformSetMatrix4x4(t_gfx *const gfx, const t_gfx_resource *const uniform, const zcl::t_mat4x4 &mat4x4);

    // ============================================================


    // ============================================================
    // @section: Frame

    struct t_vertex {
        zcl::t_v2 pos;
        zcl::t_color_rgba32f blend;
        zcl::t_v2 uv;
    };

    constexpr zcl::t_b8 FrameVertexCheckValid(const t_vertex vert) {
        return zcl::ColorCheckNormalized(vert.blend)
            && vert.uv.x >= 0.0f && vert.uv.y >= 0.0f && vert.uv.x <= 1.0f && vert.uv.y <= 1.0f;
    }

    struct t_frame_triangle {
        zcl::t_static_array<t_vertex, 3> verts;
    };

    constexpr zcl::t_b8 FrameTriangleCheckValid(const t_frame_triangle tri) {
        return FrameVertexCheckValid(tri.verts[0])
            && FrameVertexCheckValid(tri.verts[1])
            && FrameVertexCheckValid(tri.verts[2]);
    }

    constexpr zcl::t_i16 k_frame_pass_limit = 256;

    zcl::t_v2_i FrameGetSize(t_gfx *const gfx);
    void FrameSetSize(t_gfx *const gfx, const zcl::t_v2_i size);

    void FramePassConfigure(t_gfx *const gfx, const zcl::t_i32 pass_index, const zcl::t_v2_i size, const zcl::t_mat4x4 &view_mat, const zcl::t_b8 clear, const zcl::t_color_rgba32f clear_col);

    void FramePassConfigureOffscreen(t_gfx *const gfx, const zcl::t_i32 pass_index, const t_gfx_resource *const texture_target, const zcl::t_mat4x4 &view_mat, const zcl::t_b8 clear, const zcl::t_color_rgba32f clear_col);

    void FrameSubmit(t_gfx *const gfx, const zcl::t_i32 pass_index, const t_gfx_resource *const vertex_buf, const zcl::t_i32 vertices_index_begin, const zcl::t_i32 vertices_index_end, const t_gfx_resource *const texture, const t_gfx_resource *const shader_prog, const t_gfx_resource *const sampler_uniform);

    void FrameComplete(t_gfx *const gfx);
    void FramePassConfigure(t_gfx *const gfx, const zcl::t_i32 pass_index, const zcl::t_v2_i size, const zcl::t_mat4x4 &view_mat, const zcl::t_b8 clear, const zcl::t_color_rgba32f clear_col);

    void FramePassConfigureOffscreen(t_gfx *const gfx, const zcl::t_i32 pass_index, const t_gfx_resource *const texture_target, const zcl::t_mat4x4 &view_mat, const zcl::t_b8 clear, const zcl::t_color_rgba32f clear_col);

    void FrameSubmit(t_gfx *const gfx, const zcl::t_i32 pass_index, const t_gfx_resource *const vertex_buf, const zcl::t_i32 vertices_index_begin, const zcl::t_i32 vertices_index_end, const t_gfx_resource *const texture, const t_gfx_resource *const shader_prog, const t_gfx_resource *const sampler_uniform);

    void FrameComplete(t_gfx *const gfx);

    // ============================================================
}
