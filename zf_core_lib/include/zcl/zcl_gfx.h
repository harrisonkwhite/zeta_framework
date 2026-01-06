#pragma once

#include <zcl/zcl_io.h>
#include <zcl/zcl_math.h>
#include <zcl/zcl_ds.h>

namespace zf::gfx {
    // ============================================================
    // @section: Colours

    struct t_color_rgba32f {
        t_f32 r;
        t_f32 g;
        t_f32 b;
        t_f32 a;

        operator math::t_v4() const { return {r, g, b, a}; }
    };

    struct t_color_rgb24f {
        t_f32 r;
        t_f32 g;
        t_f32 b;

        operator t_color_rgba32f() const { return {r, g, b, 1.0f}; }
        operator math::t_v3() const { return {r, g, b}; }
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

        operator t_color_rgba8() const { return {r, g, b, 255}; }
    };

    inline t_color_rgb24f color_create_rgb24f(const t_f32 r, const t_f32 g, const t_f32 b) {
        ZF_ASSERT(r >= 0.0f && r <= 1.0f);
        ZF_ASSERT(g >= 0.0f && g <= 1.0f);
        ZF_ASSERT(b >= 0.0f && b <= 1.0f);

        return {r, g, b};
    }

    inline t_color_rgba32f color_create_rgba32f(const t_f32 r, const t_f32 g, const t_f32 b, const t_f32 a) {
        ZF_ASSERT(r >= 0.0f && r <= 1.0f);
        ZF_ASSERT(g >= 0.0f && g <= 1.0f);
        ZF_ASSERT(b >= 0.0f && b <= 1.0f);
        ZF_ASSERT(a >= 0.0f && a <= 1.0f);

        return {r, g, b, a};
    }

    inline t_color_rgb8 color_create_rgb8(const t_u8 r, const t_u8 g, const t_u8 b) {
        return {r, g, b};
    }

    inline t_color_rgba8 color_create_rgba8(const t_u8 r, const t_u8 g, const t_u8 b, const t_u8 a) {
        return {r, g, b, a};
    }

    inline t_b8 color_is_valid(const t_color_rgba8 col) {
        return true;
    }

    inline t_b8 color_is_valid(const t_color_rgba32f col) {
        return col.r >= 0.0f && col.r <= 1.0f
            && col.g >= 0.0f && col.g <= 1.0f
            && col.b >= 0.0f && col.b <= 1.0f
            && col.a >= 0.0f && col.a <= 1.0f;
    }

    inline const t_color_rgb24f g_color_black = color_create_rgb24f(0.0f, 0.0f, 0.0f);
    inline const t_color_rgb24f g_color_dark_gray = color_create_rgb24f(0.25f, 0.25f, 0.25f);
    inline const t_color_rgb24f g_color_gray = color_create_rgb24f(0.5f, 0.5f, 0.5f);
    inline const t_color_rgb24f g_color_light_gray = color_create_rgb24f(0.75f, 0.75f, 0.75f);
    inline const t_color_rgb24f g_color_white = color_create_rgb24f(1.0f, 1.0f, 1.0f);
    inline const t_color_rgb24f g_color_red = color_create_rgb24f(1.0f, 0.0f, 0.0f);
    inline const t_color_rgb24f g_color_orange = color_create_rgb24f(1.0f, 0.5f, 0.0f);
    inline const t_color_rgb24f g_color_yellow = color_create_rgb24f(1.0f, 1.0f, 0.0f);
    inline const t_color_rgb24f g_color_lime = color_create_rgb24f(0.75f, 1.0f, 0.0f);
    inline const t_color_rgb24f g_color_green = color_create_rgb24f(0.0f, 1.0f, 0.0f);
    inline const t_color_rgb24f g_color_teal = color_create_rgb24f(0.0f, 0.5f, 0.5f);
    inline const t_color_rgb24f g_color_cyan = color_create_rgb24f(0.0f, 1.0f, 1.0f);
    inline const t_color_rgb24f g_color_blue = color_create_rgb24f(0.0f, 0.0f, 1.0f);
    inline const t_color_rgb24f g_color_purple = color_create_rgb24f(0.5f, 0.0f, 0.5f);
    inline const t_color_rgb24f g_color_magenta = color_create_rgb24f(1.0f, 0.0f, 1.0f);
    inline const t_color_rgb24f g_color_pink = color_create_rgb24f(1.0f, 0.75f, 0.8f);
    inline const t_color_rgb24f g_color_brown = color_create_rgb24f(0.6f, 0.3f, 0.0f);

    inline t_color_rgb8 color_convert_to_rgb8(const t_color_rgb24f col) {
        return color_create_rgb8(static_cast<t_u8>(255.0f * col.r), static_cast<t_u8>(255.0f * col.g), static_cast<t_u8>(255.0f * col.b));
    }

    inline t_color_rgba8 color_convert_to_rgba8(const t_color_rgba32f col) {
        return color_create_rgba8(static_cast<t_u8>(255.0f * col.r), static_cast<t_u8>(255.0f * col.g), static_cast<t_u8>(255.0f * col.b), static_cast<t_u8>(255.0f * col.a));
    }

    inline t_color_rgb24f color_convert_to_rgb24f(const t_color_rgb8 col) {
        return color_create_rgb24f(static_cast<t_f32>(col.r) / 255.0f, static_cast<t_f32>(col.g) / 255.0f, static_cast<t_f32>(col.b) / 255.0f);
    }

