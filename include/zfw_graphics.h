#ifndef ZFW_RENDERING_H
#define ZFW_RENDERING_H

#include <assert.h>
#include <limits.h>
#include <glad/glad.h>
#include <cu.h>
#include "zfw_math.h"

#define ZFW_RGBA_CHANNEL_CNT 4

#define ZFW_ASCII_PRINTABLE_MIN ' '
#define ZFW_ASCII_PRINTABLE_MAX '~'
#define ZFW_ASCII_PRINTABLE_RANGE_LEN (ZFW_ASCII_PRINTABLE_MAX - ZFW_ASCII_PRINTABLE_MIN + 1)

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

#define ZFW_ALIGNMENT_TOP_LEFT (zfw_s_vec_2d){0.0f, 0.0f}
#define ZFW_ALIGNMENT_TOP_CENTER (zfw_s_vec_2d){0.5f, 0.0f}
#define ZFW_ALIGNMENT_TOP_RIGHT (zfw_s_vec_2d){1.0f, 0.0f}
#define ZFW_ALIGNMENT_CENTER_LEFT (zfw_s_vec_2d){0.0f, 0.5f}
#define ZFW_ALIGNMENT_CENTER (zfw_s_vec_2d){0.5f, 0.5f}
#define ZFW_ALIGNMENT_CENTER_RIGHT (zfw_s_vec_2d){1.0f, 0.5f}
#define ZFW_ALIGNMENT_BOTTOM_LEFT (zfw_s_vec_2d){0.0f, 1.0f}
#define ZFW_ALIGNMENT_BOTTOM_CENTER (zfw_s_vec_2d){0.5f, 1.0f}
#define ZFW_ALIGNMENT_BOTTOM_RIGHT (zfw_s_vec_2d){1.0f, 1.0f}

typedef GLuint zfw_t_gl_id;

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

static inline bool ZFW_IsStrAlignmentValid(const zfw_s_vec_2d alignment) {
    return alignment.x >= 0.0f && alignment.x <= 1.0f
        && alignment.y >= 0.0f && alignment.y <= 1.0f;
}

typedef struct {
    const zfw_t_gl_id* gl_ids;
    const zfw_s_vec_2d_i* sizes;

    int cnt;
} zfw_s_textures;

static inline bool ZFW_IsTexturesValid(const zfw_s_textures* const textures) {
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
    int line_height;

    zfw_s_vec_2d_i chr_offsets[ZFW_ASCII_PRINTABLE_RANGE_LEN];
    zfw_s_vec_2d_i chr_sizes[ZFW_ASCII_PRINTABLE_RANGE_LEN];
    int chr_advances[ZFW_ASCII_PRINTABLE_RANGE_LEN];
} zfw_s_font_arrangement_info;

typedef zfw_s_vec_2d_i zfw_t_tex_chr_positions[ZFW_ASCII_PRINTABLE_RANGE_LEN];

// Fonts can be manually loaded by the user. Multiple of these might be defined by the user in case they want a font group system for example.
typedef struct {
    const zfw_s_font_arrangement_info* arrangement_infos;
    const zfw_t_gl_id* tex_gl_ids;
    const zfw_s_vec_2d_i* tex_sizes;
    const zfw_t_tex_chr_positions* tex_chr_positions;

    int cnt;
} zfw_s_fonts;

