#pragma once

#include <zcl.h>
#include <glad/glad.h>

namespace zf {
    struct s_rendering_basis;
    [[nodiscard]] t_b8 InitGFX(s_mem_arena& mem_arena, s_mem_arena& temp_mem_arena, s_rendering_basis*& o_rendering_basis);
    void ShutdownGFX(const s_rendering_basis& rendering_basis);

    // ============================================================
    // @section: GFX Resources
    // ============================================================
    struct s_gfx_resource;

    struct s_gfx_resource_arena {
        s_mem_arena* mem_arena;
        s_gfx_resource* head;
        s_gfx_resource* tail;
    };

    // The memory lifetime of the given memory arena must encompass that of the GFX resource arena.
    inline s_gfx_resource_arena MakeGFXResourceArena(s_mem_arena& mem_arena) {
        return {
            .mem_arena = &mem_arena
        };
    }

    void ReleaseGFXResources(const s_gfx_resource_arena& res_arena);

    // Returns false iff the load failed. Failure DOES NOT leave the underlying resource system in an invalid state - you are safe to continue.
    [[nodiscard]] t_b8 LoadTexture(const s_rgba_texture_data_rdonly& tex_data, s_gfx_resource_arena& res_arena, s_gfx_resource*& o_tex);

    // Returns false iff the load failed. Failure DOES NOT leave the underlying resource system in an invalid state - you are safe to continue.
    [[nodiscard]] inline t_b8 LoadTextureFromRaw(const s_str_rdonly file_path, s_gfx_resource_arena& res_arena, s_mem_arena& temp_mem_arena, s_gfx_resource*& o_tex) {
        s_rgba_texture_data tex_data;

        if (!LoadRGBATextureDataFromRaw(file_path, temp_mem_arena, temp_mem_arena, tex_data)) {
            return false;
        }

        return LoadTexture(tex_data, res_arena, o_tex);
    }

    // Returns false iff the load failed. Failure DOES NOT leave the underlying resource system in an invalid state - you are safe to continue.
    [[nodiscard]] inline t_b8 LoadTextureFromPacked(const s_str_rdonly file_path, s_gfx_resource_arena& res_arena, s_mem_arena& temp_mem_arena, s_gfx_resource*& o_tex) {
        s_rgba_texture_data tex_data;

        if (!UnpackTexture(file_path, temp_mem_arena, temp_mem_arena, tex_data)) {
            return false;
        }

        return LoadTexture(tex_data, res_arena, o_tex);
    }

    s_v2<t_s32> TextureSize(const s_gfx_resource* const res);

    // Returns false iff the load failed. Failure DOES NOT leave the underlying resource system in an invalid state - you are safe to continue.
    [[nodiscard]] t_b8 LoadFontFromRaw(const s_str_rdonly file_path, const t_s32 height, const t_unicode_code_pt_bit_vec& code_pts, s_gfx_resource_arena& res_arena, s_mem_arena& temp_mem_arena, s_gfx_resource*& o_font, e_font_load_from_raw_result* const o_load_from_raw_res = nullptr, t_unicode_code_pt_bit_vec* const o_unsupported_code_pts = nullptr);

    // Returns false iff the load failed. Failure DOES NOT leave the underlying resource system in an invalid state - you are safe to continue.
    [[nodiscard]] t_b8 LoadFontFromPacked(const s_str_rdonly file_path, s_gfx_resource_arena& res_arena, s_mem_arena& temp_mem_arena, s_gfx_resource*& o_font);

    // Returns false on failure. Failure DOES NOT leave the underlying resource system in an invalid state - you are safe to continue.
    [[nodiscard]] t_b8 MakeSurface(const s_v2<t_s32> size, s_gfx_resource_arena& res_arena, s_gfx_resource*& o_surf);

    // Returns true iff the operation was successful. If it failed, the old surface state with its old size is left intact.
    [[nodiscard]] t_b8 ResizeSurface(s_gfx_resource* const surf, const s_v2<t_s32> size);

    // Returns false on failure. Failure DOES NOT leave the underlying resource system in an invalid state - you are safe to continue.
    [[nodiscard]] t_b8 MakeSurfaceShaderProg(const s_str_rdonly vert_src, const s_str_rdonly frag_src, s_gfx_resource_arena& res_arena, s_mem_arena& temp_mem_arena, s_gfx_resource*& o_prog);

    // ============================================================
    // @section: Rendering
    // ============================================================
    struct s_rendering_state;

    struct s_rendering_context {
        s_v2<t_s32> framebuffer_size_cache;

        const s_rendering_basis* basis;
        s_rendering_state* state;
    };

