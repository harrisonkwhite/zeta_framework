#include <zgl/zgl_gfx.h>

namespace zgl::gfx {
    void frame_submit_rect_rotated(t_frame_context *const context, const zf::math::t_v2 pos, const zf::math::t_v2 size, const zf::math::t_v2 origin, const zf::t_f32 rot, const zf::gfx::t_color_rgba32f color_topleft, const zf::gfx::t_color_rgba32f color_topright, const zf::gfx::t_color_rgba32f color_bottomright, const zf::gfx::t_color_rgba32f color_bottomleft) {
        ZF_ASSERT(origin_check_valid(origin));

        zf::t_static_array<zf::math::t_v2, 4> quad_pts;
        zf::mem::t_arena quad_pts_arena = zf::mem::arena_create_wrapping(zf::mem::to_bytes(quad_pts));
        const zf::math::t_poly_mut quad_poly = zf::math::poly_create_quad_rotated(pos, size, origin, rot, &quad_pts_arena);

        const zf::t_static_array<t_triangle, 2> triangles = {{
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

        frame_submit_triangles(context, zf::array_to_nonstatic(triangles));
    }

    void frame_submit_texture(t_frame_context *const context, const t_resource *const texture, const zf::math::t_v2 pos, const zf::math::t_rect_i src_rect, const zf::math::t_v2 origin, const zf::t_f32 rot) {
        const auto texture_size = texture_get_size(texture);

        zf::math::t_rect_i src_rect_to_use;

        if (zf::math::rects_check_equal(src_rect, {})) {
            src_rect_to_use = {0, 0, texture_size.x, texture_size.y};
        } else {
            ZF_ASSERT(src_rect.x >= 0 && src_rect.y >= 0 && zf::math::rect_get_right(src_rect) <= texture_size.x && zf::math::rect_get_bottom(src_rect) <= texture_size.y);
            src_rect_to_use = src_rect;
        }

        const zf::math::t_rect_f uv_rect = zf::gfx::texture_calc_uv_rect(src_rect_to_use, texture_size);

        zf::t_static_array<zf::math::t_v2, 4> quad_pts;
        zf::mem::t_arena quad_pts_arena = zf::mem::arena_create_wrapping(zf::mem::to_bytes(quad_pts));
        const zf::math::t_poly_mut quad_poly = zf::math::poly_create_quad_rotated(pos, zf::math::v2_i_to_f(zf::math::rect_get_size(src_rect_to_use)), origin, rot, &quad_pts_arena);

        const zf::t_static_array<t_triangle, 2> triangles = {{
            {
                .verts = {{
                    {.pos = quad_poly.pts[0], .blend = zf::gfx::k_color_white, .uv = zf::math::rect_get_topleft(uv_rect)},
                    {.pos = quad_poly.pts[1], .blend = zf::gfx::k_color_white, .uv = zf::math::rect_get_topright(uv_rect)},
                    {.pos = quad_poly.pts[3], .blend = zf::gfx::k_color_white, .uv = zf::math::rect_get_bottomleft(uv_rect)},
                }},
            },
            {
                .verts = {{
                    {.pos = quad_poly.pts[3], .blend = zf::gfx::k_color_white, .uv = zf::math::rect_get_bottomleft(uv_rect)},
                    {.pos = quad_poly.pts[1], .blend = zf::gfx::k_color_white, .uv = zf::math::rect_get_topright(uv_rect)},
                    {.pos = quad_poly.pts[2], .blend = zf::gfx::k_color_white, .uv = zf::math::rect_get_bottomright(uv_rect)},
                }},
            },
        }};

        frame_submit_triangles(context, zf::array_to_nonstatic(triangles), texture);
    }

    t_font font_create_from_raw(const zf::strs::t_str_rdonly file_path, const zf::t_i32 height, zf::strs::t_code_pt_bitset *const code_pts, zf::mem::t_arena *const temp_arena, t_resource_group *const resource_group) {
        zf::gfx::t_font_arrangement arrangement;
        zf::t_array_mut<zf::gfx::t_font_atlas_rgba> atlas_rgbas;

        if (!zf::gfx::font_load_from_raw(file_path, height, code_pts, resource_group->arena, temp_arena, temp_arena, &arrangement, &atlas_rgbas)) {
            ZF_FATAL();
        }

        const zf::t_array_mut<t_resource *> atlases = zf::mem::arena_push_array<t_resource *>(resource_group->arena, atlas_rgbas.len);

        for (zf::t_i32 i = 0; i < atlas_rgbas.len; i++) {
            atlases[i] = texture_create({zf::gfx::k_font_atlas_size, atlas_rgbas[i]}, resource_group);
        }

        return {
            .arrangement = arrangement,
            .atlases = atlases,
        };
    }

    t_font font_create_from_packed(const zf::strs::t_str_rdonly file_path, zf::mem::t_arena *const temp_arena, t_resource_group *const resource_group) {
        zf::gfx::t_font_arrangement arrangement;
        zf::t_array_mut<zf::gfx::t_font_atlas_rgba> atlas_rgbas;

        if (!zf::gfx::font_unpack(file_path, resource_group->arena, temp_arena, temp_arena, &arrangement, &atlas_rgbas)) {
            ZF_FATAL();
        }

        const auto atlases = zf::mem::arena_push_array<t_resource *>(resource_group->arena, atlas_rgbas.len);

        for (zf::t_i32 i = 0; i < atlas_rgbas.len; i++) {
            atlases[i] = texture_create({zf::gfx::k_font_atlas_size, atlas_rgbas[i]}, resource_group);
        }

        return {
            .arrangement = arrangement,
            .atlases = atlases,
        };
    }

    static zf::t_array_mut<zf::math::t_v2> get_str_chr_render_positions(const zf::strs::t_str_rdonly str, const zf::gfx::t_font_arrangement &font_arrangement, const zf::math::t_v2 pos, const zf::math::t_v2 alignment, zf::mem::t_arena *const arena) {
        ZF_ASSERT(zf::strs::str_check_valid_utf8(str));
        ZF_ASSERT(alignment_check_valid(alignment));

        // Calculate some useful string metadata.
        struct t_str_meta {
            zf::t_i32 len;
            zf::t_i32 line_cnt;
        };

        const auto str_meta = [str]() {
            t_str_meta meta = {.line_cnt = 1};

            ZF_WALK_STR (str, step) {
                meta.len++;

                if (step.code_pt == '\n') {
                    meta.line_cnt++;
                }
            }

            return meta;
        }();

        // Reserve memory for the character positions.
        const auto positions = zf::mem::arena_push_array<zf::math::t_v2>(arena, str_meta.len);

        // From the line count we can determine the vertical alignment offset to apply.
        const zf::t_f32 alignment_offs_y = static_cast<zf::t_f32>(-(str_meta.line_cnt * font_arrangement.line_height)) * alignment.y;

        // Calculate the position of each character.
        zf::t_i32 chr_index = 0;
        zf::math::t_v2 chr_pos_pen = {}; // The position of the current character.
        zf::t_i32 line_begin_chr_index = 0;
        zf::t_i32 line_len = 0;
        zf::strs::t_code_pt code_pt_last;

        const auto apply_hor_alignment_offs = [&]() {
            if (line_len > 0) {
                const auto line_width = chr_pos_pen.x;

                for (zf::t_i32 i = line_begin_chr_index; i < chr_index; i++) {
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
                chr_pos_pen.y += static_cast<zf::t_f32>(font_arrangement.line_height);

                line_len = 0;

                continue;
            }

            zf::gfx::t_font_glyph_info *glyph_info;

            if (!zf::ds::hash_map_find(&font_arrangement.code_pts_to_glyph_infos, step.code_pt, &glyph_info)) {
                ZF_ASSERT(false && "Unsupported code point!");
                continue;
            }

            if (chr_index > 0 && font_arrangement.has_kernings) {
                zf::t_i32 *kerning;

                if (zf::ds::hash_map_find(&font_arrangement.code_pt_pairs_to_kernings, {code_pt_last, step.code_pt}, &kerning)) {
                    chr_pos_pen.x += static_cast<zf::t_f32>(*kerning);
                }
            }

            positions[chr_index] = pos + chr_pos_pen + zf::math::v2_i_to_f(glyph_info->offs);
            positions[chr_index].y += alignment_offs_y;

            chr_pos_pen.x += static_cast<zf::t_f32>(glyph_info->adv);

            line_len++;
        }

        apply_hor_alignment_offs();

        return positions;
    }

    void frame_submit_str(t_frame_context *const context, const zf::strs::t_str_rdonly str, const t_font &font, const zf::math::t_v2 pos, zf::mem::t_arena *const temp_arena, const zf::math::t_v2 alignment, const zf::gfx::t_color_rgba32f blend) {
        ZF_ASSERT(zf::strs::str_check_valid_utf8(str));
        ZF_ASSERT(alignment_check_valid(alignment));

        if (zf::strs::str_check_empty(str)) {
            return;
        }

        const zf::t_array_mut<zf::math::t_v2> chr_positions = get_str_chr_render_positions(str, font.arrangement, pos, alignment, temp_arena);

        zf::t_i32 chr_index = 0;

        ZF_WALK_STR (str, step) {
            if (step.code_pt == ' ' || step.code_pt == '\n') {
                chr_index++;
                continue;
            }

            zf::gfx::t_font_glyph_info *glyph_info;

            if (!zf::ds::hash_map_find(&font.arrangement.code_pts_to_glyph_infos, step.code_pt, &glyph_info)) {
                ZF_ASSERT(false && "Unsupported code point!");
                continue;
            }

            frame_submit_texture(context, font.atlases[glyph_info->atlas_index], chr_positions[chr_index], glyph_info->atlas_rect);

            chr_index++;
        };
    }
}
