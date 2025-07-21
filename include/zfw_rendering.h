/*

I designed this rendering system aiming for the simplest solution possible.

A render slot is effectively the vertex data for a single textured quad (i.e. a sprite). The render batch, as part of the rendering state, is a set of slots of a fixed limit. There is a counter indicating how many of this limit have been used. The idea is that when you perform a "render texture" operation, what is really happening is the counter is being incremented, and the vertex data for the topmost slot is being updated with the texture position, rotation, etc. A true render operation doesn't occur until a "flush", where all the slots in the batch (up to the counter) get rendered at once. A flush can be done manually be the user, or it might occur as a side effect of the "render texture" operation under certain circumstances (e.g. if you switch textures or if all batch slots have been used up).

In an old version of this framework from 2024, there was a system where you actually had to manually reserve and release slots (e.g. if your game had an enemy limit of 128, you might reserve 128 render slots). You would also explicitly indicate how many batches you would be using at the start of the program, and the vertex data for them would all be allocated up front. The thought process was that if you've got a lot of static textures (e.g. tiles), it would be wasteful to have to rewrite their vertex data every single tick - surely you can just reserve a slot for each, write ther vertex data when they are first created, and not touch it again until they get destroyed. Ultimately, though, I was overestimating how much of a performance impact this would have, and the complexity cost of the developer needing to manually manage render slots was not worth a trivial increase in efficiency. Therefore, for this iteration of ZFW I decided to go with this simpler approach, where there is only ever one batch and all vertex data has to be rewritten every tick.

TODO: Finish notes on surface system.

The render surface system is inspired by the surface system in GameMaker: Studio 2, where surfaces are effectively blank canvases that you can render onto, and can be rendered as though they were normal textures. My motivation for implementing such a system was that I wanted to be able to apply post-processing effects via shaders only to specific sprites or groups of sprites, without affecting layering. For example, if the player gets damaged I might want to have the player texture flash white for a few ticks.

*/

#ifndef ZFW_RENDERING_H
#define ZFW_RENDERING_H

#include <stdint.h>
#include <stdbool.h>
#include <glad/glad.h>
#include <assert.h>
#include "zfw_math.h"
#include "zfw_utils.h"

#define ZFW_TEXTURE_CHANNEL_CNT 4

#define ZFW_FONT_CHR_RANGE_BEGIN 32
#define ZFW_FONT_CHR_RANGE_LEN 95
#define ZFW_FONT_TEXTURE_WIDTH 2048
#define ZFW_FONT_TEXTURE_HEIGHT_LIMIT 2048
 
#define ZFW_RENDER_BATCH_SHADER_PROG_VERT_CNT 13
#define ZFW_RENDER_BATCH_SLOT_CNT 256 // TODO: There seems to be an issue here. Seems to crash when this is high (e.g. at 2048).
#define ZFW_RENDER_BATCH_SLOT_VERT_CNT (ZFW_RENDER_BATCH_SHADER_PROG_VERT_CNT * 4)
#define ZFW_RENDER_BATCH_SLOT_VERTS_SIZE (ZFW_RENDER_BATCH_SLOT_VERT_CNT * ZFW_RENDER_BATCH_SLOT_CNT * sizeof(float))
#define ZFW_RENDER_BATCH_SLOT_ELEM_CNT 6

#define ZFW_RENDER_SURFACE_LIMIT 8

#define ZFW_WHITE (zfw_s_color){1.0f, 1.0f, 1.0f, 1.0f}
#define ZFW_RED (zfw_s_color){1.0f, 0.0f, 0.0f, 1.0f}
#define ZFW_GREEN (zfw_s_color){0.0f, 1.0f, 0.0f, 1.0f}
#define ZFW_BLUE (zfw_s_color){0.0f, 0.0f, 1.0f, 1.0f}
#define ZFW_BLACK (zfw_s_color){0.0f, 0.0f, 0.0f, 1.0f}
#define ZFW_YELLOW (zfw_s_color){1.0f, 1.0f, 0.0f, 1.0f}
#define ZFW_CYAN (zfw_s_color){0.0f, 1.0f, 1.0f, 1.0f}
#define ZFW_MAGENTA (zfw_s_color){1.0f, 0.0f, 1.0f, 1.0f}
#define ZFW_GRAY (zfw_s_color){0.5f, 0.5f, 0.5f, 1.0f}

typedef GLuint zfw_t_gl_id;

typedef struct {
    zfw_t_gl_id vert_array_gl_id;
    zfw_t_gl_id vert_buf_gl_id;
    zfw_t_gl_id elem_buf_gl_id;
} zfw_s_render_batch_gl_ids;

typedef struct {
    zfw_t_gl_id gl_id;
    int proj_uniform_loc;
    int view_uniform_loc;
    int textures_uniform_loc;
} zfw_s_render_batch_shader_prog;

