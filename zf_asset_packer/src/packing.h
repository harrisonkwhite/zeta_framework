#pragma once

#include <zc/io.h>
#include <zc/gfx.h>
#include <zc/mem/mem.h>
#include <zc/mem/strs.h>

namespace zf {
    bool PackAssets(const s_str_view instrs_json, const s_str_view output_file_path, c_mem_arena& temp_mem_arena);
}
