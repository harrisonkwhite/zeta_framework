#include "zfw_graphics.h"

#include <ctype.h>
#include <stb_truetype.h>
#include "zfw_io.h"

#define FONT_CHR_MARGIN (zfw_s_vec_2d_i){4, 4}

// TODO: Move this elsewhere. Also check ordinary textures with this before generating.
static inline zfw_s_vec_2d_i GLTextureSizeLimit() {
    GLint size;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
    return (zfw_s_vec_2d_i){size, size};
}

static void LoadFontArrangementInfo(zfw_s_font_arrangement_info* const arrangement_info, const stbtt_fontinfo* const font_info, const int height) {
    const float scale = stbtt_ScaleForPixelHeight(font_info, height);

    int vm_ascent, vm_descent, vm_line_gap;
    stbtt_GetFontVMetrics(font_info, &vm_ascent, &vm_descent, &vm_line_gap);

    arrangement_info->line_height = (vm_ascent - vm_descent + vm_line_gap) * scale;

    for (int i = 0; i < ZFW_ASCII_PRINTABLE_RANGE_LEN; i++) {
        const char chr = ZFW_ASCII_PRINTABLE_MIN + i;

        zfw_s_rect_edges_i bitmap_box;
        stbtt_GetCodepointBitmapBox(font_info, chr, scale, scale, &bitmap_box.left, &bitmap_box.top, &bitmap_box.right, &bitmap_box.bottom);

        arrangement_info->chr_offsets[i] = (zfw_s_vec_2d_i){bitmap_box.left, bitmap_box.top + (vm_ascent * scale)};
        arrangement_info->chr_sizes[i] = (zfw_s_vec_2d_i){bitmap_box.right - bitmap_box.left, bitmap_box.bottom - bitmap_box.top};

        int hm_advance;
        stbtt_GetCodepointHMetrics(font_info, chr, &hm_advance, NULL);
        arrangement_info->chr_advances[i] = hm_advance * scale;
    }
}

static void LoadFontTextureInfo(zfw_s_vec_2d_i* const tex_size, zfw_t_tex_chr_positions* const tex_chr_positions, const zfw_s_font_arrangement_info* const arrangement_info, const int tex_width_limit) {
    assert(tex_size && ZFW_IS_ZERO(*tex_size));
    assert(tex_chr_positions && ZFW_IS_ZERO(*tex_chr_positions));

    zfw_s_vec_2d_i chr_pos = {0};

    // Each character can be conceptualised as existing within its own container, and that container has margins on all sides.
    const int chr_container_height = FONT_CHR_MARGIN.y + arrangement_info->line_height + FONT_CHR_MARGIN.y;

    for (int i = 0; i < ZFW_ASCII_PRINTABLE_RANGE_LEN; i++) {
        const zfw_s_vec_2d_i chr_size = arrangement_info->chr_sizes[i];
        const int chr_container_width = FONT_CHR_MARGIN.x + chr_size.x + FONT_CHR_MARGIN.x;

        if (chr_pos.x + chr_container_width > tex_width_limit) {
            chr_pos.x = 0;
            chr_pos.y += chr_container_height;
        }

        (*tex_chr_positions)[i] = ZFW_Vec2DISum(chr_pos, FONT_CHR_MARGIN);

        tex_size->x = ZFW_MAX(chr_pos.x + chr_container_width, tex_size->x);
        tex_size->y = ZFW_MAX(chr_pos.y + chr_container_height, tex_size->y);

        chr_pos.x += chr_container_width;
    }
}

static zfw_t_gl_id GenFontTexture(const stbtt_fontinfo* const font_info, const int font_height, const zfw_s_vec_2d_i tex_size, const zfw_t_tex_chr_positions* const tex_chr_positions, const zfw_s_font_arrangement_info* const font_arrangement_info, zfw_s_mem_arena* const temp_mem_arena) {
    const float scale = stbtt_ScaleForPixelHeight(font_info, font_height);

    const size_t font_tex_rgba_px_data_size = ZFW_RGBA_CHANNEL_CNT * tex_size.x * tex_size.y;
    zfw_t_byte* const font_tex_rgba_px_data = ZFW_MEM_ARENA_PUSH_TYPE_MANY(temp_mem_arena, zfw_t_byte, font_tex_rgba_px_data_size);

    if (!font_tex_rgba_px_data) {
        return 0;
    }

    for (int i = 0; i < font_tex_rgba_px_data_size; i += 4) {
        font_tex_rgba_px_data[i + 0] = 255;
        font_tex_rgba_px_data[i + 1] = 255;
        font_tex_rgba_px_data[i + 2] = 255;
        font_tex_rgba_px_data[i + 3] = 0;
    }

    for (int i = 0; i < ZFW_ASCII_PRINTABLE_RANGE_LEN; i++) {
        const char chr = ZFW_ASCII_PRINTABLE_MIN + i;

        if (chr == ' ') {
            continue;
        }

        zfw_t_byte* const bitmap = stbtt_GetCodepointBitmap(font_info, scale, scale, chr, NULL, NULL, NULL, NULL);

        if (!bitmap) {
            return 0;
        }

        const zfw_s_rect_i src_rect = {
            (*tex_chr_positions)[i].x,
            (*tex_chr_positions)[i].y,
            font_arrangement_info->chr_sizes[i].x,
            font_arrangement_info->chr_sizes[i].y
        };

        for (int yo = 0; yo < src_rect.height; yo++) {
            for (int xo = 0; xo < src_rect.width; xo++) {
                const zfw_s_vec_2d_i px_pos = {src_rect.x + xo, src_rect.y + yo};
                const int px_index = ((px_pos.y * tex_size.x) + px_pos.x) * ZFW_RGBA_CHANNEL_CNT;

                const int bitmap_index = ZFW_IndexFrom2D((zfw_s_vec_2d_i){xo, yo}, src_rect.width);
                font_tex_rgba_px_data[px_index + 3] = bitmap[bitmap_index];
            }
        }

        stbtt_FreeBitmap(bitmap, NULL);
    }

    return ZFW_GenTexture(tex_size, font_tex_rgba_px_data);
}

