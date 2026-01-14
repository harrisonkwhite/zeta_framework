#pragma once

#include <zcl/zcl_math.h>
#include <zcl/zcl_hash_maps.h>
#include <zcl/zcl_strs.h>

namespace zcl {
    // ============================================================
    // @section: Colors

    struct t_color_rgba32f {
        t_f32 r;
        t_f32 g;
        t_f32 b;
        t_f32 a;

        constexpr operator t_v4() const { return {r, g, b, a}; }
    };

    struct t_color_rgba8 {
        t_u8 r;
        t_u8 g;
        t_u8 b;
        t_u8 a;
    };

    constexpr t_b8 ColorCheckNormalized(const t_color_rgba32f col) {
        return col.r >= 0.0f && col.r <= 1.0f
            && col.g >= 0.0f && col.g <= 1.0f
            && col.b >= 0.0f && col.b <= 1.0f
            && col.a >= 0.0f && col.a <= 1.0f;
    }

    constexpr t_color_rgba32f ColorCreateRGBA32F(const t_f32 r, const t_f32 g, const t_f32 b, const t_f32 a = 1.0f) {
        const t_color_rgba32f result = {r, g, b, a};
        ZCL_ASSERT(ColorCheckNormalized(result));
        return result;
    }

    constexpr t_color_rgba8 ColorCreateRGBA8(const t_u8 r, const t_u8 g, const t_u8 b, const t_u8 a = 255) {
        return {r, g, b, a};
    }

    constexpr t_color_rgba32f k_color_transparent_black = ColorCreateRGBA32F(0.0f, 0.0f, 0.0f);
    constexpr t_color_rgba32f k_color_black = ColorCreateRGBA32F(0.0f, 0.0f, 0.0f);
    constexpr t_color_rgba32f k_color_dark_gray = ColorCreateRGBA32F(0.25f, 0.25f, 0.25f);
    constexpr t_color_rgba32f k_color_gray = ColorCreateRGBA32F(0.5f, 0.5f, 0.5f);
    constexpr t_color_rgba32f k_color_light_gray = ColorCreateRGBA32F(0.75f, 0.75f, 0.75f);
    constexpr t_color_rgba32f k_color_white = ColorCreateRGBA32F(1.0f, 1.0f, 1.0f);
    constexpr t_color_rgba32f k_color_red = ColorCreateRGBA32F(1.0f, 0.0f, 0.0f);
    constexpr t_color_rgba32f k_color_orange = ColorCreateRGBA32F(1.0f, 0.5f, 0.0f);
    constexpr t_color_rgba32f k_color_yellow = ColorCreateRGBA32F(1.0f, 1.0f, 0.0f);
    constexpr t_color_rgba32f k_color_lime = ColorCreateRGBA32F(0.75f, 1.0f, 0.0f);
    constexpr t_color_rgba32f k_color_green = ColorCreateRGBA32F(0.0f, 1.0f, 0.0f);
    constexpr t_color_rgba32f k_color_teal = ColorCreateRGBA32F(0.0f, 0.5f, 0.5f);
    constexpr t_color_rgba32f k_color_cyan = ColorCreateRGBA32F(0.0f, 1.0f, 1.0f);
    constexpr t_color_rgba32f k_color_blue = ColorCreateRGBA32F(0.0f, 0.0f, 1.0f);
    constexpr t_color_rgba32f k_color_purple = ColorCreateRGBA32F(0.5f, 0.0f, 0.5f);
    constexpr t_color_rgba32f k_color_magenta = ColorCreateRGBA32F(1.0f, 0.0f, 1.0f);
    constexpr t_color_rgba32f k_color_pink = ColorCreateRGBA32F(1.0f, 0.75f, 0.8f);
    constexpr t_color_rgba32f k_color_brown = ColorCreateRGBA32F(0.6f, 0.3f, 0.0f);

    constexpr t_color_rgba8 ColorRGBA32FToRGBA8(const t_color_rgba32f col) {
        ZCL_ASSERT(ColorCheckNormalized(col));
        return ColorCreateRGBA8(static_cast<t_u8>(255.0f * col.r), static_cast<t_u8>(255.0f * col.g), static_cast<t_u8>(255.0f * col.b), static_cast<t_u8>(255.0f * col.a));
    }

    constexpr t_color_rgba32f ColorRGBA8ToRGBA32F(const t_color_rgba8 col) {
        return ColorCreateRGBA32F(static_cast<t_f32>(col.r) / 255.0f, static_cast<t_f32>(col.g) / 255.0f, static_cast<t_f32>(col.b) / 255.0f, static_cast<t_f32>(col.a) / 255.0f);
    }

