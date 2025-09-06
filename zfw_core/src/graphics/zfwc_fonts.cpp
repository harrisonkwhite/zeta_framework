#include "cu_mem.h"
#include "zfwc_graphics.h"

#include <cstdio>
#include <cctype>

static bool LoadFontFromFile(s_font_arrangement& arrangement, s_font_texture_meta& tex_meta, c_array<t_u8>& tex_rgba_px_data, c_mem_arena& tex_rgba_px_data_mem_arena, const c_array<const char> file_path) {
    FILE* const fs = fopen(file_path.Raw(), "rb");

    if (!fs) {
        //LOG_ERROR("Failed to open font file \"%s\"!", file_path.Raw());
        return false;
    }

    if (fread(&arrangement, sizeof(arrangement), 1, fs) < 1) {
        //LOG_ERROR("Failed to read font arrangement from file \"%s\"!", file_path.Raw());
        fclose(fs);
        return false;
    }

    if (fread(&tex_meta, sizeof(tex_meta), 1, fs) < 1) {
        //LOG_ERROR("Failed to read font texture metadata from file \"%s\"!", file_path.Raw());
        fclose(fs);
        return false;
    }

    tex_rgba_px_data = PushArrayToMemArena<t_u8>(tex_rgba_px_data_mem_arena, 4 * tex_meta.size.x * tex_meta.size.y);

    if (tex_rgba_px_data.IsEmpty()) {
        //LOG_ERROR("Failed to reserve memory for font texture RGBA pixel data from file \"%s\"!", file_path.Raw());
        fclose(fs);
        return false;
    }

    if (fread(tex_rgba_px_data.Raw(), 1, tex_rgba_px_data.Len(), fs) < static_cast<size_t>(tex_rgba_px_data.Len())) {
        //LOG_ERROR("Failed to read font texture RGBA pixel data from file \"%s\"!", file_path.Raw());
        fclose(fs);
        return false;
    }

    fclose(fs);

    return true;
}

bool InitFontGroupFromFiles(s_font_group& font_group, const c_array<const c_array<const char>> file_paths, c_mem_arena& mem_arena, s_gl_resource_arena& gl_res_arena, c_mem_arena& temp_mem_arena) {
    const t_s32 font_cnt = file_paths.Len();

    const auto arrangements = PushArrayToMemArena<s_font_arrangement>(mem_arena, font_cnt);

    if (arrangements.IsEmpty()) {
        //LOG_ERROR("Failed to reserve memory for font arrangements!");
        return false;
    }

    const auto tex_metas = PushArrayToMemArena<s_font_texture_meta>(mem_arena, font_cnt);

    if (tex_metas.IsEmpty()) {
        //LOG_ERROR("Failed to reserve memory for font texture metadata!");
        return false;
    }

    const c_array<t_gl_id> tex_gl_ids = PushArrayToGLResourceArena(gl_res_arena, font_cnt, ek_gl_resource_type_texture);

    if (tex_gl_ids.IsEmpty()) {
        //LOG_ERROR("Failed to reserve OpenGL texture IDs for fonts!");
        return false;
    }

    for (t_s32 i = 0; i < font_cnt; i++) {
        const auto file_path = file_paths[i];

        c_array<t_u8> tex_rgba_px_data;

        if (!LoadFontFromFile(arrangements[i], tex_metas[i], tex_rgba_px_data, temp_mem_arena, file_path)) {
            return false;
        }

        tex_gl_ids[i] = GenGLTextureFromRGBA(s_rgba_texture{.tex_size = tex_metas[i].size, .px_data = tex_rgba_px_data});

        if (!tex_gl_ids[i]) {
            //LOG_ERROR("Failed to generate OpenGL texture from font RGBA pixel data for font file \"%s\"!", file_path.Raw());
            return false;
        }
    }

    font_group = {
        .arrangements = arrangements.View(),
        .tex_metas = tex_metas.View(),
        .tex_gl_ids = tex_gl_ids.View()
    };

    return true;
}

static void CalcStrLenAndLineCnt(t_s32& len, t_s32& line_cnt, const c_array<const char> str) {
    line_cnt = 1;

    for (; str[len]; len++) {
        if (str[len] == '\n') {
            line_cnt++;
        }
    }
}

bool GenStrChrRenderPositions(c_array<s_v2>& positions, c_mem_arena& mem_arena, const c_array<const char> str, const s_font_group& font_group, const t_s32 font_index, const s_v2 pos, const s_v2 alignment) {
    assert(positions.IsEmpty());
    //assert(IsStrTerminated(str));
    assert(alignment.x >= 0.0f && alignment.x <= 1.0f && alignment.y >= 0.0f && alignment.y <= 1.0f);

    const s_font_arrangement& arrangement = font_group.arrangements[font_index];

    t_s32 str_len = 0, line_cnt = 0;
    CalcStrLenAndLineCnt(str_len, line_cnt, str);

    const float alignment_offs_y = -(line_cnt * arrangement.line_height) * alignment.y;

    positions = PushArrayToMemArena<s_v2>(mem_arena, str_len);

    if (positions.IsEmpty()) {
        //LOG_ERROR("Failed to reserve memory for character render positions!");
        return false;
    }

    s_v2 chr_pos_pen{}; 
    t_s32 line_starting_chr_index = 0;

    for (t_s32 i = 0; i <= str_len; i++) {
        const char chr = str[i];

        if (chr == '\n' || !chr) {
            const t_s32 line_len = i - line_starting_chr_index;

            if (line_len > 0) {
                const float line_width = chr_pos_pen.x;

                for (t_s32 j = line_starting_chr_index; j < i; j++) {
                    positions[j].x -= line_width * alignment.x;
                }
            }

            if (chr == '\n') {
                chr_pos_pen.x = 0.0f;
                chr_pos_pen.y += arrangement.line_height;

                line_starting_chr_index = i + 1;
            }

            continue;
        }

        assert(isprint(static_cast<unsigned char>(chr)));

        const t_s32 chr_ascii_printable_index = chr - ASCII_PRINTABLE_MIN;

        const s_v2_s32 chr_offs = arrangement.chr_offsets[chr_ascii_printable_index];

        positions[i] = {
            pos.x + chr_pos_pen.x + chr_offs.x,
            pos.y + chr_pos_pen.y + chr_offs.y + alignment_offs_y
        };

        chr_pos_pen.x += arrangement.chr_advances[chr_ascii_printable_index];
    }

    return true;
}

