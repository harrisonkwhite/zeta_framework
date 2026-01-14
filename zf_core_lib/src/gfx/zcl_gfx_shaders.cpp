#include <zcl/zcl_gfx.h>

#include <zcl/zcl_file_sys.h>

namespace zcl {
    t_b8 ShaderPack(const t_str_rdonly file_path, const t_array_rdonly<t_u8> compiled_shader_bin, t_arena *const temp_arena) {
        if (!FileCreateRecursive(file_path, temp_arena)) {
            return false;
        }

        t_file_stream fs;

        if (!FileOpen(file_path, ek_file_access_mode_write, temp_arena, &fs)) {
            return false;
        }

        ZCL_DEFER({ FileClose(&fs); });

        if (!serialize_array(fs, compiled_shader_bin)) {
            return false;
        }

        return true;
    }

    t_b8 ShaderUnpack(const t_str_rdonly file_path, t_arena *const shader_bin_arena, t_arena *const temp_arena, t_array_mut<t_u8> *const o_shader_bin) {
        t_file_stream fs;

        if (!FileOpen(file_path, ek_file_access_mode_read, temp_arena, &fs)) {
            return false;
        }

        ZCL_DEFER({ FileClose(&fs); });

        if (!deserialize_array(fs, shader_bin_arena, o_shader_bin)) {
            return false;
        }

        return true;
    }
}
