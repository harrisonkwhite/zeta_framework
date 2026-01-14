#include <zgl/zgl_gfx.h>

namespace zgl::gfx {
    void frame_submit_rect_rotated(t_frame_context *const context, const zcl::t_v2 pos, const zcl::t_v2 size, const zcl::t_v2 origin, const zcl::t_f32 rot, const zcl::t_color_rgba32f color_topleft, const zcl::t_color_rgba32f color_topright, const zcl::t_color_rgba32f color_bottomright, const zcl::t_color_rgba32f color_bottomleft) {
        ZCL_ASSERT(origin_check_valid(origin));

        zcl::t_static_array<zcl::t_v2, 4> quad_pts;
        zcl::t_arena quad_pts_arena = zcl::arena_create_wrapping(zcl::to_bytes(&quad_pts));
        const zcl::t_poly_mut quad_poly = zcl::PolyCreateQuadRotated(pos, size, origin, rot, &quad_pts_arena);

        const zcl::t_static_array<t_triangle, 2> triangles = {{
            {
                .verts = {{
                    {.pos = quad_poly.pts[0], .blend = color_topleft, .uv = {}},
                    {.pos = quad_poly.pts[1], .blend = color_topright, .uv = {}},
                    {.pos = quad_poly.pts[3], .blend = color_bottomleft, .uv = {}},
                }},
            },
            {
                .verts = {{
                    {.pos = quad_poly.pts[3], .blend = color_bottomleft, .uv = {}},
                    {.pos = quad_poly.pts[1], .blend = color_topright, .uv = {}},
                    {.pos = quad_poly.pts[2], .blend = color_bottomright, .uv = {}},
                }},
            },
        }};

        frame_submit_triangles(context, zcl::array_to_nonstatic(&triangles));
    }

    void frame_submit_texture(t_frame_context *const context, const t_resource *const texture, const zcl::t_v2 pos, const zcl::t_rect_i src_rect, const zcl::t_v2 origin, const zcl::t_f32 rot) {
        const auto texture_size = texture_get_size(texture);

        zcl::t_rect_i src_rect_to_use;

        if (zcl::RectsCheckEqual(src_rect, {})) {
            src_rect_to_use = {0, 0, texture_size.x, texture_size.y};
        } else {
            ZCL_ASSERT(src_rect.x >= 0 && src_rect.y >= 0 && zcl::RectGetRight(src_rect) <= texture_size.x && zcl::RectGetBottom(src_rect) <= texture_size.y);
            src_rect_to_use = src_rect;
        }

        const zcl::t_rect_f uv_rect = zcl::TextureCalcUVRect(src_rect_to_use, texture_size);

        zcl::t_static_array<zcl::t_v2, 4> quad_pts;
        zcl::t_arena quad_pts_arena = zcl::arena_create_wrapping(zcl::to_bytes(&quad_pts));
        const zcl::t_poly_mut quad_poly = zcl::PolyCreateQuadRotated(pos, zcl::V2IToF(zcl::RectGetSize(src_rect_to_use)), origin, rot, &quad_pts_arena);

        const zcl::t_static_array<t_triangle, 2> triangles = {{
            {
                .verts = {{
                    {.pos = quad_poly.pts[0], .blend = zcl::k_color_white, .uv = zcl::RectGetTopLeft(uv_rect)},
                    {.pos = quad_poly.pts[1], .blend = zcl::k_color_white, .uv = zcl::RectGetTopRight(uv_rect)},
                    {.pos = quad_poly.pts[3], .blend = zcl::k_color_white, .uv = zcl::RectGetBottomLeft(uv_rect)},
                }},
            },
            {
                .verts = {{
                    {.pos = quad_poly.pts[3], .blend = zcl::k_color_white, .uv = zcl::RectGetBottomLeft(uv_rect)},
                    {.pos = quad_poly.pts[1], .blend = zcl::k_color_white, .uv = zcl::RectGetTopRight(uv_rect)},
                    {.pos = quad_poly.pts[2], .blend = zcl::k_color_white, .uv = zcl::RectGetBottomRight(uv_rect)},
                }},
            },
        }};

        frame_submit_triangles(context, zcl::array_to_nonstatic(&triangles), texture);
    }

    t_font font_create_from_raw(const zcl::t_str_rdonly file_path, const zcl::t_i32 height, zcl::t_code_point_bitset *const code_pts, t_resource_group *const resource_group, zcl::t_arena *const temp_arena) {
        zcl::t_font_arrangement arrangement;
        zcl::t_array_mut<zcl::t_font_atlas_rgba> atlas_rgbas;

        if (!zcl::FontLoadFromRaw(file_path, height, code_pts, resource_group->arena, temp_arena, temp_arena, &arrangement, &atlas_rgbas)) {
            ZCL_FATAL();
        }

        const zcl::t_array_mut<t_resource *> atlases = zcl::arena_push_array<t_resource *>(resource_group->arena, atlas_rgbas.len);

        for (zcl::t_i32 i = 0; i < atlas_rgbas.len; i++) {
            atlases[i] = texture_create({zcl::k_font_atlas_size, atlas_rgbas[i]}, resource_group);
        }

        return {
            .arrangement = arrangement,
            .atlases = atlases,
        };
    }

