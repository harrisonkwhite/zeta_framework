#ifndef ZFWC_GRAPHICS_H
#define ZFWC_GRAPHICS_H

#include <limits.h>
#include <zfws.h>
#include <cu.h>
#include <glad/glad.h>

#define WHITE (u_v4){1.0f, 1.0f, 1.0f, 1.0f}
#define RED (u_v4){1.0f, 0.0f, 0.0f, 1.0f}
#define GREEN (u_v4){0.0f, 1.0f, 0.0f, 1.0f}
#define BLUE (u_v4){0.0f, 0.0f, 1.0f, 1.0f}
#define BLACK (u_v4){0.0f, 0.0f, 0.0f, 1.0f}
#define YELLOW (u_v4){1.0f, 1.0f, 0.0f, 1.0f}
#define CYAN (u_v4){0.0f, 1.0f, 1.0f, 1.0f}
#define MAGENTA (u_v4){1.0f, 0.0f, 1.0f, 1.0f}
#define GRAY (u_v4){0.5f, 0.5f, 0.5f, 1.0f}

#define ALIGNMENT_TOP_LEFT (s_v2){0.0f, 0.0f}
#define ALIGNMENT_TOP_CENTER (s_v2){0.5f, 0.0f}
#define ALIGNMENT_TOP_RIGHT (s_v2){1.0f, 0.0f}
#define ALIGNMENT_CENTER_LEFT (s_v2){0.0f, 0.5f}
#define ALIGNMENT_CENTER (s_v2){0.5f, 0.5f}
#define ALIGNMENT_CENTER_RIGHT (s_v2){1.0f, 0.5f}
#define ALIGNMENT_BOTTOM_LEFT (s_v2){0.0f, 1.0f}
#define ALIGNMENT_BOTTOM_CENTER (s_v2){0.5f, 1.0f}
#define ALIGNMENT_BOTTOM_RIGHT (s_v2){1.0f, 1.0f}

#define BATCH_SLOT_CNT 8192
#define BATCH_SLOT_VERT_CNT 4
#define BATCH_SLOT_ELEM_CNT 6
static_assert(BATCH_SLOT_ELEM_CNT * BATCH_SLOT_CNT <= USHRT_MAX, "Batch slot count is too high!");

#define GL_VERSION_MAJOR 4
#define GL_VERSION_MINOR 3

typedef struct {
    s_v2_s32 tex_size;
    s_u8_array px_data;
} s_rgba_texture;

typedef t_u32 t_gl_id;

DEF_ARRAY_TYPE(t_gl_id, gl_id, GLID);

typedef enum {
    ek_gl_resource_type_texture,
    ek_gl_resource_type_shader_prog,
    ek_gl_resource_type_vert_array,
    ek_gl_resource_type_vert_buf,
    ek_gl_resource_type_elem_buf,
    ek_gl_resource_type_framebuffer,

    eks_gl_resource_type_cnt
} e_gl_resource_type;

DEF_ARRAY_TYPE(e_gl_resource_type, gl_resource_type, GLResourceType);

typedef struct {
    s_gl_id_array ids;
    s_gl_resource_type_array res_types;

    t_s32 res_used;
    t_s32 res_limit;
} s_gl_resource_arena;

typedef struct {
    s_v2_s32_array_view sizes;
    s_gl_id_array_view gl_ids;
} s_texture_group;

typedef s_rgba_texture (*t_texture_group_rgba_generator_func)(const t_s32 tex_index, s_mem_arena* const mem_arena);

typedef enum {
    ek_builtin_texture_pixel,
    eks_builtin_texture_cnt
} e_builtin_texture;

typedef struct {
    s_font_arrangement_array_view arrangements;
    s_font_texture_meta_array_view tex_metas;
    s_gl_id_array_view tex_gl_ids;
} s_font_group;

typedef struct {
    s_gl_id_array_view gl_ids;
} s_shader_prog_group;

typedef struct {
    bool holds_srcs;

    union {
        struct {
            s_char_array_view file_path;
        };

        struct {
            s_char_array_view vert_src;
            s_char_array_view frag_src;
        };
    };
} s_shader_prog_gen_info;

DEF_ARRAY_TYPE(s_shader_prog_gen_info, shader_prog_gen_info, ShaderProgGenInfo);

