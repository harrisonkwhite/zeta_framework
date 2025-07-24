#ifndef ZFW_RENDERING_H
#define ZFW_RENDERING_H

#include <assert.h>
#include <limits.h>
#include <glad/glad.h>
#include "zfw_math.h"

#define ZFW_GL_VERSION_MAJOR 4
#define ZFW_GL_VERSION_MINOR 3

#define ZFW_WHITE (zfw_s_vec_4d){1.0f, 1.0f, 1.0f, 1.0f}
#define ZFW_RED (zfw_s_vec_4d){1.0f, 0.0f, 0.0f, 1.0f}
#define ZFW_GREEN (zfw_s_vec_4d){0.0f, 1.0f, 0.0f, 1.0f}
#define ZFW_BLUE (zfw_s_vec_4d){0.0f, 0.0f, 1.0f, 1.0f}
#define ZFW_BLACK (zfw_s_vec_4d){0.0f, 0.0f, 0.0f, 1.0f}
#define ZFW_YELLOW (zfw_s_vec_4d){1.0f, 1.0f, 0.0f, 1.0f}
#define ZFW_CYAN (zfw_s_vec_4d){0.0f, 1.0f, 1.0f, 1.0f}
#define ZFW_MAGENTA (zfw_s_vec_4d){1.0f, 0.0f, 1.0f, 1.0f}
#define ZFW_GRAY (zfw_s_vec_4d){0.5f, 0.5f, 0.5f, 1.0f}

typedef GLuint zfw_t_gl_id;

typedef enum {
    zfw_ek_str_hor_align_left,
    zfw_ek_str_hor_align_center,
    zfw_ek_str_hor_align_right
} zfw_e_str_hor_align;

typedef enum {
    zfw_ek_str_ver_align_top,
    zfw_ek_str_ver_align_center,
    zfw_ek_str_ver_align_bottom
} zfw_e_str_ver_align;

static inline bool ZFW_IsOriginValid(const zfw_s_vec_2d orig) {
    return orig.x >= 0.0f && orig.x <= 1.0f && orig.y >= 0.0f && orig.y <= 1.0f;
}

static inline bool ZFW_IsRotationValid(const float rot) {
    return isfinite(rot);
}

static inline bool ZFW_IsTextureCoordsValid(const zfw_s_rect_edges coords) {
    return coords.left >= 0.0f && coords.top >= 0.0f && coords.right <= 1.0f && coords.bottom <= 1.0f;
}

static bool ZFW_IsSrcRectValid(const zfw_s_rect_i src_rect, const zfw_s_vec_2d_i tex_size) {
    assert(tex_size.x > 0 && tex_size.y > 0);
    return src_rect.x >= 0 && src_rect.width > 0 && ZFW_RectIRight(src_rect) <= tex_size.x
        && src_rect.y >= 0 && src_rect.height > 0 && ZFW_RectIBottom(src_rect) <= tex_size.y;
}

static inline bool ZFW_IsColorValid(const zfw_s_vec_4d col) {
    return col.x >= 0.0f && col.x <= 1.0f
        && col.y >= 0.0f && col.y <= 1.0f
        && col.z >= 0.0f && col.z <= 1.0f
        && col.w >= 0.0f && col.w <= 1.0f;
}

static inline bool ZFW_IsColorRGBValid(const zfw_s_vec_3d col) {
    return col.x >= 0.0f && col.x <= 1.0f
        && col.y >= 0.0f && col.y <= 1.0f
        && col.z >= 0.0f && col.z <= 1.0f;
}

#define ZFW_TEXTURE_CHANNEL_CNT 4

#define ZFW_FONT_CHR_RANGE_BEGIN 32
#define ZFW_FONT_CHR_RANGE_LEN 95
#define ZFW_FONT_TEXTURE_WIDTH 2048
#define ZFW_FONT_TEXTURE_HEIGHT_LIMIT 2048

typedef struct {
    const zfw_t_gl_id* gl_ids;
    const zfw_s_vec_2d_i* sizes;

    int cnt;
} zfw_s_textures;

static inline bool ZFW_IsTexturesValid(const zfw_s_textures* const textures) {
    if (ZFW_IS_ZERO(*textures)) {
        return true;
    }

    if (textures->cnt <= 0 || !textures->gl_ids || !textures->sizes) {
        return false;
    }

    for (int i = 0; i < textures->cnt; i++) {
        const zfw_s_vec_2d_i tex_size = textures->sizes[i];

        if (!glIsTexture(textures->gl_ids[i]) || tex_size.x <= 0 || tex_size.y <= 0) {
            return false;
        }
    }

    return true;
}