    inline t_color_rgba32f color_convert_to_rgba32f(const t_color_rgba8 col) {
        return color_create_rgba32f(static_cast<t_f32>(col.r) / 255.0f, static_cast<t_f32>(col.g) / 255.0f, static_cast<t_f32>(col.b) / 255.0f, static_cast<t_f32>(col.a) / 255.0f);
    }

    inline t_color_rgba32f color_get_mix(const t_color_rgba32f a, const t_color_rgba32f b, const t_f32 amount) {
        ZF_ASSERT(amount >= 0.0f && amount <= 1.0f);

        return {
            math::lerp(a.r, b.r, amount),
            math::lerp(a.g, b.g, amount),
            math::lerp(a.b, b.b, amount),
            math::lerp(a.a, b.a, amount),
        };
    }

    inline t_f32 color_get_luminance(const t_color_rgba32f col) {
        return (0.2126f * col.r) + (0.7152f * col.g) + (0.0722f * col.b);
    }

    inline t_color_rgba32f color_get_grayscale(const t_color_rgba32f col) {
        const t_f32 lum = color_get_luminance(col);
        return {lum, lum, lum, col.a};
    }

    inline t_u32 color_convert_to_hex(const t_color_rgba8 col) {
        t_u32 result = 0;
        result |= static_cast<t_u32>(col.r) << 24;
        result |= static_cast<t_u32>(col.g) << 16;
        result |= static_cast<t_u32>(col.b) << 8;
        result |= static_cast<t_u32>(col.a);

        return result;
    }

    inline t_color_rgba8 color_convert_from_hex(const t_u32 hex) {
        const auto r = static_cast<t_u8>((hex & 0xFF000000) >> 24);
        const auto g = static_cast<t_u8>((hex & 0x00FF0000) >> 16);
        const auto b = static_cast<t_u8>((hex & 0x0000FF00) >> 8);
        const auto a = static_cast<t_u8>(hex & 0x000000FF);

        return {r, g, b, a};
    }

    // ============================================================


    // ============================================================
    // @section: Textures

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

    inline math::t_rect_f texture_calc_uv_rect(const math::t_rect_i src_rect, const math::t_v2_i tex_size) {
        ZF_ASSERT(tex_size.x > 0 && tex_size.y > 0);
        ZF_ASSERT(src_rect.x >= 0 && src_rect.y >= 0 && src_rect.width > 0 && src_rect.height > 0 && math::rect_get_right(src_rect) <= tex_size.x && math::rect_get_bottom(src_rect) <= tex_size.y);

        return {
            static_cast<t_f32>(src_rect.x) / static_cast<t_f32>(tex_size.x),
            static_cast<t_f32>(src_rect.y) / static_cast<t_f32>(tex_size.y),
            static_cast<t_f32>(src_rect.width) / static_cast<t_f32>(tex_size.x),
            static_cast<t_f32>(src_rect.height) / static_cast<t_f32>(tex_size.y),
        };
    }

    [[nodiscard]] t_b8 texture_load_from_raw(const strs::t_str_rdonly file_path, mem::t_arena *const texture_data_arena, mem::t_arena *const temp_arena, t_texture_data_mut *const o_texture_data);

    [[nodiscard]] t_b8 texture_pack(const strs::t_str_rdonly file_path, const t_texture_data_mut texture_data, mem::t_arena *const temp_arena);
    [[nodiscard]] t_b8 texture_unpack(const strs::t_str_rdonly file_path, mem::t_arena *const texture_data_arena, mem::t_arena *const temp_arena, t_texture_data_mut *const o_texture_data);

    // ============================================================


    // ============================================================
    // @section: Fonts

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

    [[nodiscard]] t_b8 font_load_from_raw(const strs::t_str_rdonly file_path, const t_i32 height, strs::t_code_pt_bitset *const code_pts, mem::t_arena *const arrangement_arena, mem::t_arena *const atlas_rgbas_arena, mem::t_arena *const temp_arena, t_font_arrangement *const o_arrangement, t_array_mut<t_font_atlas_rgba> *const o_atlas_rgbas);

    [[nodiscard]] t_b8 font_pack(const strs::t_str_rdonly file_path, const t_font_arrangement &arrangement, const t_array_rdonly<t_font_atlas_rgba> atlas_rgbas, mem::t_arena *const temp_arena);
    [[nodiscard]] t_b8 font_unpack(const strs::t_str_rdonly file_path, mem::t_arena *const arrangement_arena, mem::t_arena *const atlas_rgbas_arena, mem::t_arena *const temp_arena, t_font_arrangement *const o_arrangement, t_array_mut<t_font_atlas_rgba> *const o_atlas_rgbas);

    // ============================================================


    // ============================================================
    // @section: Shaders

    [[nodiscard]] t_b8 shader_pack(const strs::t_str_rdonly file_path, const t_array_rdonly<t_u8> compiled_shader_bin, mem::t_arena *const temp_arena);
    [[nodiscard]] t_b8 shader_unpack(const strs::t_str_rdonly file_path, mem::t_arena *const shader_bin_arena, mem::t_arena *const temp_arena, t_array_mut<t_u8> *const o_shader_bin);

    // ============================================================
}
