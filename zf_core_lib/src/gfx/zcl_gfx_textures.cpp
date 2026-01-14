#include <zcl/zcl_gfx.h>

#include <zcl/zcl_file_sys.h>
#include <stb_image.h>

namespace zcl {
    t_b8 TextureLoadFromRaw(const t_str_rdonly file_path, t_arena *const texture_data_arena, t_arena *const temp_arena, t_texture_data_mut *const o_texture_data) {
        const t_str_rdonly file_path_terminated = StrCloneButAddTerminator(file_path, temp_arena);

        t_v2_i size_in_pxs;
        t_u8 *const stb_px_data = stbi_load(StrToCStr(file_path_terminated), &size_in_pxs.x, &size_in_pxs.y, nullptr, 4);

        if (!stb_px_data) {
            return false;
        }

        ZCL_DEFER({ stbi_image_free(stb_px_data); });

        const t_array_rdonly<t_u8> stb_px_data_arr = {stb_px_data, 4 * size_in_pxs.x * size_in_pxs.y};
        const auto px_data = arena_push_array<t_u8>(texture_data_arena, 4 * size_in_pxs.x * size_in_pxs.y);
        array_copy(stb_px_data_arr, px_data);

        *o_texture_data = {size_in_pxs, px_data};

        return true;
    }

    t_b8 TexturePack(const t_str_rdonly file_path, const t_texture_data_mut texture_data, t_arena *const temp_arena) {
        if (!FileCreateRecursive(file_path, temp_arena)) {
            return false;
        }

        t_file_stream fs;

        if (!FileOpen(file_path, ek_file_access_mode_write, temp_arena, &fs)) {
            return false;
        }

        ZCL_DEFER({ FileClose(&fs); });

        if (!stream_write_item(fs, texture_data.size_in_pxs)) {
            return false;
        }

        if (!stream_write_items_of_array(fs, texture_data.rgba_px_data)) {
            return false;
        }

        return true;
    }

    t_b8 TextureUnpack(const t_str_rdonly file_path, t_arena *const texture_data_arena, t_arena *const temp_arena, t_texture_data_mut *const o_texture_data) {
        t_file_stream fs;

        if (!FileOpen(file_path, ek_file_access_mode_read, temp_arena, &fs)) {
            return false;
        }

        ZCL_DEFER({ FileClose(&fs); });

        t_v2_i size_in_pxs;

        if (!stream_read_item(fs, &size_in_pxs)) {
            return false;
        }

        const auto rgba_px_data = arena_push_array<t_u8>(texture_data_arena, 4 * size_in_pxs.x * size_in_pxs.y);

        if (!stream_read_items_into_array(fs, rgba_px_data, rgba_px_data.len)) {
            return false;
        }

        *o_texture_data = {size_in_pxs, rgba_px_data};

        return true;
    }
}
