#include "zfw_graphics.h"

#include <stb_truetype.h>
#include "zfw_io.h"
#include "zfw_mem.h"

static void ApplyHorAlignOffsToLine(zfw_s_vec_2d* const line_chr_positions, const int cnt, const zfw_e_str_hor_align hor_align, const float line_end_x) {
    assert(cnt > 0);

    const float line_width = line_end_x - line_chr_positions[0].x;
    const float align_offs = -(line_width * (float)hor_align * 0.5f);

    for (int i = 0; i < cnt; ++i) {
        line_chr_positions[i].x += align_offs;
    }
} 

zfw_s_fonts ZFW_LoadFontsFromFiles(zfw_s_mem_arena* const mem_arena, const int font_cnt, const t_font_index_to_load_info font_index_to_load_info, zfw_s_mem_arena* const temp_mem_arena) {
    // TODO: Clean up this catastrophe. GL textures are leaking in the case of error. Function is too big and incomprehensible.

    assert(mem_arena && ZFW_IsMemArenaValid(mem_arena));
    assert(font_cnt > 0);
    assert(font_index_to_load_info);
    assert(temp_mem_arena && ZFW_IsMemArenaValid(temp_mem_arena));

    zfw_s_font_arrangement_info* const arrangement_infos = ZFW_MEM_ARENA_PUSH_TYPE_MANY(mem_arena, zfw_s_font_arrangement_info, font_cnt);

    if (!arrangement_infos) {
        return (zfw_s_fonts){0};
    }

    zfw_t_gl_id* const tex_gl_ids = ZFW_MEM_ARENA_PUSH_TYPE_MANY(mem_arena, zfw_t_gl_id, font_cnt);

    if (!tex_gl_ids) {
        return (zfw_s_fonts){0};
    }

    int* const tex_heights = ZFW_MEM_ARENA_PUSH_TYPE_MANY(mem_arena, int, font_cnt);

    if (!tex_heights) {
        return (zfw_s_fonts){0};
    }

    zfw_t_byte* const rgba_px_data_scratch_space = ZFW_MEM_ARENA_PUSH_TYPE_MANY(temp_mem_arena, zfw_t_byte, ZFW_TEXTURE_CHANNEL_CNT * ZFW_FONT_TEXTURE_WIDTH * ZFW_FONT_TEXTURE_HEIGHT_LIMIT);

    if (!rgba_px_data_scratch_space) {
        return (zfw_s_fonts){0};
    }

    glGenTextures(font_cnt, tex_gl_ids);

    for (int i = 0; i < font_cnt; ++i) {
        const zfw_s_font_load_info load_info = font_index_to_load_info(i);

        assert(load_info.height > 0);
        assert(load_info.file_path);

        const zfw_t_byte* const font_file_data = (const zfw_t_byte*)ZFW_PushEntireFileContents(load_info.file_path, temp_mem_arena, false);

        if (!font_file_data) {
            return (zfw_s_fonts){0};
        }

        stbtt_fontinfo font_info;

        const int offs = stbtt_GetFontOffsetForIndex(font_file_data, 0);

        if (offs == -1) {
            ZFW_LogError("Failed to get font offset for font \"%s\"!", load_info.file_path);
            return (zfw_s_fonts){0};
        }

        if (!stbtt_InitFont(&font_info, font_file_data, offs)) {
            ZFW_LogError("Failed to initialise font \"%s\"!", load_info.file_path);
            return (zfw_s_fonts){0};
        }

        const float scale = stbtt_ScaleForPixelHeight(&font_info, load_info.height);

        int ascent, descent, line_gap;
        stbtt_GetFontVMetrics(&font_info, &ascent, &descent, &line_gap);

        arrangement_infos[i].line_height = (ascent - descent + line_gap) * scale;

        for (int y = 0; y < ZFW_FONT_TEXTURE_HEIGHT_LIMIT; ++y) {
            for (int x = 0; x < ZFW_FONT_TEXTURE_WIDTH; ++x) {
                const int px_index = ((y * ZFW_FONT_TEXTURE_WIDTH) + x) * ZFW_TEXTURE_CHANNEL_CNT;

                rgba_px_data_scratch_space[px_index + 0] = 255;
                rgba_px_data_scratch_space[px_index + 1] = 255;
                rgba_px_data_scratch_space[px_index + 2] = 255;
                rgba_px_data_scratch_space[px_index + 3] = 0;
            }
        }

        zfw_s_vec_2d_i chr_render_pos = {0, 0};

        for (int j = 0; j < ZFW_FONT_CHR_RANGE_LEN; ++j) {
            const int chr = ZFW_FONT_CHR_RANGE_BEGIN + j;

            int advance;
            stbtt_GetCodepointHMetrics(&font_info, chr, &advance, NULL);

            arrangement_infos[i].chr_hor_advances[j] = (int)(advance * scale);

            if (chr == ' ') {
                continue;
            }

            zfw_s_vec_2d_i bitmap_size, bitmap_offs;
            zfw_t_byte* const bitmap = stbtt_GetCodepointBitmap(&font_info, 0.0f, scale, chr, &bitmap_size.x, &bitmap_size.y, &bitmap_offs.x, &bitmap_offs.y);

            if (!bitmap) {
                return (zfw_s_fonts){0};
            }

            arrangement_infos[i].chr_hor_offsets[j] = bitmap_offs.x;
            arrangement_infos[i].chr_ver_offsets[j] = bitmap_offs.y + (int)(ascent * scale);

            if (chr_render_pos.x + bitmap_size.x > ZFW_FONT_TEXTURE_WIDTH) {
                chr_render_pos.x = 0;
                chr_render_pos.y += arrangement_infos[i].line_height;
            }

            const int chr_tex_height = chr_render_pos.y + bitmap_size.y;
            if (chr_tex_height > ZFW_FONT_TEXTURE_HEIGHT_LIMIT) {
                stbtt_FreeBitmap(bitmap, NULL);
                return (zfw_s_fonts){0};
            }

            tex_heights[i] = ZFW_MAX(tex_heights[i], chr_tex_height);

            arrangement_infos[i].chr_src_rects[j] = (zfw_s_rect_i){
                .x = chr_render_pos.x,
                .y = chr_render_pos.y,
                .width = bitmap_size.x,
                .height = bitmap_size.y
            };

            for (int y = 0; y < bitmap_size.y; ++y) {
                for (int x = 0; x < bitmap_size.x; ++x) {
                    const int px = chr_render_pos.x + x;
                    const int py = chr_render_pos.y + y;
                    const int px_index = (py * ZFW_FONT_TEXTURE_WIDTH + px) * ZFW_TEXTURE_CHANNEL_CNT;
                    const int bitmap_index = y * bitmap_size.x + x;

                    rgba_px_data_scratch_space[px_index + 3] = bitmap[bitmap_index];
                }
            }

            chr_render_pos.x += bitmap_size.x;

            stbtt_FreeBitmap(bitmap, NULL);
        }

        ZFW_SetUpTexture(tex_gl_ids[i], (zfw_s_vec_2d_i){ZFW_FONT_TEXTURE_WIDTH, tex_heights[i]}, rgba_px_data_scratch_space);
    }

    return (zfw_s_fonts){
        .arrangement_infos = arrangement_infos,
        .tex_gl_ids = tex_gl_ids,
        .tex_heights = tex_heights,
        .cnt = font_cnt
    };
}

