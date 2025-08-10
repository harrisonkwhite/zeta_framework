#include "zfwap.h"

#include <stb_truetype.h>

static s_font_arrangement LoadFontArrangement(const stbtt_fontinfo* const stb_font_info, const t_s32 height) {
    s_font_arrangement arrangement_info = {0};

    const float scale = stbtt_ScaleForPixelHeight(stb_font_info, height);

    t_s32 vm_ascent, vm_descent, vm_line_gap;
    stbtt_GetFontVMetrics(stb_font_info, &vm_ascent, &vm_descent, &vm_line_gap);

    arrangement_info.line_height = (vm_ascent - vm_descent + vm_line_gap) * scale;

    for (t_s32 i = 0; i < ASCII_PRINTABLE_RANGE_LEN; i++) {
        const char chr = ASCII_PRINTABLE_MIN + i;

        s_rect_edges_s32 bitmap_box;
        stbtt_GetCodepointBitmapBox(stb_font_info, chr, scale, scale, &bitmap_box.left, &bitmap_box.top, &bitmap_box.right, &bitmap_box.bottom);

        s_v2_s32* const chr_offs = STATIC_ARRAY_ELEM(arrangement_info.chr_offsets, i);
        *chr_offs = (s_v2_s32){bitmap_box.left, bitmap_box.top + (vm_ascent * scale)};

        s_v2_s32* const chr_size = STATIC_ARRAY_ELEM(arrangement_info.chr_sizes, i);
        *chr_size = (s_v2_s32){bitmap_box.right - bitmap_box.left, bitmap_box.bottom - bitmap_box.top};

        t_s32 hm_advance;
        stbtt_GetCodepointHMetrics(stb_font_info, chr, &hm_advance, NULL);

        t_s32* const chr_advance = STATIC_ARRAY_ELEM(arrangement_info.chr_advances, i);
        *chr_advance = hm_advance * scale;
    }

    return arrangement_info;
}

static s_font_texture_meta LoadFontTexMeta(const stbtt_fontinfo* const stb_font_info, const t_s32 height, const s_font_arrangement* const arrangement) {
    s_font_texture_meta tex_meta = {0};

    t_s32 chr_x_pen = 0;

    for (t_s32 i = 0; i < ASCII_PRINTABLE_RANGE_LEN; i++) {
        const t_s32 chr_margin = 4;

        chr_x_pen += chr_margin;

        *STATIC_ARRAY_ELEM(tex_meta.chr_xs, i) = chr_x_pen;

        const s_v2_s32 chr_size = *STATIC_ARRAY_ELEM(arrangement->chr_sizes, i);
        chr_x_pen += chr_size.x;

        chr_x_pen += chr_margin;

        tex_meta.size.y = MAX(chr_size.y, tex_meta.size.y);
    }

    tex_meta.size.x = chr_x_pen;

    return tex_meta;
}

