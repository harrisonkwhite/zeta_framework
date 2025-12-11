#include <zcl/zcl_gfx.h>

#include <stb_image.h>
#include <stb_truetype.h>

namespace zf {
    // ============================================================
    // @section: Textures
    // ============================================================
    t_b8 LoadTextureFromRaw(const s_str_rdonly file_path, s_mem_arena &tex_data_mem_arena, s_mem_arena &temp_mem_arena, s_texture_data &o_tex_data) {
        s_str file_path_terminated = {};

        if (!AllocStrCloneWithTerminator(file_path, temp_mem_arena, file_path_terminated)) {
            return false;
        }

        s_v2_i size_in_pxs = {};
        const s_ptr<t_u8> stb_px_data = stbi_load(file_path_terminated.Cstr(), &size_in_pxs.x, &size_in_pxs.y, nullptr, 4);

        if (!stb_px_data) {
            return false;
        }

        ZF_DEFER({ stbi_image_free(stb_px_data); });

        s_array<t_u8> px_data = {};

        if (!AllocArray(4 * size_in_pxs.x * size_in_pxs.y, tex_data_mem_arena, px_data)) {
            return false;
        }

        const s_array_rdonly<t_u8> stb_px_data_arr = {stb_px_data, 4 * size_in_pxs.x * size_in_pxs.y};
        stb_px_data_arr.CopyTo(px_data);

        o_tex_data = {size_in_pxs, px_data};

        return true;
    }

    t_b8 PackTexture(const s_str_rdonly file_path, const s_texture_data tex_data, s_mem_arena &temp_mem_arena) {
        if (!CreateFileAndParentDirs(file_path, temp_mem_arena)) {
            return false;
        }

        s_stream fs = {};

        if (!OpenFile(file_path, ek_file_access_mode_write, temp_mem_arena, fs)) {
            return false;
        }

        ZF_DEFER({ CloseFile(fs); });

        if (!fs.WriteItem(tex_data.SizeInPixels())) {
            return false;
        }

        if (!fs.WriteItemsOfArray(tex_data.RGBAPixelData())) {
            return false;
        }

        return true;
    }

    t_b8 UnpackTexture(const s_str_rdonly file_path, s_mem_arena &tex_data_mem_arena, s_mem_arena &temp_mem_arena, s_texture_data &o_tex_data) {
        s_stream fs = {};

        if (!OpenFile(file_path, ek_file_access_mode_read, temp_mem_arena, fs)) {
            return false;
        }

        ZF_DEFER({ CloseFile(fs); });

        s_v2_i size_in_pxs = {};

        if (!fs.ReadItem(size_in_pxs)) {
            return false;
        }

        s_array<t_u8> rgba_px_data = {};

        if (!AllocArray(4 * size_in_pxs.x * size_in_pxs.y, tex_data_mem_arena, rgba_px_data)) {
            return false;
        }

        if (!fs.ReadItemsIntoArray(rgba_px_data, rgba_px_data.Len())) {
            return false;
        }

        return true;
    }

    // ============================================================
    // @section: Fonts
    // ============================================================
    constexpr t_hash_func<t_code_pt> g_code_pt_hash_func = [](const t_code_pt &code_pt) constexpr {
        return static_cast<t_len>(code_pt);
    };

    constexpr t_hash_func<s_font_code_point_pair> g_code_pt_pair_hash_func = [](const s_font_code_point_pair &pair) constexpr {
        // Combine the 32-bit pairs into a single 64-bit integer and mask out the sign bit.
        return ((static_cast<t_len>(pair.a) << 32) & pair.b) & 0x7FFFFFFFFFFFFFFF;
    };

    constexpr t_bin_comparator<s_font_code_point_pair> g_code_pt_pair_comparator = [](const s_font_code_point_pair &pa, const s_font_code_point_pair &pb) constexpr {
        return pa.a == pb.a && pa.b == pb.b;
    };

