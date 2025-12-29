#pragma once

#include <zcl/zcl_io.h>
#include <zcl/zcl_math.h>
#include <zcl/zcl_ds.h>

namespace zf {
    // ============================================================
    // @section: Colours
    // ============================================================
    struct s_color_rgba32f {
        t_f32 r = 0.0f;
        t_f32 g = 0.0f;
        t_f32 b = 0.0f;
        t_f32 a = 0.0f;

        constexpr s_color_rgba32f() = default;

        constexpr s_color_rgba32f(const t_f32 r, const t_f32 g, const t_f32 b, const t_f32 a) : r(r), g(g), b(b), a(a) {
            ZF_ASSERT(r >= 0.0f && r <= 1.0f);
            ZF_ASSERT(g >= 0.0f && g <= 1.0f);
            ZF_ASSERT(b >= 0.0f && b <= 1.0f);
            ZF_ASSERT(a >= 0.0f && a <= 1.0f);
        }

        constexpr t_b8 IsValid() const {
            return r >= 0.0f && r <= 1.0f
                && g >= 0.0f && g <= 1.0f
                && b >= 0.0f && b <= 1.0f
                && a >= 0.0f && a <= 1.0f;
        }

        // @todo
        constexpr t_f32 R() const { return r; }
        constexpr t_f32 G() const { return g; }
        constexpr t_f32 B() const { return b; }
        constexpr t_f32 A() const { return a; }

        constexpr operator s_v4() const {
            return {r, g, b, a};
        }
    };

    struct s_color_rgb24f {
        t_f32 r = 0.0f;
        t_f32 g = 0.0f;
        t_f32 b = 0.0f;

        constexpr s_color_rgb24f() = default;

        constexpr s_color_rgb24f(const t_f32 r, const t_f32 g, const t_f32 b) : r(r), g(g), b(b) {
            ZF_ASSERT(r >= 0.0f && r <= 1.0f);
            ZF_ASSERT(g >= 0.0f && g <= 1.0f);
            ZF_ASSERT(b >= 0.0f && b <= 1.0f);
        }

        constexpr t_b8 IsValid() const {
            return r >= 0.0f && r <= 1.0f
                && g >= 0.0f && g <= 1.0f
                && b >= 0.0f && b <= 1.0f;
        }

        // @todo
        constexpr t_f32 R() const { return r; }
        constexpr t_f32 G() const { return g; }
        constexpr t_f32 B() const { return b; }

        constexpr operator s_color_rgba32f() const {
            return {r, g, b, 1.0f};
        }

        constexpr operator s_v3() const {
            return {r, g, b};
        }
    };

    struct s_color_rgba8 {
        t_u8 r = 0;
        t_u8 g = 0;
        t_u8 b = 0;
        t_u8 a = 0;

        constexpr s_color_rgba8() = default;
        constexpr s_color_rgba8(const t_u8 r, const t_u8 g, const t_u8 b, const t_u8 a) : r(r), g(g), b(b), a(a) {}
        constexpr s_color_rgba8(const s_color_rgba32f col) : r(static_cast<t_u8>(col.R() * 255.0f)), g(static_cast<t_u8>(col.G() * 255.0f)), b(static_cast<t_u8>(col.B() * 255.0f)), a(static_cast<t_u8>(col.A() * 255.0f)) {}
        constexpr s_color_rgba8(const s_color_rgb24f col) : r(static_cast<t_u8>(col.R() * 255.0f)), g(static_cast<t_u8>(col.G() * 255.0f)), b(static_cast<t_u8>(col.B() * 255.0f)), a(255) {}

        constexpr operator s_color_rgba32f() const {
            return {r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f};
        }
    };

    struct s_color_rgb8 {
        t_u8 r = 0;
        t_u8 g = 0;
        t_u8 b = 0;

