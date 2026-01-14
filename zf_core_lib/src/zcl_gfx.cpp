#include <zcl/zcl_gfx.h>

#include <zcl/zcl_file_sys.h>
#include <stb_image.h>
#include <stb_truetype.h>

namespace zcl {
    t_b8 TextureLoadFromRaw(const t_str_rdonly file_path, t_arena *const texture_data_arena, t_arena *const temp_arena, t_texture_data_mut *const o_texture_data) {
        const t_str_rdonly file_path_terminated = StrCloneButAddTerminator(file_path, temp_arena);

        t_v2_i size_in_pxs;
        t_u8 *const stb_px_data = stbi_load(StrToCStr(file_path_terminated), &size_in_pxs.x, &size_in_pxs.y, nullptr, 4);

        if (!stb_px_data) {
            return false;
        }

        ZCL_DEFER({ stbi_image_free(stb_px_data); });

        const t_array_rdonly<t_u8> stb_px_data_arr = {stb_px_data, 4 * size_in_pxs.x * size_in_pxs.y};
        const auto px_data = arena_push_array<t_u8>(texture_data_arena, 4 * size_in_pxs.x * size_in_pxs.y);
        array_copy(stb_px_data_arr, px_data);

        *o_texture_data = {size_in_pxs, px_data};

        return true;
    }