    [[nodiscard]] t_b8 MakeRenderer(s_mem_arena& mem_arena, s_mem_arena& temp_mem_arena, s_rendering_context*& o_renderer);
    void ShutdownGFX(const s_rendering_context& rc);
    [[nodiscard]] t_b8 BeginFrame(const s_rendering_basis& rendering_basis, const s_v2<t_s32> framebuffer_size_cache, s_mem_arena& mem_arena, s_rendering_context& o_rendering_context);
    void CompleteFrame(const s_rendering_context& rc);

    void Clear(const s_rendering_context& rc, const s_color_rgba32f col = {});
    void SetViewMatrix(const s_rendering_context& rc, const s_matrix_4x4& mat);
    void DrawTexture(const s_rendering_context& rc, const s_gfx_resource* const tex, const s_v2<t_f32> pos, const s_rect<t_s32> src_rect = {}, const s_v2<t_f32> origin = origins::g_topleft, const s_v2<t_f32> scale = {1.0f, 1.0f}, const t_f32 rot = 0.0f, const s_color_rgba32f blend = colors::g_white);
    void DrawRect(const s_rendering_context& rc, const s_rect<t_f32> rect, const s_color_rgba32f color);
    void DrawRectOpaqueOutlined(const s_rendering_context& rc, const s_rect<t_f32> rect, const s_color_rgba24f fill_color, const s_color_rgba32f outline_color, const t_f32 outline_thickness = 1.0f);
    void DrawRectRot(const s_rendering_context& rc, const s_v2<t_f32> pos, const s_v2<t_f32> size, const s_v2<t_f32> origin, const t_f32 rot, const s_color_rgba32f color);
    void DrawRectRotOpaqueOutlined(const s_rendering_context& rc, const s_v2<t_f32> pos, const s_v2<t_f32> size, const s_v2<t_f32> origin, const t_f32 rot, const s_color_rgba24f fill_color, const s_color_rgba32f outline_color, const t_f32 outline_thickness = 1.0f);
    void DrawLine(const s_rendering_context& rc, const s_v2<t_f32> a, const s_v2<t_f32> b, const s_color_rgba32f blend, const t_f32 thickness = 1.0f);
    [[nodiscard]] t_b8 DrawStr(const s_rendering_context& rc, const s_str_rdonly str, const s_gfx_resource* const font, const s_v2<t_f32> pos, const s_v2<t_f32> alignment, const s_color_rgba32f blend, s_mem_arena& temp_mem_arena);

    void SetSurface(const s_rendering_context& rc, const s_gfx_resource* const surf);
    void UnsetSurface(const s_rendering_context& rc);

    enum e_surface_shader_prog_uniform_val_type : t_s32 {
        ek_surface_shader_prog_uniform_val_type_s32,
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
            t_s32 s32;
            t_u32 u32;
            t_f32 f32;
            s_v2<t_f32> v2;
            s_v3<t_f32> v3;
            s_v4<t_f32> v4;
            s_matrix_4x4 mat4x4;
        } type_data;

        s_surface_shader_prog_uniform_val() = default;
        s_surface_shader_prog_uniform_val(const t_s32 v) : type(ek_surface_shader_prog_uniform_val_type_s32), type_data({.s32 = v}) {};
        s_surface_shader_prog_uniform_val(const t_u32 v) : type(ek_surface_shader_prog_uniform_val_type_u32), type_data({.u32 = v}) {};
        s_surface_shader_prog_uniform_val(const t_f32 v) : type(ek_surface_shader_prog_uniform_val_type_f32), type_data({.f32 = v}) {};
        s_surface_shader_prog_uniform_val(const s_v2<t_f32> v) : type(ek_surface_shader_prog_uniform_val_type_v2), type_data({.v2 = v}) {};
        s_surface_shader_prog_uniform_val(const s_v3<t_f32> v) : type(ek_surface_shader_prog_uniform_val_type_v3), type_data({.v3 = v}) {};
        s_surface_shader_prog_uniform_val(const s_v4<t_f32> v) : type(ek_surface_shader_prog_uniform_val_type_v4), type_data({.v4 = v}) {};
        s_surface_shader_prog_uniform_val(const s_matrix_4x4& v) : type(ek_surface_shader_prog_uniform_val_type_mat4x4), type_data({.mat4x4 = v}) {};
    };

    void SetSurfaceShaderProg(const s_rendering_context& rc, const s_gfx_resource* const prog);
    [[nodiscard]] t_b8 SetSurfaceShaderProgUniform(const s_rendering_context& rc, const s_str_rdonly name, const s_surface_shader_prog_uniform_val& val, s_mem_arena& temp_mem_arena);

    void DrawSurface(const s_rendering_context& rc, const s_gfx_resource* const surf, const s_v2<t_f32> pos);
}
