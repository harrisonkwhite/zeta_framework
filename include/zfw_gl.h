#ifndef ZFW_GL_H
#define ZFW_GL_H

#include <glad/glad.h>
#include <cu.h>
#include "zfw_math.h"

#define ZFW_GL_VERSION_MAJOR 4
#define ZFW_GL_VERSION_MINOR 3

#define ZFW_ASCII_PRINTABLE_MIN ' '
#define ZFW_ASCII_PRINTABLE_MAX '~'
#define ZFW_ASCII_PRINTABLE_RANGE_LEN (ZFW_ASCII_PRINTABLE_MAX - ZFW_ASCII_PRINTABLE_MIN + 1)

#define ZFW_WHITE (zfw_u_vec_4d){1.0f, 1.0f, 1.0f, 1.0f}
#define ZFW_RED (zfw_u_vec_4d){1.0f, 0.0f, 0.0f, 1.0f}
#define ZFW_GREEN (zfw_u_vec_4d){0.0f, 1.0f, 0.0f, 1.0f}
#define ZFW_BLUE (zfw_u_vec_4d){0.0f, 0.0f, 1.0f, 1.0f}
#define ZFW_BLACK (zfw_u_vec_4d){0.0f, 0.0f, 0.0f, 1.0f}
#define ZFW_YELLOW (zfw_u_vec_4d){1.0f, 1.0f, 0.0f, 1.0f}
#define ZFW_CYAN (zfw_u_vec_4d){0.0f, 1.0f, 1.0f, 1.0f}
#define ZFW_MAGENTA (zfw_u_vec_4d){1.0f, 0.0f, 1.0f, 1.0f}
#define ZFW_GRAY (zfw_u_vec_4d){0.5f, 0.5f, 0.5f, 1.0f}

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

typedef enum {
    zfw_ek_gl_resource_type_texture,
    zfw_ek_gl_resource_type_shader_prog,
    zfw_ek_gl_resource_type_vert_array,
    zfw_ek_gl_resource_type_vert_buf,
    zfw_ek_gl_resource_type_elem_buf,
    zfw_ek_gl_resource_type_framebuffer,

    zfw_eks_gl_resource_type_cnt
} zfw_e_gl_resource_type;

typedef struct {
    zfw_t_gl_id* ids;
    zfw_e_gl_resource_type* res_types;

    int res_used;
    int res_limit;
} zfw_s_gl_resource_arena;

static inline void ZFW_AssertGLResourceArenaValidity(const zfw_s_gl_resource_arena* const res_arena) {
    assert(res_arena);

    assert(res_arena->ids);
    assert(res_arena->res_types);
    assert(res_arena->res_used >= 0);
    assert(res_arena->res_limit > 0);
    assert(res_arena->res_used <= res_arena->res_limit);
}

typedef struct {
    const zfw_t_gl_id* gl_ids;
    const zfw_s_vec_2d_int* sizes;

    int cnt;
} zfw_s_texture_group;

static inline void ZFW_AssertTextureGroupValidity(const zfw_s_texture_group* const tex_group) {
    assert(tex_group);

    assert(tex_group->gl_ids);
    assert(tex_group->sizes);
    assert(tex_group->cnt > 0);

    for (int i = 0; i < tex_group->cnt; i++) {
        assert(glIsTexture(tex_group->gl_ids[i]));
        assert(tex_group->sizes[i].x > 0 && tex_group->sizes[i].y > 0);
    }
}

typedef struct {
    const t_byte* rgba_px_data;
    zfw_s_vec_2d_int tex_size;
} zfw_s_texture_info;

static inline void ZFW_AssertTextureInfoValidity(const zfw_s_texture_info* const tex_info) {
    assert(tex_info);

    assert(tex_info->rgba_px_data);
    assert(tex_info->tex_size.x > 0 && tex_info->tex_size.y > 0);
}

typedef zfw_s_texture_info (*zfw_t_gen_texture_info_func)(const int tex_index, s_mem_arena* const mem_arena);

typedef struct {
    int line_height;

    zfw_s_vec_2d_int chr_offsets[ZFW_ASCII_PRINTABLE_RANGE_LEN];
    zfw_s_vec_2d_int chr_sizes[ZFW_ASCII_PRINTABLE_RANGE_LEN];
    int chr_advances[ZFW_ASCII_PRINTABLE_RANGE_LEN];
} zfw_s_font_arrangement_info;

