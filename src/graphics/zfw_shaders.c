#include "mem.h"
#include "zfw_graphics.h"

#include <stdio.h>

static zfw_t_gl_id CreateShaderFromFile(const char* const fp, const bool frag, s_mem_arena* const temp_mem_arena) {
    assert(fp);
    assert(temp_mem_arena && IsMemArenaValid(temp_mem_arena));

    const char* const src = (const char*)PushEntireFileContents(fp, temp_mem_arena, true);

    if (!src) {
        LOG_ERROR("Failed to reserve memory for contents of shader \"%s\"!", fp);
        return 0;
    }

    const zfw_t_gl_id shader_gl_id = ZFW_CreateShaderFromSrc(src, frag, temp_mem_arena);

    if (!shader_gl_id) {
        LOG_ERROR("Failed to create shader \"%s\"!", fp);
    }

    return shader_gl_id;
}

zfw_t_gl_id ZFW_CreateShaderFromSrc(const char* const src, const bool frag, s_mem_arena* const temp_mem_arena) {
    assert(src);
    assert(temp_mem_arena && IsMemArenaValid(temp_mem_arena));

    const GLenum shader_type = frag ? GL_FRAGMENT_SHADER : GL_VERTEX_SHADER;
    const zfw_t_gl_id shader_gl_id = glCreateShader(shader_type);
    glShaderSource(shader_gl_id, 1, &src, NULL);
    glCompileShader(shader_gl_id);

    GLint success;
    glGetShaderiv(shader_gl_id, GL_COMPILE_STATUS, &success);

    if (!success) {
        // Try getting the OpenGL compile error message.
        GLint log_len = 0;
        glGetShaderiv(shader_gl_id, GL_INFO_LOG_LENGTH, &log_len);

        if (log_len > 0) {
            char* const log = MEM_ARENA_PUSH_TYPE_CNT(temp_mem_arena, char, log_len);

            if (log) {
                glGetShaderInfoLog(shader_gl_id, log_len, NULL, log);
                LOG_ERROR_SPECIAL("OpenGL Shader Compilation", "%s", log);
            }
        }

        glDeleteShader(shader_gl_id);

        return 0;
    }

    return shader_gl_id;
}

zfw_t_gl_id ZFW_CreateShaderProgAndDeleteShaders(const zfw_t_gl_id vs_gl_id, const zfw_t_gl_id fs_gl_id) {
    assert(glIsShader(vs_gl_id));
    assert(glIsShader(fs_gl_id));

    const zfw_t_gl_id prog_gl_id = glCreateProgram();
    glAttachShader(prog_gl_id, vs_gl_id);
    glAttachShader(prog_gl_id, fs_gl_id);
    glLinkProgram(prog_gl_id);

    glDeleteShader(fs_gl_id);
    glDeleteShader(vs_gl_id);

    return prog_gl_id;
}

zfw_s_shader_progs ZFW_CreateShaderProgsFromFiles(s_mem_arena* const mem_arena, const int prog_cnt, const zfw_t_shader_prog_index_to_file_paths prog_index_to_fps, s_mem_arena* const temp_mem_arena) {
    assert(mem_arena && IsMemArenaValid(mem_arena));
    assert(prog_cnt > 0);
    assert(prog_index_to_fps);
    assert(temp_mem_arena && IsMemArenaValid(temp_mem_arena));

    zfw_t_gl_id* const gl_ids = MEM_ARENA_PUSH_TYPE_CNT(mem_arena, zfw_t_gl_id, prog_cnt);

    if (!gl_ids) {
        LOG_ERROR("Failed to reserve memory for shader program OpenGL IDs!");
        return (zfw_s_shader_progs){0};
    }

    for (int i = 0; i < prog_cnt; i++) {
        const zfw_s_shader_prog_file_paths fps = prog_index_to_fps(i);

        const zfw_t_gl_id vs_gl_id = CreateShaderFromFile(fps.vs_fp, false, temp_mem_arena);

        if (!vs_gl_id) {
            goto error;
        }

        const zfw_t_gl_id fs_gl_id = CreateShaderFromFile(fps.fs_fp, true, temp_mem_arena);

        if (!fs_gl_id) {
            glDeleteShader(vs_gl_id);
            goto error;
        }

        gl_ids[i] = ZFW_CreateShaderProgAndDeleteShaders(vs_gl_id, fs_gl_id);

        continue;

error:
        LOG_ERROR("Failed to create shader program with vertex shader \"%s\" and fragment shader \"%s\"!", fps.vs_fp, fps.fs_fp);

        // Delete all former shader programs.
        for (int j = 0; j < i; j++) {
            glDeleteProgram(gl_ids[j]);
        }

        return (zfw_s_shader_progs){0};
    }

    return (zfw_s_shader_progs){
        .gl_ids = gl_ids,
        .cnt = prog_cnt
    };
}

void ZFW_UnloadShaderProgs(zfw_s_shader_progs* const progs) {
    for (int i = 0; i < progs->cnt; i++) {
        glDeleteProgram(progs->gl_ids[i]);
    }

    ZERO_OUT(*progs);
}
