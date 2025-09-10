#pragma once

#include <cu.h>

constexpr char g_ascii_printable_min = ' ';
constexpr char g_ascii_printable_max = '~';
constexpr int g_ascii_printable_range_len = g_ascii_printable_max - g_ascii_printable_min + 1;

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