        constexpr s_color_rgb8() = default;
        constexpr s_color_rgb8(const t_u8 r, const t_u8 g, const t_u8 b) : r(r), g(g), b(b) {}
        constexpr s_color_rgb8(const s_color_rgb24f col) : r(static_cast<t_u8>(col.R() * 255.0f)), g(static_cast<t_u8>(col.G() * 255.0f)), b(static_cast<t_u8>(col.B() * 255.0f)) {}

        constexpr operator s_color_rgba8() const {
            return {r, g, b, 255};
        }

        constexpr operator s_color_rgb24f() const {
            return {r / 255.0f, g / 255.0f, b / 255.0f};
        }

        constexpr operator s_color_rgba32f() const {
            return {r / 255.0f, g / 255.0f, b / 255.0f, 1.0f};
        }
    };

    constexpr s_color_rgb24f g_color_black = {0.0f, 0.0f, 0.0f};
    constexpr s_color_rgb24f g_color_dark_gray = {0.25f, 0.25f, 0.25f};
    constexpr s_color_rgb24f g_color_gray = {0.5f, 0.5f, 0.5f};
    constexpr s_color_rgb24f g_color_light_gray = {0.75f, 0.75f, 0.75f};
    constexpr s_color_rgb24f g_color_white = {1.0f, 1.0f, 1.0f};
    constexpr s_color_rgb24f g_color_red = {1.0f, 0.0f, 0.0f};
    constexpr s_color_rgb24f g_color_orange = {1.0f, 0.5f, 0.0f};
    constexpr s_color_rgb24f g_color_yellow = {1.0f, 1.0f, 0.0f};
    constexpr s_color_rgb24f g_color_lime = {0.75f, 1.0f, 0.0f};
    constexpr s_color_rgb24f g_color_green = {0.0f, 1.0f, 0.0f};
    constexpr s_color_rgb24f g_color_teal = {0.0f, 0.5f, 0.5f};
    constexpr s_color_rgb24f g_color_cyan = {0.0f, 1.0f, 1.0f};
    constexpr s_color_rgb24f g_color_blue = {0.0f, 0.0f, 1.0f};
    constexpr s_color_rgb24f g_color_purple = {0.5f, 0.0f, 0.5f};
    constexpr s_color_rgb24f g_color_magenta = {1.0f, 0.0f, 1.0f};
    constexpr s_color_rgb24f g_color_pink = {1.0f, 0.75f, 0.8f};
    constexpr s_color_rgb24f g_color_brown = {0.6f, 0.3f, 0.0f};

    constexpr s_color_rgba32f ColorMix(const s_color_rgba32f a, const s_color_rgba32f b, const t_f32 amount) {
        ZF_ASSERT(amount >= 0.0f && amount <= 1.0f);

        return {
            Lerp(a.R(), b.R(), amount),
            Lerp(a.G(), b.G(), amount),
            Lerp(a.B(), b.B(), amount),
            Lerp(a.A(), b.A(), amount),
        };
    }

    constexpr t_f32 ColorLuminance(const s_color_rgba32f col) {
        return (0.2126f * col.R()) + (0.7152f * col.G()) + (0.0722f * col.B());
    }

    constexpr s_color_rgba32f ColorAsGrayscale(const s_color_rgba32f col) {
        const t_f32 lum = ColorLuminance(col);
        return {lum, lum, lum, col.A()};
    }

    constexpr t_u32 ColorToHex(const s_color_rgba8 col) {
        t_u32 res = 0;
        res |= static_cast<t_u32>(col.r) << 24;
        res |= static_cast<t_u32>(col.g) << 16;
        res |= static_cast<t_u32>(col.b) << 8;
        res |= static_cast<t_u32>(col.a);

        return res;
    }

    constexpr s_color_rgba8 ColorFromHex(const t_u32 hex) {
        const auto r = static_cast<t_u8>((hex & 0xFF000000) >> 24);
        const auto g = static_cast<t_u8>((hex & 0x00FF0000) >> 16);
        const auto b = static_cast<t_u8>((hex & 0x0000FF00) >> 8);
        const auto a = static_cast<t_u8>(hex & 0x000000FF);

        return {r, g, b, a};
    }