static inline void ZFW_AssertFontArrangementInfoValidity(const zfw_s_font_arrangement_info* const arrangement_info) {
    assert(arrangement_info);

    assert(arrangement_info->line_height > 0);

    for (int i = 1; i < ZFW_ASCII_PRINTABLE_RANGE_LEN; i++) {
        assert(arrangement_info->chr_sizes[i].x > 0 && arrangement_info->chr_sizes[i].y > 0);
        assert(arrangement_info->chr_advances[i] >= 0);
    }
}

typedef zfw_s_vec_2d_int zfw_t_tex_chr_positions[ZFW_ASCII_PRINTABLE_RANGE_LEN];

typedef struct {
    const zfw_s_font_arrangement_info* arrangement_infos;
    const zfw_t_gl_id* tex_gl_ids;
    const zfw_s_vec_2d_int* tex_sizes;
    const zfw_t_tex_chr_positions* tex_chr_positions;

    int cnt;
} zfw_s_font_group;

static inline void ZFW_AssertFontGroupValidity(const zfw_s_font_group* const font_group) {
    assert(font_group);

    assert(font_group->arrangement_infos);
    assert(font_group->tex_gl_ids);
    assert(font_group->tex_sizes);
    assert(font_group->tex_chr_positions);
    assert(font_group->cnt > 0);

    for (int i = 0; i < font_group->cnt; i++) {
        assert(glIsTexture(font_group->tex_gl_ids[i]));
        assert(font_group->tex_sizes[i].x > 0 && font_group->tex_sizes[i].y > 0);
        ZFW_AssertFontArrangementInfoValidity(&font_group->arrangement_infos[i]);
    }
}

typedef struct {
    const char* file_path;
    int height;
} zfw_s_font_info;

static inline void ZFW_AssertFontInfoValidity(const zfw_s_font_info* const info) {
    assert(info);

    assert(info->file_path);
    assert(info->height > 0);
}

typedef struct {
    const zfw_t_gl_id* gl_ids;
    int cnt;
} zfw_s_shader_prog_group;

static inline void ZFW_AssertShaderProgGroupValidity(const zfw_s_shader_prog_group* const prog_group) {
    assert(prog_group);

    assert(prog_group->gl_ids);
    assert(prog_group->cnt > 0);

    for (int i = 0; i < prog_group->cnt; i++) {
        assert(glIsProgram(prog_group->gl_ids[i]));
    }
}

typedef struct {
    bool is_srcs;

    union {
        struct {
            const char* vs_src;
            const char* fs_src;
        };

        struct {
            const char* vs_file_path;
            const char* fs_file_path;
        };
    };
} zfw_s_shader_prog_gen_info;

static inline void ZFW_AssertShaderProgGenInfoValidity(const zfw_s_shader_prog_gen_info* const gen_info) {
    assert(gen_info);

    assert(gen_info->vs_src);
    assert(gen_info->fs_src);
}

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
        zfw_u_vec_3d as_v3;
        zfw_u_vec_4d as_v4;
        zfw_s_matrix_4x4 as_mat4x4;
    };
} zfw_s_shader_prog_uniform_value;

typedef struct {
    const zfw_t_gl_id* vert_array_gl_ids;
    const zfw_t_gl_id* vert_buf_gl_ids;
    const zfw_t_gl_id* elem_buf_gl_ids;

    int cnt;
} zfw_s_renderables;

static inline void ZFW_AssertRenderablesValidity(const zfw_s_renderables* const renderables) {
    assert(renderables);

    assert(renderables->vert_array_gl_ids);
    assert(renderables->vert_buf_gl_ids);
    assert(renderables->elem_buf_gl_ids);
    assert(renderables->cnt > 0);

    for (int i = 0; i < renderables->cnt; i++) {
        assert(glIsVertexArray(renderables->vert_array_gl_ids[i]));
        assert(glIsBuffer(renderables->vert_buf_gl_ids[i]));
        assert(glIsBuffer(renderables->elem_buf_gl_ids[i]));
    }
}

typedef struct {
    const zfw_t_gl_id* fb_gl_ids;
    zfw_t_gl_id* fb_tex_gl_ids;
    zfw_s_vec_2d_int* sizes;

    int cnt;
} zfw_s_surface_group;

