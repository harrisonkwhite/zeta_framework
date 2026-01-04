#include <zgl/zgl_gfx_helpers.h>

namespace zf {
    void RenderTexture(s_rendering_context *const rc, const s_gfx_resource *const texture, const s_v2 pos, const s_rect_i src_rect) {
        const auto texture_size = TextureSize(texture);

        s_rect_i src_rect_to_use;

        if (AreEqual(src_rect, {})) {
            src_rect_to_use = {0, 0, texture_size.x, texture_size.y};
        } else {
            ZF_ASSERT(src_rect.x >= 0 && src_rect.y >= 0 && Right(src_rect) <= texture_size.x && Bottom(src_rect) <= texture_size.y);
            src_rect_to_use = src_rect;
        }

        const s_rect_f rect = CreateRectF(pos, ToV2(Size(src_rect_to_use)));
        const s_rect_f uv_rect = CalcUVRect(src_rect_to_use, texture_size);

        const s_static_array<s_batch_triangle, 2> triangles = {{
            {
                .verts = {{
                    {.pos = TopLeft(rect), .blend = g_color_white, .uv = TopLeft(uv_rect)},
                    {.pos = TopRight(rect), .blend = g_color_white, .uv = TopRight(uv_rect)},
                    {.pos = BottomRight(rect), .blend = g_color_white, .uv = BottomRight(uv_rect)},
                }},
            },
            {
                .verts = {{
                    {.pos = BottomRight(rect), .blend = g_color_white, .uv = BottomRight(uv_rect)},
                    {.pos = BottomLeft(rect), .blend = g_color_white, .uv = BottomLeft(uv_rect)},
                    {.pos = TopLeft(rect), .blend = g_color_white, .uv = TopLeft(uv_rect)},
                }},
            },
        }};

        SubmitTrianglesToBatch(rc, AsNonstatic(triangles), texture);
    }

    s_font CreateFontFromRaw(const strs::StrRdonly file_path, const I32 height, strs::CodePointBitVector *const code_pts, s_arena *const temp_arena, zf_rendering_resource_group *const resource_group) {
        s_font_arrangement arrangement;
        s_array_mut<t_font_atlas_rgba> atlas_rgbas;

        if (!zf::LoadFontDataFromRaw(file_path, height, code_pts, resource_group->arena, temp_arena, temp_arena, &arrangement, &atlas_rgbas)) {
            ZF_FATAL();
        }

        const s_array_mut<s_gfx_resource *> atlases = ArenaPushArray<s_gfx_resource *>(resource_group->arena, atlas_rgbas.len);

        for (I32 i = 0; i < atlas_rgbas.len; i++) {
            atlases[i] = CreateTexture({g_font_atlas_size, atlas_rgbas[i]}, resource_group);
        }

        return {
            .arrangement = arrangement,
            .atlases = atlases,
        };
    }

    s_font CreateFontFromPacked(const strs::StrRdonly file_path, s_arena *const temp_arena, zf_rendering_resource_group *const resource_group) {
        s_font_arrangement arrangement;
        s_array_mut<t_font_atlas_rgba> atlas_rgbas;

        if (!zf::UnpackFont(file_path, resource_group->arena, temp_arena, temp_arena, &arrangement, &atlas_rgbas)) {
            ZF_FATAL();
        }

        const auto atlases = ArenaPushArray<s_gfx_resource *>(resource_group->arena, atlas_rgbas.len);

        for (I32 i = 0; i < atlas_rgbas.len; i++) {
            atlases[i] = CreateTexture({g_font_atlas_size, atlas_rgbas[i]}, resource_group);
        }

        return {
            .arrangement = arrangement,
            .atlases = atlases,
        };
    }

