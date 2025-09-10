#include "zfwap.h"

#include <stb_image.h>

static bool OutputTextureFile(const c_string_view file_path, const s_rgba_texture rgba_tex) {
    FILE* const fs = fopen(file_path.Raw(), "wb");

    if (!fs) {
        //LOG_ERROR("Failed to open \"%s\" for writing!", file_path.Raw());
        return false;
    }

    if (fwrite(&rgba_tex.tex_size, sizeof(rgba_tex.tex_size), 1, fs) < 1) {
        //LOG_ERROR("Failed to write texture size to file \"%s\"!", file_path.Raw());
        fclose(fs);
        return false;
    }

    if (fwrite(rgba_tex.px_data.Raw(), 1, rgba_tex.px_data.Len(), fs) < rgba_tex.px_data.Len()) {
        //LOG_ERROR("Failed to write pixel data to file \"%s\"!", file_path.Raw());
        fclose(fs);
        return false;
    }

    fclose(fs);

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
