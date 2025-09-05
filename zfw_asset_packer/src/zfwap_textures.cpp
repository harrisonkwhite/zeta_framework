#include "zfwap.h"

#include <stb_image.h>

static bool OutputTextureFile(const c_array<const char> file_path, const s_rgba_texture rgba_tex) {
    FILE* const fs = fopen(file_path.buf_raw, "wb");

    if (!fs) {
        LOG_ERROR("Failed to open \"%s\" for writing!", file_path.buf_raw);
        return false;
    }

    if (fwrite(&rgba_tex.tex_size, sizeof(rgba_tex.tex_size), 1, fs) < 1) {
        LOG_ERROR("Failed to write texture size to file \"%s\"!", file_path.buf_raw);
        fclose(fs);
        return false;
    }

    if (fwrite(rgba_tex.px_data.buf_raw, 1, rgba_tex.px_data.elem_cnt, fs) < rgba_tex.px_data.elem_cnt) {
        LOG_ERROR("Failed to write pixel data to file \"%s\"!", file_path.buf_raw);
        fclose(fs);
        return false;
    }

    fclose(fs);

    return true;
}

bool PackTexture(const c_array<const char> file_path, const c_array<const char> output_file_path, c_mem_arena* const temp_mem_arena) {
    s_rgba_texture rgba_tex = {0};

    if (!LoadRGBATextureFromRawFile(&rgba_tex, temp_mem_arena, file_path)) {
        return false;
    }

    if (!OutputTextureFile(output_file_path, rgba_tex)) {
        return false;
    }

    LOG_SUCCESS("Packed texture from file \"%s\"!", file_path.buf_raw);

    return true;
}
