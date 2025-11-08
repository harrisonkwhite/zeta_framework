#pragma once

#include <zf/gfx.h>

namespace zf {
    struct s_batch_vert {
        s_v2<float> vert_coord;
        s_v2<float> pos;
        s_v2<float> size;
        float rot = 0.0f;
        s_v2<float> tex_coord;
        s_v4<float> blend;
    };

    constexpr s_static_array<int, 6> g_batch_vert_attr_lens = {
        {2, 2, 2, 1, 2, 4} // This has to match the number of components per attribute above.
    };

    constexpr int g_batch_vert_component_cnt = sizeof(s_batch_vert) / sizeof(float);

    static_assert([]() {
        int sum = 0;

        for (int i = 0; i < g_batch_vert_attr_lens.Len(); i++) {
            sum += g_batch_vert_attr_lens[i];
        }

        return sum == g_batch_vert_component_cnt;
    }(), "Mismatch between specified batch vertex attribute lengths and component count!");

    constexpr int g_batch_slot_cnt = 1 << 8;
    static_assert(g_batch_slot_cnt <= 1 << 16, "Batch slot count is too large (need to account for range limits of elements).");

    constexpr int g_batch_slot_vert_cnt = 4;
    constexpr int g_batch_slot_elem_cnt = 6;

    struct s_rendering_basis {
        s_gfx_resource_handle batch_mesh_hdl;
        s_gfx_resource_handle batch_shader_prog_hdl;
        s_texture px_tex; // Used for rendering rectangles and lines via scaling, rotation, etc.

        [[nodiscard]] bool Init(c_gfx_resource_arena& gfx_res_arena, c_mem_arena& temp_mem_arena);
    };

    using t_batch_slot = s_static_array<s_batch_vert, g_batch_slot_vert_cnt>;

    class c_renderer {
    public:
        bool Init(c_gfx_resource_arena& gfx_res_arena, c_mem_arena& mem_arena, c_mem_arena& temp_mem_arena);

        void Begin() {
            m_batch_view_mat = s_matrix_4x4::Identity();
            Clear();
        }

        void End() {
            Flush();
        }

        void Clear(const s_v4<float> col = {});
        void SetViewMatrix(const s_matrix_4x4& mat);
        void DrawTexture(const s_texture& tex, const s_v2<float> pos, const s_rect<int> src_rect = {}, const s_v2<float> origin = origins::g_topleft, const s_v2<float> scale = {1.0f, 1.0f}, const float rot = 0.0f, const s_v4<float> blend = colors::g_white);

        void DrawRect(const s_rect<float> rect, const s_v4<float> color) {
            DrawTexture(m_basis.px_tex, rect.Pos(), {}, {}, rect.Size(), 0.0f, color);
        }

    private:
        s_rendering_basis m_basis;

        c_array<t_batch_slot> m_batch_slots;
        int m_batch_slots_used_cnt = 0;

        s_matrix_4x4 m_batch_view_mat;

        s_gfx_resource_handle m_batch_tex_hdl;

        void Draw(const s_gfx_resource_handle tex_hdl, const s_rect<float> tex_coords, s_v2<float> pos, s_v2<float> size, s_v2<float> origin, const float rot, const s_v4<float> blend);
        void Flush();
    };
}