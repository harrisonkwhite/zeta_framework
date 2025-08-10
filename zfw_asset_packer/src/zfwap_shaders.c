#include "zfwap.h"

bool PackShaderProg(const s_char_array_view vert_file_path, const s_char_array_view frag_file_path, const s_char_array_view output_file_path, s_mem_arena* const temp_mem_arena) {
    const s_char_array_view vert_src = CharArrayView(LoadFileContentsAsStr(vert_file_path, temp_mem_arena));

    if (IS_ZERO(vert_src)) {
        return false;
    }

    const s_char_array_view frag_src = CharArrayView(LoadFileContentsAsStr(frag_file_path, temp_mem_arena));

    if (IS_ZERO(frag_src)) {
        return false;
    }

    FILE* const fs = fopen(output_file_path.buf_raw, "wb");

    if (!fs) {
        return false;
    }

    if (fwrite(&vert_src.len, sizeof(vert_src.len), 1, fs) < 1) {
        fclose(fs);
        return false;
    }

    if (fwrite(vert_src.buf_raw, 1, vert_src.len, fs) < vert_src.len) {
        fclose(fs);
        return false;
    }

    if (fwrite(&frag_src.len, sizeof(frag_src.len), 1, fs) < 1) {
        fclose(fs);
        return false;
    }

    if (fwrite(frag_src.buf_raw, 1, frag_src.len, fs) < frag_src.len) {
        fclose(fs);
        return false;
    }

    fclose(fs);

    LOG_SUCCESS("Packed shader program from vertex shader file \"%s\" and fragment shader file \"%s\"!", vert_file_path.buf_raw, frag_file_path.buf_raw);

    return true;
}
