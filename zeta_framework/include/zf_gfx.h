#pragma once

#include <zc.h>
#include <bx/math.h>
#include "bgfx/bgfx.h"

namespace zf {
    using td_gfx_resource_id = t_u32;
    using td_gfx_backend = t_u32;

    enum class ec_gfx_resource_type {
        vert_buf,
        elem_buf,
        prog
    };

    class c_gfx_resource_arena {
    public:
        td_gfx_resource_id CreateProgram();

    private:
        c_array<bgfx::ProgramHandle> progs;
    };

    //static bgfx::ProgramHandle LoadProgFromShaderFiles(const c_string_view vs_file_path, const c_string_view fs_file_path, c_mem_arena& temp_mem_arena) {
};
