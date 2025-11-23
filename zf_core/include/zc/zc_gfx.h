#pragma once

#include <zc/zc_io.h>
#include <zc/zc_math.h>
#include <zc/ds/zc_hash_map.h>

namespace zf {
    struct s_color_rgba32f {
        t_f32 r;
        t_f32 g;
        t_f32 b;
        t_f32 a;
    };

    struct s_color_rgba8 {
        t_u8 r;
        t_u8 g;
        t_u8 b;
        t_u8 a;

        constexpr operator s_color_rgba32f() {
            return {r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f};
        }
    };

    constexpr s_color_rgba32f ColorsMixed(const s_color_rgba32f a, const s_color_rgba32f b, const t_f32 amount) {
        ZF_ASSERT(amount >= 0.0f && amount <= 1.0f);

        return {
            Lerp(a.r, b.r, amount),
            Lerp(a.g, b.g, amount),
            Lerp(a.b, b.b, amount),
            Lerp(a.a, b.a, amount)
        };
    }

    constexpr t_f32 ColorLuminance(const s_color_rgba32f col) {
        return (0.2126f * col.r) + (0.7152f * col.g) + (0.0722f * col.b);
    }

    constexpr s_color_rgba32f ColorAsGrayscale(const s_color_rgba32f col) {
        const t_f32 lum = ColorLuminance(col);
        return {lum, lum, lum, col.a};
    }

    constexpr s_color_rgba8 ColorFromHex(const t_u32 hex) {
        const auto r = static_cast<t_u8>((hex & 0xFF000000) >> 24);
        const auto g = static_cast<t_u8>((hex & 0x00FF0000) >> 16);
        const auto b = static_cast<t_u8>((hex & 0x0000FF00) >> 8);
        const auto a = static_cast<t_u8>(hex & 0x000000FF);

        return {r, g, b, a};
    }

    namespace colors {
        constexpr s_color_rgba32f g_black = {0.0f, 0.0f, 0.0f, 1.0f};
        constexpr s_color_rgba32f g_dark_gray = {0.25f, 0.25f, 0.25f, 1.0f};
        constexpr s_color_rgba32f g_gray = {0.5f, 0.5f, 0.5f, 1.0f};
        constexpr s_color_rgba32f g_light_gray = {0.75f, 0.75f, 0.75f, 1.0f};
        constexpr s_color_rgba32f g_white = {1.0f, 1.0f, 1.0f, 1.0f};
        constexpr s_color_rgba32f g_red = {1.0f, 0.0f, 0.0f, 1.0f};
        constexpr s_color_rgba32f g_orange = {1.0f, 0.5f, 0.0f, 1.0f};
        constexpr s_color_rgba32f g_yellow = {1.0f, 1.0f, 0.0f, 1.0f};
        constexpr s_color_rgba32f g_lime = {0.75f, 1.0f, 0.0f, 1.0f};
        constexpr s_color_rgba32f g_green = {0.0f, 1.0f, 0.0f, 1.0f};
        constexpr s_color_rgba32f g_teal = {0.0f, 0.5f, 0.5f, 1.0f};
        constexpr s_color_rgba32f g_cyan = {0.0f, 1.0f, 1.0f, 1.0f};
        constexpr s_color_rgba32f g_blue = {0.0f, 0.0f, 1.0f, 1.0f};
        constexpr s_color_rgba32f g_purple = {0.5f, 0.0f, 0.5f, 1.0f};
        constexpr s_color_rgba32f g_magenta = {1.0f, 0.0f, 1.0f, 1.0f};
        constexpr s_color_rgba32f g_pink = {1.0f, 0.75f, 0.8f, 1.0f};
        constexpr s_color_rgba32f g_brown = {0.6f, 0.3f, 0.0f, 1.0f};
    }

