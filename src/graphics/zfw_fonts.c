#include "zfw_graphics.h"

#include <ctype.h>
#include <stb_truetype.h>

#define FONT_TEX_CHR_MARGIN (zfw_s_vec_2d_s32){4, 4}

static void LoadFontArrangementInfo(zfw_s_font_arrangement_info* const arrangement_info, const stbtt_fontinfo* const stb_font_info, const int height) {
    assert(arrangement_info);
    assert(stb_font_info);
    assert(height > 0);

    const float scale = stbtt_ScaleForPixelHeight(stb_font_info, height);

    int vm_ascent, vm_descent, vm_line_gap;
    stbtt_GetFontVMetrics(stb_font_info, &vm_ascent, &vm_descent, &vm_line_gap);

    arrangement_info->line_height = (vm_ascent - vm_descent + vm_line_gap) * scale;

    for (int i = 0; i < ZFW_ASCII_PRINTABLE_RANGE_LEN; i++) {
        const char chr = ZFW_ASCII_PRINTABLE_MIN + i;

        zfw_s_rect_edges_s32 bitmap_box;
        stbtt_GetCodepointBitmapBox(stb_font_info, chr, scale, scale, &bitmap_box.left, &bitmap_box.top, &bitmap_box.right, &bitmap_box.bottom);

        arrangement_info->chr_offsets[i] = (zfw_s_vec_2d_s32){bitmap_box.left, bitmap_box.top + (vm_ascent * scale)};
        arrangement_info->chr_sizes[i] = (zfw_s_vec_2d_s32){bitmap_box.right - bitmap_box.left, bitmap_box.bottom - bitmap_box.top};

        int hm_advance;
        stbtt_GetCodepointHMetrics(stb_font_info, chr, &hm_advance, NULL);
        arrangement_info->chr_advances[i] = hm_advance * scale;
    }
}

static void LoadFontTextureInfo(zfw_s_vec_2d_s32* const tex_size, zfw_t_tex_chr_positions* const tex_chr_positions, const zfw_s_font_arrangement_info* const arrangement_info, const int tex_width_limit) {
    assert(tex_size && IS_ZERO(*tex_size));
    assert(tex_chr_positions && IS_ZERO(*tex_chr_positions));
    assert(arrangement_info);
    assert(tex_width_limit > 0);

    zfw_s_vec_2d_s32 chr_pos = {0};

    // Each character can be conceptualised as existing within its own container, and that container has margins on all sides.
    const int chr_container_height = FONT_TEX_CHR_MARGIN.y + arrangement_info->line_height + FONT_TEX_CHR_MARGIN.y;

    for (int i = 0; i < ZFW_ASCII_PRINTABLE_RANGE_LEN; i++) {
        const zfw_s_vec_2d_s32 chr_size = arrangement_info->chr_sizes[i];
        const int chr_container_width = FONT_TEX_CHR_MARGIN.x + chr_size.x + FONT_TEX_CHR_MARGIN.x;

        if (chr_pos.x + chr_container_width > tex_width_limit) {
            chr_pos.x = 0;
            chr_pos.y += chr_container_height;
        }

        (*tex_chr_positions)[i] = (zfw_s_vec_2d_s32){
            chr_pos.x + FONT_TEX_CHR_MARGIN.x,
            chr_pos.y + FONT_TEX_CHR_MARGIN.y
        };

        tex_size->x = ZFW_MAX(chr_pos.x + chr_container_width, tex_size->x);
        tex_size->y = ZFW_MAX(chr_pos.y + chr_container_height, tex_size->y);

        chr_pos.x += chr_container_width;
    }
}

