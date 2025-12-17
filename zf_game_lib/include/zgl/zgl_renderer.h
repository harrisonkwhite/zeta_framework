#pragma once

#include <zcl.h>

namespace zf {
    // Initialises the renderer module. This depends on the platform module being initialised beforehand.
    // The lifetime of the provided memory arena must encompass that of the renderer module.
    void InitRenderer(s_mem_arena &mem_arena, const t_i32 frame_vert_limit = 8192);

    void ShutdownRenderer();

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

    [[nodiscard]] t_b8 CreateTexture(const s_texture_data_rdonly texture_data, s_ptr<s_gfx_resource> &o_resource, const s_ptr<s_gfx_resource_arena> arena = nullptr);
    s_v2_i TextureSize(const s_gfx_resource &texture);

    // ============================================================
    // @section: Rendering
    // ============================================================
    void BeginFrame(const s_color_rgb24f clear_col);
    void EndFrame();

    void DrawTriangle(const s_static_array<s_v2, 3> &pts, const s_static_array<s_color_rgba32f, 3> &pt_colors);

    inline void DrawTriangle(const s_static_array<s_v2, 3> &pts, const s_color_rgba32f color) {
        DrawTriangle(pts, {color, color, color});
    }

    void DrawRect(const s_rect_f rect, const s_color_rgba32f color_topleft, const s_color_rgba32f color_topright, const s_color_rgba32f color_bottomright, const s_color_rgba32f color_bottomleft);

    inline void DrawRect(const s_rect_f rect, const s_color_rgba32f color) {
        DrawRect(rect, color, color, color, color);
    }

    void DrawTexture(const s_v2 pos, const s_gfx_resource &texture);
}
