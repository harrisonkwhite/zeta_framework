#include "zfws.h"

#include <stb_image.h>

bool LoadRGBATextureFromRawFile(s_rgba_texture* const tex, s_mem_arena* const mem_arena, const s_char_array_view file_path) {
    assert(IS_ZERO(*tex));

    stbi_uc* const stb_px_data = stbi_load(file_path.buf_raw, &tex->tex_size.x, &tex->tex_size.y, NULL, 4);

    if (!stb_px_data) {
        LOG_ERROR("Failed to load pixel data from file \"%s\" through STB!", file_path.buf_raw);
        LOG_ERROR_SPECIAL("STB", "%s", stbi_failure_reason());
        return false;
    }

    tex->px_data = PushU8ArrayToMemArena(mem_arena, 4 * tex->tex_size.x * tex->tex_size.y);

    if (IS_ZERO(tex->px_data)) {
        LOG_ERROR("Failed to reserve memory for RGBA texture pixel data!");
        stbi_image_free(stb_px_data);
        return false;
    }

    memcpy(tex->px_data.buf_raw, stb_px_data, sizeof(*tex->px_data.buf_raw) * tex->px_data.elem_cnt);

    stbi_image_free(stb_px_data);

    return true;
}