static zfw_t_gl_id GenFontTexture(const stbtt_fontinfo* const stb_font_info, const int font_height, const zfw_s_vec_2d_s32 tex_size, const zfw_t_tex_chr_positions* const tex_chr_positions, const zfw_s_font_arrangement_info* const font_arrangement_info, s_mem_arena* const temp_mem_arena) {
    assert(stb_font_info);
    assert(font_height > 0);
    assert(tex_size.x > 0 && tex_size.y > 0);
    assert(tex_chr_positions);
    assert(font_arrangement_info);
    assert(temp_mem_arena && IsMemArenaValid(temp_mem_arena));

    const float scale = stbtt_ScaleForPixelHeight(stb_font_info, font_height);

    const size_t font_tex_rgba_px_data_size = 4 * tex_size.x * tex_size.y;
    t_u8* const font_tex_rgba_px_data = MEM_ARENA_PUSH_TYPE_CNT(temp_mem_arena, t_u8, font_tex_rgba_px_data_size);

    if (!font_tex_rgba_px_data) {
        LOG_ERROR("Failed to reserve memory for font texture RGBA pixel data!");
        return 0;
    }

    // Clear the pixel data to transparent white.
    for (int i = 0; i < font_tex_rgba_px_data_size; i += 4) {
        font_tex_rgba_px_data[i + 0] = 255;
        font_tex_rgba_px_data[i + 1] = 255;
        font_tex_rgba_px_data[i + 2] = 255;
        font_tex_rgba_px_data[i + 3] = 0;
    }

    // Write the pixel data of each character.
    for (int i = 0; i < ZFW_ASCII_PRINTABLE_RANGE_LEN; i++) {
        const char chr = ZFW_ASCII_PRINTABLE_MIN + i;

        if (chr == ' ') {
            // No bitmap for the space character.
            continue;
        }

        t_u8* const bitmap = stbtt_GetCodepointBitmap(stb_font_info, scale, scale, chr, NULL, NULL, NULL, NULL);

        if (!bitmap) {
            LOG_ERROR("Failed to get bitmap for character '%c' through STB!", chr);
            return 0;
        }

        const zfw_s_rect_s32 src_rect = {
            (*tex_chr_positions)[i].x,
            (*tex_chr_positions)[i].y,
            font_arrangement_info->chr_sizes[i].x,
            font_arrangement_info->chr_sizes[i].y
        };

        for (int yo = 0; yo < src_rect.height; yo++) {
            for (int xo = 0; xo < src_rect.width; xo++) {
                const zfw_s_vec_2d_s32 px_pos = {src_rect.x + xo, src_rect.y + yo};
                const int px_index = ((px_pos.y * tex_size.x) + px_pos.x) * ZFW_RGBA_CHANNEL_CNT;

                const int bitmap_index = IndexFrom2D(xo, yo, src_rect.width);
                font_tex_rgba_px_data[px_index + 3] = bitmap[bitmap_index];
            }
        }

        stbtt_FreeBitmap(bitmap, NULL);
    }

    return ZFW_GenTexture(tex_size, font_tex_rgba_px_data);
}

// TODO: Move this elsewhere. Also check ordinary textures with this before generating.
static inline zfw_s_vec_2d_s32 GLTextureSizeLimit() {
    GLint size;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
    return (zfw_s_vec_2d_s32){size, size};
}