    // ============================================================
    // @section: Textures
    // ============================================================
    struct s_texture_data_rdonly {
    public:
        constexpr s_texture_data_rdonly() = default;

        constexpr s_texture_data_rdonly(const s_v2_i size_in_pxs, const s_array_rdonly<t_u8> rgba_px_data) : m_size_in_pxs(size_in_pxs), m_rgba_px_data(rgba_px_data) {
            ZF_ASSERT(rgba_px_data.Len() == 4 * size_in_pxs.x * size_in_pxs.y);
        }

        constexpr s_v2_i SizeInPixels() const { return m_size_in_pxs; }
        constexpr s_array_rdonly<t_u8> RGBAPixelData() const { return m_rgba_px_data; }

    private:
        s_v2_i m_size_in_pxs = {};
        s_array_rdonly<t_u8> m_rgba_px_data = {};
    };

    struct s_texture_data {
    public:
        constexpr s_texture_data() = default;

        constexpr s_texture_data(const s_v2_i size_in_pxs, const s_array_mut<t_u8> &rgba_px_data) : m_size_in_pxs(size_in_pxs), m_rgba_px_data(rgba_px_data) {
            ZF_ASSERT(rgba_px_data.Len() == 4 * size_in_pxs.x * size_in_pxs.y);
        }

        constexpr operator s_texture_data_rdonly() const {
            return {m_size_in_pxs, m_rgba_px_data};
        }

        constexpr s_v2_i SizeInPixels() const { return m_size_in_pxs; }
        constexpr s_array_rdonly<t_u8> RGBAPixelData() const { return m_rgba_px_data; }

    private:
        s_v2_i m_size_in_pxs = {};
        s_array_mut<t_u8> m_rgba_px_data = {};
    };

    [[nodiscard]] t_b8 LoadTextureDataFromRaw(const s_str_rdonly file_path, s_arena &texture_data_mem_arena, s_arena &temp_mem_arena, s_texture_data &o_texture_data);

    [[nodiscard]] t_b8 PackTexture(const s_str_rdonly file_path, const s_texture_data texture_data, s_arena &temp_mem_arena);
    [[nodiscard]] t_b8 UnpackTexture(const s_str_rdonly file_path, s_arena &texture_data_mem_arena, s_arena &temp_mem_arena, s_texture_data &o_texture_data);

    constexpr s_rect_f CalcUVRect(const s_rect_i src_rect, const s_v2_i tex_size) {
        ZF_ASSERT(tex_size.x > 0 && tex_size.y > 0);
        ZF_ASSERT(src_rect.x >= 0 && src_rect.y >= 0 && src_rect.width > 0 && src_rect.height > 0 && src_rect.Right() <= tex_size.x && src_rect.Bottom() <= tex_size.y);

        return {
            static_cast<t_f32>(src_rect.x) / static_cast<t_f32>(tex_size.x),
            static_cast<t_f32>(src_rect.y) / static_cast<t_f32>(tex_size.y),
            static_cast<t_f32>(src_rect.width) / static_cast<t_f32>(tex_size.x),
            static_cast<t_f32>(src_rect.height) / static_cast<t_f32>(tex_size.y),
        };
    }

    constexpr s_v2 g_origin_topleft = {0.0f, 0.0f};
    constexpr s_v2 g_origin_topcenter = {0.5f, 0.0f};
    constexpr s_v2 g_origin_topright = {1.0f, 0.0f};
    constexpr s_v2 g_origin_centerleft = {0.0f, 0.5f};
    constexpr s_v2 g_origin_center = {0.5f, 0.5f};
    constexpr s_v2 g_origin_centerright = {1.0f, 0.5f};
    constexpr s_v2 g_origin_bottomleft = {0.0f, 1.0f};
    constexpr s_v2 g_origin_bottomcenter = {0.5f, 1.0f};
    constexpr s_v2 g_origin_bottomright = {1.0f, 1.0f};