bool GenStrCollider(s_rect& rect, const c_array<const char> str, const s_font_group& font_group, const t_s32 font_index, const s_v2 pos, const s_v2 alignment, c_mem_arena& temp_mem_arena) {
    //assert(IsStrTerminated(str));
    assert(alignment.x >= 0.0f && alignment.x <= 1.0f && alignment.y >= 0.0f && alignment.y <= 1.0f);

    c_array<s_v2> chr_render_positions{};

    if (!GenStrChrRenderPositions(chr_render_positions, temp_mem_arena, str, font_group, font_index, pos, alignment)) {
        //LOG_ERROR("Failed to reserve memory for character render positions!");
        return false;
    }

    s_rect_edges collider_edges{};
    bool initted = false;

    for (t_s32 i = 0; str[i]; i++) {
        const char chr = str[i];

        if (chr == '\n') {
            continue;
        }

        assert(isprint(static_cast<unsigned char>(chr)));

        const t_s32 chr_ascii_printable_index = chr - ASCII_PRINTABLE_MIN;

        const s_font_arrangement& arrangement = font_group.arrangements[font_index];
        const s_v2_s32 chr_size = arrangement.chr_sizes[chr_ascii_printable_index];

        const s_v2 chr_render_pos = chr_render_positions[i];

        const s_rect_edges chr_rect_edges{
            .left = chr_render_pos.x,
            .top = chr_render_pos.y,
            .right = chr_render_pos.x + chr_size.x,
            .bottom = chr_render_pos.y + chr_size.y
        };

        if (!initted) {
            collider_edges = chr_rect_edges;
            initted = true;
        } else {
            collider_edges.left = Min(collider_edges.left, chr_rect_edges.left);
            collider_edges.top = Min(collider_edges.top, chr_rect_edges.top);
            collider_edges.right = Max(collider_edges.right, chr_rect_edges.right);
            collider_edges.bottom = Max(collider_edges.bottom, chr_rect_edges.bottom);
        }
    }

    assert(initted && "Cannot generate the collider of a string comprised entirely of newline characters!");

    rect = {
        collider_edges.left,
        collider_edges.top,
        collider_edges.right - collider_edges.left,
        collider_edges.bottom - collider_edges.top
    };

    return true;
}

bool RenderStr(const s_rendering_context& rendering_context, const c_array<const char> str, const s_font_group& fonts, const t_s32 font_index, const s_v2 pos, const s_v2 alignment, const u_v4 color, c_mem_arena& temp_mem_arena) {
    //assert(IsStrTerminated(str));
    assert(alignment.x >= 0.0f && alignment.x <= 1.0f && alignment.y >= 0.0f && alignment.y <= 1.0f);

    c_array<s_v2> chr_render_positions;

    if (!GenStrChrRenderPositions(chr_render_positions, temp_mem_arena, str, fonts, font_index, pos, alignment)) {
        //LOG_ERROR("Failed to reserve memory for character render positions!");
        return false;
    }

    for (t_s32 i = 0; str[i]; i++) {
        const char chr = str[i];

        if (chr == ' ' || chr == '\n') {
            continue;
        }

        assert(isprint(static_cast<unsigned char>(chr)));

        const t_s32 chr_ascii_printable_index = chr - ASCII_PRINTABLE_MIN;

        const s_font_arrangement& arrangement = fonts.arrangements[font_index];
        const s_v2_s32 chr_size = arrangement.chr_sizes[chr_ascii_printable_index];

        const s_font_texture_meta& tex_meta = fonts.tex_metas[font_index];
        const t_s32 tex_chr_x = tex_meta.chr_xs[chr_ascii_printable_index];

        const s_rect_s32 chr_src_rect = {
            tex_chr_x,
            0,
            chr_size.x,
            chr_size.y
        };

        const s_rect_edges chr_tex_coords = GenTextureCoords(chr_src_rect, tex_meta.size);

        const s_batch_slot_write_info write_info{
            .tex_gl_id = fonts.tex_gl_ids[font_index],
            .tex_coords = chr_tex_coords,
            .pos = chr_render_positions[i],
            .size = {static_cast<float>(chr_src_rect.width), static_cast<float>(chr_src_rect.height)},
            .blend = color
        };

        Render(rendering_context, write_info);
    }

    return true;
}
