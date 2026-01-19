#pragma once

#include <zcl.h>
#include <zgl/zgl_gfx_public.h>

namespace zgl {
    t_gfx *GFXStartupCore(const zcl::t_v2_i backbuffer_size, void *const window_native_hdl, void *const display_native_hdl);

    void GFXShutdownCore(t_gfx *const gfx);

    t_gfx_resource *VertexBufCreate(t_gfx *const gfx, const zcl::t_i32 vertex_cnt, t_gfx_resource_group *const resource_group);

    void VertexBufWrite(t_gfx *const gfx, t_gfx_resource *const dest_vertex_buf, const zcl::t_i32 dest_vertices_index, const zcl::t_array_rdonly<t_vertex> src_vertices);

    void PassConfigure(t_gfx *const gfx, const zcl::t_i32 pass_index, const zcl::t_v2_i size, const zcl::t_mat4x4 &view_mat, const zcl::t_b8 clear, const zcl::t_color_rgba32f clear_col);

    void PassConfigureOffscreen(t_gfx *const gfx, const zcl::t_i32 pass_index, const t_gfx_resource *const texture_target, const zcl::t_mat4x4 &view_mat, const zcl::t_b8 clear, const zcl::t_color_rgba32f clear_col);

    void PassSubmitVertices(t_gfx *const gfx, const zcl::t_i32 pass_index, const t_gfx_resource *const vertex_buf, const zcl::t_i32 vertices_index_begin, const zcl::t_i32 vertices_index_end, const t_gfx_resource *const texture, const t_gfx_resource *const shader_prog, const t_gfx_resource *const sampler_uniform);

    void PassConfigure(t_gfx *const gfx, const zcl::t_i32 pass_index, const zcl::t_v2_i size, const zcl::t_mat4x4 &view_mat, const zcl::t_b8 clear, const zcl::t_color_rgba32f clear_col);

    void PassConfigureOffscreen(t_gfx *const gfx, const zcl::t_i32 pass_index, const t_gfx_resource *const texture_target, const zcl::t_mat4x4 &view_mat, const zcl::t_b8 clear, const zcl::t_color_rgba32f clear_col);

    void PassSubmitVertices(t_gfx *const gfx, const zcl::t_i32 pass_index, const t_gfx_resource *const vertex_buf, const zcl::t_i32 vertices_index_begin, const zcl::t_i32 vertices_index_end, const t_gfx_resource *const texture, const t_gfx_resource *const shader_prog, const t_gfx_resource *const sampler_uniform);

    void FrameComplete(t_gfx *const gfx);
}
