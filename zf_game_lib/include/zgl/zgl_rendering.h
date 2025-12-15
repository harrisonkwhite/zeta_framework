#pragma once

#include <zcl.h>
#include <zgl/zgl_gfx.h>

namespace zf {
    struct s_rendering_basis {
        s_gfx_resource_arena gfx_res_arena;
        s_ptr<s_gfx_resource> batch_mesh;
        s_ptr<s_gfx_resource> batch_shader_prog;
    };

    s_rendering_basis CreateRenderingBasis(s_mem_arena &mem_arena, s_mem_arena &temp_mem_arena);

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
