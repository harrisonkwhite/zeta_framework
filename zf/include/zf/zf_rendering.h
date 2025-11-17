#pragma once

#include <zf/zf_gfx.h>

namespace zf {
    struct s_batch_vert {
        s_v2<t_f32> vert_coord;
        s_v2<t_f32> pos;
        s_v2<t_f32> size;
        t_f32 rot = 0.0f;
        s_v2<t_f32> tex_coord;
        s_color_rgba32f blend;
    };

    constexpr s_static_array<t_s32, 6> g_batch_vert_attr_lens = {
        {2, 2, 2, 1, 2, 4} // This has to match the number of components per attribute above.
    };

    constexpr t_size g_batch_vert_component_cnt = ZF_SIZE_OF(s_batch_vert) / ZF_SIZE_OF(t_f32);

    static_assert([]() {
        t_size sum = 0;

        for (t_size i = 0; i < g_batch_vert_attr_lens.Len(); i++) {
            sum += g_batch_vert_attr_lens[i];
        }

        return sum == g_batch_vert_component_cnt;
    }(), "Mismatch between specified batch vertex attribute lengths and component count!");

    constexpr t_size g_batch_slot_cnt = 1 << 8;
    static_assert(g_batch_slot_cnt <= 1 << 16, "Batch slot count is too large (need to account for range limits of elements).");

    constexpr t_size g_batch_slot_vert_cnt = 4;
    constexpr t_size g_batch_slot_elem_cnt = 6;

    struct s_rendering_basis {
        c_gfx_resource_handle batch_mesh_hdl;
        c_gfx_resource_handle batch_shader_prog_hdl;
        s_texture px_tex; // Used for rendering rectangles and lines via scaling, rotation, etc.

        [[nodiscard]] t_b8 Init(c_gfx_resource_arena& gfx_res_arena, s_mem_arena& temp_mem_arena);
    };

    using t_batch_slot = s_static_array<s_batch_vert, g_batch_slot_vert_cnt>;

    class c_renderer {
    public:
        t_b8 Init(c_gfx_resource_arena& gfx_res_arena, s_mem_arena& mem_arena, s_mem_arena& temp_mem_arena);

        void Begin() {
            m_batch_view_mat = s_matrix_4x4::Identity();
            Clear();
        }

        void End() {
            Flush();
        }

        void Clear(const s_color_rgba32f col = {}) const;
        void SetViewMatrix(const s_matrix_4x4& mat);
        void DrawTexture(const s_texture& tex, const s_v2<t_f32> pos, const s_rect<t_s32> src_rect = {}, const s_v2<t_f32> origin = origins::g_topleft, const s_v2<t_f32> scale = {1.0f, 1.0f}, const t_f32 rot = 0.0f, const s_color_rgba32f blend = colors::g_white);

        void DrawRect(const s_rect<t_f32> rect, const s_color_rgba32f color) {
            DrawTexture(m_basis.px_tex, rect.Pos(), {}, {}, rect.Size(), 0.0f, color);
        }

        void DrawLine(const s_v2<t_f32> a, const s_v2<t_f32> b, const s_color_rgba32f blend, const t_f32 width) {
            ZF_ASSERT(width > 0.0f);

            const t_f32 len = a.DistTo(b);
            const t_f32 dir = a.DirToInRads(b);
            DrawTexture(m_basis.px_tex, a, {}, origins::g_centerleft, {len, width}, dir, blend);
        }

    private:
        s_rendering_basis m_basis;

        s_array<t_batch_slot> m_batch_slots;
        t_size m_batch_slots_used_cnt = 0;

        s_matrix_4x4 m_batch_view_mat;

        c_gfx_resource_handle m_batch_tex_hdl;

        void Draw(const c_gfx_resource_handle tex_hdl, const s_rect<t_f32> tex_coords, s_v2<t_f32> pos, s_v2<t_f32> size, s_v2<t_f32> origin, const t_f32 rot, const s_color_rgba32f blend);
        void Flush();
    };
}
