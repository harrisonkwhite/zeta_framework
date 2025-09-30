#include "zap.h"

#include <stb_truetype.h>

namespace zf {
    static s_font_arrangement LoadFontArrangement(const stbtt_fontinfo& stb_font_info, const t_s32 height) {
        s_font_arrangement arrangement_info = {};

        const float scale = stbtt_ScaleForPixelHeight(&stb_font_info, height);

        t_s32 vm_ascent, vm_descent, vm_line_gap;
        stbtt_GetFontVMetrics(&stb_font_info, &vm_ascent, &vm_descent, &vm_line_gap);

        arrangement_info.line_height = (vm_ascent - vm_descent + vm_line_gap) * scale;

        for (t_s32 i = 0; i < g_ascii_printable_range_len; i++) {
            const char chr = g_ascii_printable_min + i;

            s_rect_edges_s32 bitmap_box;
            stbtt_GetCodepointBitmapBox(&stb_font_info, chr, scale, scale, &bitmap_box.left, &bitmap_box.top, &bitmap_box.right, &bitmap_box.bottom);

            arrangement_info.chr_offsets[i] = {bitmap_box.left, static_cast<t_s32>(bitmap_box.top + (vm_ascent * scale))};
            arrangement_info.chr_sizes[i] = {bitmap_box.right - bitmap_box.left, bitmap_box.bottom - bitmap_box.top};

            t_s32 hm_advance;
            stbtt_GetCodepointHMetrics(&stb_font_info, chr, &hm_advance, nullptr);

            arrangement_info.chr_advances[i] = hm_advance * scale;
        }

        return arrangement_info;
    }

    static s_font_texture_meta LoadFontTexMeta(const stbtt_fontinfo& stb_font_info, const t_s32 height, const s_font_arrangement& arrangement) {
        s_font_texture_meta tex_meta = {};

        t_s32 chr_x_pen = 0;

        for (t_s32 i = 0; i < g_ascii_printable_range_len; i++) {
            const t_s32 chr_margin = 2;

            chr_x_pen += chr_margin;

            tex_meta.chr_xs[i] = chr_x_pen;

            const s_v2_s32 chr_size = arrangement.chr_sizes[i];
            chr_x_pen += chr_size.x;

            chr_x_pen += chr_margin;

            tex_meta.size.y = Max(chr_size.y, tex_meta.size.y);
        }

        tex_meta.size.x = chr_x_pen;

        return tex_meta;
    }

    static c_array<const t_u8> GenFontTextureRGBAPixelData(c_mem_arena& mem_arena, const stbtt_fontinfo& stb_font_info, const t_s32 height, const s_font_arrangement& arrangement, const s_font_texture_meta tex_meta) {
        // Reserve the needed memory based on font texture size.
        const auto rgba_px_data = PushArrayToMemArena<t_u8>(mem_arena, 4 * tex_meta.size.x * tex_meta.size.y);

        if (rgba_px_data.IsEmpty()) {
            //ZF_LOG_ERROR("Failed to reserve memory for font texture RGBA pixel data!");
            return {};
        }

        // Clear the pixel data to transparent white.
        for (t_s32 i = 0; i < rgba_px_data.Len(); i += 4) {
            rgba_px_data[i + 0] = 255;
            rgba_px_data[i + 1] = 255;
            rgba_px_data[i + 2] = 255;
            rgba_px_data[i + 3] = 0;
        }

        // Write the pixel data of each character.
        const float scale = stbtt_ScaleForPixelHeight(&stb_font_info, height);

        for (t_s32 i = 0; i < g_ascii_printable_range_len; i++) {
            const char chr = g_ascii_printable_min + i;

            if (chr == ' ') {
                // No bitmap for the space character.
                continue;
            }

            t_u8* const stb_bitmap = stbtt_GetCodepointBitmap(&stb_font_info, scale, scale, chr, nullptr, nullptr, nullptr, nullptr);

            if (!stb_bitmap) {
                ZF_LOG_ERROR("Failed to get bitmap for character '%c' through STB!", chr);
                return {};
            }

            const s_v2_s32 chr_size = arrangement.chr_sizes[i];

            const t_s32 chr_x = tex_meta.chr_xs[i];

            for (t_s32 y = 0; y < chr_size.y; y++) {
                for (t_s32 xo = 0; xo < chr_size.x; xo++) {
                    const t_s32 px_index = ((y * tex_meta.size.x) + chr_x + xo) * 4;
                    const t_s32 stb_bitmap_index = IndexFrom2D(xo, y, chr_size.x);
                    rgba_px_data[px_index + 3] = stb_bitmap[stb_bitmap_index];
                }
            }

            stbtt_FreeBitmap(stb_bitmap, nullptr);
        }

        return rgba_px_data.View();
    }

    bool PackFontFromRawFile(c_file_writer& fw, const c_string_view file_path, const t_s32 height, c_mem_arena& temp_mem_arena) {
        if (height <= 0) {
            ZF_LOG_ERROR("Invalid font height %d!", height);
            return false;
        }

        // Get the plain font file data.
        const c_array<t_u8> file_data = LoadFileContents(file_path, temp_mem_arena);

        if (file_data.IsEmpty()) {
            ZF_LOG_ERROR("Failed to reserve memory for font file contents!");
            return false;
        }

        // Initialise the font through STB.
        stbtt_fontinfo stb_font_info;

        const t_s32 offs = stbtt_GetFontOffsetForIndex(file_data.Raw(), 0);

        if (offs == -1) {
            ZF_LOG_ERROR("Failed to get font offset!");
            return false;
        }

        if (!stbtt_InitFont(&stb_font_info, file_data.Raw(), offs)) {
            ZF_LOG_ERROR("Failed to initialise font through STB!");
            return false;
        }

        // Generate font data and output the file.
        const s_font_arrangement arrangement = LoadFontArrangement(stb_font_info, height);
        const s_font_texture_meta tex_meta = LoadFontTexMeta(stb_font_info, height, arrangement);
        const c_array<const t_u8> tex_rgba_px_data = GenFontTextureRGBAPixelData(temp_mem_arena, stb_font_info, height, arrangement, tex_meta);

        if (tex_rgba_px_data.IsEmpty()) {
            ZF_LOG_ERROR("Failed to generate font texture RGBA pixel data!");
            return false;
        }

        if (!PackFont(fw, arrangement, tex_meta, tex_rgba_px_data)) {
            ZF_LOG_ERROR("Failed to pack font!");
            return false;
        }

        ZF_LOG_SUCCESS("Packed font from file \"%s\" with height %d!", file_path.Raw(), height);

        return true;
    }
}
