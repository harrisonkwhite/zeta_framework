#pragma once

#include <climits>
#include <zfws.h>
#include <cu.h>
#include <glad/glad.h>

namespace colors {
    constexpr u_v4 g_black = {0.0f, 0.0f, 0.0f, 1.0f};
    constexpr u_v4 g_dark_gray = {0.25f, 0.25f, 0.25f, 1.0f};
    constexpr u_v4 g_gray = {0.5f, 0.5f, 0.5f, 1.0f};
    constexpr u_v4 g_light_gray = {0.75f, 0.75f, 0.75f, 1.0f};
    constexpr u_v4 g_white = {1.0f, 1.0f, 1.0f, 1.0f};
    constexpr u_v4 g_red = {1.0f, 0.0f, 0.0f, 1.0f};
    constexpr u_v4 g_orange = {1.0f, 0.5f, 0.0f, 1.0f};
    constexpr u_v4 g_yellow = {1.0f, 1.0f, 0.0f, 1.0f};
    constexpr u_v4 g_lime = {0.75f, 1.0f, 0.0f, 1.0f};
    constexpr u_v4 g_green = {0.0f, 1.0f, 0.0f, 1.0f};
    constexpr u_v4 g_teal = {0.0f, 0.5f, 0.5f, 1.0f};
    constexpr u_v4 g_cyan = {0.0f, 1.0f, 1.0f, 1.0f};
    constexpr u_v4 g_blue = {0.0f, 0.0f, 1.0f, 1.0f};
    constexpr u_v4 g_purple = {0.5f, 0.0f, 0.5f, 1.0f};
    constexpr u_v4 g_magenta = {1.0f, 0.0f, 1.0f, 1.0f};
    constexpr u_v4 g_pink = {1.0f, 0.75f, 0.8f, 1.0f};
    constexpr u_v4 g_brown = {0.6f, 0.3f, 0.0f, 1.0f};
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

DEF_ARRAY_TYPE(t_gl_id, gl_id, GLID);

enum e_gl_resource_type {
    ek_gl_resource_type_texture,
    ek_gl_resource_type_shader_prog,
    ek_gl_resource_type_vert_array,
    ek_gl_resource_type_vert_buf,
    ek_gl_resource_type_elem_buf,
    ek_gl_resource_type_framebuffer,

    eks_gl_resource_type_cnt
};

DEF_ARRAY_TYPE(e_gl_resource_type, gl_resource_type, GLResourceType);

struct s_gl_resource_arena {
public:
    [[nodiscard]]
    bool Init();
    void Clean();

    cu::c_array<t_gl_id> Push(t_s32 cnt, e_gl_resource_type res_type);

private:
    s_gl_id_array ids;
    s_gl_resource_type_array res_types;

    t_s32 res_used;
    t_s32 res_limit;
};

struct s_texture_group {
    s_v2_s32_array_view sizes;
    s_gl_id_array_view gl_ids;
};

using t_texture_group_rgba_generator_func = s_rgba_texture (*)(t_s32 tex_index, c_mem_arena& mem_arena);

enum e_builtin_texture {
    ek_builtin_texture_pixel,
    eks_builtin_texture_cnt
};

struct s_font_group {
    s_font_arrangement_array_view arrangements;
    s_font_texture_meta_array_view tex_metas;
    s_gl_id_array_view tex_gl_ids;
};

struct s_shader_prog_group {
    s_gl_id_array_view gl_ids;
};

struct s_shader_prog_gen_info {
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
};

DEF_ARRAY_TYPE(s_shader_prog_gen_info, shader_prog_gen_info, ShaderProgGenInfo);

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
        t_r32 as_r32;
        s_v2 as_v2;
        u_v3 as_v3;
        u_v4 as_v4;
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
    const t_gl_id* vert_array_gl_id;
    const t_gl_id* vert_buf_gl_id;
    const t_gl_id* elem_buf_gl_id;
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
    t_r32 rot;
    s_v2 tex_coord;
    u_v4 blend;
};

DEF_ARRAY_TYPE(s_batch_vert, batch_vert_array, BatchVert);

constexpr t_s32 g_batch_slot_cnt = 8192;
constexpr t_s32 g_batch_slot_vert_len = sizeof(s_batch_vert) / sizeof(t_r32);
constexpr t_s32 g_batch_slot_vert_cnt = 4;
constexpr t_s32 g_batch_slot_elem_cnt = 6;
static_assert(g_batch_slot_elem_cnt * g_batch_slot_cnt <= USHRT_MAX, "Batch slot count is too high!");

using t_batch_slot = s_batch_vert[g_batch_slot_vert_cnt];

struct s_batch_slot_write_info {
    t_gl_id tex_gl_id;
    s_rect_edges tex_coords;
    s_v2 pos;
    s_v2 size;
    s_v2 origin;
    t_r32 rot;
    u_v4 blend;
};

struct s_batch_state {
    t_batch_slot slots[g_batch_slot_cnt];
    t_s32 num_slots_used;

    t_gl_id tex_gl_id;
};

