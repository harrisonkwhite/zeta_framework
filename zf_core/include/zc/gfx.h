#pragma once

#include <zc/mem/arrays.h>
#include <zc/math.h>
#include <zc/io.h>

namespace zf {
#if 0
    namespace colors {
        constexpr s_v4 g_black = {0.0f, 0.0f, 0.0f, 1.0f};
        constexpr s_v4 g_dark_gray = {0.25f, 0.25f, 0.25f, 1.0f};
        constexpr s_v4 g_gray = {0.5f, 0.5f, 0.5f, 1.0f};
        constexpr s_v4 g_light_gray = {0.75f, 0.75f, 0.75f, 1.0f};
        constexpr s_v4 g_white = {1.0f, 1.0f, 1.0f, 1.0f};
        constexpr s_v4 g_red = {1.0f, 0.0f, 0.0f, 1.0f};
        constexpr s_v4 g_orange = {1.0f, 0.5f, 0.0f, 1.0f};
        constexpr s_v4 g_yellow = {1.0f, 1.0f, 0.0f, 1.0f};
        constexpr s_v4 g_lime = {0.75f, 1.0f, 0.0f, 1.0f};
        constexpr s_v4 g_green = {0.0f, 1.0f, 0.0f, 1.0f};
        constexpr s_v4 g_teal = {0.0f, 0.5f, 0.5f, 1.0f};
        constexpr s_v4 g_cyan = {0.0f, 1.0f, 1.0f, 1.0f};
        constexpr s_v4 g_blue = {0.0f, 0.0f, 1.0f, 1.0f};
        constexpr s_v4 g_purple = {0.5f, 0.0f, 0.5f, 1.0f};
        constexpr s_v4 g_magenta = {1.0f, 0.0f, 1.0f, 1.0f};
        constexpr s_v4 g_pink = {1.0f, 0.75f, 0.8f, 1.0f};
        constexpr s_v4 g_brown = {0.6f, 0.3f, 0.0f, 1.0f};
    }
#endif

    namespace origins {
        constexpr s_v2 g_origin_top_left = {0.0f, 0.0f};
        constexpr s_v2 g_origin_top_center = {0.5f, 0.0f};
        constexpr s_v2 g_origin_top_right = {1.0f, 0.0f};
        constexpr s_v2 g_origin_center_left = {0.0f, 0.5f};
        constexpr s_v2 g_origin_center = {0.5f, 0.5f};
        constexpr s_v2 g_origin_center_right = {1.0f, 0.5f};
        constexpr s_v2 g_origin_bottom_left = {0.0f, 1.0f};
        constexpr s_v2 g_origin_bottom_center = {0.5f, 1.0f};
        constexpr s_v2 g_origin_bottom_right = {1.0f, 1.0f};
    }

    namespace alignments {
        constexpr s_v2 g_alignment_top_left = {0.0f, 0.0f};
        constexpr s_v2 g_alignment_top_center = {0.5f, 0.0f};
        constexpr s_v2 g_alignment_top_right = {1.0f, 0.0f};
        constexpr s_v2 g_alignment_center_left = {0.0f, 0.5f};
        constexpr s_v2 g_alignment_center = {0.5f, 0.5f};
        constexpr s_v2 g_alignment_center_right = {1.0f, 0.5f};
        constexpr s_v2 g_alignment_bottom_left = {0.0f, 1.0f};
        constexpr s_v2 g_alignment_bottom_center = {0.5f, 1.0f};
        constexpr s_v2 g_alignment_bottom_right = {1.0f, 1.0f};
    }

#if 0
    struct s_int_rgba {
        t_byte r = 0;
        t_byte g = 0;
        t_byte b = 0;
        t_byte a = 0;

        unsigned int RGBA() const {
            return *reinterpret_cast<const unsigned int*>(this);
        }
    };

    inline s_int_rgba ToIntRGBA(const s_v4 flt) {
        return {
            static_cast<t_byte>(roundf(flt.x * 255.0f)),
            static_cast<t_byte>(roundf(flt.y * 255.0f)),
            static_cast<t_byte>(roundf(flt.z * 255.0f)),
            static_cast<t_byte>(roundf(flt.w * 255.0f))
        };
    }
#endif

    struct s_rgba_texture {
        s_v2<int> dims;
        c_array<t_byte> px_data;
    };

    struct s_font_arrangement {
        int line_height = 0;

        s_static_array<s_v2<int>, g_ascii_printable_range_len> chr_offsets;
        s_static_array<s_v2<int>, g_ascii_printable_range_len> chr_sizes;
        s_static_array<int, g_ascii_printable_range_len> chr_advances;
    };

    struct s_font_texture_meta {
        s_v2<int> size;
        s_static_array<int, g_ascii_printable_range_len> chr_xs;
    };

    [[nodiscard]] bool LoadRGBATextureFromRawFile(s_rgba_texture& tex, c_mem_arena& mem_arena, const s_str_view file_path);
    bool LoadFontFromRawFile(s_font_arrangement& arrangement, s_font_texture_meta& tex_meta, c_array<t_byte>& tex_rgba_px_data, const s_str_view file_path, const int height, c_mem_arena& temp_mem_arena);

    bool PackTexture(s_file_stream& fs, const s_rgba_texture rgba_tex);
    void UnpackTexture(s_file_stream& fs, s_rgba_texture& rgba_tex);

    bool PackFont(s_file_stream& fs, const s_font_arrangement& arrangement, const s_font_texture_meta tex_meta, const c_array<const t_byte> tex_rgba_px_data);
    void UnpackFont(s_file_stream& fs, s_font_arrangement& arrangement, s_font_texture_meta tex_meta, c_array<const t_byte>& tex_rgba_px_data);
}
