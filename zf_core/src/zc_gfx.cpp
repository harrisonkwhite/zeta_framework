#include <zc/zc_gfx.h>

#include <zc/zc_algos.h>
#include <zc/ds/zc_hash_map.h>
#include <stb_image.h>
#include <stb_truetype.h>

namespace zf {
    constexpr t_hash_func<t_code_pt> g_code_pt_hash_func = [](const t_code_pt& code_pt) constexpr {
        return static_cast<t_size>(code_pt);
    };

    constexpr t_hash_func<s_font_code_point_pair> g_code_pt_pair_hash_func = [](const s_font_code_point_pair& pair) constexpr {
        // Combine the 32-bit pairs into a single 64-bit integer and mask out the sign bit.
        return ((static_cast<t_size>(pair.a) << 32) & pair.b) & 0x7FFFFFFFFFFFFFFF;
    };

    constexpr t_bin_comparator<s_font_code_point_pair> g_code_pt_pair_comparator = [](const s_font_code_point_pair& pa, const s_font_code_point_pair& pb) constexpr {
        return pa.a == pb.a && pa.b == pb.b;
    };

    t_b8 LoadRGBATextureDataFromRaw(const s_str_ascii_rdonly file_path, s_mem_arena& mem_arena, s_rgba_texture_data& o_tex_data) {
        ZF_ASSERT(IsStrTerminatedOnlyAtEnd(file_path));

        t_u8* const stb_px_data = stbi_load(StrRaw(file_path), &o_tex_data.size_in_pxs.x, &o_tex_data.size_in_pxs.y, nullptr, 4);

        if (!stb_px_data) {
            return false;
        }

        ZF_DEFER({ stbi_image_free(stb_px_data); });

        const s_array_rdonly<t_u8> stb_px_data_arr = {stb_px_data, 4 * o_tex_data.size_in_pxs.x * o_tex_data.size_in_pxs.y};

        if (!MakeArray(mem_arena, 4 * o_tex_data.size_in_pxs.x * o_tex_data.size_in_pxs.y, o_tex_data.px_data)) {
            return false;
        }

        Copy(o_tex_data.px_data, stb_px_data_arr);

        return true;
    }

    t_b8 PackTexture(const s_str_ascii_rdonly dest_file_path, const s_str_ascii_rdonly src_file_path, s_mem_arena& temp_mem_arena) {
        ZF_ASSERT(IsStrTerminatedOnlyAtEnd(src_file_path));
        ZF_ASSERT(IsStrTerminatedOnlyAtEnd(dest_file_path));

        s_rgba_texture_data tex_data;

        if (!LoadRGBATextureDataFromRaw(src_file_path, temp_mem_arena, tex_data)) {
            return false;
        }

        if (!CreateFileAndParentDirs(dest_file_path, temp_mem_arena)) {
            return false;
        }

        s_stream fs;

        if (!OpenFile(dest_file_path, ek_file_access_mode_write, fs)) {
            return false;
        }

        ZF_DEFER({ CloseFile(fs); });

        if (!StreamWriteItem(fs, tex_data.size_in_pxs)) {
            return false;
        }

        if (!StreamWriteItemsOfArray(fs, tex_data.px_data)) {
            return false;
        }

        return true;
    }

    t_b8 UnpackTexture(const s_str_ascii_rdonly file_path, s_mem_arena& mem_arena, s_rgba_texture_data& o_tex_data) {
        ZF_ASSERT(IsStrTerminatedOnlyAtEnd(file_path));

        s_stream fs;

        if (!OpenFile(file_path, ek_file_access_mode_read, fs)) {
            return false;
        }

        ZF_DEFER({ CloseFile(fs); });

        if (!StreamReadItem(fs, o_tex_data.size_in_pxs)) {
            return false;
        }

        if (!MakeArray(mem_arena, 4 * o_tex_data.size_in_pxs.x * o_tex_data.size_in_pxs.y, o_tex_data.px_data)) {
            return false;
        }

        if (!StreamReadItemsIntoArray(fs, o_tex_data.px_data, o_tex_data.px_data.len)) {
            return false;
        }

        return true;
    }