static inline void ZFW_AssertSurfaceGroupValidity(const zfw_s_surface_group* const surfs) {
    assert(surfs);

    assert(surfs->fb_gl_ids);
    assert(surfs->fb_tex_gl_ids);
    assert(surfs->sizes);
    assert(surfs->cnt > 0);

    for (int i = 0; i < surfs->cnt; i++) {
        assert(glIsFramebuffer(surfs->fb_gl_ids[i]));
        assert(glIsTexture(surfs->fb_tex_gl_ids[i]));
        assert(surfs->sizes[i].x > 0 && surfs->sizes[i].y > 0);
    }
}

zfw_s_gl_resource_arena ZFW_GenGLResourceArena(s_mem_arena* const mem_arena, const int res_limit);
void ZFW_CleanGLResourceArena(zfw_s_gl_resource_arena* const res_arena);
zfw_t_gl_id* ZFW_ReserveGLIDs(zfw_s_gl_resource_arena* const res_arena, const int cnt, const zfw_e_gl_resource_type res_type);

zfw_s_texture_info ZFW_GenTextureInfoFromFile(const char* const file_path, s_mem_arena* const mem_arena);
zfw_s_texture_group ZFW_GenTextures(const int tex_cnt, const zfw_t_gen_texture_info_func gen_tex_info_func, zfw_s_gl_resource_arena* const gl_res_arena, s_mem_arena* const mem_arena, s_mem_arena* const temp_mem_arena);
zfw_s_rect_edges ZFW_TextureCoords(const zfw_s_rect_int src_rect, const zfw_s_vec_2d_int tex_size);

zfw_s_font_group ZFW_GenFonts(const int font_cnt, const zfw_s_font_info* const font_infos, zfw_s_gl_resource_arena* const gl_res_arena, s_mem_arena* const mem_arena, s_mem_arena* const temp_mem_arena);
zfw_s_vec_2d* ZFW_PushStrChrRenderPositions(s_mem_arena* const mem_arena, const char* const str, const zfw_s_font_group* const fonts, const int font_index, const zfw_s_vec_2d pos, const zfw_s_vec_2d alignment);
bool ZFW_LoadStrCollider(zfw_s_rect* const rect, const char* const str, const zfw_s_font_group* const fonts, const int font_index, const zfw_s_vec_2d pos, const zfw_s_vec_2d alignment, s_mem_arena* const temp_mem_arena);

zfw_s_shader_prog_group ZFW_GenShaderProgs(const int prog_cnt, const zfw_s_shader_prog_gen_info* const prog_gen_infos, zfw_s_gl_resource_arena* const gl_res_arena, s_mem_arena* const temp_mem_arena);

void ZFW_GenRenderable(zfw_t_gl_id* const va_gl_id, zfw_t_gl_id* const vb_gl_id, zfw_t_gl_id* const eb_gl_id, const float* const vert_buf, const size_t vert_buf_size, const unsigned short* const elem_buf, const size_t elem_buf_size, const int* const vert_attr_lens, const int vert_attr_cnt);

zfw_s_surface_group ZFW_GenSurfaces(const int surf_cnt, const zfw_s_vec_2d_int* const surf_sizes, zfw_s_gl_resource_arena* const gl_res_arena, s_mem_arena* const mem_arena);
bool ZFW_ResizeSurface(zfw_s_surface_group* const surfs, const int surf_index, const zfw_s_vec_2d_int size);

static inline bool ZFW_IsOriginValid(const zfw_s_vec_2d orig) {
    return orig.x >= 0.0f && orig.x <= 1.0f && orig.y >= 0.0f && orig.y <= 1.0f;
}

static inline bool ZFW_IsTextureCoordsValid(const zfw_s_rect_edges coords) {
    return coords.left >= 0.0f && coords.top >= 0.0f && coords.right <= 1.0f && coords.bottom <= 1.0f;
}

static bool ZFW_IsSrcRectValid(const zfw_s_rect_int src_rect, const zfw_s_vec_2d_int tex_size) {
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

static inline bool ZFW_IsStrAlignmentValid(const zfw_s_vec_2d alignment) {
    return alignment.x >= 0.0f && alignment.x <= 1.0f
        && alignment.y >= 0.0f && alignment.y <= 1.0f;
}

#endif
