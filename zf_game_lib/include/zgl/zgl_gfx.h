#pragma once

#include <zcl.h>

namespace zf {
    // Initialises the GFX module. This depends on the platform module being successfully initialised.
    [[nodiscard]] t_b8 InitGFX();

    void ShutdownGFX();

    // ============================================================
    // @section: Resources
    // ============================================================
    enum e_gfx_resource_type {
        ek_gfx_resource_type_mesh,
        ek_gfx_resource_type_shader_prog,
        ek_gfx_resource_type_texture
    };

    struct s_gfx_resource;

    // ============================================================
    // @section: Rendering
    // ============================================================
    enum e_render_instr {
        ek_render_instr_clear,
        ek_render_instr_set_shader_prog,
        ek_render_instr_set_shader_prog_uniform,
        ek_render_instr_set_texture,
        ek_render_instr_draw_mesh
    };

    struct s_render_instr;

    void SubmitClearInstr(s_list<s_render_instr> &instrs, const s_color_rgba32f col);

    void SubmitSetShaderProgInstr(s_list<s_render_instr> &instrs, const s_gfx_resource *const prog);

    enum e_shader_prog_uniform_val_type : t_i32 {
        ek_surface_shader_prog_uniform_val_type_i32,
        ek_surface_shader_prog_uniform_val_type_u32,
        ek_surface_shader_prog_uniform_val_type_f32,
        ek_surface_shader_prog_uniform_val_type_v2,
        ek_surface_shader_prog_uniform_val_type_v3,
        ek_surface_shader_prog_uniform_val_type_v4,
        ek_surface_shader_prog_uniform_val_type_mat4x4
    };

    struct s_shader_prog_uniform_val {
    public:
        constexpr s_shader_prog_uniform_val(const t_i32 v) : m_type(ek_surface_shader_prog_uniform_val_type_i32), m_type_data({.i32 = v}) {};
        constexpr s_shader_prog_uniform_val(const t_u32 v) : m_type(ek_surface_shader_prog_uniform_val_type_u32), m_type_data({.u32 = v}) {};
        constexpr s_shader_prog_uniform_val(const t_f32 v) : m_type(ek_surface_shader_prog_uniform_val_type_f32), m_type_data({.f32 = v}) {};
        constexpr s_shader_prog_uniform_val(const s_v2 v) : m_type(ek_surface_shader_prog_uniform_val_type_v2), m_type_data({.v2 = v}) {};
        constexpr s_shader_prog_uniform_val(const s_v3 v) : m_type(ek_surface_shader_prog_uniform_val_type_v3), m_type_data({.v3 = v}) {};
        constexpr s_shader_prog_uniform_val(const s_v4 v) : m_type(ek_surface_shader_prog_uniform_val_type_v4), m_type_data({.v4 = v}) {};
        constexpr s_shader_prog_uniform_val(const s_mat4x4 &v) : m_type(ek_surface_shader_prog_uniform_val_type_mat4x4), m_type_data({.mat4x4 = v}) {};

        constexpr e_shader_prog_uniform_val_type Type() const {
            return m_type;
        }

        constexpr t_i32 &I32() {
            ZF_ASSERT(m_type == ek_surface_shader_prog_uniform_val_type_i32);
            return m_type_data.i32;
        }

        constexpr const t_i32 &I32() const {
            ZF_ASSERT(m_type == ek_surface_shader_prog_uniform_val_type_i32);
            return m_type_data.i32;
        }

        constexpr t_u32 &U32() {
            ZF_ASSERT(m_type == ek_surface_shader_prog_uniform_val_type_u32);
            return m_type_data.u32;
        }

        constexpr const t_u32 &U32() const {
            ZF_ASSERT(m_type == ek_surface_shader_prog_uniform_val_type_u32);
            return m_type_data.u32;
        }

