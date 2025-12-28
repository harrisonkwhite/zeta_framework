#pragma once

#include <zcl.h>

namespace zf {
    // ============================================================
    // @section: General
    // ============================================================
    struct s_rendering_basis;

    // This depends on the platform module being initialised beforehand.
    s_rendering_basis &InitGFXModule(s_mem_arena &mem_arena);

    void ShutdownGFXModule(s_rendering_basis &rendering_basis);

    // ============================================================
    // @section: Resources
    // ============================================================
    enum e_gfx_resource_type {
        ek_gfx_resource_type_invalid,
        ek_gfx_resource_type_texture,
        ek_gfx_resource_type_shader_prog
    };

    struct s_gfx_resource;

    s_v2_i TextureSize(const s_gfx_resource &texture);

    struct s_gfx_resource_arena {
        s_ptr<s_mem_arena> mem_arena = nullptr;
        s_ptr<s_gfx_resource> head = nullptr;
        s_ptr<s_gfx_resource> tail = nullptr;

        s_gfx_resource_arena() = default;
        s_gfx_resource_arena(s_mem_arena &mem_arena) : mem_arena(&mem_arena) {}

        void Release();
    };

    s_gfx_resource_arena &PermGFXResourceArena();

    s_gfx_resource &CreateTextureResource(const s_texture_data_rdonly texture_data, s_gfx_resource_arena &arena = PermGFXResourceArena());

    inline s_gfx_resource &CreateTextureResourceFromRaw(const s_str_rdonly file_path, s_mem_arena &temp_mem_arena, s_gfx_resource_arena &arena = PermGFXResourceArena()) {
        s_texture_data texture_data;

        if (!LoadTextureDataFromRaw(file_path, temp_mem_arena, temp_mem_arena, texture_data)) {
            ZF_FATAL();
        }

        return CreateTextureResource(texture_data, arena);
    }

    inline s_gfx_resource &CreateTextureResourceFromPacked(const s_str_rdonly file_path, s_mem_arena &temp_mem_arena, s_gfx_resource_arena &arena = PermGFXResourceArena()) {
        s_texture_data texture_data;

        if (!UnpackTexture(file_path, temp_mem_arena, temp_mem_arena, texture_data)) {
            ZF_FATAL();
        }

        return CreateTextureResource(texture_data, arena);
    }

    s_gfx_resource &CreateShaderProgResource(const s_array_rdonly<t_u8> vert_shader_compiled_bin, const s_array_rdonly<t_u8> frag_shader_compiled_bin, s_gfx_resource_arena &arena = PermGFXResourceArena());

    inline s_gfx_resource &CreateShaderProgResourceFromPacked(const s_str_rdonly vert_shader_file_path, const s_str_rdonly frag_shader_file_path, s_mem_arena &temp_mem_arena, s_gfx_resource_arena &arena = PermGFXResourceArena()) {
        s_array<t_u8> vert_shader_compiled_bin;

        if (!UnpackShader(vert_shader_file_path, temp_mem_arena, temp_mem_arena, vert_shader_compiled_bin)) {
            ZF_FATAL();
        }

        s_array<t_u8> frag_shader_compiled_bin;

        if (!UnpackShader(frag_shader_file_path, temp_mem_arena, temp_mem_arena, frag_shader_compiled_bin)) {
            ZF_FATAL();
        }

        return CreateShaderProgResource(vert_shader_compiled_bin, frag_shader_compiled_bin, arena);
    }

    // ============================================================
    // @section: Rendering
    // ============================================================
    struct s_rendering_context;

    s_rendering_context &BeginRendering(const s_rendering_basis &rendering_basis, const s_color_rgb24f clear_col, s_mem_arena &rendering_context_mem_arena);
    void EndRendering(s_rendering_context &rendering_context);

    struct s_rendering_vert {
        s_v2 pos = {};
        s_color_rgba32f blend = {};
        s_v2 uv = {};
    };

    struct s_render_triangle {
        s_static_array<s_rendering_vert, 3> verts = {};
    };

    void RenderTriangles(s_rendering_context &rendering_context, const s_array_rdonly<s_render_triangle> triangles, const s_ptr<const s_gfx_resource> texture);
}
