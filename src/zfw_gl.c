#include "zfw_gl.h"

#include <ctype.h>
#include <stb_image.h>
#include <stb_truetype.h>

#define FONT_TEX_CHR_MARGIN (zfw_s_vec_2d_int){4, 4}

zfw_s_gl_resource_arena ZFW_GenGLResourceArena(s_mem_arena* const mem_arena, const int res_limit) {
    assert(mem_arena && IsMemArenaValid(mem_arena));
    assert(res_limit > 0);

    zfw_t_gl_id* const gl_ids = MEM_ARENA_PUSH_TYPE_CNT(mem_arena, zfw_t_gl_id, res_limit);

    if (!gl_ids) {
        LOG_ERROR("Failed to reserve memory for OpenGL resource IDs!");
        return (zfw_s_gl_resource_arena){0};
    }

    zfw_e_gl_resource_type* const res_types = MEM_ARENA_PUSH_TYPE_CNT(mem_arena, zfw_e_gl_resource_type, res_limit);

    if (!res_types) {
        LOG_ERROR("Failed to reserve memory for OpenGL resource types!");
        return (zfw_s_gl_resource_arena){0};
    }

    return (zfw_s_gl_resource_arena){
        .ids = gl_ids,
        .res_types = res_types,
        .res_limit = res_limit
    };
}

void ZFW_CleanGLResourceArena(zfw_s_gl_resource_arena* const res_arena) {
    ZFW_AssertGLResourceArenaValidity(res_arena);

    for (int i = 0; i < res_arena->res_used; i++) {
        const zfw_t_gl_id gl_id = res_arena->ids[i];

        if (!gl_id) {
            continue;
        }

        switch ((zfw_e_gl_resource_type)i) {
            case zfw_ek_gl_resource_type_texture:
                glDeleteTextures(1, &gl_id);
                break;

            case zfw_ek_gl_resource_type_shader_prog:
                glDeleteProgram(gl_id);
                break;

            case zfw_ek_gl_resource_type_vert_array:
                glDeleteVertexArrays(1, &gl_id);
                break;

            case zfw_ek_gl_resource_type_vert_buf:
            case zfw_ek_gl_resource_type_elem_buf:
                glDeleteBuffers(1, &gl_id);
                break;

            case zfw_ek_gl_resource_type_framebuffer:
                glDeleteFramebuffers(1, &gl_id);
                break;
        }
    }
}

zfw_t_gl_id* ZFW_ReserveGLIDs(zfw_s_gl_resource_arena* const res_arena, const int cnt, const zfw_e_gl_resource_type res_type) {
    ZFW_AssertGLResourceArenaValidity(res_arena);
    assert(cnt > 0);
    assert(res_type < zfw_eks_gl_resource_type_cnt);

    if (res_arena->res_used + cnt > res_arena->res_limit) {
        LOG_ERROR("OpenGL resource arena is full! Cannot reserve %d more IDs!", cnt);
        return NULL;
    }

    const int res_used_prev = res_arena->res_used;
    res_arena->res_used += cnt;
    return res_arena->ids + res_used_prev;
}

static inline zfw_s_vec_2d_int GLTextureSizeLimit() {
    int size;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
    return (zfw_s_vec_2d_int){size, size};
}

static zfw_t_gl_id GenGLTextureFromRGBAPixelData(const t_byte* const rgba_px_data, const zfw_s_vec_2d_int tex_size) {
    assert(rgba_px_data);
    assert(tex_size.x > 0 && tex_size.y > 0);

    const zfw_s_vec_2d_int tex_size_limit = GLTextureSizeLimit();

    if (tex_size.x > tex_size_limit.x || tex_size.y > tex_size_limit.y) {
        LOG_ERROR("Texture size (%d, %d) exceeds OpenGL limits (%d, %d)!", tex_size.x, tex_size.y, tex_size_limit.x, tex_size_limit.y);
        return 0;
    }

    zfw_t_gl_id tex_gl_id;
    glGenTextures(1, &tex_gl_id);

    glBindTexture(GL_TEXTURE_2D, tex_gl_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_size.x, tex_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_px_data);

    return tex_gl_id;
}

