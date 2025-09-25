#include "zap.h"

#include <stb_image.h>

namespace zf {
    static bool OutputTextureFile(const c_string_view file_path, const s_rgba_texture rgba_tex) {
        c_file_writer fw;
        fw.DeferClose();

        if (!fw.Open(file_path)) {
            //LOG_ERROR("Failed to open \"%s\" for writing!", file_path.Raw());
            return false;
        }

        if (!fw.WriteItem(rgba_tex.tex_size)) {
            //LOG_ERROR("Failed to write texture size to file \"%s\"!", file_path.Raw());
            return false;
        }

        if (fw.Write(rgba_tex.px_data.View()) < rgba_tex.px_data.Len()) {
            //LOG_ERROR("Failed to write pixel data to file \"%s\"!", file_path.Raw());
            return false;
        }

        return true;
    }

    bool PackTexture(const c_string_view file_path, const c_string_view output_file_path, c_mem_arena& temp_mem_arena) {
        s_rgba_texture rgba_tex;

        if (!LoadRGBATextureFromRawFile(rgba_tex, temp_mem_arena, file_path)) {
            return false;
        }

        if (!OutputTextureFile(output_file_path, rgba_tex)) {
            return false;
        }

        //LOG_SUCCESS("Packed texture from file \"%s\"!", file_path.Raw());

        return true;
    }
}
