#include <zcl/zcl_gfx.h>

#include <stb_image.h>
#include <stb_truetype.h>

namespace zf {
    // ============================================================
    // @section: Textures
    // ============================================================
    t_b8 LoadTextureFromRaw(const s_str_rdonly file_path, s_mem_arena *const tex_data_mem_arena, s_mem_arena *const temp_mem_arena, s_texture_data *const o_tex_data) {
        s_v2_i size_in_pxs;
        t_u8 *const stb_px_data_raw = stbi_load(file_path.Raw(), &size_in_pxs.x, &size_in_pxs.y, nullptr, 4);

        if (!stb_px_data_raw) {
            return false;
        }

        ZF_DEFER({ stbi_image_free(stb_px_data_raw); });

        s_array<t_u8> px_data;

        if (!AllocArray(4 * size_in_pxs.x * size_in_pxs.y, tex_data_mem_arena, &px_data)) {
            return false;
        }

        const s_array_rdonly<t_u8> stb_px_data = {stb_px_data_raw, 4 * size_in_pxs.x * size_in_pxs.y};
        stb_px_data.CopyTo(px_data);

        *o_tex_data = {size_in_pxs, px_data};

        return true;
    }

    t_b8 PackTexture(const s_str_rdonly file_path, const s_texture_data tex_data, s_mem_arena *const temp_mem_arena) {
        if (!CreateFileAndParentDirs(file_path, temp_mem_arena)) {
            return false;
        }

        s_stream fs;

        if (!OpenFile(file_path, e_file_access_mode::write, &fs)) {
            return false;
        }

        ZF_DEFER({ CloseFile(&fs); });

        if (!fs.WriteItem(tex_data.SizeInPixels())) {
            return false;
        }

        if (!fs.WriteItemsOfArray(tex_data.RGBAPixelData())) {
            return false;
        }

        return true;
    }

    t_b8 UnpackTexture(const s_str_rdonly file_path, s_mem_arena *const tex_data_mem_arena, s_mem_arena *const temp_mem_arena, s_texture_data *const o_tex_data) {
        s_stream fs;

        if (!OpenFile(file_path, e_file_access_mode::read, &fs)) {
            return false;
        }

        ZF_DEFER({ CloseFile(&fs); });

        s_v2_i size_in_pxs;

        if (!fs.ReadItem(&size_in_pxs)) {
            return false;
        }

        s_array<t_u8> px_data;

        if (!AllocArray(4 * size_in_pxs.x * size_in_pxs.y, tex_data_mem_arena, &px_data)) {
            return false;
        }

        if (!fs.ReadItemsIntoArray(px_data, px_data.Len())) {
            return false;
        }

        return true;
    }
}
