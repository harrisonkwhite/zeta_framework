#pragma once

#include <zcl/zcl_io.h>
#include <zcl/zcl_math.h>
#include <zcl/zcl_ds.h>

namespace zf::gfx {
    // ============================================================
    // @section: Types and Globals

    struct t_color_rgba32f {
        t_f32 r;
        t_f32 g;
        t_f32 b;
        t_f32 a;

        constexpr operator math::t_v4() const { return {r, g, b, a}; }
    };

    struct t_color_rgb24f {
        t_f32 r;
        t_f32 g;
        t_f32 b;

        constexpr operator t_color_rgba32f() const { return {r, g, b, 1.0f}; }
        constexpr operator math::t_v3() const { return {r, g, b}; }
    };

    struct t_color_rgba8 {
        t_u8 r;
        t_u8 g;
        t_u8 b;
        t_u8 a;
    };

    struct t_color_rgb8 {
        t_u8 r;
        t_u8 g;
        t_u8 b;

        constexpr operator t_color_rgba8() const { return {r, g, b, 255}; }
    };

    constexpr t_color_rgb24f g_color_black = {0.0f, 0.0f, 0.0f};
    constexpr t_color_rgb24f g_color_dark_gray = {0.25f, 0.25f, 0.25f};
    constexpr t_color_rgb24f g_color_gray = {0.5f, 0.5f, 0.5f};
    constexpr t_color_rgb24f g_color_light_gray = {0.75f, 0.75f, 0.75f};
    constexpr t_color_rgb24f g_color_white = {1.0f, 1.0f, 1.0f};
    constexpr t_color_rgb24f g_color_red = {1.0f, 0.0f, 0.0f};
    constexpr t_color_rgb24f g_color_orange = {1.0f, 0.5f, 0.0f};
    constexpr t_color_rgb24f g_color_yellow = {1.0f, 1.0f, 0.0f};
    constexpr t_color_rgb24f g_color_lime = {0.75f, 1.0f, 0.0f};
    constexpr t_color_rgb24f g_color_green = {0.0f, 1.0f, 0.0f};
    constexpr t_color_rgb24f g_color_teal = {0.0f, 0.5f, 0.5f};
    constexpr t_color_rgb24f g_color_cyan = {0.0f, 1.0f, 1.0f};
    constexpr t_color_rgb24f g_color_blue = {0.0f, 0.0f, 1.0f};
    constexpr t_color_rgb24f g_color_purple = {0.5f, 0.0f, 0.5f};
    constexpr t_color_rgb24f g_color_magenta = {1.0f, 0.0f, 1.0f};
    constexpr t_color_rgb24f g_color_pink = {1.0f, 0.75f, 0.8f};
    constexpr t_color_rgb24f g_color_brown = {0.6f, 0.3f, 0.0f};

    struct t_texture_data_rdonly {
        math::t_v2_i size_in_pxs;
        t_array_rdonly<t_u8> rgba_px_data;
    };

    struct t_texture_data_mut {
        math::t_v2_i size_in_pxs;
        t_array_mut<t_u8> rgba_px_data;

        constexpr operator t_texture_data_rdonly() const {
            return {.size_in_pxs = size_in_pxs, .rgba_px_data = rgba_px_data};
        }
    };

    constexpr math::t_v2 g_origin_topleft = {0.0f, 0.0f};
    constexpr math::t_v2 g_origin_topcenter = {0.5f, 0.0f};
    constexpr math::t_v2 g_origin_topright = {1.0f, 0.0f};
    constexpr math::t_v2 g_origin_centerleft = {0.0f, 0.5f};
    constexpr math::t_v2 g_origin_center = {0.5f, 0.5f};
    constexpr math::t_v2 g_origin_centerright = {1.0f, 0.5f};
    constexpr math::t_v2 g_origin_bottomleft = {0.0f, 1.0f};
    constexpr math::t_v2 g_origin_bottomcenter = {0.5f, 1.0f};
    constexpr math::t_v2 g_origin_bottomright = {1.0f, 1.0f};

    constexpr math::t_v2_i g_font_atlas_size = {1024, 1024};

    using t_font_atlas_rgba = t_static_array<t_u8, 4 * g_font_atlas_size.x * g_font_atlas_size.y>;

    struct t_font_glyph_info {
        math::t_v2_i offs;
        math::t_v2_i size;
        t_i32 adv;

        t_i32 atlas_index;
        math::t_rect_i atlas_rect;
    };

    struct t_font_code_pt_pair {
        strs::t_code_pt a;
        strs::t_code_pt b;
    };

    struct t_font_arrangement {
        t_i32 line_height;

        ds::t_hash_map<strs::t_code_pt, t_font_glyph_info> code_pts_to_glyph_infos;

        t_b8 has_kernings;
        ds::t_hash_map<t_font_code_pt_pair, t_i32> code_pt_pairs_to_kernings;
    };

    constexpr math::t_v2 g_alignment_topleft = {0.0f, 0.0f};
    constexpr math::t_v2 g_alignment_topcenter = {0.5f, 0.0f};
    constexpr math::t_v2 g_alignment_topright = {1.0f, 0.0f};
    constexpr math::t_v2 g_alignment_centerleft = {0.0f, 0.5f};
    constexpr math::t_v2 g_alignment_center = {0.5f, 0.5f};
    constexpr math::t_v2 g_alignment_centerright = {1.0f, 0.5f};
    constexpr math::t_v2 g_alignment_bottomleft = {0.0f, 1.0f};
    constexpr math::t_v2 g_alignment_bottomcenter = {0.5f, 1.0f};
    constexpr math::t_v2 g_alignment_bottomright = {1.0f, 1.0f};

