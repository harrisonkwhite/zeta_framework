#pragma once

#include <zf/gfx.h>

namespace zf {
    using t_vert = float;
    using t_elem = unsigned short;

    struct s_batch_vert {
        t_vert vert_coord_x = 0;
        t_vert vert_coord_y = 0;

        t_vert pos_x = 0;
        t_vert pos_y = 0;

        t_vert size_x = 0;
        t_vert size_y = 0;

        t_vert rot = 0;

        t_vert tex_coord_x = 0;
        t_vert tex_coord_y = 0;

        t_vert blend_r = 0;
        t_vert blend_g = 0;
        t_vert blend_b = 0;
        t_vert blend_a = 0;
    };

    constexpr int g_batch_slot_cnt = 8192;
    constexpr int g_batch_slot_vert_len = sizeof(s_batch_vert) / sizeof(t_vert);
    constexpr int g_batch_slot_vert_cnt = 4;
    constexpr int g_batch_slot_elem_cnt = 6;

    struct s_rendering_basis {
        s_gfx_resource_handle batch_mesh_hdl;
        s_gfx_resource_handle px_tex_hdl; // Used for rendering rectangles and lines via scaling, rotation, etc.

        [[nodiscard]] bool Init(c_gfx_resource_arena& gfx_res_arena, c_mem_arena& temp_mem_arena);
    };
}