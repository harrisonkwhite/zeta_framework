#include <zgl/zgl_gfx_helpers.h>

namespace zf {
    void RenderTexture(s_rendering_context &rendering_context, const s_gfx_resource &texture, const s_v2 pos, const s_rect_i src_rect) {
        const auto texture_size = TextureSize(texture);

        s_rect_i src_rect_to_use;

        if (src_rect == s_rect_i()) {
            src_rect_to_use = {0, 0, texture_size.x, texture_size.y};
        } else {
            ZF_ASSERT(src_rect.x >= 0 && src_rect.y >= 0 && src_rect.Right() <= texture_size.x && src_rect.Bottom() <= texture_size.y);
            src_rect_to_use = src_rect;
        }

        const s_rect_f rect = {pos.x, pos.y, static_cast<t_f32>(src_rect_to_use.Size().x), static_cast<t_f32>(src_rect_to_use.Size().y)};
        const s_rect_f uv_rect = CalcUVRect(src_rect_to_use, texture_size);

        const s_static_array<s_render_triangle, 2> triangles = {{
            {
                .verts = {{
                    {.pos = rect.TopLeft(), .blend = g_color_white, .uv = uv_rect.TopLeft()},
                    {.pos = rect.TopRight(), .blend = g_color_white, .uv = uv_rect.TopRight()},
                    {.pos = rect.BottomRight(), .blend = g_color_white, .uv = uv_rect.BottomRight()},
                }},
            },
            {
                .verts = {{
                    {.pos = rect.BottomRight(), .blend = g_color_white, .uv = uv_rect.BottomRight()},
                    {.pos = rect.BottomLeft(), .blend = g_color_white, .uv = uv_rect.BottomLeft()},
                    {.pos = rect.TopLeft(), .blend = g_color_white, .uv = uv_rect.TopLeft()},
                }},
            },
        }};

        RenderTriangles(&rendering_context, triangles.ToNonstatic(), &texture);
    }

    s_font CreateFontFromRaw(const s_str_rdonly file_path, const t_i32 height, t_code_pt_bit_vec &code_pts, s_mem_arena &temp_mem_arena, s_gfx_resource_arena &resource_arena) {
        s_font_arrangement arrangement;
        s_array_mut<t_font_atlas_rgba> atlas_rgbas;

        if (!zf::LoadFontDataFromRaw(file_path, height, code_pts, *resource_arena.mem_arena, temp_mem_arena, temp_mem_arena, arrangement, atlas_rgbas)) {
            ZF_FATAL();
        }

        const auto atlases = AllocArray<s_ptr<s_gfx_resource>>(atlas_rgbas.Len(), *resource_arena.mem_arena);

        for (t_i32 i = 0; i < atlas_rgbas.Len(); i++) {
            atlases[i] = CreateTextureResource({g_font_atlas_size, atlas_rgbas[i]}, resource_arena);
        }

        return {
            .arrangement = arrangement,
            .atlases = atlases,
        };
    }

    s_font CreateFontFromPacked(const s_str_rdonly file_path, s_mem_arena &temp_mem_arena, s_gfx_resource_arena &resource_arena) {
        s_font_arrangement arrangement;
        s_array_mut<t_font_atlas_rgba> atlas_rgbas;

        if (!zf::UnpackFont(file_path, *resource_arena.mem_arena, temp_mem_arena, temp_mem_arena, arrangement, atlas_rgbas)) {
            ZF_FATAL();
        }

        const auto atlases = AllocArray<s_ptr<s_gfx_resource>>(atlas_rgbas.Len(), *resource_arena.mem_arena);

        for (t_i32 i = 0; i < atlas_rgbas.Len(); i++) {
            atlases[i] = CreateTextureResource({g_font_atlas_size, atlas_rgbas[i]}, resource_arena);
        }

        return {
            .arrangement = arrangement,
            .atlases = atlases,
        };
    }

