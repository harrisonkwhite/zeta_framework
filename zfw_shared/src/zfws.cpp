#include "zfws.h"
#include "cu_mem.h"

#include <stb_image.h>

bool LoadRGBATextureFromRawFile(s_rgba_texture& tex, c_mem_arena& mem_arena, const c_array<const char> file_path) {
    stbi_uc* const stb_px_data = stbi_load(file_path.Raw(), &tex.tex_size.x, &tex.tex_size.y, NULL, 4);

    if (!stb_px_data) {
        //LOG_ERROR("Failed to load pixel data from file \"%s\" through STB!", file_path.buf_raw);
        //LOG_ERROR_SPECIAL("STB", "%s", stbi_failure_reason());
        return false;
    }

    tex.px_data = PushArrayToMemArena<t_u8>(mem_arena, 4 * tex.tex_size.x * tex.tex_size.y);

    if (tex.px_data.IsEmpty()) {
        //LOG_ERROR("Failed to reserve memory for RGBA texture pixel data!");
        stbi_image_free(stb_px_data);
        return false;
    }

    memcpy(tex.px_data.Raw(), stb_px_data, sizeof(*tex.px_data.Raw()) * tex.px_data.Len());

    stbi_image_free(stb_px_data);

    return true;
}
