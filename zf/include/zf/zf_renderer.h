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

    struct s_resource_hdl {
        e_resource_type type;
        t_size index;

        constexpr t_b8 operator!() const {
            return type == ek_resource_type_invalid;
        }

        constexpr t_b8 operator==(const s_resource_hdl other) const {
            return type == other.type && index == other.index; // @todo: Perhaps if both are invalid, they should be equal irrespective of index?
        }

        constexpr t_b8 operator!=(const s_resource_hdl other) const {
            return !(*this == other);
        }
    };

    struct s_resource_lifetime {
        s_list<s_resource_hdl> hdls;
    };

    s_resource_hdl CreateTexture(const s_rgba_texture_data_rdonly& tex_data);

    inline s_resource_hdl CreateTextureFromRaw(const s_str_rdonly file_path, s_mem_arena& temp_mem_arena) {
        s_rgba_texture_data tex_data;

        if (!LoadRGBATextureDataFromRaw(file_path, temp_mem_arena, temp_mem_arena, tex_data)) {
            return {};
        }

        return CreateTexture(tex_data);
    }

    inline s_resource_hdl CreateTextureFromPacked(const s_str_rdonly file_path, s_mem_arena& temp_mem_arena) {
        s_rgba_texture_data tex_data;

        if (!UnpackTexture(file_path, temp_mem_arena, temp_mem_arena, tex_data)) {
            return {};
        }

        return CreateTexture(tex_data);
    }

    s_v2<t_s32> TextureSize(const s_resource_hdl hdl);

    s_resource_hdl CreateFontFromRaw(const s_str_rdonly file_path, const t_s32 height, const t_unicode_code_pt_bit_vec& code_pts, s_mem_arena& mem_arena, s_resource_lifetime& lifetime, s_mem_arena& temp_mem_arena, e_font_load_from_raw_result* const o_load_from_raw_res, t_unicode_code_pt_bit_vec* const o_unsupported_code_pts);
    s_resource_hdl CreateFontFromPacked(const s_str_rdonly file_path, s_mem_arena& mem_arena, s_resource_lifetime& lifetime, s_mem_arena& temp_mem_arena);

    s_resource_hdl CreateSurface(const s_v2<t_size> size, s_resource_lifetime& lifetime);
    s_resource_hdl ResizeSurface(const s_resource_hdl hdl, const s_v2<t_size> size);

    // ============================================================
    // @section: Rendering
    // ============================================================
    void BeginRenderingPhase();
    void EndRenderingPhase();

    void Clear(const s_color_rgba32f col = {});

    void SetViewMatrix(const s_matrix_4x4& mat);

    void DrawTexture(const s_resource_hdl hdl, const s_v2<t_f32> pos, const s_rect<t_s32> src_rect = {}, const s_v2<t_f32> origin = origins::g_topleft, const s_v2<t_f32> scale = {1.0f, 1.0f}, const t_f32 rot = 0.0f, const s_color_rgba32f blend = colors::g_white);
    void DrawRect(const s_rect<t_f32> rect, const s_color_rgba32f color);
    void DrawLine(const s_v2<t_f32> a, const s_v2<t_f32> b, const s_color_rgba32f blend, const t_f32 width);
    [[nodiscard]] t_b8 DrawStr(const s_str_rdonly str, const s_resource_hdl font_hdl, const s_v2<t_f32> pos, const s_v2<t_f32> alignment, const s_color_rgba32f blend, s_mem_arena& temp_mem_arena);

    void SetSurface(const s_resource_hdl hdl);
    void UnsetSurface(const s_resource_hdl hdl);
    void DrawSurface(const s_resource_hdl hdl, const s_v2<t_f32> pos, const s_v2<t_f32> scale);
}
