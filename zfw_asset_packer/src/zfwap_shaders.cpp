#if 0
#include "zfwap.h"

bool PackShaderProg(const c_array<const char> vert_file_path, const c_array<const char> frag_file_path, const c_array<const char> output_file_path, c_mem_arena* const temp_mem_arena) {
    const c_array<const char> vert_src = CharArrayView(LoadFileContentsAsStr(vert_file_path, temp_mem_arena));

    if (IS_ZERO(vert_src)) {
        LOG_ERROR("Failed to load vertex shader source from file \"%s\"!", vert_file_path.buf_raw);
        return false;
    }

    const c_array<const char> frag_src = CharArrayView(LoadFileContentsAsStr(frag_file_path, temp_mem_arena));

    if (IS_ZERO(frag_src)) {
        LOG_ERROR("Failed to load fragment shader source from file \"%s\"!", frag_file_path.buf_raw);
        return false;
    }

    FILE* const fs = fopen(output_file_path.buf_raw, "wb");

    if (!fs) {
        LOG_ERROR("Failed to open \"%s\" for writing!", output_file_path.buf_raw);
        return false;
    }

    if (fwrite(&vert_src.elem_cnt, sizeof(vert_src.elem_cnt), 1, fs) < 1) {
        LOG_ERROR("Failed to write vertex shader source length to file \"%s\"!", output_file_path.buf_raw);
        fclose(fs);
        return false;
    }

    if (fwrite(vert_src.buf_raw, 1, vert_src.elem_cnt, fs) < vert_src.elem_cnt) {
        LOG_ERROR("Failed to write vertex shader source to file \"%s\"!", output_file_path.buf_raw);
        fclose(fs);
        return false;
    }

    if (fwrite(&frag_src.elem_cnt, sizeof(frag_src.elem_cnt), 1, fs) < 1) {
        LOG_ERROR("Failed to write fragment shader source length to file \"%s\"!", output_file_path.buf_raw);
        fclose(fs);
        return false;
    }

    if (fwrite(frag_src.buf_raw, 1, frag_src.elem_cnt, fs) < frag_src.elem_cnt) {
        LOG_ERROR("Failed to write fragment shader source to file \"%s\"!", output_file_path.buf_raw);
        fclose(fs);
        return false;
    }

    fclose(fs);

    LOG_SUCCESS("Packed shader program from vertex shader file \"%s\" and fragment shader file \"%s\"!", vert_file_path.buf_raw, frag_file_path.buf_raw);

    return true;
}
#endif
