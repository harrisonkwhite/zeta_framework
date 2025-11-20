#include <zc/zc_gfx.h>

#include <stb_image.h>
#include <stb_truetype.h>

namespace zf {
    t_b8 LoadRGBATextureDataFromRaw(const s_str_rdonly file_path, s_mem_arena& mem_arena, s_rgba_texture_data& o_tex_data) {
        ZF_ASSERT(IsStrTerminated(file_path));

        t_u8* const stb_px_data = stbi_load(StrRaw(file_path), &o_tex_data.size_in_pxs.x, &o_tex_data.size_in_pxs.y, nullptr, 4);

        if (!stb_px_data) {
            return false;
        }

        const s_array_rdonly<t_u8> stb_px_data_arr = {stb_px_data, 4 * o_tex_data.size_in_pxs.x * o_tex_data.size_in_pxs.y};

        if (!MakeArray(mem_arena, 4 * o_tex_data.size_in_pxs.x * o_tex_data.size_in_pxs.y, o_tex_data.px_data)) {
            stbi_image_free(stb_px_data);
            return false;
        }

        Copy(o_tex_data.px_data, stb_px_data_arr);

        stbi_image_free(stb_px_data);

        return true;
    }

    t_b8 PackTexture(const s_rgba_texture_data_rdonly& tex_data, const s_str_rdonly file_path, s_mem_arena& temp_mem_arena) {
        ZF_ASSERT(IsStrTerminated(file_path));

        if (!CreateFileAndParentDirs(file_path, temp_mem_arena)) {
            return false;
        }

        s_file_stream fs;

        if (!OpenFile(file_path, ec_file_access_mode::write, fs)) {
            return false;
        }

        const t_b8 success = [fs, tex_data]() {
            if (!WriteItemToFile(fs, tex_data.size_in_pxs)) {
                return false;
            }

            if (WriteItemArrayToFile(fs, tex_data.px_data) < tex_data.px_data.len) {
                return false;
            }

            return true;
        }();

        CloseFile(fs);

        return success;
    }

    t_b8 UnpackTexture(const s_str_rdonly file_path, s_mem_arena& mem_arena, s_rgba_texture_data& o_tex_data) {
        ZF_ASSERT(IsStrTerminated(file_path));

        s_file_stream fs;

        if (!OpenFile(file_path, ec_file_access_mode::read, fs)) {
            return false;
        }

        const t_b8 success = [fs, &mem_arena, &o_tex_data]() {
            if (!ReadItemFromFile(fs, o_tex_data.size_in_pxs)) {
                return false;
            }

            if (!MakeArray(mem_arena, 4 * o_tex_data.size_in_pxs.x * o_tex_data.size_in_pxs.y, o_tex_data.px_data)) {
                return false;
            }

            if (ReadItemArrayFromFile(fs, o_tex_data.px_data) < o_tex_data.px_data.len) {
                return false;
            }

            return true;
        }();

        CloseFile(fs);

        return success;
    }
}
