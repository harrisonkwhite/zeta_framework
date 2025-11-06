#include <zc/gfx.h>

#include <stb_image.h>
#include <stb_truetype.h>

namespace zf {
    bool LoadRGBATextureFromRaw(s_rgba_texture& tex, c_mem_arena& mem_arena, const s_str_view file_path) {
        ZF_ASSERT(file_path.IsTerminated());

        stbi_uc* const stb_px_data = stbi_load(file_path.Raw(), &tex.size_in_pxs.x, &tex.size_in_pxs.y, nullptr, 4);

        if (!stb_px_data) {
            ZF_BANANA_ERROR();
            ZF_LOG_ERROR_SPECIAL("STB", "%s", stbi_failure_reason());
            return false;
        }

        if (!tex.px_data.Init(mem_arena, 4 * tex.size_in_pxs.x * tex.size_in_pxs.y)) {
            ZF_BANANA_ERROR();
            stbi_image_free(stb_px_data);
            return false;
        }

        memcpy(tex.px_data.Raw(), stb_px_data, sizeof(*tex.px_data.Raw()) * tex.px_data.Len());

        stbi_image_free(stb_px_data);

        return true;
    }

    bool PackRGBATexture(s_file_stream& fs, const s_rgba_texture tex) {
        if (!fs.WriteItem(tex.size_in_pxs)) {
            ZF_BANANA_ERROR();
            return false;
        }

        if (fs.WriteItems(tex.px_data.View()) < tex.px_data.Len()) {
            ZF_BANANA_ERROR();
            return false;
        }

        return true;
    }

    bool UnpackRGBATexture(s_rgba_texture& tex, s_file_stream& fs) {
        if (!fs.ReadItem(tex.size_in_pxs)) {
            ZF_BANANA_ERROR();
            return false;
        }

        if (fs.ReadItems(tex.px_data) < tex.px_data.Len()) {
            ZF_BANANA_ERROR();
            return false;
        }

        return true;
    }

#if 0
    bool LoadRGBATextureFromRawFile(s_rgba_texture& tex, c_mem_arena& mem_arena, const s_str_view file_path) {
        ZF_ASSERT(file_path.IsTerminated());

        stbi_uc* const stb_px_data = stbi_load(file_path.Raw(), &tex.size_in_pxs.x, &tex.size_in_pxs.y, NULL, 4);

        if (!stb_px_data) {
            ZF_LOG_ERROR("Failed to load pixel data from file \"%s\" through STB!", file_path.Raw());
            ZF_LOG_ERROR_SPECIAL("STB", "%s", stbi_failure_reason());
            return false;
        }

        if (!tex.px_data.Init(mem_arena, 4 * tex.size_in_pxs.x * tex.size_in_pxs.y)) {
            ZF_LOG_ERROR("Failed to reserve memory for RGBA texture pixel data!");
            stbi_image_free(stb_px_data);
            return false;
        }

        memcpy(tex.px_data.Raw(), stb_px_data, sizeof(*tex.px_data.Raw()) * tex.px_data.Len());

        stbi_image_free(stb_px_data);

        return true;
    }

    static s_font_arrangement LoadFontArrangement(const stbtt_fontinfo& stb_font_info, const int height) {
        s_font_arrangement arrangement_info = {};

        const float scale = stbtt_ScaleForPixelHeight(&stb_font_info, height);

        int vm_ascent, vm_descent, vm_line_gap;
        stbtt_GetFontVMetrics(&stb_font_info, &vm_ascent, &vm_descent, &vm_line_gap);

        arrangement_info.line_height = (vm_ascent - vm_descent + vm_line_gap) * scale;

        for (int i = 0; i < g_ascii_printable_range_len; i++) {
            const char chr = g_ascii_printable_min + i;

            int bm_box_left, bm_box_top, bm_box_right, bm_box_bottom;
            stbtt_GetCodepointBitmapBox(&stb_font_info, chr, scale, scale, &bm_box_left, &bm_box_top, &bm_box_right, &bm_box_bottom);

            arrangement_info.chr_offsets[i] = {bm_box_left, static_cast<int>(bm_box_top + (vm_ascent * scale))};
            arrangement_info.chr_sizes[i] = {bm_box_right - bm_box_left, bm_box_bottom - bm_box_top};

            int hm_advance;
            stbtt_GetCodepointHMetrics(&stb_font_info, chr, &hm_advance, nullptr);

            arrangement_info.chr_advances[i] = hm_advance * scale;
        }

        return arrangement_info;
    }

