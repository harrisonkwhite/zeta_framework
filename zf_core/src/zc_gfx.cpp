#include <zc/zc_gfx.h>

#include <zc/ds/zc_hash_map.h>
#include <stb_image.h>
#include <stb_truetype.h>

namespace zf {
    const t_hash_func<s_font_codepoint_pair> g_codepoint_pair_hash_func = [](const s_font_codepoint_pair& pair) {
        // Combine the 32-bit pairs into a single 64-bit integer and mask out the sign bit.
        return ((static_cast<t_size>(pair.a) << 32) & pair.b) & 0x7FFFFFFFFFFFFFFF;
    };

    const t_bin_comparator<s_font_codepoint_pair> g_codepoint_pair_comparator = [](const s_font_codepoint_pair& pa, const s_font_codepoint_pair& pb) {
        return pa.a == pb.a && pa.b == pb.b;
    };

    t_b8 LoadRGBATextureDataFromRaw(const s_str_rdonly file_path, s_mem_arena& mem_arena, s_rgba_texture_data& o_tex_data) {
        ZF_ASSERT(IsStrTerminated(file_path));

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

    t_b8 PackTexture(const s_str_rdonly dest_file_path, const s_str_rdonly src_file_path, s_mem_arena& temp_mem_arena) {
        ZF_ASSERT(IsStrTerminated(src_file_path));
        ZF_ASSERT(IsStrTerminated(dest_file_path));

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

    t_b8 UnpackTexture(const s_str_rdonly file_path, s_mem_arena& mem_arena, s_rgba_texture_data& o_tex_data) {
        ZF_ASSERT(IsStrTerminated(file_path));

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

    t_b8 LoadFontFromRaw(const s_str_rdonly file_path, const t_s32 height, const s_array_rdonly<t_s32> codepoints_no_dups, s_mem_arena& arrangement_mem_arena, s_mem_arena& atlas_rgbas_mem_arena, s_mem_arena& temp_mem_arena, s_font_arrangement& o_arrangement, s_array<t_font_atlas_rgba>& o_atlas_rgbas) {
        ZF_ASSERT(IsStrTerminated(file_path));
        ZF_ASSERT(height > 0);
        ZF_ASSERT(!IsArrayEmpty(codepoints_no_dups));

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
        if (!MakeHashMap(arrangement_mem_arena, g_s32_hash_func, o_arrangement.codepoints_to_glyph_infos, DefaultBinComparator, codepoints_no_dups.len, codepoints_no_dups.len)) {
            return false;
        }

        t_size atlas_index = 0;
        s_v2<t_s32> atlas_pen = {};

        for (t_size i = 0; i < codepoints_no_dups.len; i++) {
            const t_s32 glyph_index = stbtt_FindGlyphIndex(&stb_font_info, codepoints_no_dups[i]);

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

            if (HashMapPut(o_arrangement.codepoints_to_glyph_infos, codepoints_no_dups[i], glyph_info) == ek_hash_map_put_result_error) {
                return false;
            }
        }

        const t_size atlas_cnt = atlas_index + 1;

        //
        // Kernings
        //
        const t_size kern_cnt = [codepoints_no_dups, &stb_font_info]() {
            t_size res = 0;

            for (t_size i = 0; i < codepoints_no_dups.len; i++) {
                for (t_size j = 0; j < codepoints_no_dups.len; j++) {
                    const t_s32 glyph_a_index = stbtt_FindGlyphIndex(&stb_font_info, codepoints_no_dups[i]);
                    const t_s32 glyph_b_index = stbtt_FindGlyphIndex(&stb_font_info, codepoints_no_dups[j]);

                    const t_s32 kern = stbtt_GetGlyphKernAdvance(&stb_font_info, glyph_a_index, glyph_b_index);

                    if (kern != 0) {
                        res++;
                    }
                }
            }

            return res;
        }();

        if (kern_cnt > 0) {
            if (!MakeHashMap<s_font_codepoint_pair, t_s32>(arrangement_mem_arena, g_codepoint_pair_hash_func, o_arrangement.codepoint_pairs_to_kernings, g_codepoint_pair_comparator, kern_cnt, kern_cnt)) {
                return false;
            }

            for (t_size i = 0; i < codepoints_no_dups.len; i++) {
                for (t_size j = 0; j < codepoints_no_dups.len; j++) {
                    const t_s32 glyph_a_index = stbtt_FindGlyphIndex(&stb_font_info, codepoints_no_dups[i]);
                    const t_s32 glyph_b_index = stbtt_FindGlyphIndex(&stb_font_info, codepoints_no_dups[j]);

                    const t_s32 kern = stbtt_GetGlyphKernAdvance(&stb_font_info, glyph_a_index, glyph_b_index);

                    if (kern != 0) {
                        if (HashMapPut(o_arrangement.codepoint_pairs_to_kernings, {codepoints_no_dups[i], codepoints_no_dups[j]}, kern) == ek_hash_map_put_result_error) {
                            return false;
                        }
                    }
                }
            }
        }

        //
        // Texture Atlases
        //
        if (!MakeArray(atlas_rgbas_mem_arena, atlas_cnt, o_atlas_rgbas)) {
            return false;
        }

        for (t_size i = 0; i < codepoints_no_dups.len; i++) {
            s_font_glyph_info glyph_info;

            if (!HashMapGet(o_arrangement.codepoints_to_glyph_infos, codepoints_no_dups[i], &glyph_info)) {
                ZF_ASSERT(false);
            }

            auto& atlas_rgba = o_atlas_rgbas[glyph_info.atlas_index];
            const auto& atlas_rect = glyph_info.atlas_rect;

            if (atlas_rect.width == 0 || atlas_rect.height == 0) {
                // Might be the ' ' character for example.
                continue;
            }

            t_u8* const stb_bitmap = stbtt_GetCodepointBitmap(&stb_font_info, scale, scale, codepoints_no_dups[i], nullptr, nullptr, nullptr, nullptr);

            if (!stb_bitmap) {
                return false;
            }

            ZF_DEFER({ stbtt_FreeBitmap(stb_bitmap, nullptr); });

            for (t_s32 y = RectTop(atlas_rect); y < RectBottom(atlas_rect); y++) {
                for (t_s32 x = RectLeft(atlas_rect); x < RectRight(atlas_rect); x++) {
                    const t_size px_index = (y * 4 * atlas_rect.width) + (x * 4);
                    const t_size stb_bitmap_index = ((y - atlas_rect.y) * atlas_rect.width) + (x - atlas_rect.x);
                    atlas_rgba[px_index + 3] = stb_bitmap[stb_bitmap_index];
                }
            }
        }

        return true;
    }

    t_b8 PackFont(const s_str_rdonly dest_file_path, const s_str_rdonly src_file_path, const t_s32 height, const s_array_rdonly<t_s32> codepoints, s_mem_arena& temp_mem_arena) {
        ZF_ASSERT(IsStrTerminated(dest_file_path));
        ZF_ASSERT(IsStrTerminated(src_file_path));

        s_font_arrangement arrangement;
        s_array<t_font_atlas_rgba> atlas_rgbas;

        if (!LoadFontFromRaw(src_file_path, height, codepoints, temp_mem_arena, temp_mem_arena, temp_mem_arena, arrangement, atlas_rgbas)) {
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

        if (!SerializeHashMap(fs, arrangement.codepoints_to_glyph_infos)) {
            return false;
        }

        if (!SerializeHashMap(fs, arrangement.codepoint_pairs_to_kernings)) {
            return false;
        }

        if (!SerializeArray(fs, atlas_rgbas)) {
            return false;
        }

        return true;
    }

    t_b8 UnpackFont(const s_str_rdonly file_path, s_mem_arena& arrangement_mem_arena, s_mem_arena& atlas_rgbas_mem_arena, s_font_arrangement& o_arrangement, s_array<t_font_atlas_rgba>& o_atlas_rgbas) {
        ZF_ASSERT(IsStrTerminated(file_path));

        s_stream fs;

        if (!OpenFile(file_path, ek_file_access_mode_read, fs)) {
            return false;
        }

        ZF_DEFER({ CloseFile(fs); });

        if (!StreamReadItem(fs, o_arrangement.line_height)) {
            return false;
        }

        if (!DeserializeHashMap(arrangement_mem_arena, fs, g_s32_hash_func, DefaultBinComparator, o_arrangement.codepoints_to_glyph_infos)) {
            return false;
        }

        if (!DeserializeHashMap(arrangement_mem_arena, fs, g_codepoint_pair_hash_func, g_codepoint_pair_comparator, o_arrangement.codepoint_pairs_to_kernings)) {
            return false;
        }

        if (!DeserializeArray(atlas_rgbas_mem_arena, fs, o_atlas_rgbas)) {
            return false;
        }

        return true;
    }





















#if 0


    t_b8 LoadStrChrPositions(const s_str_rdonly str, const s_font_arrangement& font_arrangement, const s_v2<t_f32> pos, s_mem_arena& mem_arena, s_array<s_v2<t_f32>>& o_positions) {
        ZF_ASSERT(IsValidUTF8Str(str) && IsStrTerminated(str));

        // Reserve memory for the character positions.
        const t_size str_len = CalcUTF8StrLenFastButUnsafe(str);

        if (!MakeArray(mem_arena, str_len, o_positions)) {
            return false;
        }

        // Calculate the position of each character.
        s_v2<t_f32> chr_pos_pen = {}; // The position of the current character.

        t_size chr_index = 0;

        while ((codepoint = NextCodepointInStr(str, chr_index))) {
        }

        //for (t_size i = 0; i < str.chrs.len && str.chrs[i]; i = IndexOfNextUTF8Chr(str, i + 1)) {
        //}

#if 0
        for (t_size i = 0; i <= str_len; i++) {
            const t_size chr = str.chrs[i];

            if (chr == '\n' || !chr) {
                // Apply horizontal alignment offset to all the characters of the line we just finished, only if the line was not empty.
                const t_size line_len = i - line_starting_chr_index;

                if (line_len > 0) {
                    const t_f32 line_width = chr_pos_pen.x;

                    for (t_size j = line_starting_chr_index; j < i; j++) {
                        o_positions[j].x -= line_width * alignment.x;
                    }
                }

                // If '\n', move to the next line.
                if (chr == '\n') {
                    chr_pos_pen.x = 0.0f;
                    chr_pos_pen.y += static_cast<t_f32>(font_arrangement.line_height);

                    line_starting_chr_index = i + 1;
                }

                continue;
            }

            //assert(isprint(chr));

#if 0
            const t_s32 chr_ascii_printable_index = chr - ASCII_PRINTABLE_MIN;

            const s_v2_s32 chr_offs = *STATIC_ARRAY_ELEM(arrangement->chr_offsets, chr_ascii_printable_index);

            *V2Elem(*positions, i) = (s_v2){
                pos.x + chr_pos_pen.x + chr_offs.x,
                pos.y + chr_pos_pen.y + chr_offs.y + alignment_offs_y
            };

            chr_pos_pen.x += *STATIC_ARRAY_ELEM(arrangement->chr_advances, chr_ascii_printable_index);
#endif
        }
#endif

        return true;
    }

#if 0
    bool GenStrChrRenderPositions(s_v2_array* const positions, s_mem_arena* const mem_arena, const s_char_array_view str, const s_font_group* const font_group, const t_s32 font_index, const s_v2 pos, const s_v2 alignment) {
        assert(IS_ZERO(*positions));
        assert(IsStrTerminated(str));
        assert(alignment.x >= 0.0f && alignment.x <= 1.0f && alignment.y >= 0.0f && alignment.y <= 1.0f);

        const s_font_arrangement* const arrangement = FontArrangementElemView(font_group->arrangements, font_index);

        t_s32 str_len = 0, line_cnt = 0;
        CalcStrLenAndLineCnt(&str_len, &line_cnt, str);

        // From just the string line count we can determine the vertical alignment offset to apply to all characters.
        const t_r32 alignment_offs_y = -(line_cnt * arrangement->line_height) * alignment.y;

        // Reserve memory for the character render positions.
        *positions = PushV2ArrayToMemArena(mem_arena, str_len);

        if (IS_ZERO(*positions)) {
            LOG_ERROR("Failed to reserve memory for character render positions!");
            return false;
        }

        // Calculate the render position for each character.
        s_v2 chr_pos_pen = {0}; // The position of the current character.
        t_s32 line_starting_chr_index = 0; // The index of the first character in the current line.

        for (t_s32 i = 0; i <= str_len; i++) {
            const char chr = *CharElemView(str, i);

            if (chr == '\n' || !chr) {
                // Apply horizontal alignment offset to all the characters of the line we just finished, only if the line was not empty.
                const t_s32 line_len = i - line_starting_chr_index;

                if (line_len > 0) {
                    const t_r32 line_width = chr_pos_pen.x;

                    for (t_s32 j = line_starting_chr_index; j < i; j++) {
                        V2Elem(*positions, j)->x -= line_width * alignment.x;
                    }
                }

                // If '\n', move to the next line.
                if (chr == '\n') {
                    chr_pos_pen.x = 0.0f;
                    chr_pos_pen.y += arrangement->line_height;

                    line_starting_chr_index = i + 1;
                }

                continue;
            }

            assert(isprint(chr));

            const t_s32 chr_ascii_printable_index = chr - ASCII_PRINTABLE_MIN;

            const s_v2_s32 chr_offs = *STATIC_ARRAY_ELEM(arrangement->chr_offsets, chr_ascii_printable_index);

            *V2Elem(*positions, i) = (s_v2){
                pos.x + chr_pos_pen.x + chr_offs.x,
                pos.y + chr_pos_pen.y + chr_offs.y + alignment_offs_y
            };

            chr_pos_pen.x += *STATIC_ARRAY_ELEM(arrangement->chr_advances, chr_ascii_printable_index);
        }

        return true;
    }
#endif









#endif


}
