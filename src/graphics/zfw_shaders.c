#include "zfw_graphics.h"

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
