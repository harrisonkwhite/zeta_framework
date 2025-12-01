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

     // Returns nullptr if the load failed. Failure DOES NOT leave the underlying resource system in an invalid state - you are safe to continue.
    s_resource* LoadTexture(const s_rgba_texture_data_rdonly& tex_data, s_resource_arena& res_arena);

     // Returns nullptr if the load failed. Failure DOES NOT leave the underlying resource system in an invalid state - you are safe to continue.
    inline s_resource* LoadTextureFromRaw(const s_str_rdonly file_path, s_resource_arena& res_arena, s_mem_arena& temp_mem_arena) {
        s_rgba_texture_data tex_data;

        if (!LoadRGBATextureDataFromRaw(file_path, temp_mem_arena, temp_mem_arena, tex_data)) {
            return nullptr;
        }

        return LoadTexture(tex_data, res_arena);
    }

     // Returns nullptr if the load failed. Failure DOES NOT leave the underlying resource system in an invalid state - you are safe to continue.
    inline s_resource* LoadTextureFromPacked(const s_str_rdonly file_path, s_resource_arena& res_arena, s_mem_arena& temp_mem_arena) {
        s_rgba_texture_data tex_data;

        if (!UnpackTexture(file_path, temp_mem_arena, temp_mem_arena, tex_data)) {
            return nullptr;
        }

        return LoadTexture(tex_data, res_arena);
    }

    s_v2<t_s32> TextureSize(const s_resource* const res);

    s_resource* LoadFontFromRaw(const s_str_rdonly file_path, const t_s32 height, const t_unicode_code_pt_bit_vec& code_pts, s_resource_arena& res_arena, s_mem_arena& temp_mem_arena, e_font_load_from_raw_result* const o_load_from_raw_res = nullptr, t_unicode_code_pt_bit_vec* const o_unsupported_code_pts = nullptr); // Returns nullptr if the load failed. Failure DOES NOT leave the underlying resource system in an invalid state - you are safe to continue.
    s_resource* LoadFontFromPacked(const s_str_rdonly file_path, s_resource_arena& res_arena, s_mem_arena& temp_mem_arena); // Returns nullptr if the load failed. Failure DOES NOT leave the underlying resource system in an invalid state - you are safe to continue.

    // ============================================================
    // @section: Rendering
    // ============================================================
    void BeginRenderingPhase();
    void EndRenderingPhase();

    void Clear(const s_color_rgba32f col = {});

    void SetViewMatrix(const s_matrix_4x4& mat);

    void DrawTexture(const s_resource* const tex, const s_v2<t_f32> pos, const s_rect<t_s32> src_rect = {}, const s_v2<t_f32> origin = origins::g_topleft, const s_v2<t_f32> scale = {1.0f, 1.0f}, const t_f32 rot = 0.0f, const s_color_rgba32f blend = colors::g_white);
    [[nodiscard]] t_b8 DrawStr(const s_str_rdonly str, const s_resource* const font, const s_v2<t_f32> pos, const s_v2<t_f32> alignment, const s_color_rgba32f blend, s_mem_arena& temp_mem_arena);
}
