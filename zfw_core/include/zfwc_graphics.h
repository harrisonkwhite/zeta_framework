#pragma once

#include <climits>
#include <zfws.h>
#include <cu.h>
#include <glad/glad.h>
#include "zfwc_math.h"

namespace colors {
    constexpr s_v4 g_black = {0.0f, 0.0f, 0.0f, 1.0f};
    constexpr s_v4 g_dark_gray = {0.25f, 0.25f, 0.25f, 1.0f};
    constexpr s_v4 g_gray = {0.5f, 0.5f, 0.5f, 1.0f};
    constexpr s_v4 g_light_gray = {0.75f, 0.75f, 0.75f, 1.0f};
    constexpr s_v4 g_white = {1.0f, 1.0f, 1.0f, 1.0f};
    constexpr s_v4 g_red = {1.0f, 0.0f, 0.0f, 1.0f};
    constexpr s_v4 g_orange = {1.0f, 0.5f, 0.0f, 1.0f};
    constexpr s_v4 g_yellow = {1.0f, 1.0f, 0.0f, 1.0f};
    constexpr s_v4 g_lime = {0.75f, 1.0f, 0.0f, 1.0f};
    constexpr s_v4 g_green = {0.0f, 1.0f, 0.0f, 1.0f};
    constexpr s_v4 g_teal = {0.0f, 0.5f, 0.5f, 1.0f};
    constexpr s_v4 g_cyan = {0.0f, 1.0f, 1.0f, 1.0f};
    constexpr s_v4 g_blue = {0.0f, 0.0f, 1.0f, 1.0f};
    constexpr s_v4 g_purple = {0.5f, 0.0f, 0.5f, 1.0f};
    constexpr s_v4 g_magenta = {1.0f, 0.0f, 1.0f, 1.0f};
    constexpr s_v4 g_pink = {1.0f, 0.75f, 0.8f, 1.0f};
    constexpr s_v4 g_brown = {0.6f, 0.3f, 0.0f, 1.0f};
}

namespace origins {
    constexpr s_v2 g_origin_top_left = {0.0f, 0.0f};
    constexpr s_v2 g_origin_top_center = {0.5f, 0.0f};
    constexpr s_v2 g_origin_top_right = {1.0f, 0.0f};
    constexpr s_v2 g_origin_center_left = {0.0f, 0.5f};
    constexpr s_v2 g_origin_center = {0.5f, 0.5f};
    constexpr s_v2 g_origin_center_right = {1.0f, 0.5f};
    constexpr s_v2 g_origin_bottom_left = {0.0f, 1.0f};
    constexpr s_v2 g_origin_bottom_center = {0.5f, 1.0f};
    constexpr s_v2 g_origin_bottom_right = {1.0f, 1.0f};
}

namespace alignments {
    constexpr s_v2 g_alignment_top_left = {0.0f, 0.0f};
    constexpr s_v2 g_alignment_top_center = {0.5f, 0.0f};
    constexpr s_v2 g_alignment_top_right = {1.0f, 0.0f};
    constexpr s_v2 g_alignment_center_left = {0.0f, 0.5f};
    constexpr s_v2 g_alignment_center = {0.5f, 0.5f};
    constexpr s_v2 g_alignment_center_right = {1.0f, 0.5f};
    constexpr s_v2 g_alignment_bottom_left = {0.0f, 1.0f};
    constexpr s_v2 g_alignment_bottom_center = {0.5f, 1.0f};
    constexpr s_v2 g_alignment_bottom_right = {1.0f, 1.0f};
}

constexpr t_s32 g_gl_version_major = 4;
constexpr t_s32 g_gl_version_minor = 3;

using t_gl_id = t_u32;

enum e_gl_resource_type {
    ek_gl_resource_type_texture,
    ek_gl_resource_type_shader_prog,
    ek_gl_resource_type_vert_array,
    ek_gl_resource_type_vert_buf,
    ek_gl_resource_type_elem_buf,
    ek_gl_resource_type_framebuffer,

    eks_gl_resource_type_cnt
};

struct s_gl_resource_arena {
    c_array<t_gl_id> ids;
    c_array<e_gl_resource_type> res_types;

    t_s32 res_used;
    t_s32 res_limit;
};

struct s_texture_group {
    c_array<s_v2_s32> sizes;
    c_array<t_gl_id> gl_ids;
};

using t_texture_group_rgba_loader_func = bool (*)(s_rgba_texture& rgba, const t_s32 tex_index, c_mem_arena& mem_arena);

enum e_builtin_texture {
    ek_builtin_texture_pixel,
    eks_builtin_texture_cnt
};

struct s_font_group {
    c_array<const s_font_arrangement> arrangements;
    c_array<const s_font_texture_meta> tex_metas;
    c_array<const t_gl_id> tex_gl_ids;
};

struct s_shader_prog_group {
    c_array<const t_gl_id> gl_ids;
};

struct s_shader_prog_gen_info {
    bool holds_srcs;

    union {
        struct {
            c_string_view file_path;
        };

        struct {
            c_string_view vert_src;
            c_string_view frag_src;
        };
    };
};