        constexpr t_f32 &F32() {
            ZF_ASSERT(m_type == ek_surface_shader_prog_uniform_val_type_f32);
            return m_type_data.f32;
        }

        constexpr const t_f32 &F32() const {
            ZF_ASSERT(m_type == ek_surface_shader_prog_uniform_val_type_f32);
            return m_type_data.f32;
        }

        constexpr s_v2 &V2() {
            ZF_ASSERT(m_type == ek_surface_shader_prog_uniform_val_type_v2);
            return m_type_data.v2;
        }

        constexpr const s_v2 &V2() const {
            ZF_ASSERT(m_type == ek_surface_shader_prog_uniform_val_type_v2);
            return m_type_data.v2;
        }

        constexpr s_v3 &V3() {
            ZF_ASSERT(m_type == ek_surface_shader_prog_uniform_val_type_v3);
            return m_type_data.v3;
        }

        constexpr const s_v3 &V3() const {
            ZF_ASSERT(m_type == ek_surface_shader_prog_uniform_val_type_v3);
            return m_type_data.v3;
        }

        constexpr s_v4 &V4() {
            ZF_ASSERT(m_type == ek_surface_shader_prog_uniform_val_type_v4);
            return m_type_data.v4;
        }

        constexpr const s_v4 &V4() const {
            ZF_ASSERT(m_type == ek_surface_shader_prog_uniform_val_type_v4);
            return m_type_data.v4;
        }

        constexpr s_mat4x4 &Mat4x4() {
            ZF_ASSERT(m_type == ek_surface_shader_prog_uniform_val_type_mat4x4);
            return m_type_data.mat4x4;
        }

        constexpr const s_mat4x4 &Mat4x4() const {
            ZF_ASSERT(m_type == ek_surface_shader_prog_uniform_val_type_mat4x4);
            return m_type_data.mat4x4;
        }

    private:
        e_shader_prog_uniform_val_type m_type = {};

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

    void SubmitSetShaderProgUniformInstr(s_list<s_render_instr> &instrs, const e_shader_prog_uniform_val_type val_type, const s_shader_prog_uniform_val &val);

    void ExecRender(const s_array<s_render_instr> instrs);

#if 0
    struct s_platform_layer_info;
    struct s_rendering_basis;

    namespace internal {
        [[nodiscard]] t_b8 InitGFX(const s_platform_layer_info &platform_layer_info);
        void ShutdownGFX();
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

    [[nodiscard]] t_b8 CreateFontFromRaw(const s_str_rdonly file_path, const t_i32 height, t_code_pt_bit_vec &code_pts, s_gfx_resource_arena &res_arena, s_mem_arena &temp_mem_arena, s_ptr<s_gfx_resource> &o_font);
    [[nodiscard]] t_b8 CreateFontFromPacked(const s_str_rdonly file_path, s_gfx_resource_arena &res_arena, s_mem_arena &temp_mem_arena, s_ptr<s_gfx_resource> &o_font);

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

    [[nodiscard]] t_b8 LoadStrChrDrawPositions(const s_str_rdonly str, const s_font_arrangement &font_arrangement, const s_v2 pos, const s_v2 alignment, s_mem_arena &mem_arena, s_array<s_v2> &o_positions);
    [[nodiscard]] t_b8 DrawStr(const s_rendering_context rc, const s_str_rdonly str, const s_gfx_resource &font, const s_v2 pos, s_mem_arena &temp_mem_arena, const s_v2 alignment = alignments::g_topleft, const s_color_rgba32f blend = colors::g_white);

    namespace internal {
        void BeginFrame(const s_v2_i framebuffer_size_cache);
        void EndFrame();

        //[[nodiscard]] t_b8 BeginFrame(const s_rendering_basis &rendering_basis, const s_v2_i framebuffer_size_cache, s_mem_arena &mem_arena, s_rendering_context &o_rendering_context);
        // void CompleteFrame(const s_rendering_context rc);
    }
#endif
}