typedef const char* (*zfw_t_texture_index_to_file_path)(const int index);

typedef struct {
    int chr_hor_offsets[ZFW_FONT_CHR_RANGE_LEN];
    int chr_ver_offsets[ZFW_FONT_CHR_RANGE_LEN];
    int chr_hor_advances[ZFW_FONT_CHR_RANGE_LEN];
    zfw_s_rect_i chr_src_rects[ZFW_FONT_CHR_RANGE_LEN];
    int line_height;
} zfw_s_font_arrangement_info;

// Fonts can be manually loaded by the user. Multiple of these might be defined by the user in case they want a font group system for example.
typedef struct {
    zfw_s_font_arrangement_info* arrangement_infos;
    zfw_t_gl_id* tex_gl_ids;
    int* tex_heights;
    int cnt;
} zfw_s_fonts;

static inline bool ZFW_IsFontsValid(const zfw_s_fonts* const fonts) {
    // TODO: Check validity of arrangement information!
    
    if (ZFW_IS_ZERO(*fonts)) {
        return true;
    }

    if (!fonts->arrangement_infos || !fonts->tex_gl_ids || !fonts->tex_heights || fonts->cnt <= 0) {
        return false;
    }

    for (int i = 0; i < fonts->cnt; i++) {
        if (!glIsTexture(fonts->tex_gl_ids[i]) || fonts->tex_heights[i] <= 0) {
            return false;
        }
    }

    return true;
}

typedef struct {
    const char* file_path;
    int height;
} zfw_s_font_load_info;

typedef zfw_s_font_load_info (*t_font_index_to_load_info)(const int index);

#define ZFW_BATCH_SLOT_CNT 8192
#define ZFW_BATCH_SLOT_VERT_CNT 4
#define ZFW_BATCH_SLOT_ELEM_CNT 6
static_assert(ZFW_BATCH_SLOT_ELEM_CNT * ZFW_BATCH_SLOT_CNT <= USHRT_MAX, "Batch slot count is too high!");

typedef struct {
    zfw_s_vec_2d vert_coord;
    zfw_s_vec_2d pos;
    zfw_s_vec_2d size;
    float rot;
    zfw_s_vec_2d tex_coord;
    zfw_s_vec_4d blend;
} zfw_s_batch_vertex;

typedef zfw_s_batch_vertex zfw_t_batch_slot[ZFW_BATCH_SLOT_VERT_CNT];

static const int zfw_g_batch_vertex_attrib_lens[] = {
    2,
    2,
    2,
    1,
    2,
    4
};

typedef struct {
    zfw_t_gl_id tex_gl_id;
    zfw_s_rect_edges tex_coords;
    zfw_s_vec_2d pos;
    zfw_s_vec_2d size;
    zfw_s_vec_2d origin;
    float rot;
    zfw_s_vec_4d blend;
} zfw_s_batch_slot_write_info;

static inline bool ZFW_IsBatchSlotWriteInfoValid(const zfw_s_batch_slot_write_info* const write_info) {
    assert(write_info);
    return glIsTexture(write_info->tex_gl_id)
        && ZFW_IsTextureCoordsValid(write_info->tex_coords)
        && write_info->size.x > 0.0f && write_info->size.y > 0.0f
        && ZFW_IsOriginValid(write_info->origin)
        && ZFW_IsRotationValid(write_info->rot)
        && ZFW_IsColorValid(write_info->blend);
}

typedef struct {
    zfw_t_gl_id vert_array_gl_id;
    zfw_t_gl_id vert_buf_gl_id;
    zfw_t_gl_id elem_buf_gl_id;
} zfw_s_gl_ids;

static bool ZFW_IsGLIDsValid(const zfw_s_gl_ids* const gl_ids) {
    return glIsVertexArray(gl_ids->vert_array_gl_id) && glIsBuffer(gl_ids->vert_buf_gl_id) && glIsBuffer(gl_ids->elem_buf_gl_id);
}

typedef struct {
    zfw_t_gl_id gl_id;
    int view_uniform_loc;
    int proj_uniform_loc;
} zfw_s_batch_shader_prog;

static inline bool ZFW_IsBatchShaderProgValid(const zfw_s_batch_shader_prog* const prog) {
    return glIsProgram(prog->gl_id) && prog->view_uniform_loc >= 0 && prog->proj_uniform_loc >= 0;
}

typedef struct {
    zfw_t_batch_slot slots[ZFW_BATCH_SLOT_CNT];
    int num_slots_used;

    zfw_t_gl_id tex_gl_id;
} zfw_s_batch_state;

