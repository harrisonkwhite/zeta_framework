#include <zc/zc_gfx.h>

#include <zc/zc_algos.h>
#include <zc/ds/zc_hash_map.h>
#include <stb_image.h>
#include <stb_truetype.h>

namespace zf {
    // ============================================================
    // @section: Textures
    // ============================================================
    t_b8 LoadRGBATextureDataFromRaw(const s_str_rdonly file_path, s_mem_arena& mem_arena, s_mem_arena& temp_mem_arena, s_rgba_texture_data& o_tex_data) {
        s_str file_path_terminated;

        if (!CloneStrButAddTerminator(file_path, temp_mem_arena, file_path_terminated)) {
            return false;
        }

        t_u8* const stb_px_data = stbi_load(StrRaw(file_path_terminated), &o_tex_data.size_in_pxs.x, &o_tex_data.size_in_pxs.y, nullptr, 4);

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
        s_rgba_texture_data tex_data;

        if (!LoadRGBATextureDataFromRaw(src_file_path, temp_mem_arena, temp_mem_arena, tex_data)) {
            return false;
        }

        if (!CreateFileAndParentDirs(dest_file_path, temp_mem_arena)) {
            return false;
        }

        s_stream fs;

        if (!OpenFile(dest_file_path, ek_file_access_mode_write, temp_mem_arena, fs)) {
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

    t_b8 UnpackTexture(const s_str_rdonly file_path, s_mem_arena& mem_arena, s_mem_arena& temp_mem_arena, s_rgba_texture_data& o_tex_data) {
        s_stream fs;

        if (!OpenFile(file_path, ek_file_access_mode_read, temp_mem_arena, fs)) {
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

    // ============================================================
    // @section: Fonts
    // ============================================================
    constexpr t_hash_func<t_unicode_code_pt> g_code_pt_hash_func = [](const t_unicode_code_pt& code_pt) constexpr {
        return static_cast<t_size>(code_pt);
    };

    constexpr t_hash_func<s_font_code_point_pair> g_code_pt_pair_hash_func = [](const s_font_code_point_pair& pair) constexpr {
        // Combine the 32-bit pairs into a single 64-bit integer and mask out the sign bit.
        return ((static_cast<t_size>(pair.a) << 32) & pair.b) & 0x7FFFFFFFFFFFFFFF;
    };

    constexpr t_bin_comparator<s_font_code_point_pair> g_code_pt_pair_comparator = [](const s_font_code_point_pair& pa, const s_font_code_point_pair& pb) constexpr {
        return pa.a == pb.a && pa.b == pb.b;
    };

    e_font_load_from_raw_result LoadFontFromRaw(const s_str_rdonly file_path, const t_s32 height, const t_unicode_code_pt_bit_vec& code_pts, s_mem_arena& arrangement_mem_arena, s_mem_arena& atlas_rgbas_mem_arena, s_mem_arena& temp_mem_arena, s_font_arrangement& o_arrangement, s_array<t_font_atlas_rgba>& o_atlas_rgbas, t_unicode_code_pt_bit_vec* const o_unsupported_code_pts) {
        ZF_ASSERT(height > 0);

        o_arrangement = {};

        const t_size code_pt_cnt = CntSetBits(code_pts);

        if (code_pt_cnt == 0) {
            return ek_font_load_from_raw_result_no_code_pts_given;
        }

        // Get the plain font file data.
        s_array<t_u8> font_file_data;

        if (!LoadFileContents(file_path, temp_mem_arena, temp_mem_arena, font_file_data)) {
            return ek_font_load_from_raw_result_other_err;
        }

        // Initialise the font through STB.
        stbtt_fontinfo stb_font_info;

        const t_s32 offs = stbtt_GetFontOffsetForIndex(font_file_data.buf_raw, 0);

        if (offs == -1) {
            return ek_font_load_from_raw_result_other_err;
        }

        if (!stbtt_InitFont(&stb_font_info, font_file_data.buf_raw, offs)) {
            return ek_font_load_from_raw_result_other_err;
        }

        // Check for unsupported code points.
        if (o_unsupported_code_pts) {
            t_b8 any_unsupported = false;
            ZeroOut(*o_unsupported_code_pts);

            ZF_FOR_EACH_SET_BIT(code_pts, i) {
                const auto code_pt = static_cast<t_unicode_code_pt>(i);

                const t_s32 glyph_index = stbtt_FindGlyphIndex(&stb_font_info, static_cast<t_s32>(code_pt));

                if (glyph_index == 0) {
                    SetBit(*o_unsupported_code_pts, i);
                    any_unsupported = true;
                }
            }

            if (any_unsupported) {
                return ek_font_load_from_raw_result_unsupported_code_pt;
            }
        } else {
            ZF_FOR_EACH_SET_BIT(code_pts, i) {
                const auto code_pt = static_cast<t_unicode_code_pt>(i);

                const t_s32 glyph_index = stbtt_FindGlyphIndex(&stb_font_info, static_cast<t_s32>(code_pt));

                if (glyph_index == 0) {
                    return ek_font_load_from_raw_result_unsupported_code_pt;
                }
            }
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
            return ek_font_load_from_raw_result_other_err;
        }

        t_size atlas_index = 0;
        s_v2<t_s32> atlas_pen = {};

        ZF_FOR_EACH_SET_BIT(code_pts, i) {
            const auto code_pt = static_cast<t_unicode_code_pt>(i);

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
                return ek_font_load_from_raw_result_other_err;
            }
        }

        const t_size atlas_cnt = atlas_index + 1;

        //
        // Kernings
        //

        // Compute kerning count first so we know how much memory to allocate for the hash map.
        const t_size kern_cnt = [&code_pts, &stb_font_info]() {
            t_size res = 0;

            ZF_FOR_EACH_SET_BIT(code_pts, i) {
                ZF_FOR_EACH_SET_BIT(code_pts, j) {
                    const auto cp_a = static_cast<t_unicode_code_pt>(i);
                    const auto cp_b = static_cast<t_unicode_code_pt>(j);

                    const auto glyph_a_index = stbtt_FindGlyphIndex(&stb_font_info, static_cast<t_s32>(cp_a));
                    const auto glyph_b_index = stbtt_FindGlyphIndex(&stb_font_info, static_cast<t_s32>(cp_b));

                    const t_s32 kern = stbtt_GetGlyphKernAdvance(&stb_font_info, glyph_a_index, glyph_b_index);

                    if (kern != 0) {
                        res++;
                    }
                }
            }

            return res;
        }();

        // If there were any kernings to store, set up the hash map and go through again and store them.
        if (kern_cnt > 0) {
            o_arrangement.has_kernings = true;

            if (!MakeHashMap<s_font_code_point_pair, t_s32>(arrangement_mem_arena, g_code_pt_pair_hash_func, o_arrangement.code_pt_pairs_to_kernings, g_code_pt_pair_comparator, kern_cnt, kern_cnt)) {
                return ek_font_load_from_raw_result_other_err;
            }

            ZF_FOR_EACH_SET_BIT(code_pts, i) {
                ZF_FOR_EACH_SET_BIT(code_pts, j) {
                    const auto cp_a = static_cast<t_unicode_code_pt>(i);
                    const auto cp_b = static_cast<t_unicode_code_pt>(j);

                    const auto glyph_a_index = stbtt_FindGlyphIndex(&stb_font_info, static_cast<t_s32>(cp_a));
                    const auto glyph_b_index = stbtt_FindGlyphIndex(&stb_font_info, static_cast<t_s32>(cp_b));

                    const t_s32 kern = stbtt_GetGlyphKernAdvance(&stb_font_info, glyph_a_index, glyph_b_index);

                    if (kern != 0) {
                        if (HashMapPut(o_arrangement.code_pt_pairs_to_kernings, {cp_a, cp_b}, kern) == ek_hash_map_put_result_error) {
                            return ek_font_load_from_raw_result_other_err;
                        }
                    }
                }
            }
        }

        //
        // Texture Atlases
        //
        if (!MakeArray(atlas_rgbas_mem_arena, atlas_cnt, o_atlas_rgbas)) {
            return ek_font_load_from_raw_result_other_err;
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
            const auto code_pt = static_cast<t_unicode_code_pt>(i);

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
                return ek_font_load_from_raw_result_other_err;
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

        return ek_font_load_from_raw_result_success;
    }

    t_b8 PackFont(const s_str_rdonly dest_file_path, const s_str_rdonly src_file_path, const t_s32 height, const t_unicode_code_pt_bit_vec& code_pts, s_mem_arena& temp_mem_arena, e_font_load_from_raw_result& o_font_load_from_raw_res, t_unicode_code_pt_bit_vec* const o_unsupported_code_pts) {
        s_font_arrangement arrangement;
        s_array<t_font_atlas_rgba> atlas_rgbas;

        o_font_load_from_raw_res = LoadFontFromRaw(src_file_path, height, code_pts, temp_mem_arena, temp_mem_arena, temp_mem_arena, arrangement, atlas_rgbas, o_unsupported_code_pts);

        if (o_font_load_from_raw_res != ek_font_load_from_raw_result_success) {
            return false;
        }

        if (!CreateFileAndParentDirs(dest_file_path, temp_mem_arena)) {
            return false;
        }

        s_stream fs;

        if (!OpenFile(dest_file_path, ek_file_access_mode_write, temp_mem_arena, fs)) {
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

    t_b8 UnpackFont(const s_str_rdonly file_path, s_mem_arena& arrangement_mem_arena, s_mem_arena& atlas_rgbas_mem_arena, s_mem_arena& temp_mem_arena, s_font_arrangement& o_arrangement, s_array<t_font_atlas_rgba>& o_atlas_rgbas) {
        s_stream fs;

        if (!OpenFile(file_path, ek_file_access_mode_read, temp_mem_arena, fs)) {
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

    t_b8 LoadStrChrDrawPositions(const s_str_rdonly str, const s_font_arrangement& font_arrangement, const s_v2<t_f32> pos, const s_v2<t_f32> alignment, s_mem_arena& mem_arena, s_array<s_v2<t_f32>>& o_positions) {
        ZF_ASSERT(!IsStrEmpty(str) && IsValidUTF8Str(str));
        ZF_ASSERT(alignment.x >= 0.0f && alignment.y >= 0.0f && alignment.x <= 1.0f && alignment.y <= 1.0f);

        // Calculate some useful string metadata.
        struct s_str_meta {
            t_size len;
            t_size line_cnt;
        };

        const auto str_meta = [str]() {
            s_str_meta meta = {
                .line_cnt = 1
            };

            ZF_WALK_STR(str, chr_info) {
                meta.len++;

                if (chr_info.code_pt == '\n') {
                    meta.line_cnt++;
                }
            }

            return meta;
        }();

        // Reserve memory for the character positions.
        if (!MakeArray(mem_arena, str_meta.len, o_positions)) {
            return false;
        }

        // From the line count we can determine the vertical alignment offset to apply.
        const t_f32 alignment_offs_y = static_cast<t_f32>(-(str_meta.line_cnt * font_arrangement.line_height)) * alignment.y;

        // Calculate the position of each character.
        t_size chr_index = 0;
        s_v2<t_f32> chr_pos_pen = {}; // The position of the current character.
        t_size line_begin_chr_index = 0;
        t_size line_len = 0;
        t_unicode_code_pt code_pt_last;

        const auto apply_hor_alignment_offs = [&]() {
            if (line_len > 0) {
                const auto line_width = chr_pos_pen.x;

                for (t_size i = line_begin_chr_index; i < chr_index; i++) {
                    o_positions[i].x -= line_width * alignment.x;
                }
            }
        };

        ZF_WALK_STR(str, chr_info) {
            ZF_DEFER({
                chr_index++;
                code_pt_last = chr_info.code_pt;
            });

            if (line_len == 0) {
                line_begin_chr_index = chr_index;
            }

            if (chr_info.code_pt == '\n') {
                apply_hor_alignment_offs();

                chr_pos_pen.x = 0.0f;
                chr_pos_pen.y += static_cast<t_f32>(font_arrangement.line_height);

                line_len = 0;

                continue;
            }

            s_font_glyph_info glyph_info;

            if (!HashMapGet(font_arrangement.code_pts_to_glyph_infos, chr_info.code_pt, &glyph_info)) {
                ZF_ASSERT_MSG(false, "Unsupported code point!");
                return false;
            }

            if (chr_index > 0 && font_arrangement.has_kernings) {
                t_s32 kerning;

                if (HashMapGet(font_arrangement.code_pt_pairs_to_kernings, {code_pt_last, chr_info.code_pt}, &kerning)) {
                    chr_pos_pen.x += static_cast<t_f32>(kerning);
                }
            }

            o_positions[chr_index] = pos + chr_pos_pen + static_cast<s_v2<t_f32>>(glyph_info.offs);
            o_positions[chr_index].y += alignment_offs_y;

            chr_pos_pen.x += static_cast<t_f32>(glyph_info.adv);

            line_len++;
        }

        apply_hor_alignment_offs();

        return true;
    }
}