    constexpr t_b8 IsOriginValid(const s_v2 origin) {
        return origin.x >= 0.0f && origin.x <= 1.0f && origin.y >= 0.0f && origin.y <= 1.0f;
    }

    // ============================================================
    // @section: Fonts
    // ============================================================
    constexpr s_v2_i g_font_atlas_size = {1024, 1024};

    using t_font_atlas_rgba = s_static_array<t_u8, 4 * g_font_atlas_size.x * g_font_atlas_size.y>;

    struct s_font_glyph_info {
        // These are for determining positioning relative to other characters.
        s_v2_i offs = {};
        s_v2_i size = {};
        t_i32 adv = 0;

        // In what texture atlas is this glyph, and where?
        t_i32 atlas_index = 0;
        s_rect_i atlas_rect = {};
    };

    struct s_font_code_point_pair {
        t_code_pt a = 0;
        t_code_pt b = 0;
    };

    struct s_font_arrangement {
        t_i32 line_height = 0;

        s_hash_map<t_code_pt, s_font_glyph_info> code_pts_to_glyph_infos = {}; // Some duplicity here since a single glyph might have multiple code points mapped to it.

        t_b8 has_kernings = false;
        s_hash_map<s_font_code_point_pair, t_i32> code_pt_pairs_to_kernings = {};
    };

    [[nodiscard]] t_b8 LoadFontDataFromRaw(const s_str_rdonly file_path, const t_i32 height, t_code_pt_bit_vec &code_pts, s_arena &arrangement_mem_arena, s_arena &atlas_rgbas_mem_arena, s_arena &temp_mem_arena, s_font_arrangement &o_arrangement, s_array_mut<t_font_atlas_rgba> &o_atlas_rgbas);

    [[nodiscard]] t_b8 PackFont(const s_str_rdonly file_path, const s_font_arrangement &arrangement, const s_array_rdonly<t_font_atlas_rgba> atlas_rgbas, s_arena &temp_mem_arena);
    [[nodiscard]] t_b8 UnpackFont(const s_str_rdonly file_path, s_arena &arrangement_mem_arena, s_arena &atlas_rgbas_mem_arena, s_arena &temp_mem_arena, s_font_arrangement &o_arrangement, s_array_mut<t_font_atlas_rgba> &o_atlas_rgbas);

    constexpr s_v2 g_alignment_topleft = {0.0f, 0.0f};
    constexpr s_v2 g_alignment_topcenter = {0.5f, 0.0f};
    constexpr s_v2 g_alignment_topright = {1.0f, 0.0f};
    constexpr s_v2 g_alignment_centerleft = {0.0f, 0.5f};
    constexpr s_v2 g_alignment_center = {0.5f, 0.5f};
    constexpr s_v2 g_alignment_centerright = {1.0f, 0.5f};
    constexpr s_v2 g_alignment_bottomleft = {0.0f, 1.0f};
    constexpr s_v2 g_alignment_bottomcenter = {0.5f, 1.0f};
    constexpr s_v2 g_alignment_bottomright = {1.0f, 1.0f};

    constexpr t_b8 IsAlignmentValid(const s_v2 alignment) {
        return alignment.x >= 0.0f && alignment.x <= 1.0f && alignment.y >= 0.0f && alignment.y <= 1.0f;
    }

    // ============================================================
    // @section: Shaders
    // ============================================================
    [[nodiscard]] t_b8 PackShader(const s_str_rdonly file_path, const s_array_rdonly<t_u8> compiled_shader_bin, s_arena &temp_mem_arena);
    [[nodiscard]] t_b8 UnpackShader(const s_str_rdonly file_path, s_arena &shader_bin_mem_arena, s_arena &temp_mem_arena, s_array_mut<t_u8> &o_shader_bin);
}