    t_b8 LoadFontFromRaw(const s_str_ascii_rdonly file_path, const t_s32 height, const t_unicode_code_pt_bit_vector& code_pts, s_mem_arena& arrangement_mem_arena, s_mem_arena& atlas_rgbas_mem_arena, s_mem_arena& temp_mem_arena, s_font_arrangement& o_arrangement, s_array<t_font_atlas_rgba>& o_atlas_rgbas) {
        ZF_ASSERT(IsStrTerminatedOnlyAtEnd(file_path));
        ZF_ASSERT(height > 0);

        const t_size code_pt_cnt = CountSetBits(code_pts);

        o_arrangement = {};

        // Get the plain font file data.
        s_array<t_u8> font_file_data;

        if (!LoadFileContents(temp_mem_arena, file_path, font_file_data)) {
            return false;
        }

        // Initialise the font through STB.
        stbtt_fontinfo stb_font_info;

        const t_s32 offs = stbtt_GetFontOffsetForIndex(font_file_data.buf_raw, 0);

        if (offs == -1) {
            return false;
        }

        if (!stbtt_InitFont(&stb_font_info, font_file_data.buf_raw, offs)) {
            return false;
        }

        // Compute general info.
        const t_f32 scale = stbtt_ScaleForPixelHeight(&stb_font_info, static_cast<t_f32>(height));

        t_s32 vm_ascent, vm_descent, vm_line_gap;
        stbtt_GetFontVMetrics(&stb_font_info, &vm_ascent, &vm_descent, &vm_line_gap);

        o_arrangement.line_height = static_cast<t_s32>(static_cast<t_f32>(vm_ascent - vm_descent + vm_line_gap) * scale);

        //
        // Glyph Info
        //
        if (!MakeHashMap(arrangement_mem_arena, g_code_pt_hash_func, o_arrangement.code_pts_to_glyph_infos, DefaultBinComparator, code_pt_cnt, code_pt_cnt)) {
            return false;
        }

        t_size atlas_index = 0;
        s_v2<t_s32> atlas_pen = {};

        ZF_FOR_EACH_SET_BIT(code_pts, i) {
            const auto code_pt = static_cast<t_code_pt>(i);

            const t_s32 glyph_index = stbtt_FindGlyphIndex(&stb_font_info, static_cast<t_s32>(code_pt));

            s_font_glyph_info glyph_info = {};

            t_s32 bm_box_left, bm_box_top, bm_box_right, bm_box_bottom;
            stbtt_GetGlyphBitmapBox(&stb_font_info, glyph_index, scale, scale, &bm_box_left, &bm_box_top, &bm_box_right, &bm_box_bottom);

            glyph_info.offs = {
                bm_box_left,
                bm_box_top + static_cast<t_s32>(static_cast<t_f32>(vm_ascent) * scale)
            };

            glyph_info.size = {
                bm_box_right - bm_box_left, bm_box_bottom - bm_box_top
            };

            ZF_ASSERT(glyph_info.size.x <= g_font_atlas_size.x && glyph_info.size.y <= g_font_atlas_size.y);

            t_s32 hm_advance;
            stbtt_GetGlyphHMetrics(&stb_font_info, glyph_index, &hm_advance, nullptr);

            glyph_info.adv = static_cast<t_s32>(static_cast<t_f32>(hm_advance) * scale);

            if (atlas_pen.x + glyph_info.size.x > g_font_atlas_size.x) {
                atlas_pen.x = 0;
                atlas_pen.y += o_arrangement.line_height;
            }

            if (atlas_pen.y + glyph_info.size.y > g_font_atlas_size.y) {
                atlas_pen = {};
                atlas_index++;
            }

            glyph_info.atlas_index = atlas_index;
            glyph_info.atlas_rect = {atlas_pen, glyph_info.size};
            atlas_pen.x += glyph_info.size.x;

            if (HashMapPut(o_arrangement.code_pts_to_glyph_infos, code_pt, glyph_info) == ek_hash_map_put_result_error) {
                return false;
            }
        }

        const t_size atlas_cnt = atlas_index + 1;

#if 0
        //
        // Kernings
        //
        const t_size kern_cnt = [code_pts_no_dups, &stb_font_info]() {
            t_size res = 0;

            for (t_size i = 0; i < code_pts_no_dups.len; i++) {
                for (t_size j = 0; j < code_pts_no_dups.len; j++) {
                    const auto glyph_a_index = stbtt_FindGlyphIndex(&stb_font_info, static_cast<t_s32>(code_pts_no_dups[i]));
                    const auto glyph_b_index = stbtt_FindGlyphIndex(&stb_font_info, static_cast<t_s32>(code_pts_no_dups[j]));

                    const t_s32 kern = stbtt_GetGlyphKernAdvance(&stb_font_info, glyph_a_index, glyph_b_index);

                    if (kern != 0) {
                        res++;
                    }
                }
            }

            return res;
        }();

        if (kern_cnt > 0) {
            if (!MakeHashMap<s_font_code_point_pair, t_s32>(arrangement_mem_arena, g_code_pt_pair_hash_func, o_arrangement.code_pt_pairs_to_kernings, g_code_pt_pair_comparator, kern_cnt, kern_cnt)) {
                return false;
            }

            for (t_size i = 0; i < code_pts_no_dups.len; i++) {
                for (t_size j = 0; j < code_pts_no_dups.len; j++) {
                    const auto glyph_a_index = stbtt_FindGlyphIndex(&stb_font_info, static_cast<t_s32>(code_pts_no_dups[i]));
                    const auto glyph_b_index = stbtt_FindGlyphIndex(&stb_font_info, static_cast<t_s32>(code_pts_no_dups[j]));

                    const t_s32 kern = stbtt_GetGlyphKernAdvance(&stb_font_info, glyph_a_index, glyph_b_index);

                    if (kern != 0) {
                        if (HashMapPut(o_arrangement.code_pt_pairs_to_kernings, {code_pts_no_dups[i], code_pts_no_dups[j]}, kern) == ek_hash_map_put_result_error) {
                            return false;
                        }
                    }
                }
            }
        }
#endif

        //
        // Texture Atlases
        //
        if (!MakeArray(atlas_rgbas_mem_arena, atlas_cnt, o_atlas_rgbas)) {
            return false;
        }

        // Initialise all pixels to transparent white.
        // @todo: Maybe don't use RBGA for this?
        for (t_size i = 0; i < o_atlas_rgbas.len; i++) {
            auto& atlas_rgba = o_atlas_rgbas[i];

            for (t_size j = 0; j < o_atlas_rgbas[i].g_len; j += 4) {
                atlas_rgba[j + 0] = 255;
                atlas_rgba[j + 1] = 255;
                atlas_rgba[j + 2] = 255;
                atlas_rgba[j + 3] = 0;
            }
        }

        // Write pixel data for each individual glyph.
        ZF_FOR_EACH_SET_BIT(code_pts, i) {
            const auto code_pt = static_cast<t_code_pt>(i);

            s_font_glyph_info glyph_info;

            if (!HashMapGet(o_arrangement.code_pts_to_glyph_infos, code_pt, &glyph_info)) {
                ZF_ASSERT(false);
            }

            auto& atlas_rgba = o_atlas_rgbas[glyph_info.atlas_index];
            const auto& atlas_rect = glyph_info.atlas_rect;

            if (atlas_rect.width == 0 || atlas_rect.height == 0) {
                // Might be the ' ' character for example.
                continue;
            }

            t_u8* const stb_bitmap = stbtt_GetCodepointBitmap(&stb_font_info, scale, scale, static_cast<t_s32>(code_pt), nullptr, nullptr, nullptr, nullptr);

            if (!stb_bitmap) {
                return false;
            }

            ZF_DEFER({ stbtt_FreeBitmap(stb_bitmap, nullptr); });

            for (t_s32 y = RectTop(atlas_rect); y < RectBottom(atlas_rect); y++) {
                for (t_s32 x = RectLeft(atlas_rect); x < RectRight(atlas_rect); x++) {
                    const t_size px_index = (y * 4 * g_font_atlas_size.x) + (x * 4);
                    const t_size stb_bitmap_index = ((y - atlas_rect.y) * atlas_rect.width) + (x - atlas_rect.x);
                    atlas_rgba[px_index + 3] = stb_bitmap[stb_bitmap_index];
                }
            }
        }

        return true;
    }

