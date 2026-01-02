#include <zcl/zcl_gfx.h>

#include <stb_image.h>
#include <stb_truetype.h>

namespace zf::gfx {
    constexpr t_hash_func<t_code_pt> g_code_pt_hash_func =
        [](const t_code_pt &code_pt) constexpr {
            return static_cast<t_i32>(code_pt);
        };

    constexpr t_hash_func<s_font_code_point_pair> g_code_pt_pair_hash_func =
        [](const s_font_code_point_pair &pair) constexpr {
            return 0; // @todo: Proper hash function!
        };

    constexpr t_bin_comparator<s_font_code_point_pair> g_code_pt_pair_comparator =
        [](const s_font_code_point_pair &pa, const s_font_code_point_pair &pb) constexpr {
            return pa.a == pb.a && pa.b == pb.b;
        };

    t_b8 LoadTextureDataFromRaw(const s_str_rdonly file_path, s_arena *const texture_data_arena, s_arena *const temp_arena, s_texture_data *const o_texture_data) {
        const s_str_rdonly file_path_terminated = CloneStrButAddTerminator(file_path, temp_arena);

        s_v2_i size_in_pxs;
        t_u8 *const stb_px_data = stbi_load(AsCstr(file_path_terminated), &size_in_pxs.x, &size_in_pxs.y, nullptr, 4);

        if (!stb_px_data) {
            return false;
        }

        ZF_DEFER({ stbi_image_free(stb_px_data); });

        const s_array_rdonly<t_u8> stb_px_data_arr = {stb_px_data, 4 * size_in_pxs.x * size_in_pxs.y};
        const auto px_data = PushArray<t_u8>(texture_data_arena, 4 * size_in_pxs.x * size_in_pxs.y);
        CopyAll(stb_px_data_arr, px_data);

        *o_texture_data = {size_in_pxs, px_data};

        return true;
    }

    t_b8 PackTexture(const s_str_rdonly file_path, const s_texture_data texture_data, s_arena *const temp_arena) {
        if (!CreateFileAndParentDirectories(file_path, temp_arena)) {
            return false;
        }

        c_stream fs;

        if (!FileOpen(file_path, ek_file_access_mode_write, temp_arena, &fs)) {
            return false;
        }

        ZF_DEFER({ FileClose(&fs); });

        if (!fs.WriteItem(texture_data.size_in_pxs)) {
            return false;
        }

        if (!fs.WriteItemsOfArray(texture_data.rgba_px_data)) {
            return false;
        }

        return true;
    }

    t_b8 UnpackTexture(const s_str_rdonly file_path, s_arena *const texture_data_arena, s_arena *const temp_arena, s_texture_data *const o_texture_data) {
        c_stream fs;

        if (!FileOpen(file_path, ek_file_access_mode_read, temp_arena, &fs)) {
            return false;
        }

        ZF_DEFER({ FileClose(&fs); });

        s_v2_i size_in_pxs;

        if (!fs.ReadItem(&size_in_pxs)) {
            return false;
        }

        const auto rgba_px_data = PushArray<t_u8>(texture_data_arena, 4 * size_in_pxs.x * size_in_pxs.y);

        if (!fs.ReadItemsIntoArray(rgba_px_data, rgba_px_data.len)) {
            return false;
        }

        *o_texture_data = {size_in_pxs, rgba_px_data};

        return true;
    }

