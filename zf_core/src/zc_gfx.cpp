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

















    struct s_font_glyph_info {
        // These are for determining positioning relative to other characters.
        s_v2<t_s32> offs;
        s_v2<t_s32> size;
        t_s32 adv;

        // In what texture atlas is this glyph, and where?
        t_s32 atlas_index;
        s_rect<t_s32> atlas_rect;
    };

    struct s_font {
        t_s32 line_height;
        s_hash_map<t_s32, t_s32> codepoints_to_glyph_indexes;
        s_hash_map<t_s32, s_font_glyph_info> glyph_indexes_to_info;
    };

    struct s_font_ingame {
        t_s32 line_height;
        s_array<s_font_glyph_info> glyph_infos;
        s_hash_map<t_s32, t_s32> codepoints_to_glyph_info_indexes;
        s_array<t_s32> glyph_kernings;
        s_hash_map<t_s64, t_size> codepoint_pairs_to_glyph_kerning_indexes;
    };

    [[nodiscard]] static t_b8 LoadFontFromSTBInfo(s_mem_arena& mem_arena, const stbtt_fontinfo& stb_font_info, const t_s32 height, const s_array_rdonly<t_s32> utf_codepoints, s_mem_arena& temp_mem_arena, s_font& o_font) {
        // Get general metadata.
        const t_f32 scale = stbtt_ScaleForPixelHeight(&stb_font_info, static_cast<t_f32>(height));

        t_s32 vm_ascent, vm_descent, vm_line_gap;
        stbtt_GetFontVMetrics(&stb_font_info, &vm_ascent, &vm_descent, &vm_line_gap);

        o_font.line_height = static_cast<t_s32>(static_cast<t_f32>(vm_ascent - vm_descent + vm_line_gap) * scale);

        // Figure out which and how many glyphs we'll be using.
        s_bit_vector glyphs_to_use;

        if (!MakeBitVector(temp_mem_arena, stb_font_info.numGlyphs, glyphs_to_use)) {
            return false;
        }

        for (t_size i = 0; i < utf_codepoints.len; i++) {
            const t_s32 glyph_index = stbtt_FindGlyphIndex(&stb_font_info, utf_codepoints[i]);
            SetBit(glyphs_to_use, glyph_index);
        }

        const t_size glyphs_to_use_cnt = CountSetBits(glyphs_to_use);

        // Allocate the necessary memory.
        if (!MakeHashMap(mem_arena, g_s32_hash_func, o_font.codepoints_to_glyph_indexes, DefaultComparator, glyphs_to_use_cnt, glyphs_to_use_cnt)) {
            return false;
        }

        if (!MakeHashMap(mem_arena, g_s32_hash_func, o_font.glyph_indexes_to_info, DefaultComparator, glyphs_to_use_cnt, glyphs_to_use_cnt)) {
            return false;
        }

        // Store data for each glyph.
        for (t_size i = 0; i < utf_codepoints.len; i++) {
            const t_s32 glyph_index = stbtt_FindGlyphIndex(&stb_font_info, utf_codepoints[i]);

            if (!HashMapPut(o_font.codepoints_to_glyph_indexes, utf_codepoints[i], glyph_index)) {
                return false;
            }

            if (HashMapGet(o_font.glyph_indexes_to_info, glyph_index)) {
                // Glyph already stored.
                continue;
            }

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

            t_s32 hm_advance;
            stbtt_GetGlyphHMetrics(&stb_font_info, glyph_index, &hm_advance, nullptr);

            glyph_info.adv = static_cast<t_s32>(static_cast<t_f32>(hm_advance) * scale);

            if (!HashMapPut(o_font.glyph_indexes_to_info, glyph_index, glyph_info)) {
                return false;
            }
        }

        return true;
    }

#if 0
    struct s_font_kerning {
        t_s32 glyph_a_index;
        t_s32 glyph_b_index;
        t_s32 kern;
    };

    struct s_font_arrangement {
        t_s32 line_height;

        s_array<s_v2<t_s32>> chr_offsets;
        s_array<s_v2<t_s32>> chr_sizes;
        s_array<t_s32> chr_advances;

        s_array<s_font_kerning> kernings; // Only keep non-zero ones. This can be made into a hash map in-game.
    };

    struct s_font_glyph_info {
        s_v2<t_s32> offs;
        s_v2<t_s32> size;
        t_s32 adv;
    };

    // @todo: The UTF codepoints array here should really just be a string processed normally. This means duplicate codepoints are allowed.
    [[nodiscard]] static t_b8 LoadFontArrangement(s_mem_arena& mem_arena, const stbtt_fontinfo& stb_font_info, const t_s32 height, const s_array_rdonly<t_s32> utf_codepoints, s_font_arrangement& o_arrangement) {
        ZF_ASSERT(false);
        ZF_ASSERT(!IsArrayEmpty(utf_codepoints));

        const t_f32 scale = stbtt_ScaleForPixelHeight(&stb_font_info, static_cast<t_f32>(height));

        t_s32 vm_ascent, vm_descent, vm_line_gap;
        stbtt_GetFontVMetrics(&stb_font_info, &vm_ascent, &vm_descent, &vm_line_gap);

        o_arrangement.line_height = static_cast<t_s32>(static_cast<t_f32>(vm_ascent - vm_descent + vm_line_gap) * scale);

        // Each entry maps a GLYPH INDEX to character data.
        s_hash_map<t_s32, s_font_glyph_info> glyphs_to_info = {};
        s_hash_map<t_s32, t_s32> codepoints_to_glyph_indexes = {};

        for (t_size i = 0; i < utf_codepoints.len; i++) {
            const t_s32 glyph_index = stbtt_FindGlyphIndex(&stb_font_info, utf_codepoints[i]);

            if (!HashMapPut(codepoints_to_glyph_indexes, utf_codepoints[i], glyph_index)) {
                return false;
            }

            if (HashMapGet(glyphs_to_info, glyph_index)) {
                // Glyph already stored.
                continue;
            }

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

            t_s32 hm_advance;
            stbtt_GetGlyphHMetrics(&stb_font_info, glyph_index, &hm_advance, nullptr);

            glyph_info.adv = static_cast<t_s32>(static_cast<t_f32>(hm_advance) * scale);

            if (!HashMapPut(glyphs_to_info, glyph_index, glyph_info)) {
                return false;
            }
        }

        return true;
    }
#endif





















}
