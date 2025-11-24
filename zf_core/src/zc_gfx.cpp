#include <zc/zc_gfx.h>

#include <zc/ds/zc_hash_map.h>
#include <stb_image.h>
#include <stb_truetype.h>

namespace zf {
    const t_hash_func<s_codepoint_pair> g_codepoint_pair_hash_func = [](const s_codepoint_pair& pair) {
        // Combine the 32-bit pairs into a single 64-bit integer and mask out the sign bit.
        return ((static_cast<t_size>(pair.a) << 32) & pair.b) & 0x7FFFFFFFFFFFFFFF;
    };

    const t_bin_comparator<s_codepoint_pair> g_codepoint_pair_comparator = [](const s_codepoint_pair& pa, const s_codepoint_pair& pb) {
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

    t_b8 PackTexture(const s_rgba_texture_data_rdonly& tex_data, const s_str_rdonly file_path, s_mem_arena& temp_mem_arena) {
        ZF_ASSERT(IsStrTerminated(file_path));

        ZF_DEFER_MEM_ARENA_REWIND(temp_mem_arena);

        if (!CreateFileAndParentDirs(file_path, temp_mem_arena)) {
            return false;
        }

        s_stream fs;

        if (!OpenFile(file_path, ek_file_access_mode_write, fs)) {
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

    t_b8 LoadFontFromRaw(s_mem_arena& mem_arena, const s_str_rdonly file_path, const t_s32 height, const s_array_rdonly<t_s32> codepoints_no_dups, s_mem_arena& temp_mem_arena, s_font& o_font) {
        ZF_ASSERT(IsStrTerminated(file_path));
        ZF_ASSERT(height > 0);
        ZF_ASSERT(!IsArrayEmpty(codepoints_no_dups));

        ZF_DEFER_MEM_ARENA_REWIND(temp_mem_arena);

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

        o_font.line_height = static_cast<t_s32>(static_cast<t_f32>(vm_ascent - vm_descent + vm_line_gap) * scale);

        //
        // Computing Glyph Info
        //
        if (!MakeHashMap(mem_arena, g_s32_hash_func, o_font.codepoints_to_glyph_infos, DefaultBinComparator, codepoints_no_dups.len, codepoints_no_dups.len)) {
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

            ZF_ASSERT(glyph_info.size.x <= g_atlas_size.x && glyph_info.size.y <= g_atlas_size.y);

            t_s32 hm_advance;
            stbtt_GetGlyphHMetrics(&stb_font_info, glyph_index, &hm_advance, nullptr);

            glyph_info.adv = static_cast<t_s32>(static_cast<t_f32>(hm_advance) * scale);

            if (atlas_pen.x + glyph_info.size.x > g_atlas_size.x) {
                atlas_pen.x = 0;
                atlas_pen.y += o_font.line_height;
            }

            if (atlas_pen.y + glyph_info.size.y > g_atlas_size.y) {
                atlas_pen = {};
                atlas_index++;
            }

            glyph_info.atlas_index = atlas_index;
            glyph_info.atlas_rect = {atlas_pen, glyph_info.size};
            atlas_pen.x += glyph_info.size.x;

            if (HashMapPut(o_font.codepoints_to_glyph_infos, codepoints_no_dups[i], glyph_info) == ek_hash_map_put_result_error) {
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

        if (!MakeHashMap<s_codepoint_pair, t_s32>(mem_arena, g_codepoint_pair_hash_func, o_font.codepoint_pairs_to_kernings, g_codepoint_pair_comparator, kern_cnt, kern_cnt)) {
            return false;
        }

        for (t_size i = 0; i < codepoints_no_dups.len; i++) {
            for (t_size j = 0; j < codepoints_no_dups.len; j++) {
                const t_s32 glyph_a_index = stbtt_FindGlyphIndex(&stb_font_info, codepoints_no_dups[i]);
                const t_s32 glyph_b_index = stbtt_FindGlyphIndex(&stb_font_info, codepoints_no_dups[j]);

                const t_s32 kern = stbtt_GetGlyphKernAdvance(&stb_font_info, glyph_a_index, glyph_b_index);

                if (kern != 0) {
                    if (HashMapPut(o_font.codepoint_pairs_to_kernings, {codepoints_no_dups[i], codepoints_no_dups[j]}, kern) == ek_hash_map_put_result_error) {
                        return false;
                    }
                }
            }
        }

        //
        // Texture Atlas Pixel Data
        //
        if (!MakeArray(mem_arena, atlas_cnt, o_font.atlases)) {
            return false;
        }

        for (t_size i = 0; i < codepoints_no_dups.len; i++) {
            s_font_glyph_info glyph_info;

            if (!HashMapGet(o_font.codepoints_to_glyph_infos, codepoints_no_dups[i], &glyph_info)) {
                ZF_ASSERT(false);
            }

            t_u8* const stb_bitmap = stbtt_GetCodepointBitmap(&stb_font_info, scale, scale, codepoints_no_dups[i], nullptr, nullptr, nullptr, nullptr);

            if (!stb_bitmap) {
                return false;
            }

            ZF_DEFER({ stbtt_FreeBitmap(stb_bitmap, nullptr); });

            auto& atlas = o_font.atlases[glyph_info.atlas_index];
            const auto& atlas_rect = glyph_info.atlas_rect;

            for (t_s32 y = RectTop(atlas_rect); y < RectBottom(atlas_rect); y++) {
                for (t_s32 x = RectLeft(atlas_rect); x < RectRight(atlas_rect); x++) {
                    const t_size px_index = (y * 4 * atlas_rect.width) + (x * 4);
                    const t_size stb_bitmap_index = ((y - atlas_rect.y) * atlas_rect.width) + (x - atlas_rect.x);
                    atlas[px_index + 3] = stb_bitmap[stb_bitmap_index];
                }
            }
        }

        return true;
    }

    t_b8 SerializeFont(s_byte_stream_write& bs, const s_font& font) {
        if (!SerializeItem(bs, font.line_height)) {
            return false;
        }

        if (!SerializeHashMap(bs, font.codepoints_to_glyph_infos)) {
            return false;
        }

        if (!SerializeHashMap(bs, font.codepoint_pairs_to_kernings)) {
            return false;
        }

        if (!SerializeArray(bs, font.atlases)) {
            return false;
        }

        return true;
    }

    t_b8 DeserializeFont(s_mem_arena& mem_arena, s_byte_stream_read& bs, s_font& o_font) {
        if (!DeserializeItem(bs, o_font.line_height)) {
            return false;
        }

        if (!DeserializeHashMap(mem_arena, bs, g_s32_hash_func, DefaultBinComparator, o_font.codepoints_to_glyph_infos)) {
            return false;
        }

        if (!DeserializeHashMap(mem_arena, bs, g_codepoint_pair_hash_func, g_codepoint_pair_comparator, o_font.codepoint_pairs_to_kernings)) {
            return false;
        }

        if (!DeserializeArray(mem_arena, bs, o_font.atlases)) {
            return false;
        }

        return true;
    }

#if 0
    t_b8 PackFont(const s_font& font, const s_str_rdonly file_path, s_mem_arena& temp_mem_arena) {
        ZF_ASSERT(IsStrTerminated(file_path));

        ZF_DEFER_MEM_ARENA_REWIND(temp_mem_arena);

        if (!CreateFileAndParentDirs(file_path, temp_mem_arena)) {
            return false;
        }

        s_file_stream fs;

        if (!OpenFile(file_path, ec_file_access_mode::write, fs)) {
            return false;
        }

        ZF_DEFER({ CloseFile(fs); });

        if (!WriteItemToFile(fs, font.line_height)) {
            return false;
        }

        // @todo
#if 0
        if (!SerializeHashMap(font.codepoints_to_glyph_infos, fs)) {
            return false;
        }

        if (!SerializeHashMap(font.codepoint_pairs_to_kernings, fs)) {
            return false;
        }
#endif

        if (WriteItemArrayToFile(fs, font.atlases) < font.atlases.len) {
            return false;
        }

        return true;
    }

    t_b8 UnpackFont(const s_str_rdonly file_path, s_mem_arena& mem_arena, s_font& o_font) {
        ZF_ASSERT(IsStrTerminated(file_path));

        t_b8 clean_up = false;
        ZF_DEFER_MEM_ARENA_REWIND_IF(mem_arena, clean_up);

        s_file_stream fs;

        if (!OpenFile(file_path, ec_file_access_mode::read, fs)) {
            clean_up = true;
            return false;
        }

        ZF_DEFER({ CloseFile(fs); });

        if (!ReadItemFromFile(fs, o_font.line_height)) {
            clean_up = true;
            return false;
        }

#if 0
        if (!DeserializeHashMap(font.codepoints_to_glyph_infos, fs)) {
        }
#endif

        if (ReadItemArrayFromFile(fs, o_font.atlases) < o_font.atlases.len) {
            clean_up = true;
            return false;
        }

        return true;
    }
#endif
}