    t_b8 LoadFontDataFromRaw(const s_str_rdonly file_path, const t_i32 height, t_code_pt_bit_vec *const code_pts, s_arena *const arrangement_arena, s_arena *const atlas_rgbas_arena, s_arena *const temp_arena, s_font_arrangement *const o_arrangement, s_array_mut<t_font_atlas_rgba> *const o_atlas_rgbas) {
        ZF_ASSERT(height > 0);

        // Get the plain font file data.
        s_array_mut<t_u8> font_file_data;

        if (!LoadFileContents(file_path, temp_arena, temp_arena, &font_file_data)) {
            return false;
        }

        // Initialise the font through STB.
        stbtt_fontinfo stb_font_info;

        const t_i32 offs = stbtt_GetFontOffsetForIndex(font_file_data.raw, 0);

        if (offs == -1) {
            return false;
        }

        if (!stbtt_InitFont(&stb_font_info, font_file_data.raw, offs)) {
            return false;
        }

        // Filter out unsupported code points.
        ZF_WALK_SET_BITS (*code_pts, i) {
            const auto code_pt = static_cast<t_code_pt>(i);

            const t_i32 glyph_index = stbtt_FindGlyphIndex(&stb_font_info, static_cast<t_i32>(code_pt));

            if (glyph_index == 0) {
                UnsetBit(*code_pts, i);
            }
        }

        // Compute number of leftover code points that can actually be supported, return if there are none.
        const t_i32 code_pt_cnt = CountSetBits(*code_pts);

        if (code_pt_cnt == 0) {
            return true;
        }

        // Compute general info.
        const t_f32 scale = stbtt_ScaleForPixelHeight(&stb_font_info, static_cast<t_f32>(height));

        t_i32 vm_ascent, vm_descent, vm_line_gap;
        stbtt_GetFontVMetrics(&stb_font_info, &vm_ascent, &vm_descent, &vm_line_gap);

        o_arrangement->line_height = static_cast<t_i32>(static_cast<t_f32>(vm_ascent - vm_descent + vm_line_gap) * scale);

        //
        // Glyph Info
        //
        o_arrangement->code_pts_to_glyph_infos = HashMapCreate<t_code_pt, s_font_glyph_info>(g_code_pt_hash_func, arrangement_arena, code_pt_cnt);

        t_i32 atlas_index = 0;
        s_v2_i atlas_pen = {};

        ZF_WALK_SET_BITS (*code_pts, i) {
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
                atlas_pen.y += o_arrangement->line_height;
            }

            if (atlas_pen.y + glyph_info.size.y > g_font_atlas_size.y) {
                atlas_pen = {};
                atlas_index++;
            }

            glyph_info.atlas_index = atlas_index;
            glyph_info.atlas_rect = {atlas_pen.x, atlas_pen.y, glyph_info.size.x, glyph_info.size.y};
            atlas_pen.x += glyph_info.size.x;

            HashMapPut(&o_arrangement->code_pts_to_glyph_infos, code_pt, glyph_info);
        }

        const t_i32 atlas_cnt = atlas_index + 1;

        //
        // Kernings
        //

        // If there were any kernings to store, set up the hash map and go through again and store them.
        o_arrangement->has_kernings = true;
        o_arrangement->code_pt_pairs_to_kernings = HashMapCreate<s_font_code_point_pair, t_i32>(g_code_pt_pair_hash_func, arrangement_arena, g_hash_map_cap_default, g_code_pt_pair_comparator);

        ZF_WALK_SET_BITS (*code_pts, i) {
            ZF_WALK_SET_BITS (*code_pts, j) {
                const auto cp_a = static_cast<t_code_pt>(i);
                const auto cp_b = static_cast<t_code_pt>(j);

                const auto glyph_a_index = stbtt_FindGlyphIndex(&stb_font_info, static_cast<t_i32>(cp_a));
                const auto glyph_b_index = stbtt_FindGlyphIndex(&stb_font_info, static_cast<t_i32>(cp_b));

                const t_i32 kern = stbtt_GetGlyphKernAdvance(&stb_font_info, glyph_a_index, glyph_b_index);

                if (kern != 0) {
                    HashMapPut(&o_arrangement->code_pt_pairs_to_kernings, {cp_a, cp_b}, kern);
                }
            }
        }

        //
        // Texture Atlases
        //
        *o_atlas_rgbas = PushArray<t_font_atlas_rgba>(atlas_rgbas_arena, atlas_cnt);

        // Initialise all pixels to transparent white.
        // @todo: Maybe don't use RBGA for this?
        for (t_i32 i = 0; i < o_atlas_rgbas->len; i++) {
            const auto atlas_rgba = &(*o_atlas_rgbas)[i];

            for (t_i32 j = 0; j < (*o_atlas_rgbas)[i].g_len; j += 4) {
                (*atlas_rgba)[j + 0] = 255;
                (*atlas_rgba)[j + 1] = 255;
                (*atlas_rgba)[j + 2] = 255;
                (*atlas_rgba)[j + 3] = 0;
            }
        }

