#pragma once

#include "zc_math.h"

namespace zf {
    constexpr char g_ascii_printable_min = ' ';
    constexpr char g_ascii_printable_max = '~';
    constexpr int g_ascii_printable_range_len = g_ascii_printable_max - g_ascii_printable_min + 1;

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

    struct s_rgba_texture {
        s_v2_s32 tex_size;
        c_array<t_u8> px_data;
    };

    struct s_font_arrangement {
        t_s32 line_height = 0;

        s_static_array<s_v2_s32, g_ascii_printable_range_len> chr_offsets;
        s_static_array<s_v2_s32, g_ascii_printable_range_len> chr_sizes;
        s_static_array<t_s32, g_ascii_printable_range_len> chr_advances;
    };

    struct s_font_texture_meta {
        s_v2_s32 size;
        s_static_array<t_s32, g_ascii_printable_range_len> chr_xs;
    };

    [[nodiscard]] bool LoadRGBATextureFromRawFile(s_rgba_texture& tex, c_mem_arena& mem_arena, const c_string_view file_path);
}
