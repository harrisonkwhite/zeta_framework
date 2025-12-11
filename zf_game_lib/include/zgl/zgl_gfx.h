#pragma once

#include <zcl.h>

namespace zf {
    struct s_rendering_basis;

    namespace internal {
        constexpr t_i32 g_gl_version_major = 4;
        constexpr t_i32 g_gl_version_minor = 6;
        constexpr t_b8 g_gl_core_profile = true;

        [[nodiscard]] t_b8 InitGFX(s_mem_arena &rendering_basis_mem_arena, s_mem_arena &temp_mem_arena, s_ptr<s_rendering_basis> &o_rendering_basis);
        void ShutdownGFX(s_rendering_basis &rendering_basis);
    }

    // ============================================================
    // @section: Resources
    // ============================================================
    struct s_gfx_resource;

    struct s_gfx_resource_arena {
        s_ptr<s_mem_arena> mem_arena;
        s_ptr<s_gfx_resource> head;
        s_ptr<s_gfx_resource> tail;
    };

    // The memory lifetime of the given memory arena must encompass that of the GFX resource arena.
    inline s_gfx_resource_arena CreateGFXResourceArena(s_mem_arena &mem_arena) {
        return {.mem_arena = &mem_arena};
    }

    void DestroyGFXResources(s_gfx_resource_arena &res_arena);

    [[nodiscard]] t_b8 CreateTexture(const s_texture_data_rdonly tex_data, s_gfx_resource_arena &res_arena, s_ptr<s_gfx_resource> &o_tex);
    s_v2_i TextureSize(const s_gfx_resource &res);

    [[nodiscard]] t_b8 CreateFontFromRaw(const s_str_rdonly file_path, const t_i32 height, const t_unicode_code_pt_bit_vec &code_pts, s_gfx_resource_arena &res_arena, s_mem_arena &temp_mem_arena, s_ptr<s_gfx_resource> &o_font);
    [[nodiscard]] t_b8 CreateFontFromPacked(const s_str_rdonly file_path, s_gfx_resource_arena &res_arena, s_mem_arena &temp_mem_arena, s_ptr<s_gfx_resource> &o_font);

    [[nodiscard]] t_b8 CreateSurface(const s_v2_i size, s_gfx_resource_arena &res_arena, s_ptr<s_gfx_resource> &o_surf);
    [[nodiscard]] t_b8 ResizeSurface(s_gfx_resource &surf, const s_v2_i size);

    [[nodiscard]] t_b8 CreateSurfaceShaderProg(const s_str_rdonly vert_src, const s_str_rdonly frag_src, s_gfx_resource_arena &res_arena, s_mem_arena &temp_mem_arena, s_ptr<s_gfx_resource> &o_shader_prog);

    // ============================================================
    // @section: Rendering
    // ============================================================
    struct s_rendering_state;

    struct s_rendering_context {
        s_ptr<const s_rendering_basis> basis = nullptr;
        s_ptr<s_rendering_state> state = nullptr;

        s_v2_i framebuffer_size_cache = {};
    };

    void Clear(const s_rendering_context rc, const s_color_rgba32f col = {});
    void SetViewMatrix(const s_rendering_context rc, const s_mat4x4 &mat);
    void DrawTexture(const s_rendering_context rc, const s_gfx_resource &tex, const s_v2 pos, const s_rect_i src_rect = {}, const s_v2 origin = origins::g_topleft, const s_v2 scale = {1.0f, 1.0f}, const t_f32 rot = 0.0f, const s_color_rgba32f blend = colors::g_white);
    void DrawRect(const s_rendering_context rc, const s_rect_f rect, const s_color_rgba32f color);
    void DrawRectOpaqueOutlined(const s_rendering_context rc, const s_rect_f rect, const s_color_rgb24f fill_color, const s_color_rgba32f outline_color, const t_f32 outline_thickness = 1.0f);
    void DrawRectRot(const s_rendering_context rc, const s_v2 pos, const s_v2 size, const s_v2 origin, const t_f32 rot, const s_color_rgba32f color);
    void DrawRectRotOpaqueOutlined(const s_rendering_context rc, const s_v2 pos, const s_v2 size, const s_v2 origin, const t_f32 rot, const s_color_rgb24f fill_color, const s_color_rgba32f outline_color, const t_f32 outline_thickness = 1.0f);
    void DrawLine(const s_rendering_context rc, const s_v2 a, const s_v2 b, const s_color_rgba32f blend, const t_f32 thickness = 1.0f);
    [[nodiscard]] t_b8 DrawStr(const s_rendering_context rc, const s_str_rdonly str, const s_gfx_resource &font, const s_v2 pos, s_mem_arena &temp_mem_arena, const s_v2 alignment = alignments::g_topleft, const s_color_rgba32f blend = colors::g_white);

