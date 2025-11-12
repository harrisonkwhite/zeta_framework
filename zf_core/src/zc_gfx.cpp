#include <zc/zc_gfx.h>

#include <stb_image.h>
#include <stb_truetype.h>

namespace zf {
    t_b8 LoadTextureFromRaw(const s_str_view file_path, c_mem_arena& mem_arena, s_texture_data& o_tex_data) {
        ZF_ASSERT(IsStrTerminated(file_path));

        stbi_uc* const stb_px_data = stbi_load(file_path.Raw(), &o_tex_data.size_in_pxs.x, &o_tex_data.size_in_pxs.y, nullptr, 4);

        if (!stb_px_data) {
            ZF_LOG_ERROR_SPECIAL("STB", "%s", stbi_failure_reason());
            return false;
        }

        if (!mem_arena.PushArray(4 * o_tex_data.size_in_pxs.x * o_tex_data.size_in_pxs.y, o_tex_data.rgba_px_data)) {
            stbi_image_free(stb_px_data);
            return false;
        }

        memcpy(o_tex_data.rgba_px_data.Raw(), stb_px_data, static_cast<size_t>(ZF_SIZE_OF(*o_tex_data.rgba_px_data.Raw()) * o_tex_data.rgba_px_data.Len()));

        stbi_image_free(stb_px_data);

        return true;
    }

    t_b8 LoadTextureFromPacked(const s_str_view file_path, c_mem_arena& mem_arena, s_texture_data& o_tex_data) {
        ZF_ASSERT(IsStrTerminated(file_path));

        s_file_stream fs;

        if (!fs.Open(file_path, ec_file_access_mode::read)) {
            return false;
        }

        const t_b8 success = [fs, &mem_arena, &o_tex_data]() {
            if (!fs.ReadItem(o_tex_data.size_in_pxs)) {
                return false;
            }

            if (!mem_arena.PushArray(4 * o_tex_data.size_in_pxs.x * o_tex_data.size_in_pxs.y, o_tex_data.rgba_px_data)) {
                return false;
            }

            if (fs.ReadItems(o_tex_data.rgba_px_data) < o_tex_data.rgba_px_data.Len()) {
                return false;
            }

            return true;
        }();

        fs.Close();

        return success;
    }

    t_b8 PackTexture(const s_texture_data_view& tex_data, const s_str_view file_path, c_mem_arena& temp_mem_arena) {
        ZF_ASSERT(IsStrTerminated(file_path));

        if (!CreateFileAndParentDirs(file_path, temp_mem_arena)) {
            return false;
        }

        s_file_stream fs;

        if (!fs.Open(file_path, ec_file_access_mode::write)) {
            return false;
        }

        const t_b8 success = [fs, tex_data]() {
            if (!fs.WriteItem(tex_data.size_in_pxs)) {
                return false;
            }

            if (fs.WriteItems(tex_data.rgba_px_data.View()) < tex_data.rgba_px_data.Len()) {
                return false;
            }

            return true;
        }();

        fs.Close();

        return success;
    }
}
