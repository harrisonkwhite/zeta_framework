#include "zfwc_graphics.h"

#include <ctype.h>

static bool LoadFontFromFile(s_font_arrangement* const arrangement, s_font_texture_meta* const tex_meta, s_u8_array* const tex_rgba_px_data, s_mem_arena* const tex_rgba_px_data_mem_arena, const s_char_array_view file_path) {
    assert(IS_ZERO(*arrangement));
    assert(IS_ZERO(*tex_meta));
    assert(IS_ZERO(*tex_rgba_px_data));

    FILE* const fs = fopen(file_path.buf_raw, "rb");

    if (!fs) {
        return false;
    }

    if (fread(arrangement, sizeof(*arrangement), 1, fs) < 1) {
        fclose(fs);
        return false;
    }

    // TODO: Check the validity of the arrangement!

    if (fread(tex_meta, sizeof(*tex_meta), 1, fs) < 1) {
        fclose(fs);
        return false;
    }

    // TODO: Check the validity of the texture metadata!

    *tex_rgba_px_data = PushU8ArrayToMemArena(tex_rgba_px_data_mem_arena, 4 * tex_meta->size.x * tex_meta->size.y);

    if (IS_ZERO(*tex_rgba_px_data)) {
        fclose(fs);
        return false;
    }

    if (fread(tex_rgba_px_data->buf_raw, 1, tex_rgba_px_data->len, fs) < tex_rgba_px_data->len) {
        fclose(fs);
        return false;
    }

    fclose(fs);

    return true;
}

s_font_group GenFontGroupFromFiles(const s_char_array_view_array_view file_paths, s_mem_arena *const mem_arena, s_gl_resource_arena* const gl_res_arena, s_mem_arena* const temp_mem_arena) {
    const t_s32 font_cnt = file_paths.len;

    const s_font_arrangement_array arrangements = PushFontArrangementArrayToMemArena(mem_arena, font_cnt);

    if (IS_ZERO(arrangements)) {
        LOG_ERROR("Failed to reserve memory for font arrangements!");
        return (s_font_group){0};
    }

    const s_font_texture_meta_array tex_metas = PushFontTextureMetaArrayToMemArena(mem_arena, font_cnt);

    if (IS_ZERO(tex_metas)) {
        LOG_ERROR("Failed to reserve memory for font texture metadata!");
        return (s_font_group){0};
    }

    const s_gl_id_array tex_gl_ids = PushToGLResourceArena(gl_res_arena, font_cnt, ek_gl_resource_type_texture);

    if (IS_ZERO(tex_gl_ids)) {
        LOG_ERROR("Failed to reserve OpenGL texture IDs for fonts!");
        return (s_font_group){0};
    }

    for (t_s32 i = 0; i < font_cnt; i++) {
        const s_char_array_view file_path = *CharArrayViewElemView(file_paths, i);

        s_font_arrangement* const arrangement = FontArrangementElem(arrangements, i);
        s_font_texture_meta* const tex_meta = FontTextureMetaElem(tex_metas, i);

        s_u8_array tex_rgba_px_data = {0};

        if (!LoadFontFromFile(arrangement, tex_meta, &tex_rgba_px_data, temp_mem_arena, file_path)) {
            return (s_font_group){0};
        }

        t_gl_id* const tex_gl_id = GLIDElem(tex_gl_ids, i);

        *tex_gl_id = GenGLTextureFromRGBA((s_rgba_texture){.px_data = tex_rgba_px_data, .tex_size = tex_meta->size});

        if (!*tex_gl_id) {
            return (s_font_group){0};
        }
    }

    return (s_font_group){
        .arrangements = FontArrangementArrayView(arrangements),
        .tex_metas = FontTextureMetaArrayView(tex_metas),
        .tex_gl_ids = GLIDArrayView(tex_gl_ids)
    };
}

static t_s32 CalcStrLineCnt(const s_char_array_view str) {
    t_s32 line_cnt = 1;

    for (t_s32 i = 0; i < str.len; i++) {
        if (*CharElemView(str, i) == '\n') {
            line_cnt++;
        }
    }

    return line_cnt;
}

s_v2_array CalcStrChrRenderPositions(s_mem_arena* const mem_arena, const s_char_array_view str, const s_font_group* const font_group, const t_s32 font_index, const s_v2 pos, const s_v2 alignment) {
    const s_font_arrangement* const arrangement = FontArrangementElemView(font_group->arrangements, font_index);

    const t_s32 str_line_cnt = CalcStrLineCnt(str);

    // From just the string line count we can determine the vertical alignment offset to apply to all characters.
    const float alignment_offs_y = -(str_line_cnt * arrangement->line_height) * alignment.y;

    // Reserve memory for the character render positions.
    const s_v2_array chr_render_positions = PushV2ArrayToMemArena(mem_arena, str_line_cnt);

    if (IS_ZERO(chr_render_positions)) {
        LOG_ERROR("Failed to reserve memory for character render positions!");
        return (s_v2_array){0};
    }

    // Calculate the render position for each character.
    s_v2 chr_pos_pen = {0}; // The position of the current character.
    t_s32 line_starting_chr_index = 0; // The index of the first character in the current line.

    for (t_s32 i = 0; i < str.len; i++) {
        const char chr = *CharElemView(str, i);

        if (chr == '\n' || !chr) {
            // Apply horizontal alignment offset to all the characters of the line we just finished, only if the line was not empty.
            const t_s32 line_len = i - line_starting_chr_index;

            if (line_len > 0) {
                const float line_width = chr_pos_pen.x;

                for (t_s32 j = line_starting_chr_index; j < i; j++) {
                    V2Elem(chr_render_positions, j)->x -= line_width * alignment.x;
                }
            }

            // If '\n', move to the next line.
            if (chr == '\n') {
                chr_pos_pen.x = 0.0f;
                chr_pos_pen.y += arrangement->line_height;

                line_starting_chr_index = i + 1;
            }

            continue;
        }

        assert(isprint(chr));

        const t_s32 chr_ascii_printable_index = chr - ASCII_PRINTABLE_MIN;

        const s_v2_s32 chr_offs = *STATIC_ARRAY_ELEM(arrangement->chr_offsets, chr_ascii_printable_index);

        *V2Elem(chr_render_positions, i) = (s_v2){
            pos.x + chr_pos_pen.x + chr_offs.x,
            pos.y + chr_pos_pen.y + chr_offs.y + alignment_offs_y
        };

        chr_pos_pen.x += *STATIC_ARRAY_ELEM(arrangement->chr_advances, chr_ascii_printable_index);
    }

    return chr_render_positions;
}