    void SetSurface(const s_rendering_context rc, const s_gfx_resource &surf);
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
    public:
        constexpr s_surface_shader_prog_uniform_val(const t_i32 v) : m_type(ek_surface_shader_prog_uniform_val_type_i32), m_type_data({.i32 = v}) {};
        constexpr s_surface_shader_prog_uniform_val(const t_u32 v) : m_type(ek_surface_shader_prog_uniform_val_type_u32), m_type_data({.u32 = v}) {};
        constexpr s_surface_shader_prog_uniform_val(const t_f32 v) : m_type(ek_surface_shader_prog_uniform_val_type_f32), m_type_data({.f32 = v}) {};
        constexpr s_surface_shader_prog_uniform_val(const s_v2 v) : m_type(ek_surface_shader_prog_uniform_val_type_v2), m_type_data({.v2 = v}) {};
        constexpr s_surface_shader_prog_uniform_val(const s_v3 v) : m_type(ek_surface_shader_prog_uniform_val_type_v3), m_type_data({.v3 = v}) {};
        constexpr s_surface_shader_prog_uniform_val(const s_v4 v) : m_type(ek_surface_shader_prog_uniform_val_type_v4), m_type_data({.v4 = v}) {};
        constexpr s_surface_shader_prog_uniform_val(const s_mat4x4 &v) : m_type(ek_surface_shader_prog_uniform_val_type_mat4x4), m_type_data({.mat4x4 = v}) {};

        t_i32 &I32() {
            ZF_ASSERT(m_type == ek_surface_shader_prog_uniform_val_type_i32);
            return m_type_data.i32;
        }

        const t_i32 &I32() const {
            ZF_ASSERT(m_type == ek_surface_shader_prog_uniform_val_type_i32);
            return m_type_data.i32;
        }

        t_u32 &U32() {
            ZF_ASSERT(m_type == ek_surface_shader_prog_uniform_val_type_u32);
            return m_type_data.u32;
        }

        const t_u32 &U32() const {
            ZF_ASSERT(m_type == ek_surface_shader_prog_uniform_val_type_u32);
            return m_type_data.u32;
        }

        t_f32 &F32() {
            ZF_ASSERT(m_type == ek_surface_shader_prog_uniform_val_type_f32);
            return m_type_data.f32;
        }

        const t_f32 &F32() const {
            ZF_ASSERT(m_type == ek_surface_shader_prog_uniform_val_type_f32);
            return m_type_data.f32;
        }

        s_v2 &V2() {
            ZF_ASSERT(m_type == ek_surface_shader_prog_uniform_val_type_v2);
            return m_type_data.v2;
        }

        const s_v2 &V2() const {
            ZF_ASSERT(m_type == ek_surface_shader_prog_uniform_val_type_v2);
            return m_type_data.v2;
        }

        s_v3 &V3() {
            ZF_ASSERT(m_type == ek_surface_shader_prog_uniform_val_type_v3);
            return m_type_data.v3;
        }

        const s_v3 &V3() const {
            ZF_ASSERT(m_type == ek_surface_shader_prog_uniform_val_type_v3);
            return m_type_data.v3;
        }

        s_v4 &V4() {
            ZF_ASSERT(m_type == ek_surface_shader_prog_uniform_val_type_v4);
            return m_type_data.v4;
        }

        const s_v4 &V4() const {
            ZF_ASSERT(m_type == ek_surface_shader_prog_uniform_val_type_v4);
            return m_type_data.v4;
        }

        s_mat4x4 &Mat4x4() {
            ZF_ASSERT(m_type == ek_surface_shader_prog_uniform_val_type_mat4x4);
            return m_type_data.mat4x4;
        }

        const s_mat4x4 &Mat4x4() const {
            ZF_ASSERT(m_type == ek_surface_shader_prog_uniform_val_type_mat4x4);
            return m_type_data.mat4x4;
        }

    private:
        e_surface_shader_prog_uniform_val_type m_type = {};

        union {
            t_i32 i32;
            t_u32 u32;
            t_f32 f32;
            s_v2 v2;
            s_v3 v3;
            s_v4 v4;
            s_mat4x4 mat4x4;
        } m_type_data = {};
    };

    void SetSurfaceShaderProg(const s_rendering_context rc, const s_gfx_resource &prog);
    [[nodiscard]] t_b8 SetSurfaceShaderProgUniform(const s_rendering_context rc, const s_str_rdonly name, const s_surface_shader_prog_uniform_val &val, s_mem_arena &temp_mem_arena);

    void DrawSurface(const s_rendering_context rc, const s_gfx_resource &surf, const s_v2 pos);

    namespace internal {
        [[nodiscard]] t_b8 BeginFrame(const s_rendering_basis &rendering_basis, const s_v2_i framebuffer_size_cache, s_mem_arena &mem_arena, s_rendering_context &o_rendering_context);
        void CompleteFrame(const s_rendering_context rc);
    }
}
