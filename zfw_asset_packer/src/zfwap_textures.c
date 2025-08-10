#include "zfwap.h"

#include <stb_image.h>

static bool OutputTextureFile(const s_char_array_view file_path, const s_v2_s32 tex_size, const s_u8_array_view rgba_tex_data) {
    assert(tex_size.x > 0 && tex_size.y > 0);
    assert(rgba_tex_data.buf_raw && rgba_tex_data.len > 0 && rgba_tex_data.len % 4 == 0 && rgba_tex_data.len / 4 == tex_size.x * tex_size.y);

    FILE* const fs = fopen(file_path.buf_raw, "wb");

    if (!fs) {
        LOG_ERROR("Failed to open \"%s\" for writing!", file_path.buf_raw);
        return false;
    }

    if (fwrite(&tex_size, sizeof(tex_size), 1, fs) < 1) {
        LOG_ERROR("Failed to write texture size to file \"%s\"!", file_path.buf_raw);
        fclose(fs);
        return false;
    }

    if (fwrite(rgba_tex_data.buf_raw, 1, rgba_tex_data.len, fs) < rgba_tex_data.len) {
        LOG_ERROR("Failed to write pixel data to file \"%s\"!", file_path.buf_raw);
        fclose(fs);
        return false;
    }

    fclose(fs);

    return true;
}

bool PackTexture(const s_char_array_view file_path, const s_char_array_view output_file_path) {
    s_v2_s32 tex_size = {0};

    stbi_uc* const stb_px_data = stbi_load(file_path.buf_raw, &tex_size.x, &tex_size.y, NULL, 4);

    if (!stb_px_data) {
        LOG_ERROR("Failed to load pixel data from file \"%s\"!", output_file_path.buf_raw);
        LOG_ERROR_SPECIAL("STB", "%s", stbi_failure_reason());
        return false;
    }

    const bool success = OutputTextureFile(output_file_path, tex_size, (s_u8_array_view){.buf_raw = stb_px_data, .len = 4 * tex_size.x * tex_size.y});

    stbi_image_free(stb_px_data);

    LOG_SUCCESS("Packed texture from file \"%s\"!", file_path.buf_raw);

    return success;
}
