#include "zfw_graphics.h"

static zfw_t_gl_id CreateShaderProgFromFiles(const zfw_s_shader_prog_file_paths fps, s_mem_arena* const temp_mem_arena) {
    const char* const vs_src = (const char*)PushEntireFileContents(fps.vs_fp, temp_mem_arena, true);

    if (!vs_src) {
        return 0;
    }

    const char* const fs_src = (const char*)PushEntireFileContents(fps.fs_fp, temp_mem_arena, true);

    if (!fs_src) {
        return 0;
    }

    return ZFW_CreateShaderProgFromSrcs(vs_src, fs_src);
}

zfw_t_gl_id ZFW_CreateShaderFromSrc(const char* const src, const bool frag) {
    assert(src);

    const GLenum shader_type = frag ? GL_FRAGMENT_SHADER : GL_VERTEX_SHADER;
    const zfw_t_gl_id shader_gl_id = glCreateShader(shader_type);
    glShaderSource(shader_gl_id, 1, &src, NULL);
    glCompileShader(shader_gl_id);

    GLint success;
    glGetShaderiv(shader_gl_id, GL_COMPILE_STATUS, &success);

    if (!success) {
        glDeleteShader(shader_gl_id);
        return 0;
    }

    return shader_gl_id;
}

zfw_t_gl_id ZFW_CreateShaderProgFromSrcs(const char* const vert_src, const char* const frag_src) {
    assert(vert_src);
    assert(frag_src);

    const zfw_t_gl_id vert_shader_gl_id = ZFW_CreateShaderFromSrc(vert_src, false);

    if (!vert_shader_gl_id) {
        return 0;
    }

    const zfw_t_gl_id frag_shader_gl_id = ZFW_CreateShaderFromSrc(frag_src, true);

    if (!frag_shader_gl_id) {
        glDeleteShader(vert_shader_gl_id);
        return 0;
    }

    const zfw_t_gl_id prog_gl_id = glCreateProgram();
    glAttachShader(prog_gl_id, vert_shader_gl_id);
    glAttachShader(prog_gl_id, frag_shader_gl_id);
    glLinkProgram(prog_gl_id);

    glDeleteShader(vert_shader_gl_id);
    glDeleteShader(frag_shader_gl_id);

    return prog_gl_id;
}

zfw_s_shader_progs ZFW_LoadShaderProgsFromFiles(s_mem_arena* const mem_arena, const int prog_cnt, const zfw_t_shader_prog_index_to_file_paths prog_index_to_fps, s_mem_arena* const temp_mem_arena) {
    assert(mem_arena && IsMemArenaValid(mem_arena));
    assert(prog_cnt > 0);
    assert(prog_index_to_fps);
    assert(temp_mem_arena && IsMemArenaValid(temp_mem_arena));

    zfw_t_gl_id* const gl_ids = MEM_ARENA_PUSH_TYPE_CNT(mem_arena, zfw_t_gl_id, prog_cnt);

    if (!gl_ids) {
        return (zfw_s_shader_progs){0};
    }

    for (int i = 0; i < prog_cnt; i++) {
        const zfw_s_shader_prog_file_paths fps = prog_index_to_fps(i);

        gl_ids[i] = CreateShaderProgFromFiles(fps, temp_mem_arena);

        if (!gl_ids[i]) {
            // Delete all former shader programs.
            for (int j = 0; j < i; j++) {
                glDeleteProgram(gl_ids[j]);
            }

            return (zfw_s_shader_progs){0};
        }
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
