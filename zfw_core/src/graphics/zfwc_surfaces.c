#include "zfwc_graphics.h"

static bool AttachFramebufferTexture(const t_gl_id fb_gl_id, const t_gl_id tex_gl_id, const s_v2_s32 tex_size) {
    assert(fb_gl_id != 0);
    assert(tex_gl_id != 0);
    assert(tex_size.x > 0 && tex_size.y > 0);

    glBindTexture(GL_TEXTURE_2D, tex_gl_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_size.x, tex_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, fb_gl_id);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_gl_id, 0);

    const bool success = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return success;
}

s_surface GenSurface(const s_v2_s32 size, s_gl_resource_arena* const gl_res_arena) {
    t_gl_id* const fb_gl_id = PushToGLResourceArena(gl_res_arena, 1, ek_gl_resource_type_framebuffer).buf_raw;

    if (!fb_gl_id) {
        LOG_ERROR("Failed to reserve OpenGL framebuffer ID for surface!");
        return (s_surface){0};
    }

    t_gl_id* const fb_tex_gl_id = PushToGLResourceArena(gl_res_arena, 1, ek_gl_resource_type_texture).buf_raw;

    if (!fb_tex_gl_id) {
        LOG_ERROR("Failed to reserve OpenGL texture ID for surface framebuffer!");
        return (s_surface){0};
    }

    glGenFramebuffers(1, fb_gl_id);
    glGenTextures(1, fb_tex_gl_id);

    if (!AttachFramebufferTexture(*fb_gl_id, *fb_tex_gl_id, size)) {
        LOG_ERROR("Failed to attach framebuffer texture for surface!");
        return (s_surface){0};
    }

    return (s_surface){
        .fb_gl_id = fb_gl_id,
        .fb_tex_gl_id = fb_tex_gl_id
    };
}

bool ResizeSurface(s_surface* const surf, const s_v2_s32 size) {
    // Delete old texture.
    glDeleteTextures(1, surf->fb_tex_gl_id);

    // Generate and attach new texture of the new size.
    glGenTextures(1, surf->fb_tex_gl_id);

    if (!AttachFramebufferTexture(*surf->fb_gl_id, *surf->fb_tex_gl_id, size)) {
        LOG_ERROR("Failed to attach framebuffer texture for surface during resize!");
        return false;
    }

    return true;
}

static inline t_gl_id BoundGLFramebuffer() {
    int fb;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fb);
    return fb;
}

void SetSurface(const s_rendering_context* const rendering_context, const s_surface* const surf) {
    assert(*surf->fb_gl_id != BoundGLFramebuffer() && "Trying to set a surface that is already set!");

    SubmitBatch(rendering_context);

    glBindFramebuffer(GL_FRAMEBUFFER, *surf->fb_gl_id);
}

void UnsetSurface(const s_rendering_context* const rendering_context) {
    assert(BoundGLFramebuffer() != 0 && "Trying to unset surface but no OpenGL framebuffer is bound!");

    SubmitBatch(rendering_context);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static inline t_gl_id CurrentGLShaderProgram() {
    int prog;
    glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
    return prog;
}

void SetSurfaceShaderProg(const s_rendering_context* const rendering_context, const s_shader_prog_group* const progs, const int prog_index) {
    assert(CurrentGLShaderProgram() == 0 && "Potential attempted double-assignment of surface shader program?");
    glUseProgram(*GLIDElemView(progs->gl_ids, prog_index));
}

void SetSurfaceShaderProgUniform(const s_rendering_context* const rendering_context, const char* const name, const s_shader_prog_uniform_value val) {
    assert(name);

    const t_gl_id prog_gl_id = CurrentGLShaderProgram();

    assert(prog_gl_id != 0 && "Surface shader program must be set before setting uniforms!");

    const int loc = glGetUniformLocation(prog_gl_id, name);
    assert(loc != -1 && "Failed to get location of shader uniform!");

    switch (val.type) {
        case ek_shader_prog_uniform_value_type_s32:
            glUniform1i(loc, val.as_s32);
            break;

        case ek_shader_prog_uniform_value_type_r32:
            glUniform1f(loc, val.as_r32);
            break;

        case ek_shader_prog_uniform_value_type_v2:
            glUniform2f(loc, val.as_v2.x, val.as_v2.y);
            break;

        case ek_shader_prog_uniform_value_type_v3:
            glUniform3f(loc, val.as_v3.x, val.as_v3.y, val.as_v3.z);
            break;

        case ek_shader_prog_uniform_value_type_v4:
            glUniform4f(loc, val.as_v4.x, val.as_v4.y, val.as_v4.z, val.as_v4.w);
            break;

        case ek_shader_prog_uniform_value_type_mat4x4:
            glUniformMatrix4fv(loc, 1, false, &val.as_mat4x4.elems[0][0]);
            break;
    }
}

void RenderSurface(const s_rendering_context* const rendering_context, const s_surface* const surf) {
    assert(CurrentGLShaderProgram() != 0 && "Surface shader program must be set before rendering a surface!");

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, *surf->fb_tex_gl_id);

    const s_renderable* const renderable = &rendering_context->basis->renderables[ek_renderable_surface];

    glBindVertexArray(*renderable->vert_array_gl_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *renderable->elem_buf_gl_id);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);

    glUseProgram(0);
}