typedef struct {
    zfw_s_vec_2d_i size;
    zfw_t_gl_id framebuffer_gl_ids[ZFW_RENDER_SURFACE_LIMIT];
    zfw_t_gl_id framebuffer_tex_gl_ids[ZFW_RENDER_SURFACE_LIMIT];
} zfw_s_render_surfaces;

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

typedef struct {
    zfw_s_render_batch_shader_prog batch_shader_prog; // The program to use for rendering the batch.
    zfw_s_render_batch_gl_ids batch_gl_ids;

    zfw_s_render_surfaces surfs;
    zfw_t_gl_id surf_vert_array_gl_id;
    zfw_t_gl_id surf_vert_buf_gl_id;
    zfw_t_gl_id surf_elem_buf_gl_id;

    zfw_t_gl_id px_tex_gl_id; // Scaled and used for rectangles, lines, etc.
} zfw_s_pers_render_data;

typedef struct {
    int batch_slots_used_cnt; // The number of used render slots. This is reset on flush.
    float batch_slot_verts[ZFW_RENDER_BATCH_SLOT_CNT][ZFW_RENDER_BATCH_SLOT_VERT_CNT]; // Slot vertices accumulate in this buffer.
    zfw_t_gl_id batch_tex_gl_id; // The GL ID of the texture that will be drawn on flush.

    zfw_t_gl_id surf_shader_prog_gl_id; // When a surface is rendered, this shader program is used.
    int surf_index_stack[ZFW_RENDER_SURFACE_LIMIT];
    int surf_index_stack_height;

    zfw_t_matrix_4x4 view_mat; // The view matrix that's provided as shader uniform on flush.
} zfw_s_rendering_state;

// Textures can be manually loaded by the user. Multiple of these might be defined by the user in case they want a texture group system for example.
typedef struct {
    zfw_t_gl_id* gl_ids;
    zfw_s_vec_2d_i* sizes;
    int cnt;
} zfw_s_textures;

typedef const char* (*zfw_t_texture_index_to_file_path)(const int index);

typedef struct {
    int chr_hor_offsets[ZFW_FONT_CHR_RANGE_LEN];
    int chr_ver_offsets[ZFW_FONT_CHR_RANGE_LEN];
    int chr_hor_advances[ZFW_FONT_CHR_RANGE_LEN];
    zfw_s_rect_i chr_src_rects[ZFW_FONT_CHR_RANGE_LEN];
    int line_height;
} zfw_s_font_arrangement_info;

typedef struct {
    const char* file_path;
    int height;
} zfw_s_font_load_info;

typedef zfw_s_font_load_info (*t_font_index_to_load_info)(const int index);

// Fonts can be manually loaded by the user. Multiple of these might be defined by the user in case they want a font group system for example.
typedef struct {
    zfw_s_font_arrangement_info* arrangement_infos;
    zfw_t_gl_id* tex_gl_ids;
    int* tex_heights;
    int cnt;
} zfw_s_fonts;

// Shader programs can be manually loaded by the user. Their main purpose in this system is with surfaces and surface rendering, when you want to render a surface with a particular effect. Multiple of these might be defined by the user in case they want a shader program group system for example.
typedef struct {
    zfw_t_gl_id* gl_ids;
    int cnt;
} zfw_s_shader_progs;

typedef struct {
    const char* vs_fp;
    const char* fs_fp;
} zfw_s_shader_prog_file_paths;

typedef zfw_s_shader_prog_file_paths (*zfw_t_shader_prog_index_to_file_paths)(const int index);

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

typedef struct {
    zfw_s_pers_render_data* pers;
    zfw_s_rendering_state* state;
    zfw_s_vec_2d_i display_size;
} zfw_s_rendering_context;

typedef struct {
    float r;
    float g;
    float b;
    float a;
} zfw_s_color;

static inline bool ZFWIsColorValid(const zfw_s_color col) {
    return col.r >= 0.0f && col.r <= 1.0f
        && col.g >= 0.0f && col.g <= 1.0f
        && col.b >= 0.0f && col.b <= 1.0f
        && col.a >= 0.0f && col.a <= 1.0f;
}

typedef struct {
    float r;
    float g;
    float b;
} zfw_s_color_rgb;

static inline bool ZFWIsColorRGBValid(const zfw_s_color_rgb col) {
    return col.r >= 0.0f && col.r <= 1.0f
        && col.g >= 0.0f && col.g <= 1.0f
        && col.b >= 0.0f && col.b <= 1.0f;
}

static inline zfw_s_color_rgb ZFWToColorRGB(const zfw_s_color col) {
    return (zfw_s_color_rgb){col.r, col.g, col.b};
}

bool ZFWInitPersRenderData(zfw_s_pers_render_data* const render_data, const zfw_s_vec_2d_i display_size);
void ZFWCleanPersRenderData(zfw_s_pers_render_data* const render_data);

zfw_s_render_batch_shader_prog ZFWLoadRenderBatchShaderProg();
zfw_s_render_batch_gl_ids ZFWGenRenderBatch();