typedef enum {
    ek_shader_prog_uniform_value_type_s32,
    ek_shader_prog_uniform_value_type_r32,
    ek_shader_prog_uniform_value_type_v2,
    ek_shader_prog_uniform_value_type_v3,
    ek_shader_prog_uniform_value_type_v4,
    ek_shader_prog_uniform_value_type_mat4x4,
} e_shader_prog_uniform_value_type;

typedef struct {
    e_shader_prog_uniform_value_type type;

    union {
        t_s32 as_s32;
        t_r32 as_r32;
        s_v2 as_v2;
        u_v3 as_v3;
        u_v4 as_v4;
        s_matrix_4x4 as_mat4x4;
    };
} s_shader_prog_uniform_value;

typedef enum {
    ek_builtin_shader_prog_batch,
    eks_builtin_shader_prog_cnt
} e_builtin_shader_prog;

typedef enum {
    ek_renderable_batch,
    ek_renderable_surface,

    eks_renderable_cnt
} e_renderable;

typedef struct {
    const t_gl_id* vert_array_gl_id;
    const t_gl_id* vert_buf_gl_id;
    const t_gl_id* elem_buf_gl_id;
} s_renderable;

typedef struct {
    s_texture_group builtin_textures;
    s_shader_prog_group builtin_shader_progs;
    s_renderable renderables[eks_renderable_cnt];
} s_rendering_basis;

typedef struct {
    s_v2 vert_coord;
    s_v2 pos;
    s_v2 size;
    t_r32 rot;
    s_v2 tex_coord;
    u_v4 blend;
} s_batch_vert;

DEF_ARRAY_TYPE(s_batch_vert, batch_vert_array, BatchVert);

typedef s_batch_vert t_batch_slot[BATCH_SLOT_VERT_CNT];

typedef struct {
    t_gl_id tex_gl_id;
    s_rect_edges tex_coords;
    s_v2 pos;
    s_v2 size;
    s_v2 origin;
    t_r32 rot;
    u_v4 blend;
} s_batch_slot_write_info;

typedef struct {
    t_batch_slot slots[BATCH_SLOT_CNT];
    t_s32 num_slots_used;

    t_gl_id tex_gl_id;
} s_batch_state;

typedef struct {
    s_batch_state batch;
    s_matrix_4x4 view_mat;
} s_rendering_state;

typedef struct {
    const s_rendering_basis* basis;
    s_rendering_state* state;

    s_v2_s32 window_size;
} s_rendering_context;

typedef struct {
    const t_gl_id* fb_gl_id;
    t_gl_id* fb_tex_gl_id;
} s_surface;

//
// zfwc_graphics.c
//
bool InitGLResourceArena(s_gl_resource_arena* const res_arena, s_mem_arena* const mem_arena, const t_s32 res_limit);
void CleanGLResourceArena(s_gl_resource_arena* const res_arena);
s_gl_id_array PushToGLResourceArena(s_gl_resource_arena* const res_arena, const t_s32 cnt, const e_gl_resource_type res_type);

bool InitRenderingBasis(s_rendering_basis* const basis, s_gl_resource_arena* const gl_res_arena, s_mem_arena* const mem_arena, s_mem_arena* const temp_mem_arena);
s_rendering_state* GenRenderingState(s_mem_arena* const mem_arena);

void Clear(const s_rendering_context* const rendering_context, const u_v4 col);
void SetViewMatrix(const s_rendering_context* const rendering_context, const s_matrix_4x4* const mat);
void Render(const s_rendering_context* const rendering_context, const s_batch_slot_write_info* const write_info);
void SubmitBatch(const s_rendering_context* const rendering_context);

