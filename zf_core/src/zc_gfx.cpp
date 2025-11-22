#include <zc/zc_gfx.h>

#include <zc/ds/zc_hash_map.h>
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














    // Assume one-to-many relationship between glyphs and codepoints.
    // Therefore number of codepoints >= number of glyphs.

    constexpr s_v2<t_s32> g_atlas_size = {1024, 1024};

    using t_font_atlas = s_static_array<t_u8, 4 * g_atlas_size.x * g_atlas_size.y>;

    struct s_font_glyph_info {
        // These are for determining positioning relative to other characters.
        s_v2<t_s32> offs;
        s_v2<t_s32> size;
        t_s32 adv;

        // In what texture atlas is this glyph, and where?
        t_size atlas_index;
        s_rect<t_s32> atlas_rect;
    };

    struct s_codepoint_pair {
        t_s32 a;
        t_s32 b;
    };

    struct s_font {
        t_s32 line_height;

        s_hash_map<t_s32, s_font_glyph_info> codepoints_to_glyph_infos; // Some duplicity here since a single glyph might have multiple codepoints mapped to it.
        s_hash_map<s_codepoint_pair, t_s32> codepoint_pairs_to_kernings;
        s_array<t_font_atlas> atlases;
    };

    [[nodiscard]] static t_b8 LoadFont(s_mem_arena& mem_arena, const s_str_rdonly file_path, const t_s32 height, const s_array_rdonly<t_s32> codepoints_no_dups, s_mem_arena& temp_mem_arena, s_font& o_font) {
        ZF_ASSERT(IsStrTerminated(file_path));
        ZF_ASSERT(height > 0);
        ZF_ASSERT(!IsArrayEmpty(codepoints_no_dups));

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
        // Glyph Info
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

            if (HashMapPut(o_font.codepoints_to_glyph_infos, codepoints_no_dups[i], glyph_info) == ec_hash_map_put_result::error) {
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

        const auto codepoint_pair_hash_func = [](const s_codepoint_pair& pair) {
            // Combine the 32-bit pairs into a single 64-bit integer and mask out the sign bit.
            return ((static_cast<t_size>(pair.a) << 32) & pair.b) & 0x7FFFFFFFFFFFFFFF;
        };

        const auto codepoint_pair_comparator = [](const s_codepoint_pair& pa, const s_codepoint_pair& pb) {
            return pa.a == pb.a && pa.b == pb.b;
        };

        if (!MakeHashMap<s_codepoint_pair, t_s32>(mem_arena, codepoint_pair_hash_func, o_font.codepoint_pairs_to_kernings, codepoint_pair_comparator, kern_cnt, kern_cnt)) {
            return false;
        }

        for (t_size i = 0; i < codepoints_no_dups.len; i++) {
            for (t_size j = 0; j < codepoints_no_dups.len; j++) {
                const t_s32 glyph_a_index = stbtt_FindGlyphIndex(&stb_font_info, codepoints_no_dups[i]);
                const t_s32 glyph_b_index = stbtt_FindGlyphIndex(&stb_font_info, codepoints_no_dups[j]);

                const t_s32 kern = stbtt_GetGlyphKernAdvance(&stb_font_info, glyph_a_index, glyph_b_index);

                if (kern != 0) {
                    if (HashMapPut(o_font.codepoint_pairs_to_kernings, {codepoints_no_dups[i], codepoints_no_dups[j]}, kern) == ec_hash_map_put_result::error) {
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

            auto& atlas = o_font.atlases[glyph_info.atlas_index];
            const auto& atlas_rect = glyph_info.atlas_rect;

            for (t_s32 y = RectTop(atlas_rect); y < RectBottom(atlas_rect); y++) {
                for (t_s32 x = RectLeft(atlas_rect); x < RectRight(atlas_rect); x++) {
                    const t_size px_index = (y * 4 * atlas_rect.width) + (x * 4);
                    const t_size stb_bitmap_index = ((y - atlas_rect.y) * atlas_rect.width) + (x - atlas_rect.x);
                    atlas[px_index + 3] = stb_bitmap[stb_bitmap_index];
                }
            }

            stbtt_FreeBitmap(stb_bitmap, nullptr);
        }

        return true;
    }
}
