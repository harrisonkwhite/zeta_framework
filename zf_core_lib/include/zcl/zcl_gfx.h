#pragma once

#include <zcl/zcl_io.h>
#include <zcl/zcl_math.h>
#include <zcl/zcl_ds_hash_maps.h>

namespace zf::gfx {
    // ============================================================
    // @section: Types and Globals

    struct ColorRGBA32F {
        t_f32 r;
        t_f32 g;
        t_f32 b;
        t_f32 a;

        constexpr operator s_v4() const { return {r, g, b, a}; }
    };

    struct ColorRGB24F {
        t_f32 r;
        t_f32 g;
        t_f32 b;

        constexpr operator ColorRGBA32F() const { return {r, g, b, 1.0f}; }
        constexpr operator s_v3() const { return {r, g, b}; }
    };

    struct ColorRGBA8 {
        t_u8 r;
        t_u8 g;
        t_u8 b;
        t_u8 a;
    };

    struct ColorRGB8 {
        t_u8 r;
        t_u8 g;
        t_u8 b;

        constexpr operator ColorRGBA8() const { return {r, g, b, 255}; }
    };

    constexpr ColorRGB24F g_color_black = {0.0f, 0.0f, 0.0f};
    constexpr ColorRGB24F g_color_dark_gray = {0.25f, 0.25f, 0.25f};
    constexpr ColorRGB24F g_color_gray = {0.5f, 0.5f, 0.5f};
    constexpr ColorRGB24F g_color_light_gray = {0.75f, 0.75f, 0.75f};
    constexpr ColorRGB24F g_color_white = {1.0f, 1.0f, 1.0f};
    constexpr ColorRGB24F g_color_red = {1.0f, 0.0f, 0.0f};
    constexpr ColorRGB24F g_color_orange = {1.0f, 0.5f, 0.0f};
    constexpr ColorRGB24F g_color_yellow = {1.0f, 1.0f, 0.0f};
    constexpr ColorRGB24F g_color_lime = {0.75f, 1.0f, 0.0f};
    constexpr ColorRGB24F g_color_green = {0.0f, 1.0f, 0.0f};
    constexpr ColorRGB24F g_color_teal = {0.0f, 0.5f, 0.5f};
    constexpr ColorRGB24F g_color_cyan = {0.0f, 1.0f, 1.0f};
    constexpr ColorRGB24F g_color_blue = {0.0f, 0.0f, 1.0f};
    constexpr ColorRGB24F g_color_purple = {0.5f, 0.0f, 0.5f};
    constexpr ColorRGB24F g_color_magenta = {1.0f, 0.0f, 1.0f};
    constexpr ColorRGB24F g_color_pink = {1.0f, 0.75f, 0.8f};
    constexpr ColorRGB24F g_color_brown = {0.6f, 0.3f, 0.0f};

    struct TextureDataRdonly {
        s_v2_i size_in_pxs;
        t_array_rdonly<t_u8> rgba_px_data;
    };

    struct TextureDataMut {
        s_v2_i size_in_pxs;
        t_array_mut<t_u8> rgba_px_data;

        constexpr operator TextureDataRdonly() const {
            return {.size_in_pxs = size_in_pxs, .rgba_px_data = rgba_px_data};
        }
    };

    constexpr s_v2 g_origin_topleft = {0.0f, 0.0f};
    constexpr s_v2 g_origin_topcenter = {0.5f, 0.0f};
    constexpr s_v2 g_origin_topright = {1.0f, 0.0f};
    constexpr s_v2 g_origin_centerleft = {0.0f, 0.5f};
    constexpr s_v2 g_origin_center = {0.5f, 0.5f};
    constexpr s_v2 g_origin_centerright = {1.0f, 0.5f};
    constexpr s_v2 g_origin_bottomleft = {0.0f, 1.0f};
    constexpr s_v2 g_origin_bottomcenter = {0.5f, 1.0f};
    constexpr s_v2 g_origin_bottomright = {1.0f, 1.0f};

    constexpr s_v2_i g_font_atlas_size = {1024, 1024};

    using FontAtlasRGBA = t_static_array<t_u8, 4 * g_font_atlas_size.x * g_font_atlas_size.y>;

    struct FontGlyphInfo {
        s_v2_i offs;
        s_v2_i size;
        t_i32 adv;

        t_i32 atlas_index;
        s_rect_i atlas_rect;
    };

    struct FontCodePointPair {
        strs::CodePoint a;
        strs::CodePoint b;
    };

    struct FontArrangement {
        t_i32 line_height;

        s_hash_map<strs::CodePoint, FontGlyphInfo> code_pts_to_glyph_infos;

        t_b8 has_kernings;
        s_hash_map<FontCodePointPair, t_i32> code_pt_pairs_to_kernings;
    };

    constexpr s_v2 g_alignment_topleft = {0.0f, 0.0f};
    constexpr s_v2 g_alignment_topcenter = {0.5f, 0.0f};
    constexpr s_v2 g_alignment_topright = {1.0f, 0.0f};
    constexpr s_v2 g_alignment_centerleft = {0.0f, 0.5f};
    constexpr s_v2 g_alignment_center = {0.5f, 0.5f};
    constexpr s_v2 g_alignment_centerright = {1.0f, 0.5f};
    constexpr s_v2 g_alignment_bottomleft = {0.0f, 1.0f};
    constexpr s_v2 g_alignment_bottomcenter = {0.5f, 1.0f};
    constexpr s_v2 g_alignment_bottomright = {1.0f, 1.0f};

