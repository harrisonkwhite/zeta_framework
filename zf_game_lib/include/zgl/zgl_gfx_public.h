#pragma once

#include <zcl.h>
#include <zgl/zgl_platform_public.h>

namespace zgl {
    struct t_gfx_ticket_rdonly {
        zcl::t_u64 val;
    };

    struct t_gfx_ticket_mut {
        zcl::t_u64 val;

        operator t_gfx_ticket_rdonly() const {
            return {val};
        }
    };

    struct t_gfx_resource_group;
    struct t_gfx_resource;

    struct t_gfx_vertex {
        zcl::t_v2 pos;
        zcl::t_color_rgba32f blend;
        zcl::t_v2 uv;
    };

    constexpr zcl::t_b8 GFXVertexCheckValid(const t_gfx_vertex v) {
        return zcl::ColorCheckNormalized(v.blend)
            && v.uv.x >= 0.0f && v.uv.y >= 0.0f && v.uv.x <= 1.0f && v.uv.y <= 1.0f;
    }

    struct t_gfx_triangle {
        zcl::t_static_array<t_gfx_vertex, 3> vertices;
    };

    constexpr zcl::t_b8 GFXTriangleCheckValid(const t_gfx_triangle tri) {
        return GFXVertexCheckValid(tri.vertices[0])
            && GFXVertexCheckValid(tri.vertices[1])
            && GFXVertexCheckValid(tri.vertices[2]);
    }

    zcl::t_v2_i BackbufferGetSize(const t_gfx_ticket_rdonly gfx_ticket);

    t_gfx_resource_group *GFXResourceGroupCreate(const t_gfx_ticket_mut gfx_ticket, zcl::t_arena *const arena);
    void GFXResourceGroupDestroy(const t_gfx_ticket_mut gfx_ticket, t_gfx_resource_group *const group);
    zcl::t_arena *GFXResourceGroupGetArena(const t_gfx_ticket_rdonly gfx_ticket, const t_gfx_resource_group *const group);

    t_gfx_resource *TextureCreate(const t_gfx_ticket_mut gfx_ticket, const zcl::t_texture_data_rdonly texture_data, t_gfx_resource_group *const resource_group);
    t_gfx_resource *TextureCreateFromBuilt(const t_gfx_ticket_mut gfx_ticket, const zcl::t_str_rdonly file_path, t_gfx_resource_group *const group, zcl::t_arena *const temp_arena);
    t_gfx_resource *TextureCreateFromUnbuilt(const t_gfx_ticket_mut gfx_ticket, const zcl::t_str_rdonly file_path, t_gfx_resource_group *const group, zcl::t_arena *const temp_arena);
    t_gfx_resource *TextureCreateTarget(const t_gfx_ticket_mut gfx_ticket, const zcl::t_v2_i size, t_gfx_resource_group *const resource_group);
    void TextureResizeTarget(const t_gfx_ticket_mut gfx_ticket, t_gfx_resource *const texture, const zcl::t_v2_i size);
    zcl::t_v2_i TextureGetSize(const t_gfx_ticket_rdonly gfx_ticket, const t_gfx_resource *const texture);

    // Resizes only if the given size is actually different to the current.
    inline void TextureResizeTargetIfNeeded(const t_gfx_ticket_mut gfx_ticket, t_gfx_resource *const texture, const zcl::t_v2_i size) {
        const zcl::t_v2_i size_cur = TextureGetSize(gfx_ticket, texture);

        if (size != size_cur) {
            TextureResizeTarget(gfx_ticket, texture, size);
        }
    }

    t_gfx_resource *ShaderProgCreate(const t_gfx_ticket_mut gfx_ticket, const zcl::t_array_rdonly<zcl::t_u8> vertex_shader_compiled_bin, const zcl::t_array_rdonly<zcl::t_u8> fragment_shader_compiled_bin, t_gfx_resource_group *const resource_group);
    t_gfx_resource *ShaderProgCreateFromBuilt(const t_gfx_ticket_mut gfx_ticket, const zcl::t_str_rdonly vertex_shader_file_path, const zcl::t_str_rdonly fragment_shader_file_path, t_gfx_resource_group *const group, zcl::t_arena *const temp_arena);

