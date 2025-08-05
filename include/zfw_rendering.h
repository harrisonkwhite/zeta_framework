#ifndef ZFW_GRAPHICS_H
#define ZFW_GRAPHICS_H

#include <limits.h>
#include <glad/glad.h>
#include <cu.h>
#include "zfw_math.h"
#include "zfw_gl.h"

#define ZFW_BATCH_SLOT_CNT 8192
#define ZFW_BATCH_SLOT_VERT_CNT 4
#define ZFW_BATCH_SLOT_ELEM_CNT 6
static_assert(ZFW_BATCH_SLOT_ELEM_CNT * ZFW_BATCH_SLOT_CNT <= USHRT_MAX, "Batch slot count is too high!");

typedef enum {
    zfw_ek_builtin_texture_pixel,
    zfw_eks_builtin_texture_cnt
} zfw_e_builtin_texture;

typedef enum {
    zfw_ek_builtin_shader_prog_batch,
    zfw_eks_builtin_shader_prog_cnt
} zfw_e_builtin_shader_prog;

typedef enum {
    zfw_ek_renderable_batch,
    zfw_ek_renderable_surface,

    zfw_eks_renderable_cnt
} zfw_e_renderable;

typedef struct {
    zfw_s_texture_group builtin_textures;
    zfw_s_shader_prog_group builtin_shader_progs;
    zfw_s_renderables renderables;
} zfw_s_rendering_basis;

static inline void ZFW_AssertRenderingBasisValidity(const zfw_s_rendering_basis* const basis) {
    assert(basis);

    ZFW_AssertTextureGroupValidity(&basis->builtin_textures);
    ZFW_AssertShaderProgGroupValidity(&basis->builtin_shader_progs);
    ZFW_AssertRenderablesValidity(&basis->renderables);
}

typedef struct {
    s_v2 vert_coord;
    s_v2 pos;
    s_v2 size;
    float rot;
    s_v2 tex_coord;
    u_v4 blend;
} zfw_s_batch_vertex;

static const int zfw_g_batch_vertex_attrib_lens[] = {
    2,
    2,
    2,
    1,
    2,
    4
};

typedef zfw_s_batch_vertex zfw_t_batch_slot[ZFW_BATCH_SLOT_VERT_CNT];

typedef struct {
    zfw_t_gl_id tex_gl_id;
    zfw_s_rect_edges tex_coords;
    s_v2 pos;
    s_v2 size;
    s_v2 origin;
    float rot;
    u_v4 blend;
} zfw_s_batch_slot_write_info;

static inline void ZFW_AssertBatchSlotWriteInfoValidity(const zfw_s_batch_slot_write_info* const write_info) {
    assert(write_info);
    assert(glIsTexture(write_info->tex_gl_id));
    assert(write_info->tex_coords.left >= 0.0f && write_info->tex_coords.top >= 0.0f && write_info->tex_coords.right <= 1.0f && write_info->tex_coords.bottom <= 1.0f);
    assert(write_info->size.x > 0.0f && write_info->size.y > 0.0f);
    assert(ZFW_IsOriginValid(write_info->origin));
    assert(ZFW_IsColorValid(write_info->blend));
}

typedef struct {
    zfw_t_batch_slot slots[ZFW_BATCH_SLOT_CNT];
    int num_slots_used;

    zfw_t_gl_id tex_gl_id;
} zfw_s_batch_state;

static inline void ZFW_AssertBatchStateValidity(const zfw_s_batch_state* const batch_state) {
    assert(batch_state);
    assert(batch_state->num_slots_used >= 0 && batch_state->num_slots_used <= ZFW_BATCH_SLOT_CNT);
    assert(batch_state->num_slots_used == 0 || glIsTexture(batch_state->tex_gl_id));
}

typedef struct {
    zfw_s_batch_state batch;
    zfw_s_matrix_4x4 view_mat;
} zfw_s_rendering_state;

static inline void ZFW_AssertRenderingStateValidity(const zfw_s_rendering_state* const state) {
    assert(state);
    ZFW_AssertBatchStateValidity(&state->batch);
}

typedef struct {
    const zfw_s_rendering_basis* basis;
    zfw_s_rendering_state* state;

    s_v2_int window_size;
} zfw_s_rendering_context;

static inline void ZFW_AssertRenderingContextValidity(const zfw_s_rendering_context* const rendering_context) {
    assert(rendering_context);

    ZFW_AssertRenderingBasisValidity(rendering_context->basis);
    ZFW_AssertRenderingStateValidity(rendering_context->state);
    assert(rendering_context->window_size.x > 0 && rendering_context->window_size.y > 0);
}