        // Write pixel data for each individual glyph.
        ZF_WALK_SET_BITS (*code_pts, i) {
            const auto code_pt = static_cast<t_code_pt>(i);

            s_font_glyph_info *glyph_info;

            if (!HashMapFind(&o_arrangement->code_pts_to_glyph_infos, code_pt, &glyph_info)) {
                ZF_ASSERT(false);
            }

            const auto atlas_rgba = &(*o_atlas_rgbas)[glyph_info->atlas_index];
            const auto &atlas_rect = glyph_info->atlas_rect;

            if (atlas_rect.width == 0 || atlas_rect.height == 0) {
                // Might be the ' ' character for example.
                continue;
            }

            t_u8 *const stb_bitmap = stbtt_GetCodepointBitmap(&stb_font_info, scale, scale, static_cast<t_i32>(code_pt), nullptr, nullptr, nullptr, nullptr);

            if (!stb_bitmap) {
                return false;
            }

            ZF_DEFER({ stbtt_FreeBitmap(stb_bitmap, nullptr); });

            for (t_i32 y = atlas_rect.Top(); y < atlas_rect.Bottom(); y++) {
                for (t_i32 x = atlas_rect.Left(); x < atlas_rect.Right(); x++) {
                    const t_i32 px_index = (y * 4 * g_font_atlas_size.x) + (x * 4);
                    const t_i32 stb_bitmap_index = ((y - atlas_rect.y) * atlas_rect.width) + (x - atlas_rect.x);
                    (*atlas_rgba)[px_index + 3] = stb_bitmap[stb_bitmap_index];
                }
            }
        }

        return true;
    }

    t_b8 PackFont(const s_str_rdonly file_path, const s_font_arrangement &arrangement, const s_array_rdonly<t_font_atlas_rgba> atlas_rgbas, s_arena *const temp_arena) {
        if (!CreateFileAndParentDirectories(file_path, temp_arena)) {
            return false;
        }

        c_stream fs;

        if (!FileOpen(file_path, ek_file_access_mode_write, temp_arena, &fs)) {
            return false;
        }

        ZF_DEFER({ FileClose(&fs); });

        if (!fs.WriteItem(arrangement.line_height)) {
            return false;
        }

        if (!SerializeHashMap(&fs, &arrangement.code_pts_to_glyph_infos, temp_arena)) {
            return false;
        }

        if (!SerializeHashMap(&fs, &arrangement.code_pt_pairs_to_kernings, temp_arena)) {
            return false;
        }

        if (!SerializeArray(&fs, atlas_rgbas)) {
            return false;
        }

        return true;
    }

    t_b8 UnpackFont(const s_str_rdonly file_path, s_arena *const arrangement_arena, s_arena *const atlas_rgbas_arena, s_arena *const temp_arena, s_font_arrangement *const o_arrangement, s_array_mut<t_font_atlas_rgba> *const o_atlas_rgbas) {
        c_stream fs;

        if (!FileOpen(file_path, ek_file_access_mode_read, temp_arena, &fs)) {
            return false;
        }

        ZF_DEFER({ FileClose(&fs); });

        if (!fs.ReadItem(&o_arrangement->line_height)) {
            return false;
        }

        if (!DeserializeHashMap(&fs, arrangement_arena, g_code_pt_hash_func, temp_arena, &o_arrangement->code_pts_to_glyph_infos)) {
            return false;
        }

        if (!DeserializeHashMap(&fs, arrangement_arena, g_code_pt_pair_hash_func, temp_arena, &o_arrangement->code_pt_pairs_to_kernings, g_code_pt_pair_comparator)) {
            return false;
        }

        if (!DeserializeArray(&fs, atlas_rgbas_arena, o_atlas_rgbas)) {
            return false;
        }

        return true;
    }

    t_b8 PackShader(const s_str_rdonly file_path, const s_array_rdonly<t_u8> compiled_shader_bin, s_arena *const temp_arena) {
        if (!CreateFileAndParentDirectories(file_path, temp_arena)) {
            return false;
        }

        c_stream fs;

        if (!FileOpen(file_path, ek_file_access_mode_write, temp_arena, &fs)) {
            return false;
        }

        ZF_DEFER({ FileClose(&fs); });

        if (!SerializeArray(&fs, compiled_shader_bin)) {
            return false;
        }

        return true;
    }

    t_b8 UnpackShader(const s_str_rdonly file_path, s_arena *const shader_bin_arena, s_arena *const temp_arena, s_array_mut<t_u8> *const o_shader_bin) {
        c_stream fs;

        if (!FileOpen(file_path, ek_file_access_mode_read, temp_arena, &fs)) {
            return false;
        }

        ZF_DEFER({ FileClose(&fs); });

        if (!DeserializeArray(&fs, shader_bin_arena, o_shader_bin)) {
            return false;
        }

        return true;
    }
}
