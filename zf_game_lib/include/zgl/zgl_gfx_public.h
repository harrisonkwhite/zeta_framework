#pragma once

#include <zcl.h>
#include <zgl/zgl_platform_public.h>

namespace zgl {
    struct t_gfx;

    t_gfx *GFXStartup(const t_platform_ticket_rdonly platform_ticket, zcl::t_arena *const arena, zcl::t_arena *const temp_arena);
    void GFXShutdown(t_gfx *const gfx);

    struct t_gfx_resource;

    void GFXResourceDestroy(t_gfx_resource *const resource);

    t_gfx_resource *TextureCreate(const zcl::t_texture_data_rdonly texture_data, zcl::t_arena *const arena);

    t_gfx_resource *TextureCreateTarget(const zcl::t_v2_i size, zcl::t_arena *const arena);

    void TextureResizeTarget(t_gfx_resource *const texture, const zcl::t_v2_i size);

    zcl::t_v2_i TextureGetSize(const t_gfx_resource *const texture);

    // Resizes only if the given size is actually different to the current.
    inline void TextureResizeTargetIfNeeded(t_gfx_resource *const texture, const zcl::t_v2_i size) {
        const zcl::t_v2_i size_cur = TextureGetSize(texture);

        if (size != size_cur) {
            TextureResizeTarget(texture, size);
        }
    }

    t_gfx_resource *ShaderProgCreate(const zcl::t_array_rdonly<zcl::t_u8> vertex_shader_compiled_bin, const zcl::t_array_rdonly<zcl::t_u8> frag_shader_compiled_bin, zcl::t_arena *const arena);

    enum t_uniform_type : zcl::t_i32 {
        ek_uniform_type_sampler,
        ek_uniform_type_v4,
        ek_uniform_type_mat4x4
    };

    t_gfx_resource *UniformCreate(const zcl::t_str_rdonly name, const t_uniform_type type, zcl::t_arena *const arena, zcl::t_arena *const temp_arena);

    t_uniform_type UniformGetType(const t_gfx_resource *const uniform);

    void UniformSetSampler(const t_gfx_resource *const uniform, const t_gfx_resource *const sampler_texture);

    void UniformSetV4(const t_gfx_resource *const uniform, const zcl::t_v4 v4);

    void UniformSetMatrix4x4(const t_gfx_resource *const uniform, const zcl::t_mat4x4 &mat4x4);

    void FrameSetSize(const zcl::t_v2_i size);

    constexpr zcl::t_i16 k_frame_pass_limit = 256;
}