enum e_shader_prog_uniform_value_type {
    ek_shader_prog_uniform_value_type_s32,
    ek_shader_prog_uniform_value_type_r32,
    ek_shader_prog_uniform_value_type_v2,
    ek_shader_prog_uniform_value_type_v3,
    ek_shader_prog_uniform_value_type_v4,
    ek_shader_prog_uniform_value_type_mat4x4,
};

struct s_shader_prog_uniform_value {
    e_shader_prog_uniform_value_type type;

    union {
        t_s32 as_s32;
        float as_r32;
        s_v2 as_v2;
        s_v3 as_v3;
        s_v4 as_v4;
        s_matrix_4x4 as_mat4x4;
    };
};

enum e_builtin_shader_prog {
    ek_builtin_shader_prog_batch,
    ek_builtin_shader_prog_surface_default,
    ek_builtin_shader_prog_surface_blend,

    eks_builtin_shader_prog_cnt
};

enum e_renderable {
    ek_renderable_batch,
    ek_renderable_surface,

    eks_renderable_cnt
};

struct s_renderable {
    t_gl_id vert_array_gl_id;
    t_gl_id vert_buf_gl_id;
    t_gl_id elem_buf_gl_id;
};

struct s_rendering_basis {
    s_texture_group builtin_textures;
    s_shader_prog_group builtin_shader_progs;
    s_renderable renderables[eks_renderable_cnt];
};

struct s_batch_vert {
    s_v2 vert_coord;
    s_v2 pos;
    s_v2 size;
    float rot;
    s_v2 tex_coord;
    s_v4 blend;
};

constexpr t_s32 g_batch_slot_cnt = 8192;
constexpr t_s32 g_batch_slot_vert_len = sizeof(s_batch_vert) / sizeof(float);
constexpr t_s32 g_batch_slot_vert_cnt = 4;
constexpr t_s32 g_batch_slot_elem_cnt = 6;
static_assert(g_batch_slot_elem_cnt * g_batch_slot_cnt <= USHRT_MAX, "Batch slot count is too high!");

using t_batch_slot = s_static_array<s_batch_vert, g_batch_slot_vert_cnt>;

struct s_batch_slot_write_info {
    t_gl_id tex_gl_id;
    s_rect_edges tex_coords;
    s_v2 pos;
    s_v2 size;
    s_v2 origin;
    float rot;
    s_v4 blend;
};

struct s_batch_state {
    t_batch_slot slots[g_batch_slot_cnt];
    t_s32 num_slots_used;

    t_gl_id tex_gl_id;
};

struct s_surface_framebuffer_gl_id_stack {
    s_static_array<t_gl_id, 256> buf;
    int height;
};

struct s_rendering_state {
    s_batch_state batch;
    s_matrix_4x4 view_mat;
    s_surface_framebuffer_gl_id_stack surf_fb_gl_id_stack;
};

struct s_rendering_context {
    const s_rendering_basis& basis;
    s_rendering_state& state;

    s_v2_s32 window_size;
};

struct s_surface {
    t_gl_id fb_gl_id;
    t_gl_id fb_tex_gl_id;

    s_v2_s32 size;
};

static inline t_gl_id BoundGLFramebuffer() {
    t_s32 fb;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fb);
    return static_cast<t_gl_id>(fb);
}

static inline s_rect_s32 GLViewport() {
    s_rect_s32 vp;
    glGetIntegerv(GL_VIEWPORT, reinterpret_cast<t_s32*>(&vp));
    return vp;
}

//
// zfwc_graphics.c
//
[[nodiscard]] bool InitGLResourceArena(s_gl_resource_arena& res_arena, c_mem_arena& mem_arena, t_s32 res_limit);
void CleanGLResourceArena(s_gl_resource_arena& res_arena);
t_gl_id PushToGLResourceArena(s_gl_resource_arena& res_arena, e_gl_resource_type res_type);
c_array<t_gl_id> PushArrayToGLResourceArena(s_gl_resource_arena& res_arena, t_s32 cnt, e_gl_resource_type res_type);

[[nodiscard]] bool InitRenderingBasis(s_rendering_basis& basis, s_gl_resource_arena& gl_res_arena, c_mem_arena& mem_arena, c_mem_arena& temp_mem_arena);
void InitRenderingState(s_rendering_state& state, s_v2_s32 window_size);

void Clear(const s_rendering_context& rendering_context, s_v4 col);
void SetViewMatrix(const s_rendering_context& rendering_context, const s_matrix_4x4& mat);
void Render(const s_rendering_context& rendering_context, const s_batch_slot_write_info& write_info);
void SubmitBatch(const s_rendering_context& rendering_context);

