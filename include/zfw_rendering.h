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

typedef struct {
    zfw_s_vec_2d vert_coord;
    zfw_s_vec_2d pos;
    zfw_s_vec_2d size;
    float rot;
    zfw_s_vec_2d tex_coord;
    zfw_u_vec_4d blend;
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
    zfw_s_vec_2d pos;
    zfw_s_vec_2d size;
    zfw_s_vec_2d origin;
    float rot;
    zfw_u_vec_4d blend;
} zfw_s_batch_slot_write_info;

typedef struct {
    zfw_t_batch_slot slots[ZFW_BATCH_SLOT_CNT];
    int num_slots_used;

    zfw_t_gl_id tex_gl_id;
} zfw_s_batch_state;

typedef struct {
    zfw_s_batch_state batch;
    zfw_t_matrix_4x4 view_mat;
} zfw_s_rendering_state;

typedef struct {
    const zfw_s_rendering_basis* basis;
    zfw_s_rendering_state* state;

    zfw_s_vec_2d_s32 window_size;
} zfw_s_rendering_context;

bool ZFW_InitRenderingBasis(zfw_s_rendering_basis* const basis, zfw_s_gl_resource_arena* const gl_res_arena, s_mem_arena* const mem_arena, s_mem_arena* const temp_mem_arena);
void ZFW_InitRenderingState(zfw_s_rendering_state* const state);

void ZFW_RenderClear(const zfw_u_vec_4d col);
void ZFW_Render(const zfw_s_rendering_context* const context, const zfw_s_batch_slot_write_info* const write_info);
void ZFW_RenderTexture(const zfw_s_rendering_context* const context, const int tex_index, const zfw_s_texture_group* const textures, const zfw_s_rect_s32 src_rect, const zfw_s_vec_2d pos, const zfw_s_vec_2d origin, const zfw_s_vec_2d scale, const float rot, const zfw_u_vec_4d blend);

void ZFW_SubmitBatch(const zfw_s_rendering_context* const context);

static inline void ZFW_RenderRect(const zfw_s_rendering_context* const context, const zfw_s_rect rect, const zfw_u_vec_4d blend) {
    ZFW_RenderTexture(context, zfw_ek_builtin_texture_pixel, &context->basis->builtin_textures, (zfw_s_rect_s32){0}, ZFW_RectPos(rect), (zfw_s_vec_2d){0}, ZFW_RectSize(rect), 0, blend);
}

static inline void ZFW_RenderLine(const zfw_s_rendering_context* const context, const zfw_s_vec_2d a, const zfw_s_vec_2d b, const zfw_u_vec_4d blend, const float width) {
    assert(width > 0.0f);

    const zfw_s_vec_2d diff = {b.x - a.x, b.y - a.y};
    const float len = sqrtf((diff.x * diff.x) + (diff.y * diff.y));

    ZFW_RenderTexture(context, zfw_ek_builtin_texture_pixel, &context->basis->builtin_textures, (zfw_s_rect_s32){0}, a, (zfw_s_vec_2d){0.0f, 0.5f}, (zfw_s_vec_2d){len, width}, atan2f(-diff.y, diff.x), blend);
}

#endif