zfw_s_fonts ZFW_LoadFontsFromFiles(zfw_s_mem_arena* const mem_arena, const int font_cnt, const t_font_index_to_load_info font_index_to_load_info, zfw_s_mem_arena* const temp_mem_arena) {
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

    zfw_s_vec_2d_i* const tex_sizes = ZFW_MEM_ARENA_PUSH_TYPE_MANY(mem_arena, zfw_s_vec_2d_i, font_cnt);

    if (!tex_sizes) {
        return (zfw_s_fonts){0};
    }

    zfw_t_tex_chr_positions* const tex_chr_positions = ZFW_MEM_ARENA_PUSH_TYPE_MANY(mem_arena, zfw_t_tex_chr_positions, font_cnt);

    if (!tex_chr_positions) {
        return (zfw_s_fonts){0};
    }

    bool err = false;

    for (int i = 0; i < font_cnt; i++) {
        const zfw_s_font_load_info load_info = font_index_to_load_info(i);
        assert(load_info.height > 0);
        assert(load_info.file_path);

        const zfw_t_byte* const font_file_data = (const zfw_t_byte*)ZFW_PushEntireFileContents(load_info.file_path, temp_mem_arena, false);

        if (!font_file_data) {
            err = true;
            break;
        }

        stbtt_fontinfo font_info;

        const int offs = stbtt_GetFontOffsetForIndex(font_file_data, 0);

        if (offs == -1) {
            ZFW_LogError("Failed to get font offset for font \"%s\"!", load_info.file_path);
            err = true;
            break;
        }

        if (!stbtt_InitFont(&font_info, font_file_data, offs)) {
            ZFW_LogError("Failed to initialise font \"%s\"!", load_info.file_path);
            err = true;
            break;
        }

        LoadFontArrangementInfo(&arrangement_infos[i], &font_info, load_info.height);

        const zfw_s_vec_2d_i font_tex_size_limit = GLTextureSizeLimit();

        LoadFontTextureInfo(&tex_sizes[i], &tex_chr_positions[i], &arrangement_infos[i], font_tex_size_limit.x);

        if (tex_sizes[i].y > font_tex_size_limit.y) {
            ZFW_LogError("Prospective font texture size is too large!");
            err = true;
            break;
        }

        tex_gl_ids[i] = GenFontTexture(&font_info, load_info.height, tex_sizes[i], &tex_chr_positions[i], &arrangement_infos[i], temp_mem_arena);
    }

    if (err) {
        glDeleteTextures(font_cnt, tex_gl_ids);
        return (zfw_s_fonts){0};
    }

    return (zfw_s_fonts){
        .arrangement_infos = arrangement_infos,
        .tex_gl_ids = tex_gl_ids,
        .tex_sizes = tex_sizes,
        .tex_chr_positions = tex_chr_positions,
        .cnt = font_cnt
    };
}

void ZFW_UnloadFonts(zfw_s_fonts* const fonts) {
    assert(fonts && ZFW_IsFontsValid(fonts));
    glDeleteTextures(fonts->cnt, fonts->tex_gl_ids);
    ZFW_ZERO_OUT(*fonts);
}

static zfw_s_vec_2d* PushStrChrRenderPositions(zfw_s_mem_arena* const mem_arena, const char* const str, const int font_index, const zfw_s_fonts* const fonts, const zfw_s_vec_2d pos) {
    assert(font_index >= 0 && font_index < fonts->cnt);

    const int str_len = strlen(str);
    zfw_s_vec_2d* const chr_render_positions = ZFW_MEM_ARENA_PUSH_TYPE_MANY(mem_arena, zfw_s_vec_2d, str_len);

    if (!chr_render_positions) {
        return NULL;
    }

    const zfw_s_font_arrangement_info* const font_arrangement_info = &fonts->arrangement_infos[font_index];

    zfw_s_vec_2d chr_pos = {0};

    for (int i = 0; i < str_len; i++) {
        const char chr = str[i];

        if (chr == '\n') {
            // Move to the next line.
            chr_pos.x = 0.0f;
            chr_pos.y += font_arrangement_info->line_height;

            continue;
        }

        assert(isprint(chr));

        const int chr_index = chr - ZFW_ASCII_PRINTABLE_MIN;

        chr_render_positions[i] = (zfw_s_vec_2d){
            pos.x + chr_pos.x + font_arrangement_info->chr_offsets[chr_index].x,
            pos.y + chr_pos.y + font_arrangement_info->chr_offsets[chr_index].y
        };

        chr_pos.x += font_arrangement_info->chr_advances[chr_index];
    }

    return chr_render_positions;
}

