#pragma once

#include <zc.h>

namespace zf::renderer {
    [[nodiscard]] t_b8 Init(s_mem_arena& mem_arena, s_mem_arena& temp_mem_arena);
    void Shutdown();

    // ============================================================
    // @section: Resources
    // ============================================================
    enum e_resource_type {
        ek_resource_type_invalid,
        ek_resource_type_texture,
        ek_resource_type_font,
        ek_resource_type_surface
    };

    struct s_resource;

    struct s_resource_arena {
        s_mem_arena* mem_arena;
        s_resource* head;
        s_resource* tail;
    };

    inline s_resource_arena MakeResourceArena(s_mem_arena& mem_arena) {
        return {
            .mem_arena = &mem_arena
        };
    }

    void ReleaseResources(const s_resource_arena& res_arena);

    // Returns false iff the load failed. Failure DOES NOT leave the underlying resource system in an invalid state - you are safe to continue.
    [[nodiscard]] t_b8 LoadTexture(const s_rgba_texture_data_rdonly& tex_data, s_resource*& o_tex, s_resource_arena* const res_arena = nullptr);

    // Returns false iff the load failed. Failure DOES NOT leave the underlying resource system in an invalid state - you are safe to continue.
    [[nodiscard]] inline t_b8 LoadTextureFromRaw(const s_str_rdonly file_path, s_mem_arena& temp_mem_arena, s_resource*& o_tex, s_resource_arena* const res_arena = nullptr) {
        s_rgba_texture_data tex_data;

        if (!LoadRGBATextureDataFromRaw(file_path, temp_mem_arena, temp_mem_arena, tex_data)) {
            return false;
        }

        return LoadTexture(tex_data, o_tex, res_arena);
    }

    // Returns false iff the load failed. Failure DOES NOT leave the underlying resource system in an invalid state - you are safe to continue.
    [[nodiscard]] inline t_b8 LoadTextureFromPacked(const s_str_rdonly file_path, s_mem_arena& temp_mem_arena, s_resource*& o_tex, s_resource_arena* const res_arena = nullptr) {
        s_rgba_texture_data tex_data;

        if (!UnpackTexture(file_path, temp_mem_arena, temp_mem_arena, tex_data)) {
            return false;
        }

        return LoadTexture(tex_data, o_tex, res_arena);
    }

    s_v2<t_s32> TextureSize(const s_resource* const res);

    // Returns false iff the load failed. Failure DOES NOT leave the underlying resource system in an invalid state - you are safe to continue.
    [[nodiscard]] t_b8 LoadFontFromRaw(const s_str_rdonly file_path, const t_s32 height, const t_unicode_code_pt_bit_vec& code_pts, s_mem_arena& temp_mem_arena, s_resource*& o_font, s_resource_arena* const res_arena = nullptr, e_font_load_from_raw_result* const o_load_from_raw_res = nullptr, t_unicode_code_pt_bit_vec* const o_unsupported_code_pts = nullptr);

    // Returns false iff the load failed. Failure DOES NOT leave the underlying resource system in an invalid state - you are safe to continue.
    [[nodiscard]] t_b8 LoadFontFromPacked(const s_str_rdonly file_path, s_mem_arena& temp_mem_arena, s_resource*& o_font, s_resource_arena* const res_arena = nullptr);

    // Returns false on failure. Failure DOES NOT leave the underlying resource system in an invalid state - you are safe to continue.
    [[nodiscard]] t_b8 MakeSurface(const s_v2<t_s32> size, s_resource*& o_surf, s_resource_arena* const res_arena = nullptr);

    // Returns true iff the operation was successful. If it failed, the old surface state with its old size is left intact.
    [[nodiscard]] t_b8 ResizeSurface(s_resource* const surf, const s_v2<t_s32> size);

    // ============================================================
    // @section: Rendering
    // ============================================================
    void BeginRenderingPhase();
    void EndRenderingPhase();

    void Clear(const s_color_rgba32f col = {});

    void SetViewMatrix(const s_matrix_4x4& mat);

    void DrawTexture(const s_resource* const tex, const s_v2<t_f32> pos, const s_rect<t_s32> src_rect = {}, const s_v2<t_f32> origin = origins::g_topleft, const s_v2<t_f32> scale = {1.0f, 1.0f}, const t_f32 rot = 0.0f, const s_color_rgba32f blend = colors::g_white);

    // @todo: If the pixel texture ever becomes public, these 2 functions could be made inline.
    void DrawRect(const s_rect<t_f32> rect, const s_color_rgba32f color);
    void DrawLine(const s_v2<t_f32> a, const s_v2<t_f32> b, const s_color_rgba32f blend, const t_f32 width = 1.0f);

    [[nodiscard]] t_b8 DrawStr(const s_str_rdonly str, const s_resource* const font, const s_v2<t_f32> pos, const s_v2<t_f32> alignment, const s_color_rgba32f blend, s_mem_arena& temp_mem_arena);

    void SetSurface(const s_resource* const surf);
    void UnsetSurface();
    void DrawSurface(const s_resource* const surf, const s_v2<t_f32> pos);
}