bool CalcStrCollider(s_rect* const rect, const s_char_array_view str, const s_font_group* const font_group, const t_s32 font_index, const s_v2 pos, const s_v2 alignment, s_mem_arena* const temp_mem_arena) {
    assert(rect && IS_ZERO(*rect));

    const s_v2_array chr_render_positions = CalcStrChrRenderPositions(temp_mem_arena, str, font_group, font_index, pos, alignment);

    if (IS_ZERO(chr_render_positions)) {
        LOG_ERROR("Failed to reserve memory for character render positions!");
        return false;
    }

    s_rect_edges collider_edges;
    bool initted = false;

    for (t_s32 i = 0; i < str.len - 1; i++) {
        const char chr = *CharElemView(str, i);

        if (chr == '\n') {
            continue;
        }

        assert(isprint(chr));

        const t_s32 chr_ascii_printable_index = chr - ASCII_PRINTABLE_MIN;

        const s_font_arrangement* const arrangement = FontArrangementElemView(font_group->arrangements, font_index);
        const s_v2_s32 chr_size = *STATIC_ARRAY_ELEM(arrangement->chr_sizes, chr_ascii_printable_index);

        const s_v2 chr_render_pos = *V2Elem(chr_render_positions, i);

        const s_rect_edges chr_rect_edges = {
            .left = chr_render_pos.x,
            .top = chr_render_pos.y,
            .right = chr_render_pos.x + chr_size.x,
            .bottom = chr_render_pos.y + chr_size.y
        };

        if (!initted) {
            collider_edges = chr_rect_edges;
            initted = true;
        } else {
            collider_edges.left = MIN(collider_edges.left, chr_rect_edges.left);
            collider_edges.top = MIN(collider_edges.top, chr_rect_edges.top);
            collider_edges.right = MAX(collider_edges.right, chr_rect_edges.right);
            collider_edges.bottom = MAX(collider_edges.bottom, chr_rect_edges.bottom);
        }
    }

    assert(initted && "Cannot generate the collider of a string comprised entirely of newline characters!");

    *rect = (s_rect){
        .x = collider_edges.left,
        .y = collider_edges.top,
        .width = collider_edges.right - collider_edges.left,
        .height = collider_edges.bottom - collider_edges.top
    };

    return true;
}

bool RenderStr(const s_rendering_context* const rendering_context, const s_char_array_view str, const s_font_group* const fonts, const t_s32 font_index, const s_v2 pos, const s_v2 alignment, const u_v4 color, s_mem_arena* const temp_mem_arena) {
    const s_v2_array chr_render_positions = CalcStrChrRenderPositions(temp_mem_arena, str, fonts, font_index, pos, alignment);

    if (IS_ZERO(chr_render_positions)) {
        LOG_ERROR("Failed to reserve memory for character render positions!");
        return false;
    }

    for (t_s32 i = 0; i < str.len; i++) {
        const char chr = *CharElemView(str, i);

        if (chr == ' ' || chr == '\n') {
            continue;
        }

        assert(isprint(chr));

        const t_s32 chr_ascii_printable_index = chr - ASCII_PRINTABLE_MIN;

        const s_font_arrangement* const arrangement = FontArrangementElemView(fonts->arrangements, font_index);
        const s_v2_s32 chr_size = *STATIC_ARRAY_ELEM(arrangement->chr_sizes, chr_ascii_printable_index);

        const s_font_texture_meta* const tex_meta = FontTextureMetaElemView(fonts->tex_metas, font_index);
        const t_s32 tex_chr_x = *STATIC_ARRAY_ELEM(tex_meta->chr_xs, chr_ascii_printable_index);

        const s_rect_s32 chr_src_rect = {
            .x = tex_chr_x,
            .y = 0,
            .width = chr_size.x,
            .height = chr_size.y
        };

        const s_rect_edges chr_tex_coords = GenTextureCoords(chr_src_rect, tex_meta->size);

        const s_batch_slot_write_info write_info = {
            .tex_gl_id = *GLIDElemView(fonts->tex_gl_ids, font_index),
            .tex_coords = chr_tex_coords,
            .pos = *V2Elem(chr_render_positions, i),
            .size = {chr_src_rect.width, chr_src_rect.height},
            .blend = color
        };

        Render(rendering_context, &write_info);
    }

    return true;
}
