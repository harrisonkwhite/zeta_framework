#pragma once

#include <zcl.h>

namespace zf {
    namespace internal {
        constexpr t_i32 g_gl_version_major = 4;
        constexpr t_i32 g_gl_version_minor = 6;
        constexpr t_b8 g_gl_core_profile = true;
    }

#if 0
    struct s_rendering_basis;

    namespace internal {
        [[nodiscard]] t_b8 InitGFX(s_rendering_basis **const rendering_basis, s_mem_arena *const rendering_basis_mem_arena, s_mem_arena *const temp_mem_arena);
        void ShutdownGFX(s_rendering_basis *const rendering_basis);
    }

    // ============================================================
    // @section: Resources
    // ============================================================
    struct s_gfx_resource;

    struct s_gfx_resource_arena {
        s_mem_arena *mem_arena;
        s_gfx_resource *head;
        s_gfx_resource *tail;
    };

    // The memory lifetime of the given memory arena must encompass that of the GFX resource arena.
    inline s_gfx_resource_arena CreateGFXResourceArena(s_mem_arena *const mem_arena) {
        return {.mem_arena = mem_arena};
    }

    void DestroyGFXResources(s_gfx_resource_arena *const res_arena);

    s_gfx_resource *CreateTexture(const s_rgba_texture_data_rdonly tex_data, s_gfx_resource_arena *const res_arena);

    s_v2_i TextureSize(const s_gfx_resource *const res);

    [[nodiscard]] t_b8 LoadFontFromRaw(const s_str_rdonly file_path, const t_s32 height, const t_unicode_code_pt_bit_vec &code_pts, s_gfx_resource_arena &res_arena, s_mem_arena &temp_mem_arena, s_gfx_resource *&o_font, e_font_load_from_raw_result *const o_load_from_raw_res = nullptr, t_unicode_code_pt_bit_vec *const o_unsupported_code_pts = nullptr);
    [[nodiscard]] t_b8 LoadFontFromPacked(const s_str_rdonly file_path, s_gfx_resource_arena &res_arena, s_mem_arena &temp_mem_arena, s_gfx_resource *&o_font);

    s_gfx_resource *CreateSurface(const s_v2_i size, s_gfx_resource_arena *const res_arena);
    [[nodiscard]] t_b8 ResizeSurface(s_gfx_resource *const surf, const s_v2_i size);

    s_gfx_resource *CreateSurfaceShaderProg(const s_str_rdonly vert_src, const s_str_rdonly frag_src, s_gfx_resource_arena *const res_arena, s_mem_arena *const temp_mem_arena);

    // ============================================================
    // @section: Rendering
    // ============================================================
    struct s_rendering_state;

    struct s_rendering_context {
        const s_rendering_basis *basis;
        s_rendering_state *state;

        s_v2_i framebuffer_size_cache;
    };

    void Clear(const s_rendering_context rc, const s_color_rgba32f col = {});
    void SetViewMatrix(const s_rendering_context rc, const s_mat4x4 &mat);
    void DrawTexture(const s_rendering_context rc, const s_gfx_resource *const tex, const s_v2 pos, const s_rect_i src_rect = {}, const s_v2 origin = origins::g_topleft, const s_v2 scale = {1.0f, 1.0f}, const t_f32 rot = 0.0f, const s_color_rgba32f blend = colors::g_white);
    void DrawRect(const s_rendering_context rc, const s_rect_f rect, const s_color_rgba32f color);
    void DrawRectOpaqueOutlined(const s_rendering_context rc, const s_rect_f rect, const s_color_rgba24f fill_color, const s_color_rgba32f outline_color, const t_f32 outline_thickness = 1.0f);
    void DrawRectRot(const s_rendering_context rc, const s_v2 pos, const s_v2 size, const s_v2 origin, const t_f32 rot, const s_color_rgba32f color);
    void DrawRectRotOpaqueOutlined(const s_rendering_context rc, const s_v2 pos, const s_v2 size, const s_v2 origin, const t_f32 rot, const s_color_rgba24f fill_color, const s_color_rgba32f outline_color, const t_f32 outline_thickness = 1.0f);
    void DrawLine(const s_rendering_context rc, const s_v2 a, const s_v2 b, const s_color_rgba32f blend, const t_f32 thickness = 1.0f);
    [[nodiscard]] t_b8 DrawStr(const s_rendering_context rc, const s_str_rdonly str, const s_gfx_resource *const font, const s_v2 pos, const s_v2 alignment, const s_color_rgba32f blend, s_mem_arena &temp_mem_arena);

    void SetSurface(const s_rendering_context rc, const s_gfx_resource *const surf);
    void UnsetSurface(const s_rendering_context rc);

    enum e_surface_shader_prog_uniform_val_type : t_i32 {
        ek_surface_shader_prog_uniform_val_type_i32,
        ek_surface_shader_prog_uniform_val_type_u32,
        ek_surface_shader_prog_uniform_val_type_f32,
        ek_surface_shader_prog_uniform_val_type_v2,
        ek_surface_shader_prog_uniform_val_type_v3,
        ek_surface_shader_prog_uniform_val_type_v4,
        ek_surface_shader_prog_uniform_val_type_mat4x4
    };

    struct s_surface_shader_prog_uniform_val {
        e_surface_shader_prog_uniform_val_type type;

        union {
            t_i32 i32;
            t_u32 u32;
            t_f32 f32;
            s_v2 v2;
            s_v3 v3;
            s_v4 v4;
            s_mat4x4 mat4x4;
        } type_data;

        s_surface_shader_prog_uniform_val() = default;
        s_surface_shader_prog_uniform_val(const t_i32 v) : type(ek_surface_shader_prog_uniform_val_type_i32), type_data({.i32 = v}) {};
        s_surface_shader_prog_uniform_val(const t_u32 v) : type(ek_surface_shader_prog_uniform_val_type_u32), type_data({.u32 = v}) {};
        s_surface_shader_prog_uniform_val(const t_f32 v) : type(ek_surface_shader_prog_uniform_val_type_f32), type_data({.f32 = v}) {};
        s_surface_shader_prog_uniform_val(const s_v2 v) : type(ek_surface_shader_prog_uniform_val_type_v2), type_data({.v2 = v}) {};
        s_surface_shader_prog_uniform_val(const s_v3 v) : type(ek_surface_shader_prog_uniform_val_type_v3), type_data({.v3 = v}) {};
        s_surface_shader_prog_uniform_val(const s_v4 v) : type(ek_surface_shader_prog_uniform_val_type_v4), type_data({.v4 = v}) {};
        s_surface_shader_prog_uniform_val(const s_mat4x4 &v) : type(ek_surface_shader_prog_uniform_val_type_mat4x4), type_data({.mat4x4 = v}) {};
    };

    void SetSurfaceShaderProg(const s_rendering_context rc, const s_gfx_resource *const prog);
    [[nodiscard]] t_b8 SetSurfaceShaderProgUniform(const s_rendering_context rc, const s_str_rdonly name, const s_surface_shader_prog_uniform_val &val, s_mem_arena &temp_mem_arena);

    void DrawSurface(const s_rendering_context rc, const s_gfx_resource *const surf, const s_v2 pos);

    namespace internal {
        [[nodiscard]] t_b8 BeginFrame(s_rendering_context *const rendering_context, const s_rendering_basis *const rendering_basis, const s_v2_i framebuffer_size_cache, s_mem_arena *const mem_arena);
        void CompleteFrame(const s_rendering_context rc);
    }
#endif
}