    static s_font_texture_meta LoadFontTexMeta(const stbtt_fontinfo& stb_font_info, const int height, const s_font_arrangement& arrangement) {
        s_font_texture_meta tex_meta = {};

        int chr_x_pen = 0;

        for (int i = 0; i < g_ascii_printable_range_len; i++) {
            const int chr_margin = 2;

            chr_x_pen += chr_margin;

            tex_meta.chr_xs[i] = chr_x_pen;

            const s_v2<int> chr_size = arrangement.chr_sizes[i];
            chr_x_pen += chr_size.x;

            chr_x_pen += chr_margin;

            tex_meta.size.y = Max(chr_size.y, tex_meta.size.y);
        }

        tex_meta.size.x = chr_x_pen;

        return tex_meta;
    }

    static bool LoadFontTextureRGBAPixelData(c_array<t_byte>& rgba_px_data, c_mem_arena& mem_arena, const stbtt_fontinfo& stb_font_info, const int height, const s_font_arrangement& arrangement, const s_font_texture_meta tex_meta) {
        ZF_ASSERT(rgba_px_data.IsEmpty());

        // Reserve the needed memory based on font texture size.
        if (!rgba_px_data.Init(mem_arena, 4 * tex_meta.size.x * tex_meta.size.y)) {
            ZF_LOG_ERROR("Failed to reserve memory for font texture RGBA pixel data!");
            return false;
        }

        // Clear the pixel data to transparent white.
        for (int i = 0; i < rgba_px_data.Len(); i += 4) {
            rgba_px_data[i + 0] = 255;
            rgba_px_data[i + 1] = 255;
            rgba_px_data[i + 2] = 255;
            rgba_px_data[i + 3] = 0;
        }

        // Write the pixel data of each character.
        const float scale = stbtt_ScaleForPixelHeight(&stb_font_info, height);

        for (int i = 0; i < g_ascii_printable_range_len; i++) {
            const char chr = g_ascii_printable_min + i;

            if (chr == ' ') {
                // No bitmap for the space character.
                continue;
            }

            t_byte* const stb_bitmap = stbtt_GetCodepointBitmap(&stb_font_info, scale, scale, chr, nullptr, nullptr, nullptr, nullptr);

            if (!stb_bitmap) {
                ZF_LOG_ERROR("Failed to get bitmap for character '%c' through STB!", chr);
                return false;
            }

            const s_v2<int> chr_size = arrangement.chr_sizes[i];

            const int chr_x = tex_meta.chr_xs[i];

            for (int y = 0; y < chr_size.y; y++) {
                for (int xo = 0; xo < chr_size.x; xo++) {
                    const int px_index = ((y * tex_meta.size.x) + chr_x + xo) * 4;
                    const int stb_bitmap_index = IndexFrom2D(xo, y, chr_size.x);
                    rgba_px_data[px_index + 3] = stb_bitmap[stb_bitmap_index];
                }
            }

            stbtt_FreeBitmap(stb_bitmap, nullptr);
        }

        return true;
    }

