#include "zfwc_graphics.h"

static t_gl_id GenShaderFromSrc(const s_char_array_view src, const bool frag, s_mem_arena* const temp_mem_arena) {
    const t_gl_id shader_gl_id = glCreateShader(frag ? GL_FRAGMENT_SHADER : GL_VERTEX_SHADER);
    glShaderSource(shader_gl_id, 1, &src.buf_raw, NULL);
    glCompileShader(shader_gl_id);

    GLint success;
    glGetShaderiv(shader_gl_id, GL_COMPILE_STATUS, &success);

    if (!success) {
        // Try getting the OpenGL compile error message.
        t_s32 log_len;
        glGetShaderiv(shader_gl_id, GL_INFO_LOG_LENGTH, &log_len);

        if (log_len > 1) {
            const s_char_array log = PushCharArrayToMemArena(temp_mem_arena, log_len);

            if (!IS_ZERO(log)) {
                glGetShaderInfoLog(shader_gl_id, log.len, NULL, log.buf_raw);
                LOG_ERROR_SPECIAL("OpenGL Shader Compilation", "%s", log.buf_raw);
            }
        }

        glDeleteShader(shader_gl_id);

        return 0;
    }

    return shader_gl_id;
}

static bool LoadShaderSrcsFromFile(s_char_array_view* const vert_src, s_char_array_view* const frag_src, const s_char_array_view file_path, s_mem_arena* const mem_arena) {
    assert(IS_ZERO(*vert_src));
    assert(IS_ZERO(*frag_src));

    FILE* const fs = fopen(file_path.buf_raw, "rb");

    if (!fs) {
        return false;
    }

    int vert_src_len;

    if (fread(&vert_src_len, sizeof(vert_src_len), 1, fs) < 1) {
        fclose(fs);
        return false;
    }

    const s_char_array vert_src_nonview = PushCharArrayToMemArena(mem_arena, vert_src_len);

    if (IS_ZERO(vert_src_nonview)) {
        fclose(fs);
        return false;
    }

    if (fread(vert_src_nonview.buf_raw, 1, vert_src_nonview.len, fs) < vert_src_nonview.len) {
        fclose(fs);
        return false;
    }

    int frag_src_len;

    if (fread(&frag_src_len, sizeof(frag_src_len), 1, fs) < 1) {
        fclose(fs);
        return false;
    }

    const s_char_array frag_src_nonview = PushCharArrayToMemArena(mem_arena, frag_src_len);

    if (IS_ZERO(frag_src_nonview)) {
        fclose(fs);
        return false;
    }

    if (fread(frag_src_nonview.buf_raw, 1, frag_src_nonview.len, fs) < frag_src_nonview.len) {
        fclose(fs);
        return false;
    }

    fclose(fs);

    *vert_src = CharArrayView(vert_src_nonview);
    *frag_src = CharArrayView(frag_src_nonview);

    return true;
}

static t_gl_id GenShaderProg(const s_shader_prog_gen_info gen_info, s_mem_arena* const temp_mem_arena) {
    // Determine the shader sources.
    s_char_array_view vert_src = {0}, frag_src = {0};

    if (gen_info.holds_srcs) {
        vert_src = gen_info.vert_src;
        frag_src = gen_info.frag_src;
    } else {
        if (!LoadShaderSrcsFromFile(&vert_src, &frag_src, gen_info.file_path, temp_mem_arena)) {
            return 0;
        }
    }

    // Generate the shaders from the sources.
    const t_gl_id vs_gl_id = GenShaderFromSrc(vert_src, false, temp_mem_arena);

    if (!vs_gl_id) {
        if (gen_info.holds_srcs) {
            LOG_ERROR("Failed to generate vertex shader from source!");
        } else {
            LOG_ERROR("Failed to generate vertex shader from file \"%s\"!", gen_info.file_path.buf_raw);
        }

        return 0;
    }

    const t_gl_id fs_gl_id = GenShaderFromSrc(frag_src, true, temp_mem_arena);

    if (!fs_gl_id) {
        if (gen_info.holds_srcs) {
            LOG_ERROR("Failed to generate fragment shader from source!");
        } else {
            LOG_ERROR("Failed to generate fragment shader from file \"%s\"!", gen_info.file_path.buf_raw);
        }

        glDeleteShader(vs_gl_id);

        return 0;
    }

    // Set up the shader program.
    const t_gl_id prog_gl_id = glCreateProgram();
    glAttachShader(prog_gl_id, vs_gl_id);
    glAttachShader(prog_gl_id, fs_gl_id);
    glLinkProgram(prog_gl_id);

    // Dispose the shaders, they're no longer needed.
    glDeleteShader(fs_gl_id);
    glDeleteShader(vs_gl_id);

    return prog_gl_id;
}

s_shader_prog_group GenShaderProgGroup(const s_shader_prog_gen_info_array_view gen_infos, s_gl_resource_arena* const gl_res_arena, s_mem_arena* const temp_mem_arena) {
    assert(gen_infos.len > 0);

    const int prog_cnt = gen_infos.len;

    const s_gl_id_array gl_ids = PushToGLResourceArena(gl_res_arena, prog_cnt, ek_gl_resource_type_shader_prog);

    if (IS_ZERO(gl_ids)) {
        LOG_ERROR("Failed to reserve OpenGL shader program IDs for shader program group!");
        return (s_shader_prog_group){0};
    }

    for (int i = 0; i < prog_cnt; i++) {
        const s_shader_prog_gen_info gen_info = *ShaderProgGenInfoElemView(gen_infos, i);

        t_gl_id* const gl_id = GLIDElem(gl_ids, i);

        *gl_id = GenShaderProg(gen_info, temp_mem_arena);

        if (!*gl_id) {
            LOG_ERROR("Failed to generate shader program with index %d!", i);
            return (s_shader_prog_group){0};
        }
    }

    return (s_shader_prog_group){
        .gl_ids = GLIDArrayView(gl_ids)
    };
}
