#pragma once

#include <zc.h>

namespace zf::renderer {
    [[nodiscard]] t_b8 Init(s_mem_arena& temp_mem_arena);
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
        s_list<s_resource> resources;
        s_mem_arena* mem_arena;

        s_resource_arena() = default;
        s_resource_arena(const s_resource_arena&) = delete;
        s_resource_arena& operator=(const s_resource_arena&) = delete;
    };

    struct s_resource_id {
        t_size index;
        s_resource_arena* arena;
    };

    [[nodiscard]] t_b8 MakeResourceArena(const t_size res_limit, s_mem_arena& mem_arena, s_resource_arena& o_res_arena);
    void ReleaseResources(const s_resource_arena& o_res_arena);

    [[nodiscard]] t_b8 LoadTexture(const s_rgba_texture_data_rdonly& tex_data, s_resource_arena& res_arena, s_resource_id& o_id);

    [[nodiscard]] inline t_b8 LoadTextureFromRaw(const s_str_rdonly file_path, s_resource_arena& res_arena, s_mem_arena& temp_mem_arena, s_resource_id& o_id) {
        s_rgba_texture_data tex_data;

        if (!LoadRGBATextureDataFromRaw(file_path, temp_mem_arena, temp_mem_arena, tex_data)) {
            return false;
        }

        return LoadTexture(tex_data, res_arena, o_id);
    }

    [[nodiscard]] inline t_b8 LoadTextureFromPacked(const s_str_rdonly file_path, s_resource_arena& res_arena, s_mem_arena& temp_mem_arena, s_resource_id& o_id) {
        s_rgba_texture_data tex_data;

        if (!UnpackTexture(file_path, temp_mem_arena, temp_mem_arena, tex_data)) {
            return false;
        }

        return LoadTexture(tex_data, res_arena, o_id);
    }

    s_v2<t_s32> TextureSize(const s_resource_id id);

    [[nodiscard]] t_b8 LoadFontFromRaw(const s_str_rdonly file_path, const t_s32 height, const t_unicode_code_pt_bit_vec& code_pts, s_resource_arena& res_arena, s_mem_arena& temp_mem_arena, s_resource_id& o_id, e_font_load_from_raw_result* const o_load_from_raw_res = nullptr, t_unicode_code_pt_bit_vec* const o_unsupported_code_pts = nullptr);
    [[nodiscard]] t_b8 LoadFontFromPacked(const s_str_rdonly file_path, s_resource_arena& res_arena, s_mem_arena& temp_mem_arena, s_resource_id& o_id);

    // ============================================================
    // @section: Rendering
    // ============================================================
    void BeginRenderingPhase();
    void EndRenderingPhase();

    void Clear(const s_color_rgba32f col = {});

    void SetViewMatrix(const s_matrix_4x4& mat);

    void DrawTexture(const s_resource_id tex_id, const s_v2<t_f32> pos, const s_rect<t_s32> src_rect = {}, const s_v2<t_f32> origin = origins::g_topleft, const s_v2<t_f32> scale = {1.0f, 1.0f}, const t_f32 rot = 0.0f, const s_color_rgba32f blend = colors::g_white);
    [[nodiscard]] t_b8 DrawStr(const s_str_rdonly str, const s_resource_id font_id, const s_v2<t_f32> pos, const s_v2<t_f32> alignment, const s_color_rgba32f blend, s_mem_arena& temp_mem_arena);
}