zfw_s_texture_info ZFW_GenTextureInfoFromFile(const char* const file_path, s_mem_arena* const mem_arena) {
    assert(file_path);
    assert(mem_arena && IsMemArenaValid(mem_arena));

    zfw_s_vec_2d_int tex_size = {0};
    unsigned char* const rgba_px_data = stbi_load(file_path, &tex_size.x, &tex_size.y, NULL, 4);

    if (!rgba_px_data) {
        LOG_ERROR("Failed to load pixel data from file \"%s\"!", file_path);
        LOG_ERROR_SPECIAL("STB", "%s", stbi_failure_reason());
        return (zfw_s_texture_info){0};
    }

    return (zfw_s_texture_info){
        .rgba_px_data = rgba_px_data,
        .tex_size = tex_size
    };
}

zfw_s_texture_group ZFW_GenTextures(const int tex_cnt, const zfw_t_gen_texture_info_func gen_tex_info_func, zfw_s_gl_resource_arena* const gl_res_arena, s_mem_arena* const mem_arena, s_mem_arena* const temp_mem_arena) {
    assert(tex_cnt > 0);
    assert(gen_tex_info_func);
    ZFW_AssertGLResourceArenaValidity(gl_res_arena);
    assert(mem_arena && IsMemArenaValid(mem_arena));
    assert(temp_mem_arena && IsMemArenaValid(temp_mem_arena));

    zfw_t_gl_id* const gl_ids = ZFW_ReserveGLIDs(gl_res_arena, tex_cnt, zfw_ek_gl_resource_type_texture);

    if (!gl_ids) {
        LOG_ERROR("Failed to reserve OpenGL texture IDs!");
        return (zfw_s_texture_group){0};
    }

    zfw_s_vec_2d_int* const sizes = MEM_ARENA_PUSH_TYPE_CNT(mem_arena, zfw_s_vec_2d_int, tex_cnt);

    if (!sizes) {
        LOG_ERROR("Failed to reserve memory for texture sizes!");
        return (zfw_s_texture_group){0};
    }

    for (int i = 0; i < tex_cnt; i++) {
        const zfw_s_texture_info tex_info = gen_tex_info_func(i, temp_mem_arena);

        if (IS_ZERO(tex_info)) {
            LOG_ERROR("Failed to generate texture information for texture with index %d!", i);
            return (zfw_s_texture_group){0};
        }

        gl_ids[i] = GenGLTextureFromRGBAPixelData(tex_info.rgba_px_data, tex_info.tex_size);

        if (!gl_ids[i]) {
            LOG_ERROR("Failed to generate OpenGL texture from RGBA pixel data for texture with index %d!", i);
            return (zfw_s_texture_group){0};
        }

        sizes[i] = tex_info.tex_size;
    }

    return (zfw_s_texture_group){
        .gl_ids = gl_ids,
        .sizes = sizes,
        .cnt = tex_cnt
    };
}

zfw_s_rect_edges ZFW_TextureCoords(const zfw_s_rect_int src_rect, const zfw_s_vec_2d_int tex_size) {
    assert(ZFW_IsSrcRectValid(src_rect, tex_size));
    assert(tex_size.x > 0 && tex_size.y > 0);

    return (zfw_s_rect_edges){
        .left = (float)src_rect.x / tex_size.x,
        .top = (float)src_rect.y / tex_size.y,
        .right = (float)(src_rect.x + src_rect.width) / tex_size.x,
        .bottom = (float)(src_rect.y + src_rect.height) / tex_size.y
    };
}

static void LoadFontArrangementInfo(zfw_s_font_arrangement_info* const arrangement_info, const stbtt_fontinfo* const stb_font_info, const int height) {
    assert(arrangement_info && IS_ZERO(*arrangement_info));
    assert(stb_font_info);
    assert(height > 0);

    const float scale = stbtt_ScaleForPixelHeight(stb_font_info, height);

    int vm_ascent, vm_descent, vm_line_gap;
    stbtt_GetFontVMetrics(stb_font_info, &vm_ascent, &vm_descent, &vm_line_gap);

    arrangement_info->line_height = (vm_ascent - vm_descent + vm_line_gap) * scale;

    for (int i = 0; i < ZFW_ASCII_PRINTABLE_RANGE_LEN; i++) {
        const char chr = ZFW_ASCII_PRINTABLE_MIN + i;

        zfw_s_rect_edges_int bitmap_box;
        stbtt_GetCodepointBitmapBox(stb_font_info, chr, scale, scale, &bitmap_box.left, &bitmap_box.top, &bitmap_box.right, &bitmap_box.bottom);

        arrangement_info->chr_offsets[i] = (zfw_s_vec_2d_int){bitmap_box.left, bitmap_box.top + (vm_ascent * scale)};
        arrangement_info->chr_sizes[i] = (zfw_s_vec_2d_int){bitmap_box.right - bitmap_box.left, bitmap_box.bottom - bitmap_box.top};

        int hm_advance;
        stbtt_GetCodepointHMetrics(stb_font_info, chr, &hm_advance, NULL);
        arrangement_info->chr_advances[i] = hm_advance * scale;
    }
}

