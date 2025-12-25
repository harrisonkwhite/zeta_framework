#pragma once

#include <zcl.h>

namespace zf {
    [[nodiscard]] t_b8 RunPacker(const s_str_rdonly instrs_json_file_path);
    [[nodiscard]] t_b8 Test();
}
