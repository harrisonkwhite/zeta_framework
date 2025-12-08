#pragma once

#include <zcl/zcl_io.h>
#include <zcl/zcl_math.h>
#include <zcl/zcl_ds.h>

namespace zf {
    // ============================================================
    // @section: Colours
    // ============================================================
    struct s_color_rgba32f {
        t_f32 r;
        t_f32 g;
        t_f32 b;
        t_f32 a;

        constexpr s_color_rgba32f() = default;
        constexpr s_color_rgba32f(const t_f32 r, const t_f32 g, const t_f32 b, const t_f32 a)
            : r(r), g(g), b(b), a(a) {}
        constexpr s_color_rgba32f(const s_v4<t_f32> v) : r(v.x), g(v.y), b(v.z), a(v.w) {}

        constexpr operator s_v4<t_f32>() const {
            return {r, g, b, a};
        }
    };

    struct s_color_rgba24f {
        t_f32 r;
        t_f32 g;
        t_f32 b;

        constexpr s_color_rgba24f() = default;
        constexpr s_color_rgba24f(const t_f32 r, const t_f32 g, const t_f32 b)
            : r(r), g(g), b(b) {}
        constexpr s_color_rgba24f(const s_v3<t_f32> v) : r(v.x), g(v.y), b(v.z) {}

        constexpr operator s_color_rgba32f() const {
            return {r, g, b, 1.0f};
        }

        constexpr operator s_v3<t_f32>() const {
            return {r, g, b};
        }
    };

    struct s_color_rgba8 {
        t_u8 r;
        t_u8 g;
        t_u8 b;
        t_u8 a;

        constexpr operator s_color_rgba32f() const {
            return {r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f};
        }
    };

    constexpr t_b8 IsColorValid(const s_color_rgba32f color) {
        return color.r >= 0.0f && color.r <= 1.0f && color.g >= 0.0f && color.g <= 1.0f &&
               color.b >= 0.0f && color.b <= 1.0f && color.a >= 0.0f && color.a <= 1.0f;
    }

    constexpr s_color_rgba32f ColorsMixed(const s_color_rgba32f a, const s_color_rgba32f b,
                                          const t_f32 amount) {
        ZF_ASSERT(amount >= 0.0f && amount <= 1.0f);

        return {Lerp(a.r, b.r, amount), Lerp(a.g, b.g, amount), Lerp(a.b, b.b, amount),
                Lerp(a.a, b.a, amount)};
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
        constexpr s_color_rgba24f g_black = {0.0f, 0.0f, 0.0f};
        constexpr s_color_rgba24f g_dark_gray = {0.25f, 0.25f, 0.25f};
        constexpr s_color_rgba24f g_gray = {0.5f, 0.5f, 0.5f};
        constexpr s_color_rgba24f g_light_gray = {0.75f, 0.75f, 0.75f};
        constexpr s_color_rgba24f g_white = {1.0f, 1.0f, 1.0f};
        constexpr s_color_rgba24f g_red = {1.0f, 0.0f, 0.0f};
        constexpr s_color_rgba24f g_orange = {1.0f, 0.5f, 0.0f};
        constexpr s_color_rgba24f g_yellow = {1.0f, 1.0f, 0.0f};
        constexpr s_color_rgba24f g_lime = {0.75f, 1.0f, 0.0f};
        constexpr s_color_rgba24f g_green = {0.0f, 1.0f, 0.0f};
        constexpr s_color_rgba24f g_teal = {0.0f, 0.5f, 0.5f};
        constexpr s_color_rgba24f g_cyan = {0.0f, 1.0f, 1.0f};
        constexpr s_color_rgba24f g_blue = {0.0f, 0.0f, 1.0f};
        constexpr s_color_rgba24f g_purple = {0.5f, 0.0f, 0.5f};
        constexpr s_color_rgba24f g_magenta = {1.0f, 0.0f, 1.0f};
        constexpr s_color_rgba24f g_pink = {1.0f, 0.75f, 0.8f};
        constexpr s_color_rgba24f g_brown = {0.6f, 0.3f, 0.0f};
    }

    // ============================================================
    // @section: Textures
    // ============================================================
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

    constexpr t_b8 IsOriginValid(const s_v2<t_f32> origin) {
        return origin.x >= 0.0f && origin.x <= 1.0f && origin.y >= 0.0f && origin.y <= 1.0f;
    }

    struct s_rgba_texture_data_rdonly {
        s_v2<t_s32> size_in_pxs;
        s_array_rdonly<t_u8> px_data;
    };

    struct s_texture_data {
        s_v2<t_s32> size_in_pxs;
        s_array<t_u8> rgba_px_data;

        constexpr operator s_rgba_texture_data_rdonly() const {
            return {size_in_pxs, rgba_px_data};
        }
    };

    [[nodiscard]] t_b8 LoadTextureDataFromRaw(const s_str_rdonly file_path,
                                              s_texture_data *const tex_data,
                                              s_mem_arena *const tex_data_mem_arena,
                                              s_mem_arena *const temp_mem_arena);