static void LoadFontTextureInfo(zfw_s_vec_2d_int* const tex_size, zfw_t_tex_chr_positions* const tex_chr_positions, const zfw_s_font_arrangement_info* const arrangement_info, const int tex_width_limit) {
    assert(tex_size && IS_ZERO(*tex_size));
    assert(tex_chr_positions && IS_ZERO(*tex_chr_positions));
    ZFW_AssertFontArrangementInfoValidity(arrangement_info);
    assert(tex_width_limit > 0);

    zfw_s_vec_2d_int chr_pos = {0};

    // Each character can be conceptualised as existing within its own container, and that container has margins on all sides.
    const int chr_container_height = FONT_TEX_CHR_MARGIN.y + arrangement_info->line_height + FONT_TEX_CHR_MARGIN.y;

    for (int i = 0; i < ZFW_ASCII_PRINTABLE_RANGE_LEN; i++) {
        const zfw_s_vec_2d_int chr_size = arrangement_info->chr_sizes[i];
        const int chr_container_width = FONT_TEX_CHR_MARGIN.x + chr_size.x + FONT_TEX_CHR_MARGIN.x;

        if (chr_pos.x + chr_container_width > tex_width_limit) {
            chr_pos.x = 0;
            chr_pos.y += chr_container_height;
        }

        (*tex_chr_positions)[i] = (zfw_s_vec_2d_int){
            chr_pos.x + FONT_TEX_CHR_MARGIN.x,
            chr_pos.y + FONT_TEX_CHR_MARGIN.y
        };

        tex_size->x = MAX(chr_pos.x + chr_container_width, tex_size->x);
        tex_size->y = MAX(chr_pos.y + chr_container_height, tex_size->y);

        chr_pos.x += chr_container_width;
    }
}

static zfw_t_gl_id GenFontTexture(const stbtt_fontinfo* const stb_font_info, const int font_height, const zfw_s_vec_2d_int tex_size, const zfw_t_tex_chr_positions* const tex_chr_positions, const zfw_s_font_arrangement_info* const font_arrangement_info, s_mem_arena* const temp_mem_arena) {
    assert(stb_font_info);
    assert(font_height > 0);
    assert(tex_size.x > 0 && tex_size.y > 0);
    assert(tex_chr_positions);
    ZFW_AssertFontArrangementInfoValidity(font_arrangement_info);
    assert(temp_mem_arena && IsMemArenaValid(temp_mem_arena));

    const float scale = stbtt_ScaleForPixelHeight(stb_font_info, font_height);

    const size_t font_tex_rgba_px_data_size = 4 * tex_size.x * tex_size.y;
    t_byte* const font_tex_rgba_px_data = MEM_ARENA_PUSH_TYPE_CNT(temp_mem_arena, t_byte, font_tex_rgba_px_data_size);

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

        t_byte* const bitmap = stbtt_GetCodepointBitmap(stb_font_info, scale, scale, chr, NULL, NULL, NULL, NULL);

        if (!bitmap) {
            LOG_ERROR("Failed to get bitmap for character '%c' through STB!", chr);
            return 0;
        }

        const zfw_s_rect_int src_rect = {
            (*tex_chr_positions)[i].x,
            (*tex_chr_positions)[i].y,
            font_arrangement_info->chr_sizes[i].x,
            font_arrangement_info->chr_sizes[i].y
        };

        for (int yo = 0; yo < src_rect.height; yo++) {
            for (int xo = 0; xo < src_rect.width; xo++) {
                const zfw_s_vec_2d_int px_pos = {src_rect.x + xo, src_rect.y + yo};
                const int px_index = ((px_pos.y * tex_size.x) + px_pos.x) * 4;

                const int bitmap_index = IndexFrom2D(xo, yo, src_rect.width);
                font_tex_rgba_px_data[px_index + 3] = bitmap[bitmap_index];
            }
        }

        stbtt_FreeBitmap(bitmap, NULL);
    }

    return GenGLTextureFromRGBAPixelData(font_tex_rgba_px_data, tex_size);
}

