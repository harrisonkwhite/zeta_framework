#include <zc/zc_gfx.h>

#include <stb_image.h>
#include <stb_truetype.h>

namespace zf {
    t_b8 LoadRGBATextureDataFromRaw(const s_str_rdonly file_path, s_mem_arena& mem_arena, s_rgba_texture_data& o_tex_data) {
        ZF_ASSERT(IsStrTerminated(file_path));

        t_u8* const stb_px_data = stbi_load(StrRaw(file_path), &o_tex_data.size_in_pxs.x, &o_tex_data.size_in_pxs.y, nullptr, 4);

        if (!stb_px_data) {
            return false;
        }

        const s_array_rdonly<t_u8> stb_px_data_arr = {stb_px_data, 4 * o_tex_data.size_in_pxs.x * o_tex_data.size_in_pxs.y};

        if (!MakeArray(mem_arena, 4 * o_tex_data.size_in_pxs.x * o_tex_data.size_in_pxs.y, o_tex_data.px_data)) {
            stbi_image_free(stb_px_data);
            return false;
        }

        Copy(o_tex_data.px_data, stb_px_data_arr);

        stbi_image_free(stb_px_data);

        return true;
    }

    t_b8 PackTexture(const s_rgba_texture_data_rdonly& tex_data, const s_str_rdonly file_path, s_mem_arena& temp_mem_arena) {
        ZF_ASSERT(IsStrTerminated(file_path));

        if (!CreateFileAndParentDirs(file_path, temp_mem_arena)) {
            return false;
        }

        s_file_stream fs;

        if (!OpenFile(file_path, ec_file_access_mode::write, fs)) {
            return false;
        }

        const t_b8 success = [fs, &tex_data]() {
            if (!WriteItemToFile(fs, tex_data.size_in_pxs)) {
                return false;
            }

            if (WriteItemArrayToFile(fs, tex_data.px_data) < tex_data.px_data.len) {
                return false;
            }

            return true;
        }();

        CloseFile(fs);

        return success;
    }

    t_b8 UnpackTexture(const s_str_rdonly file_path, s_mem_arena& mem_arena, s_rgba_texture_data& o_tex_data) {
        ZF_ASSERT(IsStrTerminated(file_path));

        s_file_stream fs;

        if (!OpenFile(file_path, ec_file_access_mode::read, fs)) {
            return false;
        }

        const t_b8 success = [fs, &mem_arena, &o_tex_data]() {
            if (!ReadItemFromFile(fs, o_tex_data.size_in_pxs)) {
                return false;
            }

            if (!MakeArray(mem_arena, 4 * o_tex_data.size_in_pxs.x * o_tex_data.size_in_pxs.y, o_tex_data.px_data)) {
                return false;
            }

            if (ReadItemArrayFromFile(fs, o_tex_data.px_data) < o_tex_data.px_data.len) {
                return false;
            }

            return true;
        }();

        CloseFile(fs);

        return success;
    }





























    struct s_font_arrangement {
        t_s32 line_height;

        s_array<s_v2<t_s32>> chr_offsets;
        s_array<s_v2<t_s32>> chr_sizes;
        s_array<t_s32> chr_advances;

        // @todo: Also need kerning pairs, only keep the non-0 ones and index into it with a hash map.
    };

    [[nodiscard]] static t_b8 LoadFontArrangement(s_mem_arena& mem_arena, const stbtt_fontinfo& stb_font_info, const t_s32 height, const s_array_rdonly<t_s32> utf_codepoints, s_font_arrangement& o_arrangement) {
        ZF_ASSERT(!IsArrayEmpty(utf_codepoints));

        if (!MakeArray(mem_arena, utf_codepoints.len, o_arrangement.chr_offsets)) {
            return false;
        }

        if (!MakeArray(mem_arena, utf_codepoints.len, o_arrangement.chr_sizes)) {
            return false;
        }

        if (!MakeArray(mem_arena, utf_codepoints.len, o_arrangement.chr_advances)) {
            return false;
        }

        const t_f32 scale = stbtt_ScaleForPixelHeight(&stb_font_info, static_cast<t_f32>(height)); // @todo: Why the hell is the height a float?

        t_s32 vm_ascent, vm_descent, vm_line_gap;
        stbtt_GetFontVMetrics(&stb_font_info, &vm_ascent, &vm_descent, &vm_line_gap);

        o_arrangement.line_height = static_cast<t_s32>(static_cast<t_f32>(vm_ascent - vm_descent + vm_line_gap) * scale);

        for (t_size i = 0; i < utf_codepoints.len; i++) {
            // @todo: Handle case where codepoint is unsupported.
            // @todo: We need kerning!

            // @todo: Apparently this doesn't always give a tight box? Test with CJK glyphs.
            t_s32 bm_box_left, bm_box_top, bm_box_right, bm_box_bottom;
            stbtt_GetCodepointBitmapBox(&stb_font_info, utf_codepoints[i], scale, scale, &bm_box_left, &bm_box_top, &bm_box_right, &bm_box_bottom);

            o_arrangement.chr_offsets[i] = {
                bm_box_left,
                bm_box_top + static_cast<t_s32>(static_cast<t_f32>(vm_ascent) * scale)
            };

            o_arrangement.chr_sizes[i] = {
                bm_box_right - bm_box_left, bm_box_bottom - bm_box_top
            };

            t_s32 hm_advance;
            stbtt_GetCodepointHMetrics(&stb_font_info, utf_codepoints[i], &hm_advance, nullptr);

            o_arrangement.chr_advances[i] = static_cast<t_s32>(static_cast<t_f32>(hm_advance) * scale);
        }

        return true;
    }





















}