    [[nodiscard]] t_b8 PackTexture(const s_str_rdonly dest_file_path,
                                   const s_str_rdonly src_file_path,
                                   s_mem_arena &temp_mem_arena);
    [[nodiscard]] t_b8 UnpackTexture(const s_str_rdonly file_path, s_mem_arena &mem_arena,
                                     s_mem_arena &temp_mem_arena, s_texture_data &o_tex_data);

    inline s_rect<t_f32> CalcTextureCoords(const s_rect<t_s32> src_rect,
                                           const s_v2<t_s32> tex_size) {
        return {static_cast<t_f32>(src_rect.x) / static_cast<t_f32>(tex_size.x),
                static_cast<t_f32>(src_rect.y) / static_cast<t_f32>(tex_size.y),
                static_cast<t_f32>(src_rect.width) / static_cast<t_f32>(tex_size.x),
                static_cast<t_f32>(src_rect.height) / static_cast<t_f32>(tex_size.y)};
    }

    // ============================================================
    // @section: Fonts
    // ============================================================
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

    constexpr t_b8 IsAlignmentValid(const s_v2<t_f32> alignment) {
        return alignment.x >= 0.0f && alignment.x <= 1.0f && alignment.y >= 0.0f &&
               alignment.y <= 1.0f;
    }

    constexpr s_v2<t_s32> g_font_atlas_size = {1024, 1024};

    using t_font_atlas_rgba =
        s_static_array<t_u8, 4 * g_font_atlas_size.x * g_font_atlas_size.y>;

    struct s_font_glyph_info {
        // These are for determining positioning relative to other characters.
        s_v2<t_s32> offs;
        s_v2<t_s32> size;
        t_s32 adv;

        // In what texture atlas is this glyph, and where?
        t_size atlas_index;
        s_rect<t_s32> atlas_rect;
    };

    struct s_font_code_point_pair {
        t_unicode_code_pt a;
        t_unicode_code_pt b;
    };

    struct s_font_arrangement {
        t_s32 line_height;

        s_hash_map<t_unicode_code_pt, s_font_glyph_info>
            code_pts_to_glyph_infos; // Some duplicity here since a single glyph might have
                                     // multiple code points mapped to it.

        t_b8 has_kernings;
        s_hash_map<s_font_code_point_pair, t_s32> code_pt_pairs_to_kernings;
    };

    enum e_font_load_from_raw_result : t_s32 {
        ek_font_load_from_raw_result_success,
        ek_font_load_from_raw_result_no_code_pts_given,
        ek_font_load_from_raw_result_unsupported_code_pt,
        ek_font_load_from_raw_result_other_err
    };

    [[nodiscard]] e_font_load_from_raw_result LoadFontFromRaw(
        const s_str_rdonly file_path, const t_s32 height,
        const t_unicode_code_pt_bit_vec &code_pts, s_mem_arena &arrangement_mem_arena,
        s_mem_arena &atlas_rgbas_mem_arena, s_mem_arena &temp_mem_arena,
        s_font_arrangement &o_arrangement, s_array<t_font_atlas_rgba> &o_atlas_rgbas,
        t_unicode_code_pt_bit_vec *const o_unsupported_code_pts = nullptr);

    [[nodiscard]] t_b8 PackFont(
        const s_str_rdonly dest_file_path, const s_str_rdonly src_file_path,
        const t_s32 height, const t_unicode_code_pt_bit_vec &code_pts,
        s_mem_arena &temp_mem_arena, e_font_load_from_raw_result &o_font_load_from_raw_res,
        t_unicode_code_pt_bit_vec *const o_unsupported_code_pts = nullptr);
    [[nodiscard]] t_b8 UnpackFont(const s_str_rdonly file_path,
                                  s_mem_arena &arrangement_mem_arena,
                                  s_mem_arena &atlas_rgbas_mem_arena,
                                  s_mem_arena &temp_mem_arena,
                                  s_font_arrangement &o_arrangement,
                                  s_array<t_font_atlas_rgba> &o_atlas_rgbas);
    [[nodiscard]] t_b8 LoadStrChrDrawPositions(
        const s_str_rdonly str, const s_font_arrangement &font_arrangement,
        const s_v2<t_f32> pos, const s_v2<t_f32> alignment, s_mem_arena &mem_arena,
        s_array<s_v2<t_f32>>
            &o_positions); // Creates an array of length n, where n is the number of CHARACTERS
                           // (not bytes) in the string, including non-printable characters.
                           // Each element is the top-left position of the corresponding
                           // character. Ignore any elements associated with non-printable
                           // characters, including '\n'.

    // ============================================================
    // @section: Shaders
    // ============================================================
    [[nodiscard]] t_b8 PackShaderProg(const s_str_rdonly dest_file_path,
                                      const s_str_rdonly vert_file_path,
                                      const s_str_rdonly frag_file_path,
                                      s_mem_arena &temp_mem_arena);
    [[nodiscard]] t_b8 UnpackShaderProg(const s_str_rdonly file_path, s_mem_arena &mem_arena,
                                        s_mem_arena &temp_mem_arena, s_str &o_vert_src,
                                        s_str &o_frag_src);
}