    e_font_load_from_raw_result LoadFontFromRaw(const s_str_rdonly file_path, const t_i32 height, const t_unicode_code_pt_bit_vec &code_pts, s_mem_arena &arrangement_mem_arena, s_mem_arena &atlas_rgbas_mem_arena, s_mem_arena &temp_mem_arena, s_font_arrangement &o_arrangement, s_array<t_font_atlas_rgba> &o_atlas_rgbas, const s_ptr<t_unicode_code_pt_bit_vec> o_unsupported_code_pts) {
        ZF_ASSERT(height > 0);

        const t_len code_pt_cnt = CntSetBits(code_pts);

        if (code_pt_cnt == 0) {
            return ek_font_load_from_raw_result_no_code_pts_given;
        }

        // Get the plain font file data.
        s_array<t_u8> font_file_data = {};

        if (!LoadFileContents(file_path, temp_mem_arena, temp_mem_arena, font_file_data)) {
            return ek_font_load_from_raw_result_other_err;
        }

        // Initialise the font through STB.
        stbtt_fontinfo stb_font_info = {};

        const t_i32 offs = stbtt_GetFontOffsetForIndex(font_file_data.Ptr(), 0);

        if (offs == -1) {
            return ek_font_load_from_raw_result_other_err;
        }

        if (!stbtt_InitFont(&stb_font_info, font_file_data.Ptr(), offs)) {
            return ek_font_load_from_raw_result_other_err;
        }

        // Check for unsupported code points.
        if (o_unsupported_code_pts) {
            t_b8 any_unsupported = false;
            UnsetAllBits(*o_unsupported_code_pts);

            ZF_FOR_EACH_SET_BIT(code_pts, i) {
                const auto code_pt = static_cast<t_code_pt>(i);

                const t_i32 glyph_index = stbtt_FindGlyphIndex(&stb_font_info, static_cast<t_i32>(code_pt));

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
                const auto code_pt = static_cast<t_code_pt>(i);

                const t_i32 glyph_index = stbtt_FindGlyphIndex(&stb_font_info, static_cast<t_i32>(code_pt));

                if (glyph_index == 0) {
                    return ek_font_load_from_raw_result_unsupported_code_pt;
                }
            }
        }

        // Compute general info.
        const t_f32 scale = stbtt_ScaleForPixelHeight(&stb_font_info, static_cast<t_f32>(height));

        t_i32 vm_ascent = 0, vm_descent = 0, vm_line_gap = 0;
        stbtt_GetFontVMetrics(&stb_font_info, &vm_ascent, &vm_descent, &vm_line_gap);

        o_arrangement.line_height = static_cast<t_i32>(static_cast<t_f32>(vm_ascent - vm_descent + vm_line_gap) * scale);

        //
        // Glyph Info
        //
        if (!o_arrangement.code_pts_to_glyph_infos.Init(g_code_pt_hash_func, arrangement_mem_arena, code_pt_cnt)) {
            return ek_font_load_from_raw_result_other_err;
        }

        t_len atlas_index = 0;
        s_v2_i atlas_pen = {};

        ZF_FOR_EACH_SET_BIT(code_pts, i) {
            const auto code_pt = static_cast<t_code_pt>(i);

            const t_i32 glyph_index = stbtt_FindGlyphIndex(&stb_font_info, static_cast<t_i32>(code_pt));

            s_font_glyph_info glyph_info = {};

            t_i32 bm_box_left, bm_box_top, bm_box_right, bm_box_bottom;
            stbtt_GetGlyphBitmapBox(&stb_font_info, glyph_index, scale, scale, &bm_box_left, &bm_box_top, &bm_box_right, &bm_box_bottom);

            glyph_info.offs = {bm_box_left, bm_box_top + static_cast<t_i32>(static_cast<t_f32>(vm_ascent) * scale)};

            glyph_info.size = {bm_box_right - bm_box_left, bm_box_bottom - bm_box_top};

            ZF_ASSERT(glyph_info.size.x <= g_font_atlas_size.x && glyph_info.size.y <= g_font_atlas_size.y);

            t_i32 hm_advance;
            stbtt_GetGlyphHMetrics(&stb_font_info, glyph_index, &hm_advance, nullptr);

            glyph_info.adv = static_cast<t_i32>(static_cast<t_f32>(hm_advance) * scale);

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

            if (o_arrangement.code_pts_to_glyph_infos.Put(code_pt, glyph_info) == ek_hash_map_put_result_error) {
                return ek_font_load_from_raw_result_other_err;
            }
        }

        const t_len atlas_cnt = atlas_index + 1;

        //
        // Kernings
        //

        // If there were any kernings to store, set up the hash map and go through again and store them.
        o_arrangement.has_kernings = true;

        if (!o_arrangement.code_pt_pairs_to_kernings.Init(g_code_pt_pair_hash_func, arrangement_mem_arena, g_hash_map_cap_default, g_code_pt_pair_comparator)) {
            return ek_font_load_from_raw_result_other_err;
        }

        ZF_FOR_EACH_SET_BIT(code_pts, i) {
            ZF_FOR_EACH_SET_BIT(code_pts, j) {
                const auto cp_a = static_cast<t_code_pt>(i);
                const auto cp_b = static_cast<t_code_pt>(j);

                const auto glyph_a_index = stbtt_FindGlyphIndex(&stb_font_info, static_cast<t_i32>(cp_a));
                const auto glyph_b_index = stbtt_FindGlyphIndex(&stb_font_info, static_cast<t_i32>(cp_b));

                const t_i32 kern = stbtt_GetGlyphKernAdvance(&stb_font_info, glyph_a_index, glyph_b_index);

                if (kern != 0) {
                    if (o_arrangement.code_pt_pairs_to_kernings.Put({cp_a, cp_b}, kern) == ek_hash_map_put_result_error) {
                        return ek_font_load_from_raw_result_other_err;
                    }
                }
            }
        }

        //
        // Texture Atlases
        //
        if (!AllocArray(atlas_cnt, atlas_rgbas_mem_arena, o_atlas_rgbas)) {
            return ek_font_load_from_raw_result_other_err;
        }

        // Initialise all pixels to transparent white.
        // @todo: Maybe don't use RBGA for this?
        for (t_len i = 0; i < o_atlas_rgbas.Len(); i++) {
            auto &atlas_rgba = o_atlas_rgbas[i];

            for (t_len j = 0; j < o_atlas_rgbas[i].g_len; j += 4) {
                atlas_rgba[j + 0] = 255;
                atlas_rgba[j + 1] = 255;
                atlas_rgba[j + 2] = 255;
                atlas_rgba[j + 3] = 0;
            }
        }

        // Write pixel data for each individual glyph.
        ZF_FOR_EACH_SET_BIT(code_pts, i) {
            const auto code_pt = static_cast<t_code_pt>(i);

            s_font_glyph_info glyph_info = {};

            if (!o_arrangement.code_pts_to_glyph_infos.Get(code_pt, &glyph_info)) {
                ZF_ASSERT(false);
            }

            auto &atlas_rgba = o_atlas_rgbas[glyph_info.atlas_index];
            const auto &atlas_rect = glyph_info.atlas_rect;

            if (atlas_rect.width == 0 || atlas_rect.height == 0) {
                // Might be the ' ' character for example.
                continue;
            }

            const s_ptr<t_u8> stb_bitmap = stbtt_GetCodepointBitmap(&stb_font_info, scale, scale, static_cast<t_i32>(code_pt), nullptr, nullptr, nullptr, nullptr);

            if (!stb_bitmap) {
                return ek_font_load_from_raw_result_other_err;
            }

            ZF_DEFER({ stbtt_FreeBitmap(stb_bitmap, nullptr); });

            for (t_i32 y = atlas_rect.Top(); y < atlas_rect.Bottom(); y++) {
                for (t_i32 x = atlas_rect.Left(); x < atlas_rect.Right(); x++) {
                    const t_len px_index = (y * 4 * g_font_atlas_size.x) + (x * 4);
                    const t_len stb_bitmap_index = ((y - atlas_rect.y) * atlas_rect.width) + (x - atlas_rect.x);
                    atlas_rgba[px_index + 3] = stb_bitmap[stb_bitmap_index];
                }
            }
        }

        return ek_font_load_from_raw_result_success;
    }