    // ============================================================


    // ============================================================
    // @section: Functions

    inline ColorRGBA32F calc_color_mix(const ColorRGBA32F a, const ColorRGBA32F b, const t_f32 amount) {
        ZF_ASSERT(amount >= 0.0f && amount <= 1.0f);

        return {
            Lerp(a.r, b.r, amount),
            Lerp(a.g, b.g, amount),
            Lerp(a.b, b.b, amount),
            Lerp(a.a, b.a, amount),
        };
    }

    inline t_f32 get_color_luminance(const ColorRGBA32F col) {
        return (0.2126f * col.r) + (0.7152f * col.g) + (0.0722f * col.b);
    }

    inline ColorRGBA32F get_color_as_grayscale(const ColorRGBA32F col) {
        const t_f32 lum = get_color_luminance(col);
        return {lum, lum, lum, col.a};
    }

    inline t_u32 convert_color_to_hex(const ColorRGBA8 col) {
        t_u32 result = 0;
        result |= static_cast<t_u32>(col.r) << 24;
        result |= static_cast<t_u32>(col.g) << 16;
        result |= static_cast<t_u32>(col.b) << 8;
        result |= static_cast<t_u32>(col.a);

        return result;
    }

    inline ColorRGBA8 convert_hex_to_color(const t_u32 hex) {
        const auto r = static_cast<t_u8>((hex & 0xFF000000) >> 24);
        const auto g = static_cast<t_u8>((hex & 0x00FF0000) >> 16);
        const auto b = static_cast<t_u8>((hex & 0x0000FF00) >> 8);
        const auto a = static_cast<t_u8>(hex & 0x000000FF);

        return {r, g, b, a};
    }

    inline s_rect_f calc_uv_rect(const s_rect_i src_rect, const s_v2_i tex_size) {
        ZF_ASSERT(tex_size.x > 0 && tex_size.y > 0);
        ZF_ASSERT(src_rect.x >= 0 && src_rect.y >= 0 && src_rect.width > 0 && src_rect.height > 0 && Right(src_rect) <= tex_size.x && Bottom(src_rect) <= tex_size.y);

        return {
            static_cast<t_f32>(src_rect.x) / static_cast<t_f32>(tex_size.x),
            static_cast<t_f32>(src_rect.y) / static_cast<t_f32>(tex_size.y),
            static_cast<t_f32>(src_rect.width) / static_cast<t_f32>(tex_size.x),
            static_cast<t_f32>(src_rect.height) / static_cast<t_f32>(tex_size.y),
        };
    }

    inline t_b8 get_is_origin_valid(const s_v2 origin) {
        return origin.x >= 0.0f && origin.x <= 1.0f && origin.y >= 0.0f && origin.y <= 1.0f;
    }

    inline t_b8 get_is_alignment_valid(const s_v2 alignment) {
        return alignment.x >= 0.0f && alignment.x <= 1.0f && alignment.y >= 0.0f && alignment.y <= 1.0f;
    }

    [[nodiscard]] t_b8 load_texture_from_raw(const strs::StrRdonly file_path, t_arena *const texture_data_arena, t_arena *const temp_arena, TextureDataMut *const o_texture_data);
    [[nodiscard]] t_b8 pack_texture(const strs::StrRdonly file_path, const TextureDataMut texture_data, t_arena *const temp_arena);
    [[nodiscard]] t_b8 unpack_texture(const strs::StrRdonly file_path, t_arena *const texture_data_arena, t_arena *const temp_arena, TextureDataMut *const o_texture_data);

    [[nodiscard]] t_b8 load_font_from_raw(const strs::StrRdonly file_path, const t_i32 height, strs::CodePointBitVector *const code_pts, t_arena *const arrangement_arena, t_arena *const atlas_rgbas_arena, t_arena *const temp_arena, FontArrangement *const o_arrangement, t_array_mut<FontAtlasRGBA> *const o_atlas_rgbas);
    [[nodiscard]] t_b8 pack_font(const strs::StrRdonly file_path, const FontArrangement &arrangement, const t_array_rdonly<FontAtlasRGBA> atlas_rgbas, t_arena *const temp_arena);
    [[nodiscard]] t_b8 unpack_font(const strs::StrRdonly file_path, t_arena *const arrangement_arena, t_arena *const atlas_rgbas_arena, t_arena *const temp_arena, FontArrangement *const o_arrangement, t_array_mut<FontAtlasRGBA> *const o_atlas_rgbas);

    [[nodiscard]] t_b8 pack_shader(const strs::StrRdonly file_path, const t_array_rdonly<t_u8> compiled_shader_bin, t_arena *const temp_arena);
    [[nodiscard]] t_b8 unpack_shader(const strs::StrRdonly file_path, t_arena *const shader_bin_arena, t_arena *const temp_arena, t_array_mut<t_u8> *const o_shader_bin);

    // ============================================================
}