struct s_surface_framebuffer_gl_id_stack {
    t_gl_id buf[256];
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
    const t_gl_id* fb_gl_id;
    t_gl_id* fb_tex_gl_id;

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
s_gl_id_array PushToGLResourceArena(s_gl_resource_arena& res_arena, t_s32 cnt, e_gl_resource_type res_type);

[[nodiscard]] bool InitRenderingBasis(s_rendering_basis& basis, s_gl_resource_arena& gl_res_arena, c_mem_arena& mem_arena, c_mem_arena& temp_mem_arena);
void InitRenderingState(s_rendering_state& state, s_v2_s32 window_size);

void Clear(const s_rendering_context& rendering_context, u_v4 col);
void SetViewMatrix(const s_rendering_context& rendering_context, const s_matrix_4x4& mat);
void Render(const s_rendering_context& rendering_context, const s_batch_slot_write_info& write_info);
void SubmitBatch(const s_rendering_context& rendering_context);

//
// zfwc_textures.c
//
s_rect_edges GenTextureCoords(s_rect_s32 src_rect, s_v2_s32 tex_size);
s_rgba_texture LoadRGBATextureFromPackedFile(s_char_array_view file_path, c_mem_arena& mem_arena);
t_gl_id GenGLTextureFromRGBA(const s_rgba_texture& rgba_tex);
[[nodiscard]] bool InitTextureGroup(s_texture_group& texture_group, t_s32 tex_cnt, t_texture_group_rgba_generator_func rgba_generator_func, c_mem_arena& mem_arena, s_gl_resource_arena& gl_res_arena, c_mem_arena& temp_mem_arena);
void RenderTexture(const s_rendering_context& rendering_context, const s_texture_group& textures, t_s32 tex_index, s_rect_s32 src_rect, s_v2 pos, s_v2 origin, s_v2 scale, t_r32 rot, u_v4 blend);
void RenderRectWithOutline(const s_rendering_context& rendering_context, s_rect rect, u_v4 fill_color, u_v4 outline_color, t_r32 outline_thickness);
void RenderRectWithOutlineAndOpaqueFill(const s_rendering_context& rendering_context, s_rect rect, u_v3 fill_color, u_v4 outline_color, t_r32 outline_thickness);
void RenderBarHor(const s_rendering_context& rendering_context, s_rect rect, t_r32 perc, u_v4 front_color, u_v4 bg_color);
void RenderBarVertical(const s_rendering_context& rendering_context, s_rect rect, t_r32 perc, u_v4 front_color, u_v4 bg_color);

static inline void RenderRect(const s_rendering_context& rendering_context, s_rect rect, u_v4 color) {
    RenderTexture(rendering_context, rendering_context.basis.builtin_textures, ek_builtin_texture_pixel, s_rect_s32{0}, RectPos(rect), s_v2{0}, RectSize(rect), 0, color);
}

static inline void RenderBarHorReverse(const s_rendering_context& rendering_context, s_rect rect, t_r32 perc, u_v4 front_color, u_v4 bg_color) {
    RenderBarHor(rendering_context, rect, 1.0f - perc, bg_color, front_color);
}

static inline void RenderBarVerticalReverse(const s_rendering_context& rendering_context, s_rect rect, t_r32 perc, u_v4 front_color, u_v4 bg_color) {
    RenderBarVertical(rendering_context, rect, 1.0f - perc, bg_color, front_color);
}

static inline void RenderLine(const s_rendering_context& rendering_context, s_v2 a, s_v2 b, u_v4 blend, t_r32 width) {
    const s_v2 diff{b.x - a.x, b.y - a.y};
    const t_r32 len = sqrtf((diff.x * diff.x) + (diff.y * diff.y));

    RenderTexture(rendering_context, rendering_context.basis.builtin_textures, ek_builtin_texture_pixel, s_rect_s32{0}, a, s_v2{0.0f, 0.5f}, s_v2{len, width}, atan2f(-diff.y, diff.x), blend);
}

//
// zfwc_fonts.c
//
[[nodiscard]] bool InitFontGroupFromFiles(s_font_group& font_group, const s_char_array_view_array_view file_paths, c_mem_arena& mem_arena, s_gl_resource_arena& gl_res_arena, c_mem_arena& temp_mem_arena);
[[nodiscard]] bool GenStrChrRenderPositions(s_v2_array& positions, c_mem_arena& mem_arena, const s_char_array_view str, const s_font_group& font_group, t_s32 font_index, s_v2 pos, s_v2 alignment);
[[nodiscard]] bool GenStrCollider(s_rect& rect, const s_char_array_view str, const s_font_group& font_group, t_s32 font_index, s_v2 pos, s_v2 alignment, c_mem_arena& temp_mem_arena);
[[nodiscard]] bool RenderStr(const s_rendering_context& rendering_context, const s_char_array_view str, const s_font_group& fonts, t_s32 font_index, s_v2 pos, s_v2 alignment, u_v4 color, c_mem_arena& temp_mem_arena);

//
// zfwc_shaders.c
//
extern const char g_surface_default_vert_shader_src[];
extern const char g_surface_default_frag_shader_src[];
extern const char g_surface_blend_vert_shader_src[];
extern const char g_surface_blend_frag_shader_src[];

[[nodiscard]]
bool InitShaderProgGroup(s_shader_prog_group& prog_group, s_shader_prog_gen_info_array_view gen_infos, s_gl_resource_arena& gl_res_arena, c_mem_arena& temp_mem_arena);

//
// zfwc_surfaces.c
//
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