    s_array_mut<s_v2> CalcStrChrRenderPositions(const strs::StrRdonly str, const s_font_arrangement &font_arrangement, const s_v2 pos, const s_v2 alignment, s_arena *const arena) {
        ZF_ASSERT(determine_is_valid_utf8(str));
        ZF_ASSERT(IsStrAlignmentValid(alignment));

        // Calculate some useful string metadata.
        struct s_str_meta {
            I32 len;
            I32 line_cnt;
        };

        const auto str_meta = [str]() {
            s_str_meta meta = {.line_cnt = 1};

            ZF_WALK_STR (str, step) {
                meta.len++;

                if (step.code_pt == '\n') {
                    meta.line_cnt++;
                }
            }

            return meta;
        }();

        // Reserve memory for the character positions.
        const auto positions = ArenaPushArray<s_v2>(arena, str_meta.len);

        // From the line count we can determine the vertical alignment offset to apply.
        const F32 alignment_offs_y = static_cast<F32>(-(str_meta.line_cnt * font_arrangement.line_height)) * alignment.y;

        // Calculate the position of each character.
        I32 chr_index = 0;
        s_v2 chr_pos_pen = {}; // The position of the current character.
        I32 line_begin_chr_index = 0;
        I32 line_len = 0;
        strs::CodePoint code_pt_last;

        const auto apply_hor_alignment_offs = [&]() {
            if (line_len > 0) {
                const auto line_width = chr_pos_pen.x;

                for (I32 i = line_begin_chr_index; i < chr_index; i++) {
                    positions[i].x -= line_width * alignment.x;
                }
            }
        };

        ZF_WALK_STR (str, step) {
            ZF_DEFER({
                chr_index++;
                code_pt_last = step.code_pt;
            });

            if (line_len == 0) {
                line_begin_chr_index = chr_index;
            }

            if (step.code_pt == '\n') {
                apply_hor_alignment_offs();

                chr_pos_pen.x = 0.0f;
                chr_pos_pen.y += static_cast<F32>(font_arrangement.line_height);

                line_len = 0;

                continue;
            }

            s_font_glyph_info *glyph_info;

            if (!HashMapPut(&font_arrangement.code_pts_to_glyph_infos, step.code_pt, &glyph_info)) {
                ZF_ASSERT(false && "Unsupported code point!");
                continue;
            }

            if (chr_index > 0 && font_arrangement.has_kernings) {
                I32 *kerning;

                if (HashMapPut(&font_arrangement.code_pt_pairs_to_kernings, {code_pt_last, step.code_pt}, &kerning)) {
                    chr_pos_pen.x += static_cast<F32>(*kerning);
                }
            }

            positions[chr_index] = pos + chr_pos_pen + ToV2(glyph_info->offs);
            positions[chr_index].y += alignment_offs_y;

            chr_pos_pen.x += static_cast<F32>(glyph_info->adv);

            line_len++;
        }

        apply_hor_alignment_offs();

        return positions;
    }

    void RenderStr(s_rendering_context *const rc, const strs::StrRdonly str, const s_font &font, const s_v2 pos, s_arena *const temp_arena, const s_v2 alignment, const s_color_rgba32f blend) {
        ZF_ASSERT(determine_is_valid_utf8(str));
        ZF_ASSERT(IsStrAlignmentValid(alignment));

        if (strs::get_is_empty(str)) {
            return;
        }

        const s_array_mut<s_v2> chr_positions = CalcStrChrRenderPositions(str, font.arrangement, pos, alignment, temp_arena);

        I32 chr_index = 0;

        ZF_WALK_STR (str, step) {
            if (step.code_pt == ' ' || step.code_pt == '\n') {
                chr_index++;
                continue;
            }

            s_font_glyph_info *glyph_info;

            if (!HashMapPut(&font.arrangement.code_pts_to_glyph_infos, step.code_pt, &glyph_info)) {
                ZF_ASSERT(false && "Unsupported code point!");
                continue;
            }

            RenderTexture(rc, font.atlases[glyph_info->atlas_index], chr_positions[chr_index], glyph_info->atlas_rect);

            chr_index++;
        };
    }
}