void ZFW_UnloadFonts(zfw_s_fonts* const fonts) {
    assert(fonts && ZFW_IsFontsValid(fonts));
    glDeleteTextures(fonts->cnt, fonts->tex_gl_ids);
    ZFW_ZERO_OUT(*fonts);
}

const zfw_s_vec_2d* ZFW_PushStrChrPositions(const char* const str, zfw_s_mem_arena* const mem_arena, const int font_index, const zfw_s_fonts* const fonts, const zfw_s_vec_2d pos, const zfw_e_str_hor_align hor_align, const zfw_e_str_ver_align ver_align) {
    assert(font_index >= 0 && font_index < fonts->cnt);

    const int str_len = (int)strlen(str);
    assert(str_len > 0);

    zfw_s_vec_2d* const chr_positions = ZFW_MEM_ARENA_PUSH_TYPE_MANY(mem_arena, zfw_s_vec_2d, str_len);

    if (!chr_positions) {
        return NULL;
    }

    const zfw_s_font_arrangement_info* const font_ai = &fonts->arrangement_infos[font_index];

    int cur_line_begin_chr_index = 0;
    zfw_s_vec_2d chr_base_pos_pen = {0};

    for (int i = 0; i < str_len; ++i) {
        const char chr = str[i];

        if (chr == '\0') {
            continue;
        }

        if (chr == '\n') {
            const int line_cnt = i - cur_line_begin_chr_index;
            ApplyHorAlignOffsToLine(
                &chr_positions[cur_line_begin_chr_index],
                line_cnt,
                hor_align,
                chr_base_pos_pen.x + pos.x
            );

            cur_line_begin_chr_index = i + 1;
            chr_base_pos_pen.x = 0.0f;
            chr_base_pos_pen.y += (float)font_ai->line_height;
            continue;
        }

        const int chr_index = chr - ZFW_FONT_CHR_RANGE_BEGIN;

        chr_positions[i].x = chr_base_pos_pen.x + pos.x + (float)font_ai->chr_hor_offsets[chr_index];
        chr_positions[i].y = chr_base_pos_pen.y + pos.y + (float)font_ai->chr_ver_offsets[chr_index];

        chr_base_pos_pen.x += font_ai->chr_hor_advances[chr_index];
    }

    const int remaining_cnt = str_len - cur_line_begin_chr_index;

    ApplyHorAlignOffsToLine(
        &chr_positions[cur_line_begin_chr_index],
        remaining_cnt,
        hor_align,
        chr_base_pos_pen.x + pos.x
    );

    const float total_height = chr_base_pos_pen.y + (float)font_ai->line_height;
    const float ver_align_offs = -(total_height * (float)ver_align * 0.5f);

    for (int i = 0; i < str_len; ++i) {
        chr_positions[i].y += ver_align_offs;
    }

    return chr_positions;
}

