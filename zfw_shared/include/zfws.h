#ifndef ZFWS_H
#define ZFWS_H

#include <cu.h>

#define ASCII_PRINTABLE_MIN ' '
#define ASCII_PRINTABLE_MAX '~'
#define ASCII_PRINTABLE_RANGE_LEN (ASCII_PRINTABLE_MAX - ASCII_PRINTABLE_MIN + 1)

typedef struct {
    t_s32 line_height;

    s_v2_s32 chr_offsets[ASCII_PRINTABLE_RANGE_LEN];
    s_v2_s32 chr_sizes[ASCII_PRINTABLE_RANGE_LEN];
    t_s32 chr_advances[ASCII_PRINTABLE_RANGE_LEN];
} s_font_arrangement;

DEF_ARRAY_TYPE(s_font_arrangement, font_arrangement, FontArrangement);

typedef struct {
    s_v2_s32 size;
    t_s32 chr_xs[ASCII_PRINTABLE_RANGE_LEN];
} s_font_texture_meta;

DEF_ARRAY_TYPE(s_font_texture_meta, font_texture_meta, FontTextureMeta);

#endif
