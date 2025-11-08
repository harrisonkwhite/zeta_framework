#pragma once

#include <zf/gfx.h>

namespace zf {
    struct s_rendering_basis {
        s_gfx_resource_handle px_tex_hdl;
        s_gfx_resource_handle batch_mesh_hdl;

        bool Init(c_gfx_resource_arena& gfx_res_arena, c_mem_arena& temp_mem_arena);
    };
}