bool ZFW_InitRenderingBasis(zfw_s_rendering_basis* const basis, zfw_s_gl_resource_arena* const gl_res_arena, s_mem_arena* const mem_arena, s_mem_arena* const temp_mem_arena);
zfw_s_rendering_state* ZFW_PushRenderingState(s_mem_arena* const mem_arena);

void ZFW_Clear(const zfw_s_rendering_context* const rendering_context, const u_v4 col);
void ZFW_SetViewMatrix(const zfw_s_rendering_context* const rendering_context, const zfw_s_matrix_4x4* const mat);
void ZFW_Render(const zfw_s_rendering_context* const rendering_context, const zfw_s_batch_slot_write_info* const write_info);
void ZFW_RenderTexture(const zfw_s_rendering_context* const rendering_context, const zfw_s_texture_group* const textures, const int tex_index, const zfw_s_rect_int src_rect, const s_v2 pos, const s_v2 origin, const s_v2 scale, const float rot, const u_v4 blend);
void ZFW_RenderRectWithOutline(const zfw_s_rendering_context* const rendering_context, const zfw_s_rect rect, const u_v4 fill_color, const u_v4 outline_color, const float outline_thickness);
void ZFW_RenderRectWithOutlineAndOpaqueFill(const zfw_s_rendering_context* const rendering_context, const zfw_s_rect rect, const u_v3 fill_color, const u_v4 outline_color, const float outline_thickness);
void ZFW_RenderBarHor(const zfw_s_rendering_context* const rendering_context, const zfw_s_rect rect, const float perc, const u_v4 front_color, const u_v4 bg_color);
void ZFW_RenderBarVer(const zfw_s_rendering_context* const rendering_context, const zfw_s_rect rect, const float perc, const u_v4 front_color, const u_v4 bg_color);
bool ZFW_RenderStr(const zfw_s_rendering_context* const rendering_context, const char* const str, const zfw_s_font_group* const fonts, const int font_index, const s_v2 pos, const s_v2 alignment, const u_v4 color, s_mem_arena* const temp_mem_arena);

void ZFW_SubmitBatch(const zfw_s_rendering_context* const rendering_context);

void ZFW_SetSurface(const zfw_s_rendering_context* const rendering_context, const zfw_s_surface_group* const surfs, const int surf_index);
void ZFW_UnsetSurface(const zfw_s_rendering_context* const rendering_context);
void ZFW_SetSurfaceShaderProg(const zfw_s_rendering_context* const rendering_context, const zfw_s_shader_prog_group* const progs, const int prog_index);
void ZFW_SetSurfaceShaderProgUniform(const zfw_s_rendering_context* const rendering_context, const char* const name, const zfw_s_shader_prog_uniform_value val);
void ZFW_RenderSurface(const zfw_s_rendering_context* const rendering_context, const zfw_s_surface_group* const surfs, const int surf_index);

static inline void ZFW_RenderRect(const zfw_s_rendering_context* const rendering_context, const zfw_s_rect rect, const u_v4 color) {
    ZFW_RenderTexture(rendering_context, &rendering_context->basis->builtin_textures, zfw_ek_builtin_texture_pixel, (zfw_s_rect_int){0}, ZFW_RectPos(rect), (s_v2){0}, ZFW_RectSize(rect), 0, color);
}

static inline void ZFW_RenderBarHorRev(const zfw_s_rendering_context* const rendering_context, const zfw_s_rect rect, const float perc, const u_v4 front_color, const u_v4 bg_color) {
    ZFW_RenderBarHor(rendering_context, rect, 1.0f - perc, bg_color, front_color);
}

static inline void ZFW_RenderBarVerRev(const zfw_s_rendering_context* const rendering_context, const zfw_s_rect rect, const float perc, const u_v4 front_color, const u_v4 bg_color) {
    ZFW_RenderBarVer(rendering_context, rect, 1.0f - perc, bg_color, front_color);
}

static inline void ZFW_RenderLine(const zfw_s_rendering_context* const rendering_context, const s_v2 a, const s_v2 b, const u_v4 blend, const float width) {
    assert(width > 0.0f);

    const s_v2 diff = {b.x - a.x, b.y - a.y};
    const float len = sqrtf((diff.x * diff.x) + (diff.y * diff.y));

    ZFW_RenderTexture(rendering_context, &rendering_context->basis->builtin_textures, zfw_ek_builtin_texture_pixel, (zfw_s_rect_int){0}, a, (s_v2){0.0f, 0.5f}, (s_v2){len, width}, atan2f(-diff.y, diff.x), blend);
}

#endif
