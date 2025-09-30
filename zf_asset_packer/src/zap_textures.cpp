#include "zap.h"

namespace zf {
    bool PackTextureFromRawFile(c_file_writer& fw, const c_string_view file_path, c_mem_arena& temp_mem_arena) {
        s_rgba_texture rgba_tex;

        if (!LoadRGBATextureFromRawFile(rgba_tex, temp_mem_arena, file_path)) {
            return false;
        }

        if (!PackTexture(fw, rgba_tex)) {
            return false;
        }

        return true;
    }
}