    t_b8 PackFont(const s_str_ascii_rdonly dest_file_path, const s_str_ascii_rdonly src_file_path, const t_s32 height, const t_unicode_code_pt_bit_vector& code_pts, s_mem_arena& temp_mem_arena) {
        ZF_ASSERT(IsStrTerminatedOnlyAtEnd(dest_file_path));
        ZF_ASSERT(IsStrTerminatedOnlyAtEnd(src_file_path));

        s_font_arrangement arrangement;
        s_array<t_font_atlas_rgba> atlas_rgbas;

        if (!LoadFontFromRaw(src_file_path, height, code_pts, temp_mem_arena, temp_mem_arena, temp_mem_arena, arrangement, atlas_rgbas)) {
            return false;
        }

        if (!CreateFileAndParentDirs(dest_file_path, temp_mem_arena)) {
            return false;
        }

        s_stream fs;

        if (!OpenFile(dest_file_path, ek_file_access_mode_write, fs)) {
            return false;
        }

        ZF_DEFER({ CloseFile(fs); });

        if (!StreamWriteItem(fs, arrangement.line_height)) {
            return false;
        }

        if (!SerializeHashMap(fs, arrangement.code_pts_to_glyph_infos)) {
            return false;
        }

        if (!SerializeHashMap(fs, arrangement.code_pt_pairs_to_kernings)) {
            return false;
        }

        if (!SerializeArray(fs, atlas_rgbas)) {
            return false;
        }

        return true;
    }

