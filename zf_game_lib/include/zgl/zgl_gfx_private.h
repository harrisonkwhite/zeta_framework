#pragma once

#include <zgl/zgl_gfx_public.h>

namespace zgl {
    void GFXStartupCore(const zcl::t_v2_i frame_size, void *const window_native_hdl, void *const display_native_hdl, zcl::t_arena *const arena, zcl::t_arena *const temp_arena);

    void GFXShutdownCore();

    t_gfx_resource *VertexBufCreate(const zcl::t_i32 vertex_cnt, zcl::t_arena *const arena);

    // @todo: Use array of actual vertex struct as source.
    void VertexBufWrite(t_gfx_resource *const dest_vertex_buf, const zcl::t_i32 dest_vertices_index_begin, const zcl::t_i32 dest_vertices_index_end, const zcl::t_array_rdonly<zcl::t_f32> src_vertices);

    void FrameSetSize(const zcl::t_v2_i size);

    void FramePassConfigure(const zcl::t_v2_i size, const zcl::t_mat4x4 &view_mat, const zcl::t_b8 clear, const zcl::t_color_rgba32f clear_col);

    void FramePassConfigureOffscreen(const t_gfx_resource *const texture_target, const zcl::t_mat4x4 &view_mat, const zcl::t_b8 clear, const zcl::t_color_rgba32f clear_col);

    void FrameRender(const zcl::t_i32 pass_index, const t_gfx_resource *const vertex_buf, const zcl::t_i32 vertices_index_begin, const zcl::t_i32 vertices_index_end, t_gfx_resource *const texture, const t_gfx_resource *const shader_prog, const t_gfx_resource *const sampler_uniform);

    void FrameComplete();
}