    t_b8 PackFont(const s_str_rdonly dest_file_path, const s_str_rdonly src_file_path, const t_i32 height, const t_unicode_code_pt_bit_vec &code_pts, s_mem_arena &temp_mem_arena, e_font_load_from_raw_result &o_font_load_from_raw_res, const s_ptr<t_unicode_code_pt_bit_vec> o_unsupported_code_pts) {
        s_font_arrangement arrangement = {};
        s_array<t_font_atlas_rgba> atlas_rgbas = {};

        o_font_load_from_raw_res = LoadFontFromRaw(src_file_path, height, code_pts, temp_mem_arena, temp_mem_arena, temp_mem_arena, arrangement, atlas_rgbas, o_unsupported_code_pts);

        if (o_font_load_from_raw_res != ek_font_load_from_raw_result_success) {
            return false;
        }

        if (!CreateFileAndParentDirs(dest_file_path, temp_mem_arena)) {
            return false;
        }

        s_stream fs = {};

        if (!OpenFile(dest_file_path, ek_file_access_mode_write, temp_mem_arena, fs)) {
            return false;
        }

        ZF_DEFER({ CloseFile(fs); });

        if (!fs.WriteItem(arrangement.line_height)) {
            return false;
        }

        if (!SerializeHashMap(fs, arrangement.code_pts_to_glyph_infos, temp_mem_arena)) {
            return false;
        }

        if (!SerializeHashMap(fs, arrangement.code_pt_pairs_to_kernings, temp_mem_arena)) {
            return false;
        }

        if (!SerializeArray(fs, atlas_rgbas)) {
            return false;
        }

        return true;
    }

