#pragma once

#include <cu.h>

#define ASCII_PRINTABLE_MIN ' '
#define ASCII_PRINTABLE_MAX '~'
#define ASCII_PRINTABLE_RANGE_LEN (ASCII_PRINTABLE_MAX - ASCII_PRINTABLE_MIN + 1)

struct s_rgba_texture {
    s_v2_s32 tex_size;
    c_array<t_u8> px_data;
};

struct s_font_arrangement {
    t_s32 line_height = 0;

    c_static_array<s_v2_s32, ASCII_PRINTABLE_RANGE_LEN> chr_offsets;
    c_static_array<s_v2_s32, ASCII_PRINTABLE_RANGE_LEN> chr_sizes;
    c_static_array<t_s32, ASCII_PRINTABLE_RANGE_LEN> chr_advances;
};

struct s_font_texture_meta {
    s_v2_s32 size;
    c_static_array<t_s32, ASCII_PRINTABLE_RANGE_LEN> chr_xs;
};

[[nodiscard]] bool LoadRGBATextureFromRawFile(s_rgba_texture& tex, c_mem_arena& mem_arena, const c_array<const char> file_path);
