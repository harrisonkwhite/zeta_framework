#ifndef ZFW_GL_H
#define ZFW_GL_H

#include <glad/glad.h>
#include <cu.h>
#include "zfw_math.h"

#define ZFW_GL_VERSION_MAJOR 4
#define ZFW_GL_VERSION_MINOR 3

#define ZFW_WHITE (zfw_u_vec_4d){1.0f, 1.0f, 1.0f, 1.0f}
#define ZFW_RED (zfw_u_vec_4d){1.0f, 0.0f, 0.0f, 1.0f}
#define ZFW_GREEN (zfw_u_vec_4d){0.0f, 1.0f, 0.0f, 1.0f}
#define ZFW_BLUE (zfw_u_vec_4d){0.0f, 0.0f, 1.0f, 1.0f}
#define ZFW_BLACK (zfw_u_vec_4d){0.0f, 0.0f, 0.0f, 1.0f}
#define ZFW_YELLOW (zfw_u_vec_4d){1.0f, 1.0f, 0.0f, 1.0f}
#define ZFW_CYAN (zfw_u_vec_4d){0.0f, 1.0f, 1.0f, 1.0f}
#define ZFW_MAGENTA (zfw_u_vec_4d){1.0f, 0.0f, 1.0f, 1.0f}
#define ZFW_GRAY (zfw_u_vec_4d){0.5f, 0.5f, 0.5f, 1.0f}

typedef GLuint zfw_t_gl_id;

typedef enum {
    zfw_ek_gl_resource_type_texture,
    zfw_ek_gl_resource_type_shader_prog,
    zfw_ek_gl_resource_type_vert_array,
    zfw_ek_gl_resource_type_vert_buf,
    zfw_ek_gl_resource_type_elem_buf,

    zfw_eks_gl_resource_type_cnt
} zfw_e_gl_resource_type;

typedef struct {
    zfw_t_gl_id* ids;
    zfw_e_gl_resource_type* res_types;

    int res_used;
    int res_limit;
} zfw_s_gl_resource_arena;

typedef struct {
    const zfw_t_gl_id* gl_ids;
    const zfw_s_vec_2d_s32* sizes;

    int cnt;
} zfw_s_texture_group;

typedef struct {
    const zfw_t_gl_id* gl_ids;
    int cnt;
} zfw_s_shader_prog_group;

typedef struct {
    const zfw_t_gl_id* vert_array_gl_ids;
    const zfw_t_gl_id* vert_buf_gl_ids;
    const zfw_t_gl_id* elem_buf_gl_ids;

    int cnt;
} zfw_s_renderables;

zfw_s_gl_resource_arena ZFW_GenGLResourceArena(s_mem_arena* const mem_arena, const int res_limit);
void ZFW_CleanGLResourceArena(zfw_s_gl_resource_arena* const res_arena);
zfw_t_gl_id* ZFW_ReserveGLIDs(zfw_s_gl_resource_arena* const res_arena, const int cnt, const zfw_e_gl_resource_type res_type);

zfw_t_gl_id ZFW_GenGLTextureFromRGBAPixelData(const t_u8* const rgba_px_data, const zfw_s_vec_2d_s32 tex_size);
bool ZFW_GenGLTextureFromFile(zfw_t_gl_id* const tex_gl_id, zfw_s_vec_2d_s32* const tex_size, const char* const file_path);
zfw_s_texture_group ZFW_GenTexturesFromFiles(zfw_s_gl_resource_arena* const gl_res_arena, s_mem_arena* const mem_arena, const int tex_cnt, const char* const* const file_paths);
zfw_s_rect_edges ZFW_TextureCoords(const zfw_s_rect_s32 src_rect, const zfw_s_vec_2d_s32 tex_size);

zfw_t_gl_id ZFW_GenShaderFromSrc(const char* const src, const bool frag, s_mem_arena* const temp_mem_arena);
zfw_t_gl_id ZFW_GenShaderProg(const char* const vs_src, const char* const fs_src, s_mem_arena* const temp_mem_arena);

void ZFW_GenRenderable(zfw_t_gl_id* const va_gl_id, zfw_t_gl_id* const vb_gl_id, zfw_t_gl_id* const eb_gl_id, const float* const vert_buf, const size_t vert_buf_size, const unsigned short* const elem_buf, const size_t elem_buf_size, const int* const vert_attr_lens, const int vert_attr_cnt);

static inline bool ZFW_IsOriginValid(const zfw_s_vec_2d orig) {
    return orig.x >= 0.0f && orig.x <= 1.0f && orig.y >= 0.0f && orig.y <= 1.0f;
}

static inline bool ZFW_IsTextureCoordsValid(const zfw_s_rect_edges coords) {
    return coords.left >= 0.0f && coords.top >= 0.0f && coords.right <= 1.0f && coords.bottom <= 1.0f;
}

static bool ZFW_IsSrcRectValid(const zfw_s_rect_s32 src_rect, const zfw_s_vec_2d_s32 tex_size) {
    assert(tex_size.x > 0 && tex_size.y > 0);
    return src_rect.x >= 0 && src_rect.width > 0 && src_rect.x + src_rect.width <= tex_size.x
        && src_rect.y >= 0 && src_rect.height > 0 && src_rect.y + src_rect.height <= tex_size.y;
}

static inline bool ZFW_IsColorValid(const zfw_u_vec_4d col) {
    return col.r >= 0.0f && col.r <= 1.0f
        && col.g >= 0.0f && col.g <= 1.0f
        && col.b >= 0.0f && col.b <= 1.0f
        && col.a >= 0.0f && col.a <= 1.0f;
}

static inline bool ZFW_IsColorRGBValid(const zfw_u_vec_3d col) {
    return col.r >= 0.0f && col.r <= 1.0f
        && col.g >= 0.0f && col.g <= 1.0f
        && col.b >= 0.0f && col.b <= 1.0f;
}

#endif
