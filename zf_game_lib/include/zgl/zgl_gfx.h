#pragma once

#include <zcl.h>

namespace zf {
    struct s_rendering_basis;

    // Initialises the GFX module. This depends on the platform module being initialised beforehand.
    // The lifetime of the provided memory arena must encompass that of the GFX module.
    s_rendering_basis &InitGFX(s_mem_arena &mem_arena);

    void ShutdownGFX(s_rendering_basis &rendering_basis);

    // ============================================================
    // @section: Resources
    // ============================================================
    enum e_gfx_resource_type {
        ek_gfx_resource_type_invalid,
        ek_gfx_resource_type_texture
    };

    struct s_gfx_resource;

    struct s_gfx_resource_arena {
        s_ptr<s_mem_arena> mem_arena;
        s_ptr<s_gfx_resource> head;
        s_ptr<s_gfx_resource> tail;
    };

    inline s_gfx_resource_arena CreateGFXResourceArena(s_mem_arena &mem_arena) {
        return {.mem_arena = &mem_arena};
    }

    void DestroyGFXResources(s_gfx_resource_arena &arena);

    [[nodiscard]] t_b8 CreateTextureResource(const s_texture_data_rdonly texture_data, s_ptr<s_gfx_resource> &o_resource, const s_ptr<s_gfx_resource_arena> arena = nullptr);

    [[nodiscard]] inline t_b8 CreateTextureResourceFromRaw(const s_str_rdonly file_path, s_mem_arena &temp_mem_arena, s_ptr<s_gfx_resource> &o_resource, const s_ptr<s_gfx_resource_arena> arena = nullptr) {
        zf::s_texture_data tex_data;

        if (!zf::LoadTextureFromRaw(file_path, temp_mem_arena, temp_mem_arena, tex_data)) {
            return false;
        }

        return zf::CreateTextureResource(tex_data, o_resource, arena);
    }

    [[nodiscard]] inline t_b8 CreateTextureResourceFromPacked(const s_str_rdonly file_path, s_mem_arena &temp_mem_arena, s_ptr<s_gfx_resource> &o_resource, const s_ptr<s_gfx_resource_arena> arena = nullptr) {
        zf::s_texture_data tex_data;

        if (!zf::UnpackTexture(file_path, temp_mem_arena, temp_mem_arena, tex_data)) {
            return false;
        }

        return zf::CreateTextureResource(tex_data, o_resource, arena);
    }

    s_v2_i TextureSize(const s_gfx_resource &texture);

    // ============================================================
    // @section: Rendering
    // ============================================================
    struct s_rendering_context;

    void DrawTriangle(s_rendering_context &rc, const s_static_array<s_v2, 3> &pts, const s_static_array<s_color_rgba32f, 3> &pt_colors);
    void DrawRect(s_rendering_context &rc, const s_rect_f rect, const s_color_rgba32f color_topleft, const s_color_rgba32f color_topright, const s_color_rgba32f color_bottomright, const s_color_rgba32f color_bottomleft);
    void DrawTexture(s_rendering_context &rc, const s_v2 pos, const s_gfx_resource &texture);

    namespace internal {
        s_rendering_context &BeginFrame(const s_rendering_basis &rendering_basis, const s_color_rgb24f clear_col, s_mem_arena &mem_arena);
        void EndFrame(s_rendering_context &rc);
    }
}