    s_array_mut<s_v2> CalcStrChrRenderPositions(const s_str_rdonly str, const s_font_arrangement &font_arrangement, const s_v2 pos, const s_v2 alignment, s_mem_arena &mem_arena) {
        ZF_ASSERT(IsStrValidUTF8(str));
        ZF_ASSERT(IsAlignmentValid(alignment));

        // Calculate some useful string metadata.
        struct s_str_meta {
            t_i32 len = 0;
            t_i32 line_cnt = 0;
        };

        const auto str_meta = [str]() {
            s_str_meta meta = {.line_cnt = 1};

            ZF_WALK_STR(str, chr_info) {
                meta.len++;

                if (chr_info.code_pt == '\n') {
                    meta.line_cnt++;
                }
            }

            return meta;
        }();

        // Reserve memory for the character positions.
        const auto positions = AllocArray<s_v2>(str_meta.len, mem_arena);

        // From the line count we can determine the vertical alignment offset to apply.
        const t_f32 alignment_offs_y = static_cast<t_f32>(-(str_meta.line_cnt * font_arrangement.line_height)) * alignment.y;

        // Calculate the position of each character.
        t_i32 chr_index = 0;
        s_v2 chr_pos_pen = {}; // The position of the current character.
        t_i32 line_begin_chr_index = 0;
        t_i32 line_len = 0;
        t_code_pt code_pt_last;

        const auto apply_hor_alignment_offs = [&]() {
            if (line_len > 0) {
                const auto line_width = chr_pos_pen.x;

                for (t_i32 i = line_begin_chr_index; i < chr_index; i++) {
                    positions[i].x -= line_width * alignment.x;
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

            s_ptr<s_font_glyph_info> glyph_info;

            if (!font_arrangement.code_pts_to_glyph_infos.Find(chr_info.code_pt, glyph_info)) {
                ZF_ASSERT(false && "Unsupported code point!");
                continue;
            }

            if (chr_index > 0 && font_arrangement.has_kernings) {
                s_ptr<t_i32> kerning;

                if (font_arrangement.code_pt_pairs_to_kernings.Find({code_pt_last, chr_info.code_pt}, kerning)) {
                    chr_pos_pen.x += static_cast<t_f32>(*kerning);
                }
            }

            positions[chr_index] = pos + chr_pos_pen + glyph_info->offs.ToV2();
            positions[chr_index].y += alignment_offs_y;

            chr_pos_pen.x += static_cast<t_f32>(glyph_info->adv);

            line_len++;
        }

        apply_hor_alignment_offs();

        return positions;
    }

    void RenderStr(s_rendering_context &rendering_context, const s_str_rdonly str, const s_font &font, const s_v2 pos, s_mem_arena &temp_mem_arena, const s_v2 alignment, const s_color_rgba32f blend) {
        ZF_ASSERT(IsStrValidUTF8(str));
        ZF_ASSERT(IsAlignmentValid(alignment));

        if (IsStrEmpty(str)) {
            return;
        }

        const auto &font_arrangement = font.arrangement;
        const auto &font_atlases = font.atlases;

        const s_array_mut<s_v2> chr_positions = CalcStrChrRenderPositions(str, font_arrangement, pos, alignment, temp_mem_arena);

        t_i32 chr_index = 0;

        ZF_WALK_STR(str, chr_info) {
            if (chr_info.code_pt == ' ' || chr_info.code_pt == '\n') {
                chr_index++;
                continue;
            }

            s_ptr<s_font_glyph_info> glyph_info;

            if (!font_arrangement.code_pts_to_glyph_infos.Find(chr_info.code_pt, glyph_info)) {
                ZF_ASSERT(false && "Unsupported code point!");
                continue;
            }

            RenderTexture(rendering_context, *font_atlases[glyph_info->atlas_index], chr_positions[chr_index], glyph_info->atlas_rect);

            chr_index++;
        };
    }
}