    enum t_uniform_type : zcl::t_i32 {
        ek_uniform_type_sampler,
        ek_uniform_type_v4,
        ek_uniform_type_mat4x4
    };

    t_gfx_resource *UniformCreate(const t_gfx_ticket_mut gfx_ticket, const zcl::t_str_rdonly name, const t_uniform_type type, t_gfx_resource_group *const resource_group, zcl::t_arena *const temp_arena);
    t_uniform_type UniformGetType(const t_gfx_ticket_rdonly gfx_ticket, const t_gfx_resource *const uniform);
    void UniformSetSampler(const t_gfx_ticket_mut gfx_ticket, const t_gfx_resource *const uniform, const t_gfx_resource *const sampler_texture);
    void UniformSetV4(const t_gfx_ticket_mut gfx_ticket, const t_gfx_resource *const uniform, const zcl::t_v4 v4);
    void UniformSetMatrix4x4(const t_gfx_ticket_mut gfx_ticket, const t_gfx_resource *const uniform, const zcl::t_mat4x4 &mat4x4);

    struct t_font {
        zcl::t_font_arrangement arrangement;
        zcl::t_array_mut<t_gfx_resource *> atlas_textures;
    };

    t_font FontCreateFromBuilt(const t_gfx_ticket_mut gfx_ticket, const zcl::t_str_rdonly file_path, t_gfx_resource_group *const resource_group, zcl::t_arena *const temp_arena);
    t_font FontCreateFromUnbuilt(const t_gfx_ticket_mut gfx_ticket, const zcl::t_str_rdonly file_path, const zcl::t_i32 height, zcl::t_code_point_bitset *const code_pts, t_gfx_resource_group *const resource_group, zcl::t_arena *const temp_arena);

    namespace internal {
        t_gfx_ticket_mut GFXStartup(const t_platform_ticket_rdonly platform_ticket);
        void GFXShutdown(const t_gfx_ticket_mut gfx_ticket);

        t_gfx_resource *VertexBufCreate(const t_gfx_ticket_mut gfx_ticket, const zcl::t_i32 vertex_cnt, t_gfx_resource_group *const resource_group);
        void VertexBufWrite(const t_gfx_ticket_mut gfx_ticket, t_gfx_resource *const dest_vertex_buf, const zcl::t_i32 dest_vertices_index, const zcl::t_array_rdonly<t_gfx_vertex> src_vertices);
        zcl::t_i32 VertexBufGetVertexCount(const t_gfx_ticket_rdonly gfx_ticket, const t_gfx_resource *const vertex_buf);

        // The given size must be different to the current backbuffer size.
        void BackbufferResize(const t_gfx_ticket_mut gfx_ticket, const zcl::t_v2_i size);

        constexpr zcl::t_i16 k_frame_pass_limit = 256;

        void FrameBegin(const t_gfx_ticket_mut gfx_ticket);
        void FrameEnd(const t_gfx_ticket_mut gfx_ticket);
        void FramePassConfigure(const t_gfx_ticket_mut gfx_ticket, const zcl::t_i32 pass_index, const zcl::t_v2_i size, const zcl::t_mat4x4 &view_mat, const zcl::t_b8 clear, const zcl::t_color_rgba32f clear_col);
        void FramePassConfigureOffscreen(const t_gfx_ticket_mut gfx_ticket, const zcl::t_i32 pass_index, const t_gfx_resource *const texture_target, const zcl::t_mat4x4 &view_mat, const zcl::t_b8 clear, const zcl::t_color_rgba32f clear_col);
        void FrameSubmit(const t_gfx_ticket_mut gfx_ticket, const zcl::t_i32 pass_index, const t_gfx_resource *const vertex_buf, const zcl::t_i32 vertices_index_begin, const zcl::t_i32 vertices_index_end, const t_gfx_resource *const texture, const t_gfx_resource *const shader_prog, const t_gfx_resource *const sampler_uniform);
    }
}