static s_u8_array_view GenFontTextureRGBAPixelData(s_mem_arena* const mem_arena, const stbtt_fontinfo* const stb_font_info, const t_s32 height, const s_font_arrangement* const arrangement, const s_font_texture_meta tex_meta) {
    // Reserve the needed memory based on font texture size.
    const s_u8_array rgba_px_data = PushU8ArrayToMemArena(mem_arena, 4 * tex_meta.size.x * tex_meta.size.y);

    if (IS_ZERO(rgba_px_data)) {
        LOG_ERROR("Failed to reserve memory for font texture RGBA pixel data!");
        return (s_u8_array_view){0};
    }

    // Clear the pixel data to transparent white.
    for (t_s32 i = 0; i < rgba_px_data.len; i += 4) {
        *U8Elem(rgba_px_data, i + 0) = 255;
        *U8Elem(rgba_px_data, i + 1) = 255;
        *U8Elem(rgba_px_data, i + 2) = 255;
        *U8Elem(rgba_px_data, i + 3) = 0;
    }

    // Write the pixel data of each character.
    const float scale = stbtt_ScaleForPixelHeight(stb_font_info, height);

    for (t_s32 i = 0; i < ASCII_PRINTABLE_RANGE_LEN; i++) {
        const char chr = ASCII_PRINTABLE_MIN + i;

        if (chr == ' ') {
            // No bitmap for the space character.
            continue;
        }

        unsigned char* const stb_bitmap = stbtt_GetCodepointBitmap(stb_font_info, scale, scale, chr, NULL, NULL, NULL, NULL);

        if (!stb_bitmap) {
            LOG_ERROR("Failed to get bitmap for character '%c' through STB!", chr);
            return (s_u8_array_view){0};
        }

        const s_v2_s32 chr_size = *STATIC_ARRAY_ELEM(arrangement->chr_sizes, i);

        const t_s32 chr_x = *STATIC_ARRAY_ELEM(tex_meta.chr_xs, i);

        for (t_s32 y = 0; y < chr_size.y; y++) {
            for (t_s32 xo = 0; xo < chr_size.x; xo++) {
                const t_s32 px_index = ((y * tex_meta.size.x) + chr_x + xo) * 4;
                const t_s32 stb_bitmap_index = IndexFrom2D(xo, y, chr_size.x);
                *U8Elem(rgba_px_data, px_index + 3) = stb_bitmap[stb_bitmap_index];
            }
        }

        stbtt_FreeBitmap(stb_bitmap, NULL);
    }

    return U8ArrayView(rgba_px_data);
}

static bool OutputFontFile(const s_char_array_view file_path, const s_font_arrangement* const arrangement, const s_font_texture_meta tex_meta, const s_u8_array_view tex_rgba_px_data) {
    FILE* const fs = fopen(file_path.buf_raw, "wb");

    if (!fs) {
        return false;
    }

    if (fwrite(arrangement, sizeof(*arrangement), 1, fs) < 1) {
        fclose(fs);
        return false;
    }

    if (fwrite(&tex_meta, sizeof(tex_meta), 1, fs) < 1) {
        fclose(fs);
        return false;
    }

    if (fwrite(tex_rgba_px_data.buf_raw, 1, tex_rgba_px_data.len, fs) < tex_rgba_px_data.len) {
        fclose(fs);
        return false;
    }

    fclose(fs);

    return true;
}

bool PackFont(const s_char_array_view file_path, const t_s32 height, const s_char_array_view output_file_path, s_mem_arena* const temp_mem_arena) {
    // Get the plain font file data.
    const s_u8_array file_data = LoadFileContents(file_path, temp_mem_arena, false);

    if (IS_ZERO(file_data)) {
        LOG_ERROR("Failed to reserve memory for font file contents!");
        return false;
    }

    // Initialise the font through STB.
    stbtt_fontinfo stb_font_info;

    const t_s32 offs = stbtt_GetFontOffsetForIndex(file_data.buf_raw, 0);

    if (offs == -1) {
        LOG_ERROR("Failed to get font offset!");
        return false;
    }

    if (!stbtt_InitFont(&stb_font_info, file_data.buf_raw, offs)) {
        LOG_ERROR("Failed to initialise font through STB!");
        return false;
    }

    // Generate font data and output the file.
    const s_font_arrangement arrangement = LoadFontArrangement(&stb_font_info, height);
    const s_font_texture_meta tex_meta = LoadFontTexMeta(&stb_font_info, height, &arrangement);
    const s_u8_array_view tex_rgba_px_data = GenFontTextureRGBAPixelData(temp_mem_arena, &stb_font_info, height, &arrangement, tex_meta);

    if (!OutputFontFile(output_file_path, &arrangement, tex_meta, tex_rgba_px_data)) {
        return false;
    }

    LOG_SUCCESS("Packed font from file \"%s\" with height \"%d\"!", file_path.buf_raw, height);

    return true;
}