// This does not check whether the slot vertex data is valid.
static inline bool ZFW_IsBatchStateValid(const zfw_s_batch_state* const state) {
    if (state->num_slots_used == 0) {
        return state->tex_gl_id == 0;
    }

    return state->num_slots_used > 0 && glIsTexture(state->tex_gl_id);
}

typedef struct {
    zfw_s_gl_ids batch_gl_ids;
    zfw_s_batch_shader_prog batch_shader_prog;
} zfw_s_rendering_basis;

static inline bool ZFW_IsRenderingBasisValid(const zfw_s_rendering_basis* const basis) {
    return ZFW_IsGLIDsValid(&basis->batch_gl_ids) && ZFW_IsBatchShaderProgValid(&basis->batch_shader_prog);
}

typedef struct {
    zfw_s_batch_state batch;
    zfw_t_matrix_4x4 view_mat;
} zfw_s_rendering_state;

static inline bool ZFW_IsRenderingStateValid(const zfw_s_rendering_state* const state) {
    return ZFW_IsBatchStateValid(&state->batch);
}

// This is passed into various render functions, and provides simple access into everything that might be needed.
typedef struct {
    zfw_s_rendering_basis* basis;
    zfw_s_rendering_state* state;

    zfw_s_vec_2d_i window_size;
} zfw_s_rendering_context;

static inline bool ZFW_IsRenderingContextValid(const zfw_s_rendering_context* const context) {
    return ZFW_IsRenderingBasisValid(context->basis) && ZFW_IsRenderingStateValid(context->state) && context->window_size.x > 0 && context->window_size.y > 0;
}


//
// zfw_rendering.c
//
bool ZFW_InitRenderingBasis(zfw_s_rendering_basis* const basis, zfw_s_mem_arena* const temp_mem_arena);
void ZFW_CleanRenderingBasis(zfw_s_rendering_basis* const basis);

void ZFW_InitRenderingState(zfw_s_rendering_state* const state);

void ZFW_RenderClear(const zfw_s_vec_4d col);
void ZFW_Render(const zfw_s_rendering_context* const context, const zfw_s_batch_slot_write_info* const write_info);

void ZFW_SubmitBatch(const zfw_s_rendering_context* const context);

//
// zfw_textures.c
//
bool ZFW_LoadTexturesFromFiles(zfw_s_textures* const textures, zfw_s_mem_arena* const mem_arena, const int tex_cnt, const zfw_t_texture_index_to_file_path tex_index_to_fp);
void ZFW_UnloadTextures(zfw_s_textures* const textures);

void ZFW_RenderTexture(const zfw_s_rendering_context* const context, const int tex_index, const zfw_s_textures* const textures, const zfw_s_rect_i src_rect, const zfw_s_vec_2d pos, const zfw_s_vec_2d origin, const zfw_s_vec_2d scale, const float rot, const zfw_s_vec_4d blend);

zfw_s_rect_edges ZFW_TextureCoords(const zfw_s_rect_i src_rect, const zfw_s_vec_2d_i tex_size);

//
// zfw_fonts.c
//
bool ZFW_LoadFontsFromFiles(zfw_s_fonts* const fonts, zfw_s_mem_arena* const mem_arena, const int font_cnt, const t_font_index_to_load_info font_index_to_load_info, zfw_s_mem_arena* const temp_mem_arena);
void ZFW_UnloadFonts(zfw_s_fonts* const fonts);

const zfw_s_vec_2d* ZFW_PushStrChrPositions(const char* const str, zfw_s_mem_arena* const mem_arena, const int font_index, const zfw_s_fonts* const fonts, const zfw_s_vec_2d pos, const zfw_e_str_hor_align hor_align, const zfw_e_str_ver_align ver_align);
bool ZFW_LoadStrCollider(zfw_s_rect* const rect, const char* const str, const int font_index, const zfw_s_fonts* const fonts, const zfw_s_vec_2d pos, const zfw_e_str_hor_align hor_align, const zfw_e_str_ver_align ver_align, zfw_s_mem_arena* const temp_mem_arena);

bool ZFW_RenderStr(const zfw_s_rendering_context* const context, const char* const str, const int font_index, const zfw_s_fonts* const fonts, const zfw_s_vec_2d pos, const zfw_e_str_hor_align hor_align, const zfw_e_str_ver_align ver_align, const zfw_s_vec_4d blend, zfw_s_mem_arena* const temp_mem_arena);

//
// zfw_shaders.c
//
zfw_t_gl_id ZFW_CreateShaderFromSrc(const char* const src, const bool frag);
zfw_t_gl_id ZFW_CreateShaderProgFromSrcs(const char* const vert_src, const char* const frag_src);

#endif
