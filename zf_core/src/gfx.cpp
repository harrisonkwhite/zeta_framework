#include <zc/gfx.h>

#include <stb_image.h>
#include <stb_truetype.h>
#include <reproc/run.h>
#include <zc/io.h>

namespace zf {
    bool LoadRGBATextureFromRawFile(s_rgba_texture& tex, c_mem_arena& mem_arena, const s_str_view file_path) {
        assert(file_path.IsTerminated());

        stbi_uc* const stb_px_data = stbi_load(file_path.Raw(), &tex.dims.x, &tex.dims.y, NULL, 4);

        if (!stb_px_data) {
            ZF_LOG_ERROR("Failed to load pixel data from file \"%s\" through STB!", file_path.Raw());
            ZF_LOG_ERROR_SPECIAL("STB", "%s", stbi_failure_reason());
            return false;
        }

        if (!tex.px_data.Init(mem_arena, 4 * tex.dims.x * tex.dims.y)) {
            ZF_LOG_ERROR("Failed to reserve memory for RGBA texture pixel data!");
            stbi_image_free(stb_px_data);
            return false;
        }

        memcpy(tex.px_data.Raw(), stb_px_data, sizeof(*tex.px_data.Raw()) * tex.px_data.Len());

        stbi_image_free(stb_px_data);

        return true;
    }

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

    static bool LoadFontTextureRGBAPixelData(c_array<t_u8>& rgba_px_data, c_mem_arena& mem_arena, const stbtt_fontinfo& stb_font_info, const t_s32 height, const s_font_arrangement& arrangement, const s_font_texture_meta tex_meta) {
        assert(!rgba_px_data.IsInitted());

        // Reserve the needed memory based on font texture size.
        if (!rgba_px_data.Init(mem_arena, 4 * tex_meta.size.x * tex_meta.size.y)) {
            ZF_LOG_ERROR("Failed to reserve memory for font texture RGBA pixel data!");
            return false;
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
                return false;
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

        return true;
    }

    bool LoadFontFromRawFile(s_font_arrangement& arrangement, s_font_texture_meta& tex_meta, c_array<t_u8>& tex_rgba_px_data, const s_str_view file_path, const t_s32 height, c_mem_arena& temp_mem_arena) {
        if (height <= 0) {
            ZF_LOG_ERROR("Invalid font height %d!", height);
            return false;
        }

        // Get the plain font file data.
        c_array<t_u8> file_data;

        if (!LoadFileContents(file_data, temp_mem_arena, file_path)) {
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

        // Load font data.
        arrangement = LoadFontArrangement(stb_font_info, height);
        tex_meta = LoadFontTexMeta(stb_font_info, height, arrangement);

        if (!LoadFontTextureRGBAPixelData(tex_rgba_px_data, temp_mem_arena, stb_font_info, height, arrangement, tex_meta)) {
            ZF_LOG_ERROR("Failed to generate font texture RGBA pixel data!");
            return false;
        }

        return true;
    }

    static bool PackShaderFromRawFile(c_file_writer& fw, const s_str_view file_path, const bool is_fs, const s_str_view varying_def_file_path) {
        assert(file_path.IsTerminated());
        assert(varying_def_file_path.IsTerminated());

        const char* args[] = {
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
        options.redirect.out.file = fw.Raw();

        const int res = reproc_run(args, options);

        if (res < 0) {
            ZF_LOG_ERROR_SPECIAL("shaderc", "%d", res);
            return false;
        }

        return true;
    }

    static bool PackShaderProgFromRawFiles(c_file_writer& fw, const s_str_view vs_file_path, const s_str_view fs_file_path, const s_str_view varying_def_file_path) {
        if (!PackShaderFromRawFile(fw, vs_file_path, false, varying_def_file_path)) {
            return false;
        }

        if (!PackShaderFromRawFile(fw, fs_file_path, true, varying_def_file_path)) {
            return false;
        }

        return true;
    }

    bool PackTexture(c_file_writer& fw, const s_rgba_texture rgba_tex) {
        if (!fw.WriteItem(rgba_tex.dims)) {
            ZF_LOG_ERROR("Failed to write texture size during packing!");
            return false;
        }

        if (fw.Write(rgba_tex.px_data.View()) < rgba_tex.px_data.Len()) {
            ZF_LOG_ERROR("Failed to write pixel data during packing!");
            return false;
        }

        return true;
    }

    void UnpackTexture(c_file_reader& fr, s_rgba_texture& rgba_tex) {
    }

    bool PackFont(c_file_writer& fw, const s_font_arrangement& arrangement, const s_font_texture_meta tex_meta, const c_array<const t_u8> tex_rgba_px_data) {
        if (!fw.WriteItem(arrangement)) {
            ZF_LOG_ERROR("Failed to write font arrangement during packing!");
            return false;
        }

        if (!fw.WriteItem(tex_meta)) {
            ZF_LOG_ERROR("Failed to write font texture metadata during packing!");
            return false;
        }

        if (fw.Write(tex_rgba_px_data) < tex_rgba_px_data.Len()) {
            ZF_LOG_ERROR("Failed to write font texture RGBA pixel data during packing!");
            return false;
        }

        return true;
    }

    void UnpackFont(c_file_reader& fr, s_font_arrangement& arrangement, s_font_texture_meta tex_meta, c_array<const t_u8>& tex_rgba_px_data) {
    }
}
