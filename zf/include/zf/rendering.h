#pragma once

#include <zf/gfx.h>

namespace zf {
    using t_vert_component = float;
    using t_elem = unsigned short;

    struct s_batch_vert {
        t_vert_component vert_coord_x = 0;
        t_vert_component vert_coord_y = 0;

        t_vert_component pos_x = 0;
        t_vert_component pos_y = 0;

        t_vert_component size_x = 0;
        t_vert_component size_y = 0;

        t_vert_component rot = 0;

        t_vert_component tex_coord_x = 0;
        t_vert_component tex_coord_y = 0;

        t_vert_component blend_r = 0;
        t_vert_component blend_g = 0;
        t_vert_component blend_b = 0;
        t_vert_component blend_a = 0;
    };

    constexpr s_static_array<int, 6> g_batch_vert_attr_lens = {
        {2, 2, 2, 1, 2, 4} // This has to match the number of components per attribute above.
    };

    constexpr int g_batch_vert_component_cnt = sizeof(s_batch_vert) / sizeof(t_vert_component);

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
        s_gfx_resource_handle px_tex_hdl; // Used for rendering rectangles and lines via scaling, rotation, etc.

        [[nodiscard]] bool Init(c_gfx_resource_arena& gfx_res_arena, c_mem_arena& temp_mem_arena);
    };
}