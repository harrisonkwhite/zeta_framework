#include <zgl/zgl_gfx_second.h>

namespace zf {
    void DrawTriangle(s_rendering_context &rc, const s_static_array<s_v2, 3> &pts, const s_static_array<s_color_rgba32f, 3> &pt_colors) {
        const s_batch_triangle tri = {
            .verts = {
                {.pos = pts[0], .blend = pt_colors[0], .uv = {}},
                {.pos = pts[1], .blend = pt_colors[1], .uv = {}},
                {.pos = pts[2], .blend = pt_colors[2], .uv = {}},
            },
        };

        SubmitTriangle(rc, tri, nullptr);
    }

#if 0
    void DrawRect(s_rendering_context &rc, const s_rect_f rect, const s_color_rgba32f color_topleft, const s_color_rgba32f color_topright, const s_color_rgba32f color_bottomright, const s_color_rgba32f color_bottomleft) {
        const auto verts = ReserveBatchVerts(rc, 6, nullptr);

        verts[0] = {rect.TopLeft(), color_topleft, {0.0f, 0.0f}};
        verts[1] = {rect.TopRight(), color_topright, {1.0f, 0.0f}};
        verts[2] = {rect.BottomRight(), color_bottomright, {1.0f, 1.0f}};

        verts[3] = {rect.BottomRight(), color_bottomright, {1.0f, 1.0f}};
        verts[4] = {rect.BottomLeft(), color_bottomleft, {0.0f, 1.0f}};
        verts[5] = {rect.TopLeft(), color_topleft, {0.0f, 0.0f}};
    }

    void DrawTexture(s_rendering_context &rc, const s_gfx_resource &texture, const s_v2 pos, const s_rect_i src_rect) {
        const auto verts = ReserveBatchVerts(rc, 6, &texture);

        const auto texture_size = texture.Texture().size;

        s_rect_i src_rect_to_use;

        if (src_rect == s_rect_i()) {
            src_rect_to_use = {{}, texture.Texture().size};
        } else {
            ZF_ASSERT(src_rect.x >= 0 && src_rect.y >= 0 && src_rect.Right() <= texture_size.x && src_rect.Bottom() <= texture_size.y);
            src_rect_to_use = src_rect;
        }

        const s_rect_f rect = {pos, src_rect_to_use.Size().ToV2()};
        const s_rect_f uv_rect = UVRect(src_rect_to_use, texture_size);

        verts[0] = {rect.TopLeft(), colors::g_white, uv_rect.TopLeft()};
        verts[1] = {rect.TopRight(), colors::g_white, uv_rect.TopRight()};
        verts[2] = {rect.BottomRight(), colors::g_white, uv_rect.BottomRight()};

        verts[3] = {rect.BottomRight(), colors::g_white, uv_rect.BottomRight()};
        verts[4] = {rect.BottomLeft(), colors::g_white, uv_rect.BottomLeft()};
        verts[5] = {rect.TopLeft(), colors::g_white, uv_rect.TopLeft()};
    }
#endif

    s_array<s_v2> LoadStrChrDrawPositions(const s_str_rdonly str, const s_font_arrangement &font_arrangement, const s_v2 pos, const s_v2 alignment, s_mem_arena &mem_arena) {
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
}
