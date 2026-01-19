#pragma once

#include <zcl.h>

namespace zgl::test {
    void Startup(const zcl::t_v2_i frame_size, void *const window_native_hdl, void *const display_native_hdl, zcl::t_arena *const arena, zcl::t_arena *const temp_arena);

    void Shutdown();

    struct t_resource;

    void ResourceDestroy(t_resource *const resource);

    t_resource *VertexBufCreate(const zcl::t_i32 vertex_cnt, zcl::t_arena *const arena);

    // @todo: Use array of actual vertex struct as source.
    void VertexBufWrite(t_resource *const dest_vertex_buf, const zcl::t_i32 dest_vertices_index_begin, const zcl::t_i32 dest_vertices_index_end, const zcl::t_array_rdonly<zcl::t_f32> src_vertices);

    t_resource *TextureCreate(const zcl::t_texture_data_rdonly texture_data, zcl::t_arena *const arena);

    t_resource *TextureCreateTarget(const zcl::t_v2_i size, zcl::t_arena *const arena);

    void TextureResizeTarget(t_resource *const texture, const zcl::t_v2_i size);

    zcl::t_v2_i TextureGetSize(const t_resource *const texture);

    // Resizes only if the given size is actually different to the current.
    inline void TextureResizeTargetIfNeeded(t_resource *const texture, const zcl::t_v2_i size) {
        const zcl::t_v2_i size_cur = TextureGetSize(texture);

        if (size != size_cur) {
            TextureResizeTarget(texture, size);
        }
    }

    t_resource *ShaderProgCreate(const zcl::t_array_rdonly<zcl::t_u8> vertex_shader_compiled_bin, const zcl::t_array_rdonly<zcl::t_u8> frag_shader_compiled_bin, zcl::t_arena *const arena);

    enum t_uniform_type : zcl::t_i32 {
        ek_uniform_type_sampler,
        ek_uniform_type_v4,
        ek_uniform_type_mat4x4
    };

    t_resource *UniformCreate(const zcl::t_str_rdonly name, const t_uniform_type type, zcl::t_arena *const arena, zcl::t_arena *const temp_arena);

    t_uniform_type UniformGetType(const t_resource *const uniform);

    void UniformSetSampler(const t_resource *const uniform, const t_resource *const sampler_texture);

    void UniformSetV4(const t_resource *const uniform, const zcl::t_v4 v4);

    void UniformSetMatrix4x4(const t_resource *const uniform, const zcl::t_mat4x4 &mat4x4);

    void FrameSetSize(const zcl::t_v2_i size);

    constexpr zcl::t_i16 k_frame_pass_limit = 256;

    void FramePassConfigure(const zcl::t_v2_i size, const zcl::t_mat4x4 &view_mat, const zcl::t_b8 clear, const zcl::t_color_rgba32f clear_col);

    void FramePassConfigureOffscreen(const t_resource *const texture_target, const zcl::t_mat4x4 &view_mat, const zcl::t_b8 clear, const zcl::t_color_rgba32f clear_col);

    void FrameRender(const zcl::t_i32 pass_index, const t_resource *const vertex_buf, const zcl::t_i32 vertices_index_begin, const zcl::t_i32 vertices_index_end, t_resource *const texture, const t_resource *const shader_prog, const t_resource *const sampler_uniform);

    void FrameComplete();
}