    bool LoadFontFromRawFile(s_font_arrangement& arrangement, s_font_texture_meta& tex_meta, c_array<t_byte>& tex_rgba_px_data, const s_str_view file_path, const int height, c_mem_arena& temp_mem_arena) {
        if (height <= 0) {
            ZF_LOG_ERROR("Invalid font height %d!", height);
            return false;
        }

        // Get the plain font file data.
        c_array<t_byte> file_data;

        if (!LoadFileContents(file_data, temp_mem_arena, file_path)) {
            ZF_LOG_ERROR("Failed to reserve memory for font file contents!");
            return false;
        }

        // Initialise the font through STB.
        stbtt_fontinfo stb_font_info;

        const int offs = stbtt_GetFontOffsetForIndex(file_data.Raw(), 0);

        if (offs == -1) {
            ZF_LOG_ERROR("Failed to get font offset!");
            return false;
        }

        if (!stbtt_InitFont(&stb_font_info, file_data.Raw(), offs)) {
            ZF_LOG_ERROR("Failed to initialise font through STB!");
            return false;
        }

        // Load font data.
        arrangement = LoadFontArrangement(stb_font_info, height);
        tex_meta = LoadFontTexMeta(stb_font_info, height, arrangement);

        if (!LoadFontTextureRGBAPixelData(tex_rgba_px_data, temp_mem_arena, stb_font_info, height, arrangement, tex_meta)) {
            ZF_LOG_ERROR("Failed to generate font texture RGBA pixel data!");
            return false;
        }

        return true;
    }

    static bool PackShaderFromRawFile(s_file_stream& fs, const s_str_view file_path, const bool is_fs, const s_str_view varying_def_file_path) {
        ZF_ASSERT(file_path.IsTerminated());
        ZF_ASSERT(varying_def_file_path.IsTerminated());

        const char* const args[] = {
            "shaderc",
            "-f", file_path.Raw(),
            "--type", is_fs ? "fragment" : "vertex",
            "--platform", "windows", // @todo: Update!
            "--profile", "s_5_0",
            "--varyingdef", varying_def_file_path.Raw(),
            nullptr
        };

        reproc_options options = {};
        options.redirect.out.type = REPROC_REDIRECT_FILE;
        options.redirect.out.file = fs.raw;

        const int res = reproc_run(args, options);

        if (res < 0) {
            ZF_LOG_ERROR_SPECIAL("shaderc", "%d", res);
            return false;
        }

        return true;
    }

    static bool PackShaderProgFromRawFiles(s_file_stream& fs, const s_str_view vs_file_path, const s_str_view fs_file_path, const s_str_view varying_def_file_path) {
        if (!PackShaderFromRawFile(fs, vs_file_path, false, varying_def_file_path)) {
            return false;
        }

        if (!PackShaderFromRawFile(fs, fs_file_path, true, varying_def_file_path)) {
            return false;
        }

        return true;
    }

    bool PackTexture(s_file_stream& fs, const s_rgba_texture rgba_tex) {
        if (!fs.WriteItem(rgba_tex.size_in_pxs)) {
            ZF_LOG_ERROR("Failed to write texture size during packing!");
            return false;
        }

        if (fs.WriteItems(rgba_tex.px_data.View()) < rgba_tex.px_data.Len()) {
            ZF_LOG_ERROR("Failed to write pixel data during packing!");
            return false;
        }

        return true;
    }

    void UnpackTexture(s_file_stream& fs, s_rgba_texture& rgba_tex) {
    }

    bool PackFont(s_file_stream& fs, const s_font_arrangement& arrangement, const s_font_texture_meta tex_meta, const c_array<const t_byte> tex_rgba_px_data) {
        if (!fs.WriteItem(arrangement)) {
            ZF_LOG_ERROR("Failed to write font arrangement during packing!");
            return false;
        }

        if (!fs.WriteItem(tex_meta)) {
            ZF_LOG_ERROR("Failed to write font texture metadata during packing!");
            return false;
        }

        if (fs.WriteItems(tex_rgba_px_data) < tex_rgba_px_data.Len()) {
            ZF_LOG_ERROR("Failed to write font texture RGBA pixel data during packing!");
            return false;
        }

        return true;
    }

    void UnpackFont(s_file_stream& fs, s_font_arrangement& arrangement, s_font_texture_meta tex_meta, c_array<const t_byte>& tex_rgba_px_data) {
    }
#endif
}