zfw_s_font_group ZFW_GenFonts(const int font_cnt, const zfw_s_font_info* const font_infos, zfw_s_gl_resource_arena* const gl_res_arena, s_mem_arena* const mem_arena, s_mem_arena* const temp_mem_arena) {
    assert(font_cnt > 0);
    assert(font_infos);
    ZFW_AssertGLResourceArenaValidity(gl_res_arena);
    assert(mem_arena && IsMemArenaValid(mem_arena));
    assert(temp_mem_arena && IsMemArenaValid(temp_mem_arena));

    // Reserve memory for font data.
    zfw_s_font_arrangement_info* const arrangement_infos = MEM_ARENA_PUSH_TYPE_CNT(mem_arena, zfw_s_font_arrangement_info, font_cnt);

    if (!arrangement_infos) {
        LOG_ERROR("Failed to reserve memory for font arrangement information!");
        return (zfw_s_font_group){0};
    }

    zfw_t_gl_id* const tex_gl_ids = MEM_ARENA_PUSH_TYPE_CNT(mem_arena, zfw_t_gl_id, font_cnt);

    if (!tex_gl_ids) {
        LOG_ERROR("Failed to reserve memory for font texture OpenGL IDs!");
        return (zfw_s_font_group){0};
    }

    zfw_s_vec_2d_int* const tex_sizes = MEM_ARENA_PUSH_TYPE_CNT(mem_arena, zfw_s_vec_2d_int, font_cnt);

    if (!tex_sizes) {
        LOG_ERROR("Failed to reserve memory for font texture sizes!");
        return (zfw_s_font_group){0};
    }

    zfw_t_tex_chr_positions* const tex_chr_positions = MEM_ARENA_PUSH_TYPE_CNT(mem_arena, zfw_t_tex_chr_positions, font_cnt);

    if (!tex_chr_positions) {
        LOG_ERROR("Failed to reserve memory for font texture character positions!");
        return (zfw_s_font_group){0};
    }

    // Load each font.
    for (int i = 0; i < font_cnt; i++) {
        const zfw_s_font_info* const info = &font_infos[i];
        ZFW_AssertFontInfoValidity(info);

        const t_byte* const font_file_data = PushEntireFileContents(info->file_path, temp_mem_arena, false);

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

        LoadFontArrangementInfo(&arrangement_infos[i], &stb_font_info, info->height);

        const zfw_s_vec_2d_int font_tex_size_limit = GLTextureSizeLimit();

        LoadFontTextureInfo(&tex_sizes[i], &tex_chr_positions[i], &arrangement_infos[i], font_tex_size_limit.x);

        if (tex_sizes[i].y > font_tex_size_limit.y) {
            LOG_ERROR("Prospective font texture size is too large!");
            goto error;
        }

        tex_gl_ids[i] = GenFontTexture(&stb_font_info, info->height, tex_sizes[i], &tex_chr_positions[i], &arrangement_infos[i], temp_mem_arena);

        if (!tex_gl_ids[i]) {
            LOG_ERROR("Failed to generate font texture!");
            goto error;
        }

        continue;

error:
        LOG_ERROR("Failed to load font \"%s\" with height %d!", info->file_path, info->height);

        glDeleteTextures(font_cnt, tex_gl_ids);

        return (zfw_s_font_group){0};
    }

    return (zfw_s_font_group){
        .arrangement_infos = arrangement_infos,
        .tex_gl_ids = tex_gl_ids,
        .tex_sizes = tex_sizes,
        .tex_chr_positions = tex_chr_positions,
        .cnt = font_cnt
    };
}

typedef struct {
    int len;
    int line_cnt;
} s_str_len_and_line_cnt;