zfw_s_fonts ZFW_LoadFontsFromFiles(s_mem_arena* const mem_arena, const int font_cnt, const t_font_index_to_load_info font_index_to_load_info, s_mem_arena* const temp_mem_arena) {
    assert(mem_arena && IsMemArenaValid(mem_arena));
    assert(font_cnt > 0);
    assert(font_index_to_load_info);
    assert(temp_mem_arena && IsMemArenaValid(temp_mem_arena));

    // Reserve memory for font data.
    zfw_s_font_arrangement_info* const arrangement_infos = MEM_ARENA_PUSH_TYPE_CNT(mem_arena, zfw_s_font_arrangement_info, font_cnt);

    if (!arrangement_infos) {
        LOG_ERROR("Failed to reserve memory for font arrangement information!");
        return (zfw_s_fonts){0};
    }

    zfw_t_gl_id* const tex_gl_ids = MEM_ARENA_PUSH_TYPE_CNT(mem_arena, zfw_t_gl_id, font_cnt);

    if (!tex_gl_ids) {
        LOG_ERROR("Failed to reserve memory for font texture OpenGL IDs!");
        return (zfw_s_fonts){0};
    }

    zfw_s_vec_2d_s32* const tex_sizes = MEM_ARENA_PUSH_TYPE_CNT(mem_arena, zfw_s_vec_2d_s32, font_cnt);

    if (!tex_sizes) {
        LOG_ERROR("Failed to reserve memory for font texture sizes!");
        return (zfw_s_fonts){0};
    }

    zfw_t_tex_chr_positions* const tex_chr_positions = MEM_ARENA_PUSH_TYPE_CNT(mem_arena, zfw_t_tex_chr_positions, font_cnt);

    if (!tex_chr_positions) {
        LOG_ERROR("Failed to reserve memory for font texture character positions!");
        return (zfw_s_fonts){0};
    }

    // Load each font.
    for (int i = 0; i < font_cnt; i++) {
        const zfw_s_font_load_info load_info = font_index_to_load_info(i);
        assert(load_info.file_path);
        assert(load_info.height > 0);

        const t_u8* const font_file_data = PushEntireFileContents(load_info.file_path, temp_mem_arena, false);

        if (!font_file_data) {
            LOG_ERROR("Failed to reserve memory for font file contents!");
            goto error;
        }

        stbtt_fontinfo stb_font_info;

        const int offs = stbtt_GetFontOffsetForIndex(font_file_data, 0);

        if (offs == -1) {
            LOG_ERROR("Failed to get font offset!");
            goto error;
        }

        if (!stbtt_InitFont(&stb_font_info, font_file_data, offs)) {
            LOG_ERROR("Failed to initialise font through STB!");
            goto error;
        }

        LoadFontArrangementInfo(&arrangement_infos[i], &stb_font_info, load_info.height);

        const zfw_s_vec_2d_s32 font_tex_size_limit = GLTextureSizeLimit();

        LoadFontTextureInfo(&tex_sizes[i], &tex_chr_positions[i], &arrangement_infos[i], font_tex_size_limit.x);

        if (tex_sizes[i].y > font_tex_size_limit.y) {
            LOG_ERROR("Prospective font texture size is too large!");
            goto error;
        }

        tex_gl_ids[i] = GenFontTexture(&stb_font_info, load_info.height, tex_sizes[i], &tex_chr_positions[i], &arrangement_infos[i], temp_mem_arena);

        if (!tex_gl_ids[i]) {
            LOG_ERROR("Failed to generate font texture!");
            goto error;
        }

        continue;

error:
        LOG_ERROR("Failed to load font \"%s\" with height %d!", load_info.file_path, load_info.height);

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
    ZERO_OUT(*fonts);
}

typedef struct {
    int len;
    int line_cnt;
} s_str_len_and_line_cnt;

static s_str_len_and_line_cnt StrLenAndLineCnt(const char* const str) {
    int len = 0;
    int line_cnt = 1;

    while (str[len]) {
        if (str[len] == '\n') {
            line_cnt++;
        }

        len++;
    }

    return (s_str_len_and_line_cnt){
        .len = len,
        .line_cnt = line_cnt
    };
}

static zfw_s_vec_2d* PushStrChrRenderPositions(s_mem_arena* const mem_arena, const char* const str, const int font_index, const zfw_s_fonts* const fonts, const zfw_s_vec_2d pos, const zfw_s_vec_2d alignment) {
    assert(mem_arena && IsMemArenaValid(mem_arena));
    assert(str && str[0]);
    assert(font_index >= 0 && font_index < fonts->cnt);
    assert(fonts && ZFW_IsFontsValid(fonts));
    assert(ZFW_IsStrAlignmentValid(alignment));

    const zfw_s_font_arrangement_info* const arrangement_info = &fonts->arrangement_infos[font_index];

    const s_str_len_and_line_cnt str_len_and_line_cnt = StrLenAndLineCnt(str);

    // From just the string line count we can determine the vertical alignment offset to apply to all characters.
    const float alignment_offs_y = -(str_len_and_line_cnt.line_cnt * arrangement_info->line_height) * alignment.y;

    // Reserve memory for the character render positions.
    zfw_s_vec_2d* const chr_render_positions = MEM_ARENA_PUSH_TYPE_CNT(mem_arena, zfw_s_vec_2d, str_len_and_line_cnt.len);

    if (!chr_render_positions) {
        LOG_ERROR("Failed to reserve memory for character render positions!");
        return NULL;
    }

    // Calculate the render position for each character.
    zfw_s_vec_2d chr_pos_pen = {0}; // The position of the current character.
    int line_starting_chr_index = 0; // The index of the first character in the current line.

    for (int i = 0; i <= str_len_and_line_cnt.len; i++) {
        const char chr = str[i];

        if (chr == '\n' || !chr) {
            // Apply horizontal alignment offset to all the characters of the line we just finished, only if the line was not empty.
            const int line_len = i - line_starting_chr_index;

            if (line_len > 0) {
                const float line_width = chr_pos_pen.x;

                for (int j = line_starting_chr_index; j < i; j++) {
                    chr_render_positions[j].x -= line_width * alignment.x;
                }
            }

            // If '\n', move to the next line.
            if (chr == '\n') {
                chr_pos_pen.x = 0.0f;
                chr_pos_pen.y += arrangement_info->line_height;

                line_starting_chr_index = i + 1;
            }

            continue;
        }

        assert(isprint(chr));

        const int chr_ascii_printable_index = chr - ZFW_ASCII_PRINTABLE_MIN;

        chr_render_positions[i] = (zfw_s_vec_2d){
            pos.x + chr_pos_pen.x + arrangement_info->chr_offsets[chr_ascii_printable_index].x,
            pos.y + chr_pos_pen.y + arrangement_info->chr_offsets[chr_ascii_printable_index].y + alignment_offs_y
        };

        chr_pos_pen.x += arrangement_info->chr_advances[chr_ascii_printable_index];
    }

    return chr_render_positions;
}

bool ZFW_LoadStrCollider(zfw_s_rect* const rect, const char* const str, const int font_index, const zfw_s_fonts* const fonts, const zfw_s_vec_2d pos, const zfw_s_vec_2d alignment, s_mem_arena* const temp_mem_arena) {
    assert(rect && IS_ZERO(*rect));
    assert(str && str[0]);
    assert(font_index >= 0 && font_index < fonts->cnt);
    assert(ZFW_IsStrAlignmentValid(alignment));
    assert(temp_mem_arena && IsMemArenaValid(temp_mem_arena));

    const zfw_s_vec_2d* const chr_render_positions = PushStrChrRenderPositions(temp_mem_arena, str, font_index, fonts, pos, alignment);

    if (!chr_render_positions) {
        LOG_ERROR("Failed to reserve memory for character render positions!");
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

        const int chr_ascii_printable_index = chr - ZFW_ASCII_PRINTABLE_MIN;

        const zfw_s_vec_2d_s32 chr_size = fonts->arrangement_infos[font_index].chr_sizes[chr_ascii_printable_index];

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

bool ZFW_RenderStr(const zfw_s_rendering_context* const context, const char* const str, const int font_index, const zfw_s_fonts* const fonts, const zfw_s_vec_2d pos, const zfw_s_vec_2d alignment, const zfw_u_vec_4d blend, s_mem_arena* const temp_mem_arena) {
    assert(context && ZFW_IsRenderingContextValid(context));
    assert(str && str[0]);
    assert(font_index >= 0 && font_index < fonts->cnt);
    assert(fonts && ZFW_IsFontsValid(fonts));
    assert(ZFW_IsStrAlignmentValid(alignment));
    assert(ZFW_IsColorValid(blend));
    assert(temp_mem_arena && IsMemArenaValid(temp_mem_arena));

    const zfw_s_vec_2d* const chr_render_positions = PushStrChrRenderPositions(temp_mem_arena, str, font_index, fonts, pos, alignment);

    if (!chr_render_positions) {
        LOG_ERROR("Failed to reserve memory for character render positions!");
        return false;
    }

    for (int i = 0; str[i]; i++) {
        const char chr = str[i];

        if (chr == ' ' || chr == '\n') {
            continue;
        }

        assert(isprint(chr));

        const int chr_ascii_printable_index = chr - ZFW_ASCII_PRINTABLE_MIN;

        const zfw_s_rect_s32 chr_src_rect = {
            .x = fonts->tex_chr_positions[font_index][chr_ascii_printable_index].x,
            .y = fonts->tex_chr_positions[font_index][chr_ascii_printable_index].y,
            .width = fonts->arrangement_infos[font_index].chr_sizes[chr_ascii_printable_index].x,
            .height = fonts->arrangement_infos[font_index].chr_sizes[chr_ascii_printable_index].y
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
