#pragma once

#include <zf/zf_gfx.h>

namespace zf {
    constexpr s_color_rgba32f g_default_bg_color = s_color_rgba8(147, 207, 249, 255);

    struct s_batch_vert {
        s_v2<t_f32> vert_coord;
        s_v2<t_f32> pos;
        s_v2<t_f32> size;
        t_f32 rot;
        s_v2<t_f32> tex_coord;
        s_color_rgba32f blend;
    };

    constexpr s_static_array<t_s32, 6> g_batch_vert_attr_lens = {
        {2, 2, 2, 1, 2, 4} // This has to match the number of components per attribute above.
    };

    constexpr t_size g_batch_vert_component_cnt = ZF_SIZE_OF(s_batch_vert) / ZF_SIZE_OF(t_f32);

    static_assert([]() {
        t_size sum = 0;

        for (t_size i = 0; i < g_batch_vert_attr_lens.g_len; i++) {
            sum += g_batch_vert_attr_lens[i];
        }

        return sum == g_batch_vert_component_cnt;
    }(), "Mismatch between specified batch vertex attribute lengths and component count!");

    constexpr t_size g_batch_slot_cnt = 1 << 8;
    static_assert(g_batch_slot_cnt <= 1 << 16, "Batch slot count is too large (need to account for range limits of elements).");

    constexpr t_size g_batch_slot_vert_cnt = 4;
    constexpr t_size g_batch_slot_elem_cnt = 6;

    struct s_rendering_basis {
        gfx::s_resource_handle batch_mesh_hdl;
        gfx::s_resource_handle batch_shader_prog_hdl;
        gfx::s_texture_asset px_tex; // Used for rendering rectangles and lines via scaling, rotation, etc.
    };

    using t_batch_slot = s_static_array<s_batch_vert, g_batch_slot_vert_cnt>;

    struct s_rendering_state;

    struct s_rendering_context {
        const s_rendering_basis& basis;
        s_rendering_state& state;
    };

    [[nodiscard]] t_b8 MakeRenderingBasis(gfx::s_resource_arena& gfx_res_arena, s_mem_arena& temp_mem_arena, s_rendering_basis& o_basis);

    s_rendering_state* PrepareRenderingPhase(s_mem_arena& mem_arena); // Returns a newly created rendering state, or nullptr on failure.
    void CompleteRenderingPhase(const s_rendering_context& rc);

    void DrawClear(const s_color_rgba32f col = {});

    void UpdateViewMatrix(const s_rendering_context& rc, const s_matrix_4x4& mat);
    void DrawTexture(const s_rendering_context& rc, const gfx::s_texture_asset& tex, const s_v2<t_f32> pos, const s_rect<t_s32> src_rect = {}, const s_v2<t_f32> origin = origins::g_topleft, const s_v2<t_f32> scale = {1.0f, 1.0f}, const t_f32 rot = 0.0f, const s_color_rgba32f blend = colors::g_white);
    void DrawStr(const s_rendering_context& rc, const s_str_rdonly str, const gfx::s_font_asset& font, const s_v2<t_f32> pos);

    inline void DrawRect(const s_rendering_context& rc, const s_rect<t_f32> rect, const s_color_rgba32f color) {
        DrawTexture(rc, rc.basis.px_tex, RectPos(rect), {}, {}, RectSize(rect), 0.0f, color);
    }

    inline void DrawLine(const s_rendering_context& rc, const s_v2<t_f32> a, const s_v2<t_f32> b, const s_color_rgba32f blend, const t_f32 width) {
        ZF_ASSERT(width > 0.0f);

        const t_f32 len = CalcDist(a, b);
        const t_f32 dir = CalcDirInRads(a, b);
        DrawTexture(rc, rc.basis.px_tex, a, {}, origins::g_centerleft, {len, width}, dir, blend);
    }
}