    // ============================================================


    // ============================================================
    // @section: Functions

    inline t_color_rgba32f f_calc_color_mix(const t_color_rgba32f a, const t_color_rgba32f b, const t_f32 amount) {
        ZF_ASSERT(amount >= 0.0f && amount <= 1.0f);

        return {
            math::lerp(a.r, b.r, amount),
            math::lerp(a.g, b.g, amount),
            math::lerp(a.b, b.b, amount),
            math::lerp(a.a, b.a, amount),
        };
    }

    inline t_f32 f_get_color_luminance(const t_color_rgba32f col) {
        return (0.2126f * col.r) + (0.7152f * col.g) + (0.0722f * col.b);
    }

    inline t_color_rgba32f f_get_color_as_grayscale(const t_color_rgba32f col) {
        const t_f32 lum = f_get_color_luminance(col);
        return {lum, lum, lum, col.a};
    }

    inline t_u32 f_convert_color_to_hex(const t_color_rgba8 col) {
        t_u32 result = 0;
        result |= static_cast<t_u32>(col.r) << 24;
        result |= static_cast<t_u32>(col.g) << 16;
        result |= static_cast<t_u32>(col.b) << 8;
        result |= static_cast<t_u32>(col.a);

        return result;
    }

    inline t_color_rgba8 f_convert_hex_to_color(const t_u32 hex) {
        const auto r = static_cast<t_u8>((hex & 0xFF000000) >> 24);
        const auto g = static_cast<t_u8>((hex & 0x00FF0000) >> 16);
        const auto b = static_cast<t_u8>((hex & 0x0000FF00) >> 8);
        const auto a = static_cast<t_u8>(hex & 0x000000FF);

        return {r, g, b, a};
    }

    inline math::t_rect_f f_calc_uv_rect(const math::t_rect_i src_rect, const math::t_v2_i tex_size) {
        ZF_ASSERT(tex_size.x > 0 && tex_size.y > 0);
        ZF_ASSERT(src_rect.x >= 0 && src_rect.y >= 0 && src_rect.width > 0 && src_rect.height > 0 && math::rect_get_right(src_rect) <= tex_size.x && math::rect_get_bottom(src_rect) <= tex_size.y);

        return {
            static_cast<t_f32>(src_rect.x) / static_cast<t_f32>(tex_size.x),
            static_cast<t_f32>(src_rect.y) / static_cast<t_f32>(tex_size.y),
            static_cast<t_f32>(src_rect.width) / static_cast<t_f32>(tex_size.x),
            static_cast<t_f32>(src_rect.height) / static_cast<t_f32>(tex_size.y),
        };
    }

    inline t_b8 f_is_origin_valid(const math::t_v2 origin) {
        return origin.x >= 0.0f && origin.x <= 1.0f && origin.y >= 0.0f && origin.y <= 1.0f;
    }

    inline t_b8 f_is_alignment_valid(const math::t_v2 alignment) {
        return alignment.x >= 0.0f && alignment.x <= 1.0f && alignment.y >= 0.0f && alignment.y <= 1.0f;
    }

    [[nodiscard]] t_b8 f_load_texture_from_raw(const strs::t_str_rdonly file_path, mem::t_arena *const texture_data_arena, mem::t_arena *const temp_arena, t_texture_data_mut *const o_texture_data);
    [[nodiscard]] t_b8 f_pack_texture(const strs::t_str_rdonly file_path, const t_texture_data_mut texture_data, mem::t_arena *const temp_arena);
    [[nodiscard]] t_b8 f_unpack_texture(const strs::t_str_rdonly file_path, mem::t_arena *const texture_data_arena, mem::t_arena *const temp_arena, t_texture_data_mut *const o_texture_data);

    [[nodiscard]] t_b8 f_load_font_from_raw(const strs::t_str_rdonly file_path, const t_i32 height, strs::t_code_pt_bitset *const code_pts, mem::t_arena *const arrangement_arena, mem::t_arena *const atlas_rgbas_arena, mem::t_arena *const temp_arena, t_font_arrangement *const o_arrangement, t_array_mut<t_font_atlas_rgba> *const o_atlas_rgbas);
    [[nodiscard]] t_b8 f_pack_font(const strs::t_str_rdonly file_path, const t_font_arrangement &arrangement, const t_array_rdonly<t_font_atlas_rgba> atlas_rgbas, mem::t_arena *const temp_arena);
    [[nodiscard]] t_b8 f_unpack_font(const strs::t_str_rdonly file_path, mem::t_arena *const arrangement_arena, mem::t_arena *const atlas_rgbas_arena, mem::t_arena *const temp_arena, t_font_arrangement *const o_arrangement, t_array_mut<t_font_atlas_rgba> *const o_atlas_rgbas);

    [[nodiscard]] t_b8 f_pack_shader(const strs::t_str_rdonly file_path, const t_array_rdonly<t_u8> compiled_shader_bin, mem::t_arena *const temp_arena);
    [[nodiscard]] t_b8 f_unpack_shader(const strs::t_str_rdonly file_path, mem::t_arena *const shader_bin_arena, mem::t_arena *const temp_arena, t_array_mut<t_u8> *const o_shader_bin);

    // ============================================================
}