bool ZFW_LoadStrCollider(zfw_s_rect* const rect, const char* const str, const int font_index, const zfw_s_fonts* const fonts, const zfw_s_vec_2d pos, const zfw_e_str_hor_align hor_align, const zfw_e_str_ver_align ver_align, zfw_s_mem_arena* const temp_mem_arena) {
    assert(ZFW_IS_ZERO(*rect));

    assert(str[0]);

    const zfw_s_vec_2d* const chr_render_positions = PushStrChrRenderPositions(temp_mem_arena, str, font_index, fonts, pos);

    if (!chr_render_positions) {
        return false;
    }

    zfw_s_rect_edges collider_edges;
    bool initted = false;

    for (int i = 0; str[i]; i++) {
        const char chr = str[i];

        if (chr == '\n') {
            continue;
        }

        assert(isprint(chr));

        const int chr_index = chr - ZFW_ASCII_PRINTABLE_MIN;

        const zfw_s_vec_2d_i chr_size = fonts->arrangement_infos[font_index].chr_sizes[chr_index];

        const zfw_s_rect_edges chr_rect_edges = {
            .left = chr_render_positions[i].x,
            .top = chr_render_positions[i].y,
            .right = chr_render_positions[i].x + chr_size.x,
            .bottom = chr_render_positions[i].y + chr_size.y
        };

        if (!initted) {
            collider_edges = chr_rect_edges;
            initted = true;
        } else {
            collider_edges.left = ZFW_MIN(collider_edges.left, chr_rect_edges.left);
            collider_edges.top = ZFW_MIN(collider_edges.top, chr_rect_edges.top);
            collider_edges.right = ZFW_MAX(collider_edges.right, chr_rect_edges.right);
            collider_edges.bottom = ZFW_MAX(collider_edges.bottom, chr_rect_edges.bottom);
        }
    }

    assert(initted && "Cannot generate the collider of a string comprised entirely of '\n'!");

    *rect = (zfw_s_rect){
        .x = collider_edges.left,
        .y = collider_edges.top,
        .width = collider_edges.right - collider_edges.left,
        .height = collider_edges.bottom - collider_edges.top
    };

    return true;
}

bool ZFW_RenderStr(const zfw_s_rendering_context* const context, const char* const str, const int font_index, const zfw_s_fonts* const fonts, const zfw_s_vec_2d pos, const zfw_e_str_hor_align hor_align, const zfw_e_str_ver_align ver_align, const zfw_s_vec_4d blend, zfw_s_mem_arena* const temp_mem_arena) {
    assert(context && ZFW_IsRenderingContextValid(context));
    assert(str && str[0]);
    assert(font_index >= 0 && font_index < fonts->cnt);
    assert(fonts && ZFW_IsFontsValid(fonts));
    assert(ZFW_IsColorValid(blend));
    assert(temp_mem_arena && ZFW_IsMemArenaValid(temp_mem_arena));

    const zfw_s_vec_2d* const chr_render_positions = PushStrChrRenderPositions(temp_mem_arena, str, font_index, fonts, pos);

    if (!chr_render_positions) {
        return false;
    }

    for (int i = 0; str[i]; i++) {
        const char chr = str[i];

        if (chr == ' ' || chr == '\n') {
            continue;
        }

        assert(isprint(chr));

        const int chr_index = chr - ZFW_ASCII_PRINTABLE_MIN;

        const zfw_s_rect_i chr_src_rect = {
            .x = fonts->tex_chr_positions[font_index][chr_index].x,
            .y = fonts->tex_chr_positions[font_index][chr_index].y,
            .width = fonts->arrangement_infos[font_index].chr_sizes[chr_index].x,
            .height = fonts->arrangement_infos[font_index].chr_sizes[chr_index].y
        };

        const zfw_s_rect_edges chr_tex_coords = ZFW_TextureCoords(chr_src_rect, fonts->tex_sizes[font_index]);

        const zfw_s_batch_slot_write_info write_info = {
            .tex_gl_id = fonts->tex_gl_ids[font_index],
            .tex_coords = chr_tex_coords,
            .pos = chr_render_positions[i],
            .size = {chr_src_rect.width, chr_src_rect.height},
            .blend = blend
        };

        ZFW_Render(context, &write_info);
    }

    return true;
}