    t_b8 UnpackFont(const s_str_rdonly file_path, s_mem_arena &arrangement_mem_arena, s_mem_arena &atlas_rgbas_mem_arena, s_mem_arena &temp_mem_arena, s_font_arrangement &o_arrangement, s_array<t_font_atlas_rgba> &o_atlas_rgbas) {
        o_arrangement = {};
        o_atlas_rgbas = {};

        s_stream fs = {};

        if (!OpenFile(file_path, ek_file_access_mode_read, temp_mem_arena, fs)) {
            return false;
        }

        ZF_DEFER({ CloseFile(fs); });

        if (!fs.ReadItem(o_arrangement.line_height)) {
            return false;
        }

        if (!DeserializeHashMap(fs, arrangement_mem_arena, g_code_pt_hash_func, temp_mem_arena, o_arrangement.code_pts_to_glyph_infos)) {
            return false;
        }

        if (!DeserializeHashMap(fs, arrangement_mem_arena, g_code_pt_pair_hash_func, temp_mem_arena, o_arrangement.code_pt_pairs_to_kernings, g_code_pt_pair_comparator)) {
            return false;
        }

        if (!DeserializeArray(fs, atlas_rgbas_mem_arena, o_atlas_rgbas)) {
            return false;
        }

        return true;
    }

    // ============================================================
    // @section: Shaders
    // ============================================================
    t_b8 PackShaderProg(const s_str_rdonly file_path, const s_str vert_src, const s_str frag_src, s_mem_arena &temp_mem_arena) {
        s_stream fs = {};

        if (!OpenFile(file_path, ek_file_access_mode_write, temp_mem_arena, fs)) {
            return false;
        }

        ZF_DEFER({ CloseFile(fs); });

        if (!SerializeArray(fs, vert_src.bytes)) {
            return false;
        }

        if (!SerializeArray(fs, frag_src.bytes)) {
            return false;
        }

        return true;
    }

    t_b8 UnpackShaderProg(const s_str_rdonly file_path, s_mem_arena &mem_arena, s_mem_arena &temp_mem_arena, s_str &o_vert_src, s_str &o_frag_src) {
        s_stream fs = {};

        if (!OpenFile(file_path, ek_file_access_mode_read, temp_mem_arena, fs)) {
            return false;
        }

        ZF_DEFER({ CloseFile(fs); });

        o_vert_src = {};

        if (!DeserializeArray(fs, mem_arena, o_vert_src.bytes)) {
            return false;
        }

        o_frag_src = {};

        if (!DeserializeArray(fs, mem_arena, o_frag_src.bytes)) {
            return false;
        }

        return true;
    }
}