bool ZFWLoadTexturesFromFiles(zfw_s_textures* const textures, zfw_s_mem_arena* const mem_arena, const int tex_cnt, const zfw_t_texture_index_to_file_path tex_index_to_fp);
void ZFWUnloadTextures(zfw_s_textures* const textures);

bool ZFWLoadFontsFromFiles(zfw_s_fonts* const fonts, zfw_s_mem_arena* const mem_arena, const int font_cnt, const t_font_index_to_load_info font_index_to_load_info, zfw_s_mem_arena* const temp_mem_arena);
void ZFWUnloadFonts(zfw_s_fonts* const fonts);

bool ZFWLoadShaderProgsFromFiles(zfw_s_shader_progs* const progs, zfw_s_mem_arena* const mem_arena, const int prog_cnt, const zfw_t_shader_prog_index_to_file_paths prog_index_to_fps, zfw_s_mem_arena* const temp_mem_arena);
void ZFWUnloadShaderProgs(zfw_s_shader_progs* const progs);

void ZFWBeginRendering(zfw_s_rendering_state* const state);

void ZFWRenderClear(const zfw_s_color col);

void ZFWRender(const zfw_s_rendering_context* const context, const zfw_t_gl_id tex_gl_id, const zfw_s_rect_edges tex_coords, const zfw_s_vec_2d pos, const zfw_s_vec_2d size, const zfw_s_vec_2d origin, const float rot, const zfw_s_color blend);
void ZFWRenderTexture(const zfw_s_rendering_context* const context, const int tex_index, const zfw_s_textures* const textures, const zfw_s_rect_i src_rect, const zfw_s_vec_2d pos, const zfw_s_vec_2d origin, const zfw_s_vec_2d scale, const float rot, const zfw_s_color blend);
bool ZFWRenderStr(const zfw_s_rendering_context* const context, const char* const str, const int font_index, const zfw_s_fonts* const fonts, const zfw_s_vec_2d pos, const zfw_e_str_hor_align hor_align, const zfw_e_str_ver_align ver_align, const zfw_s_color blend, zfw_s_mem_arena* const temp_mem_arena);
void ZFWRenderRect(const zfw_s_rendering_context* const context, const zfw_s_rect rect, const zfw_s_color blend);
void ZFWRenderRectOutline(const zfw_s_rendering_context* const context, const zfw_s_rect rect, const zfw_s_color blend, const float thickness);
void ZFWRenderLine(const zfw_s_rendering_context* const context, const zfw_s_vec_2d a, const zfw_s_vec_2d b, const zfw_s_color blend, const float width);
void ZFWRenderPolyOutline(const zfw_s_rendering_context* const context, const zfw_s_poly poly, const zfw_s_color blend, const float width);
void ZFWRenderBarHor(const zfw_s_rendering_context* const context, const zfw_s_rect rect, const float perc, const zfw_s_color_rgb col_front, const zfw_s_color_rgb col_back);

void ZFWSetSurface(const zfw_s_rendering_context* const rendering_context, const int surf_index);
void ZFWUnsetSurface(const zfw_s_rendering_context* const rendering_context);
void ZFWSetSurfaceShaderProg(const zfw_s_rendering_context* const rendering_context, const zfw_t_gl_id gl_id);
void ZFWSetSurfaceShaderProgUniform(const zfw_s_rendering_context* const rendering_context, const char* const name, const zfw_s_shader_prog_uniform_value val);
void ZFWRenderSurface(const zfw_s_rendering_context* const rendering_context, const int surf_index);

void ZFWFlush(const zfw_s_rendering_context* const context);

bool ZFWInitRenderSurfaces(zfw_s_render_surfaces* const surfs, const zfw_s_vec_2d_i size);
bool ZFWResizeRenderSurfaces(zfw_s_render_surfaces* const surfs, const zfw_s_vec_2d_i size);

zfw_s_rect_edges ZFWCalcTextureCoords(const zfw_s_rect_i src_rect, const zfw_s_vec_2d_i tex_size);

const zfw_s_vec_2d* ZFWPushStrChrPositions(const char* const str, zfw_s_mem_arena* const mem_arena, const int font_index, const zfw_s_fonts* const fonts, const zfw_s_vec_2d pos, const zfw_e_str_hor_align hor_align, const zfw_e_str_ver_align ver_align);

bool ZFWLoadStrCollider(zfw_s_rect* const rect, const char* const str, const int font_index, const zfw_s_fonts* const fonts, const zfw_s_vec_2d pos, const zfw_e_str_hor_align hor_align, const zfw_e_str_ver_align ver_align, zfw_s_mem_arena* const temp_mem_arena);

static inline bool ZFWIsOriginValid(const zfw_s_vec_2d origin) {
    return origin.x >= 0.0f && origin.y >= 0.0f && origin.x <= 1.0f && origin.y <= 1.0f;
}

static inline bool ZFWHasFlushed(const zfw_s_rendering_state* const rs) {
    return rs->batch_slots_used_cnt == 0;
}

#endif