    constexpr t_color_rgba32f ColorCalcMix(const t_color_rgba32f a, const t_color_rgba32f b, const t_f32 amount) {
        ZCL_ASSERT(amount >= 0.0f && amount <= 1.0f);

        return {
            Lerp(a.r, b.r, amount),
            Lerp(a.g, b.g, amount),
            Lerp(a.b, b.b, amount),
            Lerp(a.a, b.a, amount),
        };
    }

    constexpr t_f32 ColorCalcLuminance(const t_color_rgba32f col) {
        return (0.2126f * col.r) + (0.7152f * col.g) + (0.0722f * col.b);
    }

    constexpr t_color_rgba32f ColorToGrayscale(const t_color_rgba32f col) {
        const t_f32 lum = ColorCalcLuminance(col);
        return {lum, lum, lum, col.a};
    }

    constexpr t_u32 ColorRGBA8ToHex(const t_color_rgba8 col) {
        t_u32 result = 0;
        result |= static_cast<t_u32>(col.r) << 24;
        result |= static_cast<t_u32>(col.g) << 16;
        result |= static_cast<t_u32>(col.b) << 8;
        result |= static_cast<t_u32>(col.a);

        return result;
    }

    constexpr t_color_rgba8 ColorHexToRGBA8(const t_u32 hex) {
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
        t_v2_i size_in_pxs;
        t_array_rdonly<t_u8> rgba_px_data;
    };

    struct t_texture_data_mut {
        t_v2_i size_in_pxs;
        t_array_mut<t_u8> rgba_px_data;

        operator t_texture_data_rdonly() const {
            return {.size_in_pxs = size_in_pxs, .rgba_px_data = rgba_px_data};
        }
    };

    [[nodiscard]] t_b8 TextureLoadFromRaw(const t_str_rdonly file_path, t_arena *const texture_data_arena, t_arena *const temp_arena, t_texture_data_mut *const o_texture_data);

    constexpr t_rect_f TextureCalcUVRect(const t_rect_i src_rect, const t_v2_i texture_size) {
        ZCL_ASSERT(texture_size.x > 0 && texture_size.y > 0);
        ZCL_ASSERT(src_rect.x >= 0 && src_rect.y >= 0 && src_rect.width > 0 && src_rect.height > 0 && RectGetRight(src_rect) <= texture_size.x && RectGetBottom(src_rect) <= texture_size.y);

        return {
            static_cast<t_f32>(src_rect.x) / static_cast<t_f32>(texture_size.x),
            static_cast<t_f32>(src_rect.y) / static_cast<t_f32>(texture_size.y),
            static_cast<t_f32>(src_rect.width) / static_cast<t_f32>(texture_size.x),
            static_cast<t_f32>(src_rect.height) / static_cast<t_f32>(texture_size.y),
        };
    }

    // ============================================================


    // ============================================================
    // @section: Fonts

    constexpr t_v2_i k_font_atlas_size = {1024, 1024};

    using t_font_atlas_rgba = t_static_array<t_u8, 4 * k_font_atlas_size.x * k_font_atlas_size.y>;

    struct t_font_glyph_info {
        t_v2_i offs;
        t_v2_i size;
        t_i32 adv;

        t_i32 atlas_index;
        t_rect_i atlas_rect;
    };

    struct t_font_code_point_pair {
        t_code_point a;
        t_code_point b;
    };

    struct t_font_arrangement {
        t_i32 line_height;

        t_hash_map<t_code_point, t_font_glyph_info> code_pts_to_glyph_infos;

        t_b8 has_kernings;
        t_hash_map<t_font_code_point_pair, t_i32> code_pt_pairs_to_kernings;
    };

    constexpr t_hash_func<t_code_point> k_font_code_point_hash_func =
        [](const t_code_point &code_pt) {
            return static_cast<t_i32>(code_pt);
        };

    constexpr t_hash_func<t_font_code_point_pair> k_font_code_point_pair_hash_func =
        [](const t_font_code_point_pair &pair) {
            return 0; // @todo: Proper hash function!
        };

    constexpr t_comparator_bin<t_font_code_point_pair> k_font_code_point_pair_comparator =
        [](const t_font_code_point_pair &pa, const t_font_code_point_pair &pb) {
            return pa.a == pb.a && pa.b == pb.b;
        };

    [[nodiscard]] t_b8 FontLoadFromRaw(const t_str_rdonly file_path, const t_i32 height, t_code_point_bitset *const code_pts, t_arena *const arrangement_arena, t_arena *const atlas_rgbas_arena, t_arena *const temp_arena, t_font_arrangement *const o_arrangement, t_array_mut<t_font_atlas_rgba> *const o_atlas_rgbas);

    // ============================================================
}