//
// zfwc_textures.c
//
s_rect_edges GenTextureCoords(const s_rect_s32 src_rect, const s_v2_s32 tex_size);
s_rgba_texture LoadRGBATextureFromFile(const s_char_array_view file_path, s_mem_arena *const mem_arena);
t_gl_id GenGLTextureFromRGBA(const s_rgba_texture rgba_tex);
s_texture_group GenTextureGroup(const t_s32 tex_cnt, const t_texture_group_rgba_generator_func rgba_generator_func, s_mem_arena *const mem_arena, s_gl_resource_arena* const gl_res_arena, s_mem_arena* const temp_mem_arena);
void RenderTexture(const s_rendering_context* const rendering_context, const s_texture_group* const textures, const t_s32 tex_index, const s_rect_s32 src_rect, const s_v2 pos, const s_v2 origin, const s_v2 scale, const t_r32 rot, const u_v4 blend);
void RenderRectWithOutline(const s_rendering_context* const rendering_context, const s_rect rect, const u_v4 fill_color, const u_v4 outline_color, const t_r32 outline_thickness);
void RenderRectWithOutlineAndOpaqueFill(const s_rendering_context* const rendering_context, const s_rect rect, const u_v3 fill_color, const u_v4 outline_color, const t_r32 outline_thickness);
void RenderBarHor(const s_rendering_context* const rendering_context, const s_rect rect, const t_r32 perc, const u_v4 front_color, const u_v4 bg_color);
void RenderBarVertical(const s_rendering_context* const rendering_context, const s_rect rect, const t_r32 perc, const u_v4 front_color, const u_v4 bg_color);

static inline void RenderRect(const s_rendering_context* const rendering_context, const s_rect rect, const u_v4 color) {
    RenderTexture(rendering_context, &rendering_context->basis->builtin_textures, ek_builtin_texture_pixel, (s_rect_s32){0}, RectPos(rect), (s_v2){0}, RectSize(rect), 0, color);
}

static inline void RenderBarHorReverse(const s_rendering_context* const rendering_context, const s_rect rect, const t_r32 perc, const u_v4 front_color, const u_v4 bg_color) {
    RenderBarHor(rendering_context, rect, 1.0f - perc, bg_color, front_color);
}

static inline void RenderBarVerticalReverse(const s_rendering_context* const rendering_context, const s_rect rect, const t_r32 perc, const u_v4 front_color, const u_v4 bg_color) {
    RenderBarVertical(rendering_context, rect, 1.0f - perc, bg_color, front_color);
}

static inline void RenderLine(const s_rendering_context* const rendering_context, const s_v2 a, const s_v2 b, const u_v4 blend, const t_r32 width) {
    const s_v2 diff = {b.x - a.x, b.y - a.y};
    const t_r32 len = sqrtf((diff.x * diff.x) + (diff.y * diff.y));

    RenderTexture(rendering_context, &rendering_context->basis->builtin_textures, ek_builtin_texture_pixel, (s_rect_s32){0}, a, (s_v2){0.0f, 0.5f}, (s_v2){len, width}, atan2f(-diff.y, diff.x), blend);
}

//
// zfwc_fonts.c
//
s_font_group GenFontGroupFromFiles(const s_char_array_view_array_view file_paths, s_mem_arena *const mem_arena, s_gl_resource_arena* const gl_res_arena, s_mem_arena* const temp_mem_arena);
s_v2_array CalcStrChrRenderPositions(s_mem_arena* const mem_arena, const s_char_array_view str, const s_font_group* const font_group, const t_s32 font_index, const s_v2 pos, const s_v2 alignment);
bool CalcStrCollider(s_rect* const rect, const s_char_array_view str, const s_font_group* const font_group, const t_s32 font_index, const s_v2 pos, const s_v2 alignment, s_mem_arena* const temp_mem_arena);
bool RenderStr(const s_rendering_context* const rendering_context, const s_char_array_view str, const s_font_group* const fonts, const t_s32 font_index, const s_v2 pos, const s_v2 alignment, const u_v4 color, s_mem_arena* const temp_mem_arena);

//
// zfwc_shaders.c
//
s_shader_prog_group GenShaderProgGroup(const s_shader_prog_gen_info_array_view gen_infos, s_gl_resource_arena* const gl_res_arena, s_mem_arena* const temp_mem_arena);

//
// zfwc_surfaces.c
//
s_surface GenSurface(const s_v2_s32 size, s_gl_resource_arena* const gl_res_arena);
bool ResizeSurface(s_surface* const surf, const s_v2_s32 size);
void SetSurface(const s_rendering_context* const rendering_context, const s_surface* const surf);
void UnsetSurface(const s_rendering_context* const rendering_context);
void SetSurfaceShaderProg(const s_rendering_context* const rendering_context, const s_shader_prog_group* const progs, const t_s32 prog_index);
void SetSurfaceShaderProgUniform(const s_rendering_context* const rendering_context, const char* const name, const s_shader_prog_uniform_value val);
void RenderSurface(const s_rendering_context* const rendering_context, const s_surface* const surf);

#endif