    namespace origins {
        constexpr s_v2<t_f32> g_topleft = {0.0f, 0.0f};
        constexpr s_v2<t_f32> g_topcenter = {0.5f, 0.0f};
        constexpr s_v2<t_f32> g_topright = {1.0f, 0.0f};
        constexpr s_v2<t_f32> g_centerleft = {0.0f, 0.5f};
        constexpr s_v2<t_f32> g_center = {0.5f, 0.5f};
        constexpr s_v2<t_f32> g_centerright = {1.0f, 0.5f};
        constexpr s_v2<t_f32> g_bottomleft = {0.0f, 1.0f};
        constexpr s_v2<t_f32> g_bottomcenter = {0.5f, 1.0f};
        constexpr s_v2<t_f32> g_bottomright = {1.0f, 1.0f};
    }

    namespace alignments {
        constexpr s_v2<t_f32> g_topleft = {0.0f, 0.0f};
        constexpr s_v2<t_f32> g_topcenter = {0.5f, 0.0f};
        constexpr s_v2<t_f32> g_topright = {1.0f, 0.0f};
        constexpr s_v2<t_f32> g_centerleft = {0.0f, 0.5f};
        constexpr s_v2<t_f32> g_center = {0.5f, 0.5f};
        constexpr s_v2<t_f32> g_centerright = {1.0f, 0.5f};
        constexpr s_v2<t_f32> g_bottomleft = {0.0f, 1.0f};
        constexpr s_v2<t_f32> g_bottomcenter = {0.5f, 1.0f};
        constexpr s_v2<t_f32> g_bottomright = {1.0f, 1.0f};
    }

    struct s_rgba_texture_data_rdonly {
        s_v2<t_s32> size_in_pxs;
        s_array_rdonly<t_u8> px_data;
    };

    struct s_rgba_texture_data {
        s_v2<t_s32> size_in_pxs;
        s_array<t_u8> px_data;

        constexpr operator s_rgba_texture_data_rdonly() const {
            return {size_in_pxs, px_data};
        }
    };

    [[nodiscard]] t_b8 LoadRGBATextureDataFromRaw(const s_str_rdonly file_path, s_mem_arena& mem_arena, s_rgba_texture_data& o_tex_data);

    [[nodiscard]] t_b8 PackTexture(const s_rgba_texture_data_rdonly& tex_data, const s_str_rdonly file_path, s_mem_arena& temp_mem_arena);
    [[nodiscard]] t_b8 UnpackTexture(const s_str_rdonly file_path, s_mem_arena& mem_arena, s_rgba_texture_data& o_tex_data);

// Assume one-to-many relationship between glyphs and codepoints.
    // Therefore number of codepoints >= number of glyphs.

    constexpr s_v2<t_s32> g_atlas_size = {1024, 1024};

    using t_font_atlas = s_static_array<t_u8, 4 * g_atlas_size.x * g_atlas_size.y>;

    struct s_font_glyph_info {
        // These are for determining positioning relative to other characters.
        s_v2<t_s32> offs;
        s_v2<t_s32> size;
        t_s32 adv;

        // In what texture atlas is this glyph, and where?
        t_size atlas_index;
        s_rect<t_s32> atlas_rect;
    };

    struct s_codepoint_pair {
        t_s32 a;
        t_s32 b;
    };

    struct s_font {
        t_s32 line_height;

        s_hash_map<t_s32, s_font_glyph_info> codepoints_to_glyph_infos; // Some duplicity here since a single glyph might have multiple codepoints mapped to it.
        s_hash_map<s_codepoint_pair, t_s32> codepoint_pairs_to_kernings;
        s_array<t_font_atlas> atlases;
    };

    [[nodiscard]] t_b8 LoadFontFromRaw(s_mem_arena& mem_arena, const s_str_rdonly file_path, const t_s32 height, const s_array_rdonly<t_s32> codepoints_no_dups, s_mem_arena& temp_mem_arena, s_font& o_font);

    [[nodiscard]] t_b8 PackFont(const s_font& font, const s_str_rdonly file_path, s_mem_arena& temp_mem_arena);
    [[nodiscard]] t_b8 UnpackFont(const s_str_rdonly file_path, s_mem_arena& mem_arena, s_font& o_font);
}