static inline bool ZFW_IsFontsValid(const zfw_s_fonts* const fonts) {
    // TODO: Check validity of arrangement information!

    if (!fonts->arrangement_infos || !fonts->tex_gl_ids || !fonts->tex_sizes || !fonts->tex_chr_positions || fonts->cnt <= 0) {
        return false;
    }

    for (int i = 0; i < fonts->cnt; i++) {
        if (!glIsTexture(fonts->tex_gl_ids[i]) || fonts->tex_sizes[i].x <= 0 || fonts->tex_sizes[i].y <= 0) {
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

// Shader programs can be manually loaded by the user. Their main purpose in this system is with surfaces and surface rendering, when you want to render a surface with a particular effect. Multiple of these might be defined by the user in case they want a shader program group system for example.
typedef struct {
    const zfw_t_gl_id* gl_ids;
    int cnt;
} zfw_s_shader_progs;

static inline bool ZFW_IsShaderProgsValid(const zfw_s_shader_progs* const progs) {
    if (progs->cnt <= 0 || !progs->gl_ids) {
        return false;
    }

    for (int i = 0; i < progs->cnt; i++) {
        if (!glIsProgram(progs->gl_ids[i])) {
            return false;
        }
    }

    return true;
}

typedef struct {
    const char* vs_fp;
    const char* fs_fp;
} zfw_s_shader_prog_file_paths;

typedef zfw_s_shader_prog_file_paths (*zfw_t_shader_prog_index_to_file_paths)(const int index);

typedef enum {
    zfw_ek_shader_prog_uniform_value_type_int,
    zfw_ek_shader_prog_uniform_value_type_float,
    zfw_ek_shader_prog_uniform_value_type_v2,
    zfw_ek_shader_prog_uniform_value_type_v3,
    zfw_ek_shader_prog_uniform_value_type_v4,
    zfw_ek_shader_prog_uniform_value_type_mat4x4,
} zfw_e_shader_prog_uniform_value_type;

typedef struct {
    zfw_e_shader_prog_uniform_value_type type;

    union {
        int as_int;
        float as_float;
        zfw_s_vec_2d as_v2;
        zfw_s_vec_3d as_v3;
        zfw_s_vec_4d as_v4;
        zfw_t_matrix_4x4 as_mat4x4;
    };
} zfw_s_shader_prog_uniform_value;

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
} zfw_s_renderable;

static bool ZFW_IsRenderableValid(const zfw_s_renderable* const gl_ids) {
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

#define ZFW_SURFACE_LIMIT 32

typedef struct {
    zfw_t_gl_id fb_gl_ids[ZFW_SURFACE_LIMIT];
    zfw_t_gl_id fb_tex_gl_ids[ZFW_SURFACE_LIMIT];
    int cnt;

    zfw_s_vec_2d_i size;
} zfw_s_surfaces;

static inline bool ZFW_IsSurfacesValid(const zfw_s_surfaces* const surfs) {
    assert(surfs);

    if (IS_ZERO(*surfs)) {
        return true;
    }

    if (surfs->cnt <= 0 || surfs->cnt > ZFW_SURFACE_LIMIT || surfs->size.x <= 0 || surfs->size.y <= 0) {
        return false;
    }
    
    for (int i = 0; i < surfs->cnt; i++) {
        if (!glIsFramebuffer(surfs->fb_gl_ids[i]) || !glIsTexture(surfs->fb_tex_gl_ids[i])) {
            return false;
        }
    }

    return true;
}

typedef struct {
    zfw_s_renderable batch_renderable;
    zfw_s_batch_shader_prog batch_shader_prog;
    zfw_t_gl_id px_tex_gl_id;
    zfw_s_surfaces surfs;
    zfw_s_renderable surf_renderable;
} zfw_s_rendering_basis;

static inline bool ZFW_IsRenderingBasisValid(const zfw_s_rendering_basis* const basis) {
    return ZFW_IsRenderableValid(&basis->batch_renderable)
        && ZFW_IsBatchShaderProgValid(&basis->batch_shader_prog)
        && glIsTexture(basis->px_tex_gl_id)
        && ZFW_IsSurfacesValid(&basis->surfs)
        && ZFW_IsRenderableValid(&basis->surf_renderable);
}

typedef struct {
    int buf[ZFW_SURFACE_LIMIT];
    int height;
} zfw_s_surface_index_stack;

static inline bool ZFW_IsSurfaceIndexStackValid(const zfw_s_surface_index_stack* const stack, const int surf_cnt) {
    assert(stack);
    assert(surf_cnt >= 0 && surf_cnt <= ZFW_SURFACE_LIMIT);

    if (stack->height < 0 || stack->height > surf_cnt) {
        return false;
    }

    for (int i = 0; i < stack->height; i++) {
        if (stack->buf[i] < 0 || stack->buf[i] > surf_cnt) {
            return false;
        }

        // Check there are no duplicates.
        for (int j = 0; j < i; j++) {
            if (stack->buf[i] == stack->buf[j]) {
                return false;
            }
        }
    }

    return true;
}

typedef struct {
    zfw_s_batch_state batch;
    zfw_t_matrix_4x4 view_mat;

    zfw_t_gl_id surf_shader_prog_gl_id; // When a surface is rendered, this shader program is used.
    zfw_s_surface_index_stack surf_index_stack;
} zfw_s_rendering_state;

static inline bool ZFW_IsRenderingStateValid(const zfw_s_rendering_state* const state, const int surf_cnt) {
    assert(state);
    assert(surf_cnt >= 0 && surf_cnt <= ZFW_SURFACE_LIMIT);

    return ZFW_IsBatchStateValid(&state->batch)
        && (state->surf_shader_prog_gl_id == 0 || glIsProgram(state->surf_shader_prog_gl_id))
        && ZFW_IsSurfaceIndexStackValid(&state->surf_index_stack, surf_cnt);
}

// This is passed into various render functions, and provides simple access into everything that might be needed.
typedef struct {
    zfw_s_rendering_basis* basis;
    zfw_s_rendering_state* state;

    zfw_s_vec_2d_i window_size;
} zfw_s_rendering_context;

static inline bool ZFW_IsRenderingContextValid(const zfw_s_rendering_context* const context) {
    return ZFW_IsRenderingBasisValid(context->basis) && ZFW_IsRenderingStateValid(context->state, context->basis->surfs.cnt) && context->window_size.x > 0 && context->window_size.y > 0;
}

//
// zfw_rendering.c
//
zfw_s_renderable ZFW_GenRenderable(const float* const vert_buf, const size_t vert_buf_size, const unsigned short* const elem_buf, const size_t elem_buf_size, const int* const vert_attr_lens, const int vert_attr_cnt);
void ZFW_CleanRenderable(zfw_s_renderable* const renderable);

bool ZFW_InitRenderingBasis(zfw_s_rendering_basis* const basis, s_mem_arena* const mem_arena, const int surf_cnt, const zfw_s_vec_2d_i window_size, s_mem_arena* const temp_mem_arena);
void ZFW_CleanRenderingBasis(zfw_s_rendering_basis* const basis);

void ZFW_InitRenderingState(zfw_s_rendering_state* const state);

void ZFW_RenderClear(const zfw_s_vec_4d col);
void ZFW_Render(const zfw_s_rendering_context* const context, const zfw_s_batch_slot_write_info* const write_info);
void ZFW_RenderRect(const zfw_s_rendering_context* const context, const zfw_s_rect rect, const zfw_s_vec_4d blend);
void ZFW_RenderRectOutline(const zfw_s_rendering_context* const context, const zfw_s_rect rect, const zfw_s_vec_4d blend, const float thickness);
void ZFW_RenderLine(const zfw_s_rendering_context* const context, const zfw_s_vec_2d a, const zfw_s_vec_2d b, const zfw_s_vec_4d blend, const float width);
void ZFW_RenderPolyOutline(const zfw_s_rendering_context* const context, const zfw_s_poly poly, const zfw_s_vec_4d blend, const float width);
void ZFW_RenderBarHor(const zfw_s_rendering_context* const context, const zfw_s_rect rect, const float perc, const zfw_s_vec_3d col_front, const zfw_s_vec_3d col_back);

void ZFW_SubmitBatch(const zfw_s_rendering_context* const context);

//
// zfw_surfaces.c
//
bool ZFW_InitSurfaces(zfw_s_surfaces* const surfs, s_mem_arena* const mem_arena, const int cnt, const zfw_s_vec_2d_i size);
void ZFW_CleanSurfaces(zfw_s_surfaces* const surfs);
bool ZFW_ResizeSurfaces(zfw_s_surfaces* const surfs, const zfw_s_vec_2d_i size);

zfw_s_renderable ZFW_GenSurfaceRenderable();

bool ZFW_SetSurface(const zfw_s_rendering_context* const rendering_context, const int surf_index);
void ZFW_UnsetSurface(const zfw_s_rendering_context* const rendering_context);
void ZFW_SetSurfaceShaderProg(const zfw_s_rendering_context* const rendering_context, const int prog_index, const zfw_s_shader_progs* const progs);
void ZFW_SetSurfaceShaderProgUniform(const zfw_s_rendering_context* const rendering_context, const char* const name, const zfw_s_shader_prog_uniform_value val);
void ZFW_RenderSurface(const zfw_s_rendering_context* const rendering_context, const int surf_index);

//
// zfw_textures.c
//
void ZFW_SetUpTexture(const zfw_t_gl_id tex_gl_id, const zfw_s_vec_2d_i tex_size, const t_u8* const rgba_px_data);

static inline zfw_t_gl_id ZFW_GenTexture(const zfw_s_vec_2d_i tex_size, const t_u8* const rgba_px_data) {
    assert(tex_size.x > 0 && tex_size.y > 0);
    assert(rgba_px_data);

    zfw_t_gl_id gl_id;
    glGenTextures(1, &gl_id);
    ZFW_SetUpTexture(gl_id, tex_size, rgba_px_data);
    return gl_id;
}

zfw_s_textures ZFW_LoadTexturesFromFiles(s_mem_arena* const mem_arena, const int tex_cnt, const zfw_t_texture_index_to_file_path tex_index_to_fp);
void ZFW_UnloadTextures(zfw_s_textures* const textures);

void ZFW_RenderTexture(const zfw_s_rendering_context* const context, const int tex_index, const zfw_s_textures* const textures, const zfw_s_rect_i src_rect, const zfw_s_vec_2d pos, const zfw_s_vec_2d origin, const zfw_s_vec_2d scale, const float rot, const zfw_s_vec_4d blend);

zfw_s_rect_edges ZFW_TextureCoords(const zfw_s_rect_i src_rect, const zfw_s_vec_2d_i tex_size);

//
// zfw_fonts.c
//
zfw_s_fonts ZFW_LoadFontsFromFiles(s_mem_arena* const mem_arena, const int font_cnt, const t_font_index_to_load_info font_index_to_load_info, s_mem_arena* const temp_mem_arena);
void ZFW_UnloadFonts(zfw_s_fonts* const fonts);

bool ZFW_LoadStrCollider(zfw_s_rect* const rect, const char* const str, const int font_index, const zfw_s_fonts* const fonts, const zfw_s_vec_2d pos, const zfw_s_vec_2d alignment, s_mem_arena* const temp_mem_arena);
bool ZFW_RenderStr(const zfw_s_rendering_context* const context, const char* const str, const int font_index, const zfw_s_fonts* const fonts, const zfw_s_vec_2d pos, const zfw_s_vec_2d alignment, const zfw_s_vec_4d blend, s_mem_arena* const temp_mem_arena);

//
// zfw_shaders.c
//
zfw_t_gl_id ZFW_CreateShaderFromSrc(const char* const src, const bool frag);
zfw_t_gl_id ZFW_CreateShaderProgFromSrcs(const char* const vert_src, const char* const frag_src);
zfw_s_shader_progs ZFW_LoadShaderProgsFromFiles(s_mem_arena* const mem_arena, const int prog_cnt, const zfw_t_shader_prog_index_to_file_paths prog_index_to_fps, s_mem_arena* const temp_mem_arena);
void ZFW_UnloadShaderProgs(zfw_s_shader_progs* const progs);

#endif