    t_b8 UnpackFont(const s_str_ascii_rdonly file_path, s_mem_arena& arrangement_mem_arena, s_mem_arena& atlas_rgbas_mem_arena, s_font_arrangement& o_arrangement, s_array<t_font_atlas_rgba>& o_atlas_rgbas) {
        ZF_ASSERT(IsStrTerminatedOnlyAtEnd(file_path));

        s_stream fs;

        if (!OpenFile(file_path, ek_file_access_mode_read, fs)) {
            return false;
        }

        ZF_DEFER({ CloseFile(fs); });

        if (!StreamReadItem(fs, o_arrangement.line_height)) {
            return false;
        }

        if (!DeserializeHashMap(arrangement_mem_arena, fs, g_code_pt_hash_func, DefaultBinComparator, o_arrangement.code_pts_to_glyph_infos)) {
            return false;
        }

        if (!DeserializeHashMap(arrangement_mem_arena, fs, g_code_pt_pair_hash_func, g_code_pt_pair_comparator, o_arrangement.code_pt_pairs_to_kernings)) {
            return false;
        }

        if (!DeserializeArray(fs, atlas_rgbas_mem_arena, o_atlas_rgbas)) {
            return false;
        }

        return true;
    }

    t_b8 LoadStrChrPositions(const s_str_rdonly str, const s_font_arrangement& font_arrangement, const s_v2<t_f32> pos, s_mem_arena& mem_arena, s_array<s_v2<t_f32>>& o_positions) {
        ZF_ASSERT(IsValidUTF8Str(str));

        // Reserve memory for the character positions.
        const t_size str_len = CalcStrLen(str);

        if (!MakeArray(mem_arena, str_len, o_positions)) {
            return false;
        }

        // Calculate the position of each character.
        t_size pos_index = 0;
        s_v2<t_f32> chr_pos_pen = {}; // The position of the current character.

        ZF_UTF8_STR(str, code_pt) {
            if (code_pt == '\n') {
                chr_pos_pen.x = 0.0f;
                chr_pos_pen.y += static_cast<t_f32>(font_arrangement.line_height);
                continue;
            }

            s_font_glyph_info glyph_info;

            if (!HashMapGet(font_arrangement.code_pts_to_glyph_infos, code_pt, &glyph_info)) {
                ZF_LOG_WARNING("Trying to calculate character positions for a string containing unicode code point %u which is not supported by the given font data!", code_pt);
                continue;
            }

            o_positions[pos_index] = pos + chr_pos_pen + static_cast<s_v2<t_f32>>(glyph_info.offs);

            chr_pos_pen.x += static_cast<t_f32>(glyph_info.adv);

            pos_index++;
        }

        return true;
    }
}
