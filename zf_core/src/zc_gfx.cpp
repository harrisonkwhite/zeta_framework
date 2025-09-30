#include "zc_gfx.h"

#include <stb_image.h>
#include "zc_io.h"

namespace zf {
    bool LoadRGBATextureFromRawFile(s_rgba_texture& tex, c_mem_arena& mem_arena, const c_string_view file_path) {
        stbi_uc* const stb_px_data = stbi_load(file_path.Raw(), &tex.dims.x, &tex.dims.y, NULL, 4);

        if (!stb_px_data) {
            ZF_LOG_ERROR("Failed to load pixel data from file \"%s\" through STB!", file_path.Raw());
            ZF_LOG_ERROR_SPECIAL("STB", "%s", stbi_failure_reason());
            return false;
        }

        tex.px_data = PushArrayToMemArena<t_u8>(mem_arena, 4 * tex.dims.x * tex.dims.y);

        if (tex.px_data.IsEmpty()) {
            ZF_LOG_ERROR("Failed to reserve memory for RGBA texture pixel data!");
            stbi_image_free(stb_px_data);
            return false;
        }

        memcpy(tex.px_data.Raw(), stb_px_data, sizeof(*tex.px_data.Raw()) * tex.px_data.Len());

        stbi_image_free(stb_px_data);

        return true;
    }

    bool PackTexture(c_file_writer& fw, const s_rgba_texture rgba_tex) {
        if (!fw.WriteItem(rgba_tex.dims)) {
            ZF_LOG_ERROR("Failed to write texture size during packing!");
            return false;
        }

        if (fw.Write(rgba_tex.px_data.View()) < rgba_tex.px_data.Len()) {
            ZF_LOG_ERROR("Failed to write pixel data during packing!");
            return false;
        }

        return true;
    }

    void UnpackTexture(c_file_reader& fr, s_rgba_texture& rgba_tex) {
    }

    bool PackFont(c_file_writer& fw, const s_font_arrangement& arrangement, const s_font_texture_meta tex_meta, const c_array<const t_u8> tex_rgba_px_data) {
        if (!fw.WriteItem(arrangement)) {
            ZF_LOG_ERROR("Failed to write font arrangement during packing!");
            return false;
        }

        if (!fw.WriteItem(tex_meta)) {
            ZF_LOG_ERROR("Failed to write font texture metadata during packing!");
            return false;
        }

        if (fw.Write(tex_rgba_px_data) < tex_rgba_px_data.Len()) {
            ZF_LOG_ERROR("Failed to write font texture RGBA pixel data during packing!");
            return false;
        }

        return true;
    }

    void UnpackFont(c_file_reader& fr, s_font_arrangement& arrangement, s_font_texture_meta tex_meta, c_array<const t_u8>& tex_rgba_px_data) {
    }
}