static s_str_len_and_line_cnt StrLenAndLineCnt(const char* const str) {
    assert(str);

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

zfw_s_vec_2d* ZFW_PushStrChrRenderPositions(s_mem_arena* const mem_arena, const char* const str, const zfw_s_font_group* const fonts, const int font_index, const zfw_s_vec_2d pos, const zfw_s_vec_2d alignment) {
    assert(mem_arena && IsMemArenaValid(mem_arena));
    assert(str && str[0]);
    ZFW_AssertFontGroupValidity(fonts);
    assert(font_index >= 0 && font_index < fonts->cnt);
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

bool ZFW_LoadStrCollider(zfw_s_rect* const rect, const char* const str, const zfw_s_font_group* const fonts, const int font_index, const zfw_s_vec_2d pos, const zfw_s_vec_2d alignment, s_mem_arena* const temp_mem_arena) {
    assert(rect && IS_ZERO(*rect));
    assert(str && str[0]);
    ZFW_AssertFontGroupValidity(fonts);
    assert(font_index >= 0 && font_index < fonts->cnt);
    assert(ZFW_IsStrAlignmentValid(alignment));
    assert(temp_mem_arena && IsMemArenaValid(temp_mem_arena));

    const zfw_s_vec_2d* const chr_render_positions = ZFW_PushStrChrRenderPositions(temp_mem_arena, str, fonts, font_index, pos, alignment);

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

        const zfw_s_vec_2d_int chr_size = fonts->arrangement_infos[font_index].chr_sizes[chr_ascii_printable_index];

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
            collider_edges.left = MIN(collider_edges.left, chr_rect_edges.left);
            collider_edges.top = MIN(collider_edges.top, chr_rect_edges.top);
            collider_edges.right = MAX(collider_edges.right, chr_rect_edges.right);
            collider_edges.bottom = MAX(collider_edges.bottom, chr_rect_edges.bottom);
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

static zfw_t_gl_id GenShaderFromSrc(const char* const src, const bool frag, s_mem_arena* const temp_mem_arena) {
    assert(src);
    assert(temp_mem_arena && IsMemArenaValid(temp_mem_arena));

    const GLenum shader_type = frag ? GL_FRAGMENT_SHADER : GL_VERTEX_SHADER;
    const zfw_t_gl_id shader_gl_id = glCreateShader(shader_type);
    glShaderSource(shader_gl_id, 1, &src, NULL);
    glCompileShader(shader_gl_id);

    GLint success;
    glGetShaderiv(shader_gl_id, GL_COMPILE_STATUS, &success);

    if (!success) {
        // Try getting the OpenGL compile error message.
        GLint log_len = 0;
        glGetShaderiv(shader_gl_id, GL_INFO_LOG_LENGTH, &log_len);

        if (log_len > 0) {
            char* const log = MEM_ARENA_PUSH_TYPE_CNT(temp_mem_arena, char, log_len);

            if (log) {
                glGetShaderInfoLog(shader_gl_id, log_len, NULL, log);
                LOG_ERROR_SPECIAL("OpenGL Shader Compilation", "%s", log);
            }
        }

        glDeleteShader(shader_gl_id);

        return 0;
    }

    return shader_gl_id;
}

static zfw_t_gl_id GenShaderProg(const zfw_s_shader_prog_gen_info* const gen_info, s_mem_arena* const temp_mem_arena) {
    ZFW_AssertShaderProgGenInfoValidity(gen_info);
    assert(temp_mem_arena && IsMemArenaValid(temp_mem_arena));

    // Determine the shader sources.
    const char *vs_src, *fs_src;

    if (gen_info->is_srcs) {
        vs_src = gen_info->vs_src;
        fs_src = gen_info->fs_src;
    } else {
        vs_src = (const char*)PushEntireFileContents(gen_info->vs_file_path, temp_mem_arena, true);

        if (!vs_src) {
            LOG_ERROR("Failed to get contents of vertex shader source file \"%s\"!", gen_info->vs_file_path);
            return 0;
        }

        fs_src = (const char*)PushEntireFileContents(gen_info->fs_file_path, temp_mem_arena, true);

        if (!fs_src) {
            LOG_ERROR("Failed to get contents of fragment shader source file \"%s\"!", gen_info->fs_file_path);
            return 0;
        }
    }

    // Generate the shaders from the sources.
    const zfw_t_gl_id vs_gl_id = GenShaderFromSrc(vs_src, false, temp_mem_arena);

    if (!vs_gl_id) {
        if (gen_info->is_srcs) {
            LOG_ERROR("Failed to generate vertex shader from source!");
        } else {
            LOG_ERROR("Failed to generate vertex shader from source file \"%s\"!", gen_info->vs_file_path);
        }

        return 0;
    }

    const zfw_t_gl_id fs_gl_id = GenShaderFromSrc(fs_src, true, temp_mem_arena);

    if (!fs_gl_id) {
        if (gen_info->is_srcs) {
            LOG_ERROR("Failed to generate fragment shader from source!");
        } else {
            LOG_ERROR("Failed to generate fragment shader from source file \"%s\"!", gen_info->fs_file_path);
        }

        glDeleteShader(vs_gl_id);

        return 0;
    }

    // Set up the shader program.
    const zfw_t_gl_id prog_gl_id = glCreateProgram();
    glAttachShader(prog_gl_id, vs_gl_id);
    glAttachShader(prog_gl_id, fs_gl_id);
    glLinkProgram(prog_gl_id);

    // Dispose the shaders, they're no longer needed.
    glDeleteShader(fs_gl_id);
    glDeleteShader(vs_gl_id);

    return prog_gl_id;
}

zfw_s_shader_prog_group ZFW_GenShaderProgs(const int prog_cnt, const zfw_s_shader_prog_gen_info* const prog_gen_infos, zfw_s_gl_resource_arena* const gl_res_arena, s_mem_arena* const temp_mem_arena) {
    assert(prog_cnt > 0);
    assert(prog_gen_infos);
    ZFW_AssertGLResourceArenaValidity(gl_res_arena);
    assert(temp_mem_arena && IsMemArenaValid(temp_mem_arena));

    zfw_t_gl_id* const gl_ids = ZFW_ReserveGLIDs(gl_res_arena, prog_cnt, zfw_ek_gl_resource_type_shader_prog);

    if (!gl_ids) {
        LOG_ERROR("Failed to reserve OpenGL shader program IDs for built-in shader programs!");
        return (zfw_s_shader_prog_group){0};
    }

    for (int i = 0; i < prog_cnt; i++) {
        const zfw_s_shader_prog_gen_info* const gen_info = &prog_gen_infos[i];
        ZFW_AssertShaderProgGenInfoValidity(gen_info);

        gl_ids[i] = GenShaderProg(gen_info, temp_mem_arena);

        if (!gl_ids[i]) {
            LOG_ERROR("Failed to generate shader program with index %d!", i);
            return (zfw_s_shader_prog_group){0};
        }
    }

    return (zfw_s_shader_prog_group){
        .gl_ids = gl_ids,
        .cnt = prog_cnt
    };
}

static size_t Stride(const int* const vert_attr_lens, const int vert_attr_cnt) {
    assert(vert_attr_lens);
    assert(vert_attr_cnt > 0);

    int stride = 0;

    for (int i = 0; i < vert_attr_cnt; i++) {
        assert(vert_attr_lens[i] > 0);
        stride += sizeof(float) * vert_attr_lens[i];
    }

    return stride;
}

void ZFW_GenRenderable(zfw_t_gl_id* const va_gl_id, zfw_t_gl_id* const vb_gl_id, zfw_t_gl_id* const eb_gl_id, const float* const vert_buf, const size_t vert_buf_size, const unsigned short* const elem_buf, const size_t elem_buf_size, const int* const vert_attr_lens, const int vert_attr_cnt) {
    assert(va_gl_id && !(*va_gl_id));
    assert(vb_gl_id && !(*vb_gl_id));
    assert(eb_gl_id && !(*eb_gl_id));
    assert(vert_buf_size > 0);
    assert(elem_buf && elem_buf_size > 0);
    assert(vert_attr_lens && vert_attr_cnt > 0);

    glGenVertexArrays(1, va_gl_id);
    glBindVertexArray(*va_gl_id);

    glGenBuffers(1, vb_gl_id);
    glBindBuffer(GL_ARRAY_BUFFER, *vb_gl_id);
    glBufferData(GL_ARRAY_BUFFER, vert_buf_size, vert_buf, GL_DYNAMIC_DRAW);

    glGenBuffers(1, eb_gl_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *eb_gl_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, elem_buf_size, elem_buf, GL_STATIC_DRAW);

    const GLsizei stride = Stride(vert_attr_lens, vert_attr_cnt);
    int offs = 0;

    for (int i = 0; i < vert_attr_cnt; i++) {
        assert(vert_attr_lens[i] > 0);

        glVertexAttribPointer(i, vert_attr_lens[i], GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * offs));
        glEnableVertexAttribArray(i);

        offs += vert_attr_lens[i];
    }

    glBindVertexArray(0);
}

static bool AttachFramebufferTexture(const zfw_t_gl_id fb_gl_id, const zfw_t_gl_id tex_gl_id, const zfw_s_vec_2d_int tex_size) {
    assert(fb_gl_id != 0);
    assert(tex_gl_id != 0);
    assert(tex_size.x > 0 && tex_size.y > 0);

    glBindTexture(GL_TEXTURE_2D, tex_gl_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_size.x, tex_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, fb_gl_id);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_gl_id, 0);

    const bool success = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return success;
}

zfw_s_surface_group ZFW_GenSurfaces(const int surf_cnt, const zfw_s_vec_2d_int* const surf_sizes, zfw_s_gl_resource_arena* const gl_res_arena, s_mem_arena* const mem_arena) {
    assert(surf_cnt > 0);
    assert(surf_sizes);
    ZFW_AssertGLResourceArenaValidity(gl_res_arena);
    assert(mem_arena && IsMemArenaValid(mem_arena));

    zfw_t_gl_id* const fb_gl_ids = ZFW_ReserveGLIDs(gl_res_arena, surf_cnt, zfw_ek_gl_resource_type_framebuffer);

    if (!fb_gl_ids) {
        LOG_ERROR("Failed to reserve OpenGL framebuffer IDs!");
        return (zfw_s_surface_group){0};
    }

    zfw_t_gl_id* const fb_tex_gl_ids = ZFW_ReserveGLIDs(gl_res_arena, surf_cnt, zfw_ek_gl_resource_type_texture);

    if (!fb_tex_gl_ids) {
        LOG_ERROR("Failed to reserve OpenGL texture IDs for framebuffers!");
        return (zfw_s_surface_group){0};
    }

    zfw_s_vec_2d_int* const surf_sizes_pers = MEM_ARENA_PUSH_TYPE_CNT(mem_arena, zfw_s_vec_2d_int, surf_cnt);

    if (!surf_sizes_pers) {
        LOG_ERROR("Failed to reserve memory for surface sizes!");
        return (zfw_s_surface_group){0};
    }

    glGenFramebuffers(surf_cnt, fb_gl_ids);
    glGenTextures(surf_cnt, fb_tex_gl_ids);

    for (int i = 0; i < surf_cnt; i++) {
        assert(surf_sizes[i].x > 0 && surf_sizes[i].y > 0);

        if (!AttachFramebufferTexture(fb_gl_ids[i], fb_tex_gl_ids[i], surf_sizes[i])) {
            LOG_ERROR("Failed to attach framebuffer texture for surface %d!", i);
            return (zfw_s_surface_group){0};
        }
    }

    memcpy(surf_sizes_pers, surf_sizes, sizeof(*surf_sizes) * surf_cnt);

    return (zfw_s_surface_group){
        .fb_gl_ids = fb_gl_ids,
        .fb_tex_gl_ids = fb_tex_gl_ids,
        .sizes = surf_sizes_pers,
        .cnt = surf_cnt
    };
}

bool ZFW_ResizeSurface(zfw_s_surface_group* const surfs, const int surf_index, const zfw_s_vec_2d_int size) {
    ZFW_AssertSurfaceGroupValidity(surfs);
    assert(surf_index >= 0 && surf_index < surfs->cnt);
    assert(size.x > 0 && size.y > 0);
    assert(size.x != surfs->sizes[surf_index].x || size.y != surfs->sizes[surf_index].y);

    // Delete old texture.
    glDeleteTextures(surfs->cnt, &surfs->fb_tex_gl_ids[surf_index]);

    // Generate and attach new texture of the new size.
    glGenTextures(surfs->cnt, &surfs->fb_tex_gl_ids[surf_index]);

    for (int i = 0; i < surfs->cnt; i++) {
        if (!AttachFramebufferTexture(surfs->fb_gl_ids[i], surfs->fb_tex_gl_ids[i], size)) {
            LOG_ERROR("Failed to attach framebuffer texture for surface %d!", i);
            return false;
        }
    }

    return true;
}