    t_b8 FontLoadFromRaw(const t_str_rdonly file_path, const t_i32 height, t_code_point_bitset *const code_pts, t_arena *const arrangement_arena, t_arena *const atlas_rgbas_arena, t_arena *const temp_arena, t_font_arrangement *const o_arrangement, t_array_mut<t_font_atlas_rgba> *const o_atlas_rgbas) {
        ZCL_ASSERT(height > 0);

        // Get the plain font file data.
        t_array_mut<t_u8> font_file_data;

        if (!FileLoadContents(file_path, temp_arena, temp_arena, &font_file_data)) {
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
        ZCL_BITSET_WALK_ALL_SET (*code_pts, i) {
            const auto code_pt = static_cast<t_code_point>(i);

            const t_i32 glyph_index = stbtt_FindGlyphIndex(&stb_font_info, static_cast<t_i32>(code_pt));

            if (glyph_index == 0) {
                BitsetUnset(*code_pts, i);
            }
        }

        // Compute number of leftover code points that can actually be supported, return if there are none.
        const t_i32 code_pt_cnt = BitsetCountSet(*code_pts);

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
        o_arrangement->code_pts_to_glyph_infos = HashMapCreate<t_code_point, t_font_glyph_info>(k_font_code_point_hash_func, arrangement_arena, code_pt_cnt);

        t_i32 atlas_index = 0;
        t_v2_i atlas_pen = {};

        constexpr t_i32 k_glyph_padding = 4;

        ZCL_BITSET_WALK_ALL_SET (*code_pts, i) {
            const auto code_pt = static_cast<t_code_point>(i);

            const t_i32 glyph_index = stbtt_FindGlyphIndex(&stb_font_info, static_cast<t_i32>(code_pt));

            t_font_glyph_info glyph_info = {};

            t_i32 bm_box_left, bm_box_top, bm_box_right, bm_box_bottom;
            stbtt_GetGlyphBitmapBox(&stb_font_info, glyph_index, scale, scale, &bm_box_left, &bm_box_top, &bm_box_right, &bm_box_bottom);

            glyph_info.offs = {bm_box_left, bm_box_top + static_cast<t_i32>(static_cast<t_f32>(vm_ascent) * scale)};
            glyph_info.size = {bm_box_right - bm_box_left, bm_box_bottom - bm_box_top};

            ZCL_ASSERT(glyph_info.size.x <= k_font_atlas_size.x && glyph_info.size.y <= k_font_atlas_size.y);

            t_i32 hm_advance;
            stbtt_GetGlyphHMetrics(&stb_font_info, glyph_index, &hm_advance, nullptr);

            glyph_info.adv = static_cast<t_i32>(static_cast<t_f32>(hm_advance) * scale);

            if (atlas_pen.x + glyph_info.size.x + (k_glyph_padding * 2) > k_font_atlas_size.x) {
                atlas_pen.x = 0;
                atlas_pen.y += o_arrangement->line_height + (k_glyph_padding * 2);
            }

            if (atlas_pen.y + glyph_info.size.y + (k_glyph_padding * 2) > k_font_atlas_size.y) {
                atlas_pen = {};
                atlas_index++;
            }

            glyph_info.atlas_index = atlas_index;
            glyph_info.atlas_rect = RectCreateI(atlas_pen + t_v2_i{k_glyph_padding, k_glyph_padding}, glyph_info.size);
            atlas_pen.x += glyph_info.size.x + (k_glyph_padding * 2);

            HashMapPut(&o_arrangement->code_pts_to_glyph_infos, code_pt, glyph_info);
        }

        const t_i32 atlas_cnt = atlas_index + 1;

        //
        // Kernings
        //

        // If there were any kernings to store, set up the hash map and go through again and store them.
        o_arrangement->has_kernings = true;
        o_arrangement->code_pt_pairs_to_kernings = HashMapCreate<t_font_code_point_pair, t_i32>(k_font_code_point_pair_hash_func, arrangement_arena, k_hash_map_cap_default, k_font_code_point_pair_comparator);

        ZCL_BITSET_WALK_ALL_SET (*code_pts, i) {
            ZCL_BITSET_WALK_ALL_SET (*code_pts, j) {
                const auto cp_a = static_cast<t_code_point>(i);
                const auto cp_b = static_cast<t_code_point>(j);

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
        *o_atlas_rgbas = arena_push_array<t_font_atlas_rgba>(atlas_rgbas_arena, atlas_cnt);

        // Initialise all pixels to transparent white.
        // @todo: Maybe don't use RBGA for this?
        for (t_i32 i = 0; i < o_atlas_rgbas->len; i++) {
            const auto atlas_rgba = &(*o_atlas_rgbas)[i];

            for (t_i32 j = 0; j < (*o_atlas_rgbas)[i].k_len; j += 4) {
                (*atlas_rgba)[j + 0] = 255;
                (*atlas_rgba)[j + 1] = 255;
                (*atlas_rgba)[j + 2] = 255;
                (*atlas_rgba)[j + 3] = 0;
            }
        }

        // Write pixel data for each individual glyph.
        ZCL_BITSET_WALK_ALL_SET (*code_pts, i) {
            const auto code_pt = static_cast<t_code_point>(i);

            t_font_glyph_info *glyph_info;

            if (!HashMapFind(&o_arrangement->code_pts_to_glyph_infos, code_pt, &glyph_info)) {
                ZCL_ASSERT(false);
            }

            const auto atlas_rgba = &(*o_atlas_rgbas)[glyph_info->atlas_index];
            const auto atlas_rect = glyph_info->atlas_rect;

            if (atlas_rect.width == 0 || atlas_rect.height == 0) {
                // Might be the ' ' character for example.
                continue;
            }

            t_u8 *const stb_bitmap = stbtt_GetCodepointBitmap(&stb_font_info, scale, scale, static_cast<t_i32>(code_pt), nullptr, nullptr, nullptr, nullptr);

            if (!stb_bitmap) {
                return false;
            }

            ZCL_DEFER({ stbtt_FreeBitmap(stb_bitmap, nullptr); });

            for (t_i32 y = RectGetTop(atlas_rect); y < RectGetBottom(atlas_rect); y++) {
                for (t_i32 x = RectGetLeft(atlas_rect); x < RectGetRight(atlas_rect); x++) {
                    const t_i32 px_index = (y * 4 * k_font_atlas_size.x) + (x * 4);
                    const t_i32 stb_bitmap_index = ((y - atlas_rect.y) * atlas_rect.width) + (x - atlas_rect.x);
                    (*atlas_rgba)[px_index + 3] = stb_bitmap[stb_bitmap_index];
                }
            }
        }

        return true;
    }
}
