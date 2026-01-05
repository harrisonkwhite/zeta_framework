#include <zgl/zgl_rendering.h>

namespace zf {
    void f_rendering_submit_texture(t_rendering_context *const context, const t_rendering_resource *const texture, const t_v2 pos, const t_rect_i src_rect) {
        const auto texture_size = f_rendering_get_texture_size(texture);

        t_rect_i src_rect_to_use;

        if (f_math_are_rects_equal(src_rect, {})) {
            src_rect_to_use = {0, 0, texture_size.x, texture_size.y};
        } else {
            ZF_ASSERT(src_rect.x >= 0 && src_rect.y >= 0 && f_math_get_rect_right(src_rect) <= texture_size.x && f_math_get_rect_bottom(src_rect) <= texture_size.y);
            src_rect_to_use = src_rect;
        }

        const t_rect_f rect = f_math_create_rect_f(pos, f_math_convert_to_v2(f_math_get_rect_size(src_rect_to_use)));
        const t_rect_f uv_rect = f_gfx_calc_uv_rect(src_rect_to_use, texture_size);

        const t_static_array<t_batch_triangle, 2> triangles = {{
            {
                .verts = {{
                    {.pos = f_math_get_rect_topleft(rect), .blend = g_gfx_color_white, .uv = f_math_get_rect_topleft(uv_rect)},
                    {.pos = f_math_get_rect_topright(rect), .blend = g_gfx_color_white, .uv = f_math_get_rect_topright(uv_rect)},
                    {.pos = f_math_get_rect_bottomleft(rect), .blend = g_gfx_color_white, .uv = f_math_get_rect_bottomleft(uv_rect)},
                }},
            },
            {
                .verts = {{
                    {.pos = f_math_get_rect_bottomleft(rect), .blend = g_gfx_color_white, .uv = f_math_get_rect_bottomleft(uv_rect)},
                    {.pos = f_math_get_rect_bottomleft(rect), .blend = g_gfx_color_white, .uv = f_math_get_rect_bottomleft(uv_rect)},
                    {.pos = f_math_get_rect_topleft(rect), .blend = g_gfx_color_white, .uv = f_math_get_rect_topleft(uv_rect)},
                }},
            },
        }};

        f_rendering_submit_triangle(context, f_array_get_as_nonstatic(triangles), texture);
    }

    t_font f_rendering_create_font_from_raw(const t_str_rdonly file_path, const t_i32 height, t_code_pt_bit_vec *const code_pts, t_arena *const temp_arena, t_rendering_resource_group *const resource_group) {
        t_font_arrangement arrangement;
        t_array_mut<t_font_atlas_rgba> atlas_rgbas;

        if (!f_gfx_load_font_from_raw(file_path, height, code_pts, resource_group->arena, temp_arena, temp_arena, &arrangement, &atlas_rgbas)) {
            ZF_FATAL();
        }

        const t_array_mut<t_rendering_resource *> atlases = f_mem_arena_push_array<t_rendering_resource *>(resource_group->arena, atlas_rgbas.len);

        for (t_i32 i = 0; i < atlas_rgbas.len; i++) {
            atlases[i] = f_rendering_create_texture({g_gfx_font_atlas_size, atlas_rgbas[i]}, resource_group);
        }

        return {
            .arrangement = arrangement,
            .atlases = atlases,
        };
    }

    t_font f_rendering_create_font_from_packed(const t_str_rdonly file_path, t_arena *const temp_arena, t_rendering_resource_group *const resource_group) {
        t_font_arrangement arrangement;
        t_array_mut<t_font_atlas_rgba> atlas_rgbas;

        if (!f_gfx_unpack_font(file_path, resource_group->arena, temp_arena, temp_arena, &arrangement, &atlas_rgbas)) {
            ZF_FATAL();
        }

        const auto atlases = f_mem_arena_push_array<t_rendering_resource *>(resource_group->arena, atlas_rgbas.len);

        for (t_i32 i = 0; i < atlas_rgbas.len; i++) {
            atlases[i] = f_rendering_create_texture({g_gfx_font_atlas_size, atlas_rgbas[i]}, resource_group);
        }

        return {
            .arrangement = arrangement,
            .atlases = atlases,
        };
    }

    t_array_mut<t_v2> f_rendering_get_str_chr_render_positions(const t_str_rdonly str, const t_font_arrangement &font_arrangement, const t_v2 pos, const t_v2 alignment, t_arena *const arena) {
        ZF_ASSERT(f_strs_is_valid_utf8(str));
        ZF_ASSERT(f_gfx_is_alignment_valid(alignment));

        // Calculate some useful string metadata.
        struct s_str_meta {
            t_i32 len;
            t_i32 line_cnt;
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
        const auto positions = f_mem_arena_push_array<t_v2>(arena, str_meta.len);

        // From the line count we can determine the vertical alignment offset to apply.
        const t_f32 alignment_offs_y = static_cast<t_f32>(-(str_meta.line_cnt * font_arrangement.line_height)) * alignment.y;

        // Calculate the position of each character.
        t_i32 chr_index = 0;
        t_v2 chr_pos_pen = {}; // The position of the current character.
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
                chr_pos_pen.y += static_cast<t_f32>(font_arrangement.line_height);

                line_len = 0;

                continue;
            }

            t_font_glyph_info *glyph_info;

            if (!HashMapPut(&font_arrangement.code_pts_to_glyph_infos, step.code_pt, &glyph_info)) {
                ZF_ASSERT(false && "Unsupported code point!");
                continue;
            }

            if (chr_index > 0 && font_arrangement.has_kernings) {
                t_i32 *kerning;

                if (HashMapPut(&font_arrangement.code_pt_pairs_to_kernings, {code_pt_last, step.code_pt}, &kerning)) {
                    chr_pos_pen.x += static_cast<t_f32>(*kerning);
                }
            }

            positions[chr_index] = pos + chr_pos_pen + f_math_convert_to_v2(glyph_info->offs);
            positions[chr_index].y += alignment_offs_y;

            chr_pos_pen.x += static_cast<t_f32>(glyph_info->adv);

            line_len++;
        }

        apply_hor_alignment_offs();

        return positions;
    }

    void f_rendering_submit_str(t_rendering_context *const context, const t_str_rdonly str, const t_font &font, const t_v2 pos, t_arena *const temp_arena, const t_v2 alignment, const t_color_rgba32f blend) {
        ZF_ASSERT(f_strs_is_valid_utf8(str));
        ZF_ASSERT(f_gfx_is_alignment_valid(alignment));

        if (f_strs_is_empty(str)) {
            return;
        }

        const t_array_mut<t_v2> chr_positions = f_rendering_get_str_chr_render_positions(str, font.arrangement, pos, alignment, temp_arena);

        t_i32 chr_index = 0;

        ZF_WALK_STR (str, step) {
            if (step.code_pt == ' ' || step.code_pt == '\n') {
                chr_index++;
                continue;
            }

            t_font_glyph_info *glyph_info;

            if (!HashMapPut(&font.arrangement.code_pts_to_glyph_infos, step.code_pt, &glyph_info)) {
                ZF_ASSERT(false && "Unsupported code point!");
                continue;
            }

            f_rendering_submit_texture(context, font.atlases[glyph_info->atlas_index], chr_positions[chr_index], glyph_info->atlas_rect);

            chr_index++;
        };
    }
}