    t_font font_create_from_packed(const zcl::t_str_rdonly file_path, t_resource_group *const resource_group, zcl::t_arena *const temp_arena) {
        zcl::t_font_arrangement arrangement;
        zcl::t_array_mut<zcl::t_font_atlas_rgba> atlas_rgbas;

        {
            zcl::t_file_stream file_stream;

            if (!zcl::FileOpen(file_path, zcl::t_file_access_mode::ek_file_access_mode_read, temp_arena, &file_stream)) {
                ZCL_FATAL();
            }


            if (!zcl::DeserializeFont(file_stream, resource_group->arena, temp_arena, temp_arena, &arrangement, &atlas_rgbas)) {
                ZCL_FATAL();
            }

            zcl::FileClose(&file_stream);
        }

        const auto atlases = zcl::arena_push_array<t_resource *>(resource_group->arena, atlas_rgbas.len);

        for (zcl::t_i32 i = 0; i < atlas_rgbas.len; i++) {
            atlases[i] = texture_create({zcl::k_font_atlas_size, atlas_rgbas[i]}, resource_group);
        }

        return {
            .arrangement = arrangement,
            .atlases = atlases,
        };
    }

    static zcl::t_array_mut<zcl::t_v2> calc_str_chr_render_positions(const zcl::t_str_rdonly str, const zcl::t_font_arrangement &font_arrangement, const zcl::t_v2 pos, const zcl::t_v2 alignment, zcl::t_arena *const arena) {
        ZCL_ASSERT(zcl::StrCheckValidUTF8(str));
        ZCL_ASSERT(alignment_check_valid(alignment));

        // Calculate some useful string metadata.
        struct t_str_meta {
            zcl::t_i32 len;
            zcl::t_i32 line_cnt;
        };

        const auto str_meta = [str]() {
            t_str_meta meta = {.line_cnt = 1};

            ZCL_STR_WALK (str, step) {
                meta.len++;

                if (step.code_pt == '\n') {
                    meta.line_cnt++;
                }
            }

            return meta;
        }();

        // Reserve memory for the character positions.
        const auto positions = zcl::arena_push_array<zcl::t_v2>(arena, str_meta.len);

        // From the line count we can determine the vertical alignment offset to apply.
        const zcl::t_f32 alignment_offs_y = static_cast<zcl::t_f32>(-(str_meta.line_cnt * font_arrangement.line_height)) * alignment.y;

        // Calculate the position of each character.
        zcl::t_i32 chr_index = 0;
        zcl::t_v2 chr_pos_pen = {}; // The position of the current character.
        zcl::t_i32 line_begin_chr_index = 0;
        zcl::t_i32 line_len = 0;
        zcl::t_code_point code_pt_last;

        const auto apply_hor_alignment_offs = [&]() {
            if (line_len > 0) {
                const auto line_width = chr_pos_pen.x;

                for (zcl::t_i32 i = line_begin_chr_index; i < chr_index; i++) {
                    positions[i].x -= line_width * alignment.x;
                }
            }
        };

        ZCL_STR_WALK (str, step) {
            ZCL_DEFER({
                chr_index++;
                code_pt_last = step.code_pt;
            });

            if (line_len == 0) {
                line_begin_chr_index = chr_index;
            }

            if (step.code_pt == '\n') {
                apply_hor_alignment_offs();

                chr_pos_pen.x = 0.0f;
                chr_pos_pen.y += static_cast<zcl::t_f32>(font_arrangement.line_height);

                line_len = 0;

                continue;
            }

            zcl::t_font_glyph_info *glyph_info;

            if (!zcl::HashMapFind(&font_arrangement.code_pts_to_glyph_infos, step.code_pt, &glyph_info)) {
                ZCL_ASSERT(false && "Unsupported code point!");
                continue;
            }

            if (chr_index > 0 && font_arrangement.has_kernings) {
                zcl::t_i32 *kerning;

                if (zcl::HashMapFind(&font_arrangement.code_pt_pairs_to_kernings, {code_pt_last, step.code_pt}, &kerning)) {
                    chr_pos_pen.x += static_cast<zcl::t_f32>(*kerning);
                }
            }

            positions[chr_index] = pos + chr_pos_pen + zcl::V2IToF(glyph_info->offs);
            positions[chr_index].y += alignment_offs_y;

            chr_pos_pen.x += static_cast<zcl::t_f32>(glyph_info->adv);

            line_len++;
        }

        apply_hor_alignment_offs();

        return positions;
    }

    void frame_submit_str(t_frame_context *const context, const zcl::t_str_rdonly str, const t_font &font, const zcl::t_v2 pos, zcl::t_arena *const temp_arena, const zcl::t_v2 alignment, const zcl::t_color_rgba32f blend) {
        ZCL_ASSERT(zcl::StrCheckValidUTF8(str));
        ZCL_ASSERT(alignment_check_valid(alignment));

        if (zcl::StrCheckEmpty(str)) {
            return;
        }

        const zcl::t_array_mut<zcl::t_v2> chr_positions = calc_str_chr_render_positions(str, font.arrangement, pos, alignment, temp_arena);

        zcl::t_i32 chr_index = 0;

        ZCL_STR_WALK (str, step) {
            if (step.code_pt == ' ' || step.code_pt == '\n') {
                chr_index++;
                continue;
            }

            zcl::t_font_glyph_info *glyph_info;

            if (!zcl::HashMapFind(&font.arrangement.code_pts_to_glyph_infos, step.code_pt, &glyph_info)) {
                ZCL_ASSERT(false && "Unsupported code point!");
                continue;
            }

            frame_submit_texture(context, font.atlases[glyph_info->atlas_index], chr_positions[chr_index], glyph_info->atlas_rect);

            chr_index++;
        };
    }
}
