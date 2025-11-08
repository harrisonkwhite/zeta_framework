#include <zc/gfx.h>

#include <stb_image.h>
#include <stb_truetype.h>

namespace zf {
    bool c_rgba_texture::LoadFromRaw(c_mem_arena& mem_arena, const s_str_view file_path) {
        //ZF_ASSERT(!IsLoaded());
        ZF_ASSERT(file_path.IsTerminated());

        stbi_uc* const stb_px_data = stbi_load(file_path.Raw(), &m_size_in_pxs.x, &m_size_in_pxs.y, nullptr, 4);

        if (!stb_px_data) {
            ZF_BANANA_ERROR();
            ZF_LOG_ERROR_SPECIAL("STB", "%s", stbi_failure_reason());
            return false;
        }

        c_array<t_byte> px_data;

        if (!px_data.Init(mem_arena, 4 * m_size_in_pxs.x * m_size_in_pxs.y)) {
            ZF_BANANA_ERROR();
            stbi_image_free(stb_px_data);
            return false;
        }

        memcpy(px_data.Raw(), stb_px_data, sizeof(*px_data.Raw()) * px_data.Len());

        stbi_image_free(stb_px_data);

        *this = {m_size_in_pxs, px_data};

        return true;
    }

    bool PackTexture(const s_str_view file_path, const c_rgba_texture tex, c_mem_arena& temp_mem_arena) {
        if (!CreateFileAndParentDirs(file_path, temp_mem_arena)) {
            return false;
        }

        s_file_stream fs;

        if (!fs.Open(file_path, true)) {
            return false;
        }

        const bool success = [tex, fs]() {
            if (!fs.WriteItem(tex.SizeInPixels())) {
                return false;
            }

            if (fs.WriteItems(tex.PixelData().View()) < tex.PixelData().Len()) {
                return false;
            }

            return true;
        }();

        fs.Close();

        return success;
    }

    bool UnpackTexture(c_rgba_texture& tex, const s_str_view file_path) {
#if 0
        if (!fs.ReadItem(tex.size_in_pxs)) {
            ZF_BANANA_ERROR();
            return false;
        }

        if (fs.ReadItems(tex.px_data) < tex.px_data.Len()) {
            ZF_BANANA_ERROR();
            return false;
        }
#endif

        return true;
    }
}