bool ZFW_LoadStrCollider(zfw_s_rect* const rect, const char* const str, const int font_index, const zfw_s_fonts* const fonts, const zfw_s_vec_2d pos, const zfw_e_str_hor_align hor_align, const zfw_e_str_ver_align ver_align, zfw_s_mem_arena* const temp_mem_arena) {
    assert(ZFW_IS_ZERO(*rect));

    const int str_len = strlen(str);
    assert(str_len > 0);

    const zfw_s_vec_2d* const chr_positions = ZFW_PushStrChrPositions(str, temp_mem_arena, font_index, fonts, pos, hor_align, ver_align);

    if (!chr_positions) {
        return false;
    }

    zfw_s_rect_edges collider_edges;
    bool initted = false;

    for (int i = 0; i < str_len; ++i) {
        const char chr = str[i];

        if (chr == '\n') {
            continue;
        }

        const int chr_index = (int)chr - ZFW_FONT_CHR_RANGE_BEGIN;
        const zfw_s_vec_2d_i size = ZFW_RectISize(fonts->arrangement_infos[font_index].chr_src_rects[chr_index]);

        const float left = chr_positions[i].x;
        const float top = chr_positions[i].y;
        const float right = left + (float)size.x;
        const float bottom = top + (float)size.y;

        if (!initted) {
            collider_edges.left = left;
            collider_edges.top = top;
            collider_edges.right = right;
            collider_edges.bottom = bottom;

            initted = true;
        } else {
            collider_edges.left = ZFW_MIN(collider_edges.left, left);
            collider_edges.top = ZFW_MIN(collider_edges.top, top);
            collider_edges.right = ZFW_MAX(collider_edges.right, right);
            collider_edges.bottom = ZFW_MAX(collider_edges.bottom, bottom);
        }
    }

    assert(initted);

    const float width = collider_edges.right - collider_edges.left;
    const float height = collider_edges.bottom - collider_edges.top;

    *rect = (zfw_s_rect){
        .x = collider_edges.left,
        .y = collider_edges.top,
        .width = width,
        .height = height
    };

    return true;
}

bool ZFW_RenderStr(const zfw_s_rendering_context* const context, const char* const str, const int font_index, const zfw_s_fonts* const fonts, const zfw_s_vec_2d pos, const zfw_e_str_hor_align hor_align, const zfw_e_str_ver_align ver_align, const zfw_s_vec_4d blend, zfw_s_mem_arena* const temp_mem_arena) {
    assert(context && ZFW_IsRenderingContextValid(context));
    assert(str);
    assert(font_index >= 0 && font_index < fonts->cnt);
    assert(fonts && ZFW_IsFontsValid(fonts));
    assert(ZFW_IsColorValid(blend));
    assert(temp_mem_arena && ZFW_IsMemArenaValid(temp_mem_arena));

    if (!str[0]) {
        return true;
    }

    const int str_len = strlen(str);
    const zfw_s_vec_2d* const str_chr_positions = ZFW_PushStrChrPositions(str, temp_mem_arena, font_index, fonts, pos, hor_align, ver_align);

    if (!str_chr_positions) {
        return false;
    }

    const zfw_t_gl_id font_tex_gl_id = fonts->tex_gl_ids[font_index];

    const zfw_s_vec_2d_i font_tex_size = {
        .x = ZFW_FONT_TEXTURE_WIDTH,
        .y = fonts->tex_heights[font_index]
    };

    for (int i = 0; i < str_len; ++i) {
        const char c = str[i];

        if (c == '\0' || c == ' ') {
            continue;
        }

        const int chr_index = c - ZFW_FONT_CHR_RANGE_BEGIN;
        const zfw_s_rect_i chr_src_rect = fonts->arrangement_infos[font_index].chr_src_rects[chr_index];
        const zfw_s_rect_edges chr_tex_coords = ZFW_TextureCoords(chr_src_rect, font_tex_size);

        const zfw_s_batch_slot_write_info write_info = {
            .tex_gl_id = font_tex_gl_id,
            .tex_coords = chr_tex_coords,
            .pos = str_chr_positions[i],
            .size = {chr_src_rect.width, chr_src_rect.height},
            .blend = blend
        };

        ZFW_Render(context, &write_info);
    }

    return true;
}
