#include "zap.h"

namespace zf {
    bool PackShaderProg(const c_string_view vert_file_path, const c_string_view frag_file_path, const c_string_view output_file_path, c_mem_arena& temp_mem_arena) {


        /*
        const c_array<const t_u8> vert_src_bytes = LoadFileContents(vert_file_path, temp_mem_arena, true).View();

        if (vert_src_bytes.IsEmpty()) {
            ZF_LOG_ERROR("Failed to load vertex shader source from file \"%s\"!", vert_file_path.Raw());
            return false;
        }

        const c_array<const t_u8> frag_src_bytes = LoadFileContents(frag_file_path, temp_mem_arena, true).View();

        if (frag_src_bytes.IsEmpty()) {
            ZF_LOG_ERROR("Failed to load fragment shader source from file \"%s\"!", frag_file_path.Raw());
            return false;
        }

        c_file_writer fw;
        fw.DeferClose();

        if (!fw.Open(output_file_path)) {
            ZF_LOG_ERROR("Failed to open \"%s\" for writing!", output_file_path.Raw());
            return false;
        }

        const t_s32 vert_src_size = vert_src_bytes.Len();

        if (!fw.WriteItem(vert_src_size)) {
            ZF_LOG_ERROR("Failed to write vertex shader source length to file \"%s\"!", output_file_path.Raw());
            return false;
        }

        if (fw.Write(vert_src_bytes) < vert_src_bytes.Len()) {
            ZF_LOG_ERROR("Failed to write vertex shader source to file \"%s\"!", output_file_path.Raw());
            return false;
        }

        const t_s32 frag_src_size = frag_src_bytes.Len();

        if (!fw.WriteItem(frag_src_size)) {
            ZF_LOG_ERROR("Failed to write fragment shader source length to file \"%s\"!", output_file_path.Raw());
            return false;
        }

        if (fw.Write(frag_src_bytes) < frag_src_bytes.Len()) {
            ZF_LOG_ERROR("Failed to write fragment shader source to file \"%s\"!", output_file_path.Raw());
            return false;
        }

        ZF_LOG_SUCCESS("Packed shader program from vertex shader file \"%s\" and fragment shader file \"%s\"!", vert_file_path.Raw(), frag_file_path.Raw());*/

        return true;
    }
}