//
// zfwc_textures.c
//
s_rect_edges GenTextureCoords(s_rect_s32 src_rect, s_v2_s32 tex_size);
[[nodiscard]] bool LoadRGBATextureFromPackedFile(s_rgba_texture& tex, c_string_view file_path, c_mem_arena& mem_arena);
t_gl_id GenGLTextureFromRGBA(const s_rgba_texture& rgba_tex);
[[nodiscard]] bool InitTextureGroup(s_texture_group& texture_group, t_s32 tex_cnt, t_texture_group_rgba_loader_func rgba_loader_func, c_mem_arena& mem_arena, s_gl_resource_arena& gl_res_arena, c_mem_arena& temp_mem_arena);
void RenderTexture(const s_rendering_context& rendering_context, const s_texture_group& textures, t_s32 tex_index, s_rect_s32 src_rect, s_v2 pos, s_v2 origin, s_v2 scale, float rot, s_v4 blend);
void RenderRectWithOutline(const s_rendering_context& rendering_context, s_rect rect, s_v4 fill_color, s_v4 outline_color, float outline_thickness);
void RenderRectWithOutlineAndOpaqueFill(const s_rendering_context& rendering_context, s_rect rect, s_v3 fill_color, s_v4 outline_color, float outline_thickness);
void RenderBarHor(const s_rendering_context& rendering_context, s_rect rect, float perc, s_v4 front_color, s_v4 bg_color);
void RenderBarVertical(const s_rendering_context& rendering_context, s_rect rect, float perc, s_v4 front_color, s_v4 bg_color);

static inline void RenderRect(const s_rendering_context& rendering_context, s_rect rect, s_v4 color) {
    RenderTexture(rendering_context, rendering_context.basis.builtin_textures, ek_builtin_texture_pixel, {}, rect.Pos(), {}, rect.Size(), 0, color);
}

static inline void RenderBarHorReverse(const s_rendering_context& rendering_context, s_rect rect, float perc, s_v4 front_color, s_v4 bg_color) {
    RenderBarHor(rendering_context, rect, 1.0f - perc, bg_color, front_color);
}

static inline void RenderBarVerticalReverse(const s_rendering_context& rendering_context, s_rect rect, float perc, s_v4 front_color, s_v4 bg_color) {
    RenderBarVertical(rendering_context, rect, 1.0f - perc, bg_color, front_color);
}

static inline void RenderLine(const s_rendering_context& rendering_context, s_v2 a, s_v2 b, s_v4 blend, float width) {
    const s_v2 diff{b.x - a.x, b.y - a.y};
    const float len = sqrtf((diff.x * diff.x) + (diff.y * diff.y));

    RenderTexture(rendering_context, rendering_context.basis.builtin_textures, ek_builtin_texture_pixel, {}, a, s_v2{0.0f, 0.5f}, s_v2{len, width}, atan2f(-diff.y, diff.x), blend);
}

//
// zfwc_fonts.c
//
[[nodiscard]] bool InitFontGroupFromFiles(s_font_group& font_group, const c_array<const c_string_view> file_paths, c_mem_arena& mem_arena, s_gl_resource_arena& gl_res_arena, c_mem_arena& temp_mem_arena);
[[nodiscard]] bool GenStrChrRenderPositions(c_array<s_v2>& positions, c_mem_arena& mem_arena, const c_string_view str, const s_font_group& font_group, t_s32 font_index, s_v2 pos, s_v2 alignment);
[[nodiscard]] bool GenStrCollider(s_rect& rect, const c_string_view str, const s_font_group& font_group, t_s32 font_index, s_v2 pos, s_v2 alignment, c_mem_arena& temp_mem_arena);
[[nodiscard]] bool RenderStr(const s_rendering_context& rendering_context, const c_string_view str, const s_font_group& fonts, t_s32 font_index, s_v2 pos, s_v2 alignment, s_v4 color, c_mem_arena& temp_mem_arena);

//
// zfwc_shaders.c
//
[[nodiscard]] bool InitShaderProgGroup(s_shader_prog_group& prog_group, c_array<const s_shader_prog_gen_info> gen_infos, s_gl_resource_arena& gl_res_arena, c_mem_arena& temp_mem_arena);

//
// zfwc_surfaces.c
//
extern const c_string_view g_surface_default_vert_shader_src;
extern const c_string_view g_surface_default_frag_shader_src;
extern const c_string_view g_surface_blend_vert_shader_src;
extern const c_string_view g_surface_blend_frag_shader_src;

[[nodiscard]] bool InitSurface(s_surface& surf, s_v2_s32 size, s_gl_resource_arena& gl_res_arena);
[[nodiscard]] bool ResizeSurface(s_surface& surf, s_v2_s32 size);
void SetSurface(const s_rendering_context& rendering_context, const s_surface& surf);
void UnsetSurface(const s_rendering_context& rendering_context);
void SetSurfaceShaderProg(const s_rendering_context& rendering_context, const s_shader_prog_group& progs, t_s32 prog_index);
void SetSurfaceShaderProgUniform(const s_rendering_context& rendering_context, const char* name, s_shader_prog_uniform_value val);
void RenderSurface(const s_rendering_context& rendering_context, const s_surface& surf, s_v2 pos, s_v2 scale, bool blend);

static inline s_v2 SurfaceTexelSize(const s_surface& surf) {
    return s_v2{1.0f / surf.size.x, 1.0f / surf.size.y};
}
