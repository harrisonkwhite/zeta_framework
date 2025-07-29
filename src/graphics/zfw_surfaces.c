#include "zfw_graphics.h"

static bool AttachFramebufferTexture(const zfw_t_gl_id fb_gl_id, const zfw_t_gl_id tex_gl_id, const zfw_s_vec_2d_s32 tex_size) {
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

static void PushSurfaceIndex(zfw_s_surface_index_stack* const stack, const int index, const int surf_cnt) {
    assert(stack && ZFW_IsSurfaceIndexStackValid(stack, surf_cnt));
    assert(index >= 0 && index < surf_cnt);
    assert(surf_cnt > 0 && surf_cnt <= ZFW_SURFACE_LIMIT);
    assert(stack->height < surf_cnt && "Cannot push to a full stack!");

    for (int i = 0; i < stack->height; i++) {
        assert(stack->buf[i] != index && "Cannot push a surface index already in the stack!");
    }

    stack->buf[stack->height] = index;
    stack->height++;
}

static int PopSurfaceIndex(zfw_s_surface_index_stack* const stack, const int surf_cnt) {
    assert(stack && ZFW_IsSurfaceIndexStackValid(stack, surf_cnt));
    assert(surf_cnt > 0 && surf_cnt <= ZFW_SURFACE_LIMIT);
    assert(stack->height > 0 && "Cannot pop from an empty stack!");

    stack->height--;
    return stack->buf[stack->height];
}

bool ZFW_InitSurfaces(zfw_s_surfaces* const surfs, s_mem_arena* const mem_arena, const int cnt, const zfw_s_vec_2d_s32 size) {
    assert(surfs && IS_ZERO(*surfs));
    assert(cnt > 0);
    assert(size.x > 0 && size.y > 0);

    const size_t mem_arena_init_offs = mem_arena->offs;

    glGenFramebuffers(cnt, surfs->fb_gl_ids);
    glGenTextures(cnt, surfs->fb_tex_gl_ids);

    for (int i = 0; i < cnt; i++) {
        if (!AttachFramebufferTexture(surfs->fb_gl_ids[i], surfs->fb_tex_gl_ids[i], size)) {
            LOG_ERROR("Failed to attach framebuffer texture for surface %d!", i);

            glDeleteTextures(cnt, surfs->fb_tex_gl_ids);
            glDeleteFramebuffers(cnt, surfs->fb_gl_ids);

            ZERO_OUT(*surfs);

            return false;
        }
    }

    surfs->cnt = cnt;
    surfs->size = size;

    return true;
}

void ZFW_CleanSurfaces(zfw_s_surfaces* const surfs) {
    assert(surfs && ZFW_IsSurfacesValid(surfs));

    if (surfs->cnt > 0) {
        glDeleteTextures(surfs->cnt, surfs->fb_tex_gl_ids);
        glDeleteFramebuffers(surfs->cnt, surfs->fb_gl_ids);
    }

    ZERO_OUT(*surfs);
}

bool ZFW_ResizeSurfaces(zfw_s_surfaces* const surfs, const zfw_s_vec_2d_s32 size) {
    assert(surfs && ZFW_IsSurfacesValid(surfs));
    assert(size.x > 0 && size.y > 0);

    surfs->size = size;

    // Delete old textures.
    glDeleteTextures(surfs->cnt, surfs->fb_tex_gl_ids);

    // Generate new ones with the given size.
    glGenTextures(surfs->cnt, surfs->fb_tex_gl_ids);

    for (int i = 0; i < surfs->cnt; i++) {
        if (!AttachFramebufferTexture(surfs->fb_gl_ids[i], surfs->fb_tex_gl_ids[i], size)) {
            LOG_ERROR("Failed to attach framebuffer texture for surface %d!", i);
            return false;
        }
    }

    return true;
}

zfw_s_renderable ZFW_GenSurfaceRenderable() {
    const float verts[] = {
        -1.0, -1.0, 0.0, 0.0,
        1.0, -1.0, 1.0, 0.0,
        1.0,  1.0, 1.0, 1.0,
        -1.0,  1.0, 0.0, 1.0
    };

    const unsigned short elems[] = {
        0, 1, 2,
        2, 3, 0
    };

    const int vert_attr_lens[] = {
        2,
        2
    };

    return ZFW_GenRenderable(verts, sizeof(verts), elems, sizeof(elems), vert_attr_lens, STATIC_ARRAY_LEN(vert_attr_lens));
}

bool ZFW_SetSurface(const zfw_s_rendering_context* const rendering_context, const int surf_index) {
    assert(rendering_context && ZFW_IsRenderingContextValid(rendering_context));

    zfw_s_rendering_basis* const rb = rendering_context->basis;
    zfw_s_rendering_state* const rs = rendering_context->state;

    assert(rs->batch.num_slots_used == 0 && "Submit the current batch before changing surface!");

    assert(surf_index >= 0 && surf_index < rb->surfs.cnt);

    if (rs->surf_index_stack.height == rb->surfs.cnt) {
        LOG_ERROR("Failed to set surface as the limit has been reached!");
        return false;
    }

    PushSurfaceIndex(&rs->surf_index_stack, surf_index, rb->surfs.cnt);

    // Bind the surface framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, rendering_context->basis->surfs.fb_gl_ids[surf_index]);

    return true;
}

void ZFW_UnsetSurface(const zfw_s_rendering_context* const rendering_context) {
    assert(rendering_context && ZFW_IsRenderingContextValid(rendering_context));

    zfw_s_rendering_basis* const rb = rendering_context->basis;
    zfw_s_rendering_state* const rs = rendering_context->state;

    assert(rs->batch.num_slots_used == 0 && "Submit the current batch before changing surface!");
    assert(rs->surf_index_stack.height > 0 && "There must be a surface to unset!");

    (void)PopSurfaceIndex(&rs->surf_index_stack, rb->surfs.cnt);

    if (rs->surf_index_stack.height == 0) {
        // No more surfaces, so revert to default FB.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    } else {
        // Use the FB of the surface now at the top of the stack.
        const int new_surf_index = rs->surf_index_stack.buf[rs->surf_index_stack.height - 1];
        const zfw_t_gl_id fb_gl_id = rendering_context->basis->surfs.fb_gl_ids[new_surf_index];
        glBindFramebuffer(GL_FRAMEBUFFER, fb_gl_id);
    }
}

void ZFW_SetSurfaceShaderProg(const zfw_s_rendering_context* const rendering_context, const int prog_index, const zfw_s_shader_progs* const progs) {
    assert(rendering_context && ZFW_IsRenderingContextValid(rendering_context));
    assert(prog_index >= 0 && prog_index < progs->cnt);
    assert(progs && ZFW_IsShaderProgsValid(progs));

    assert(rendering_context->state->surf_shader_prog_gl_id == 0); // Prevent accidental double assignments.

    rendering_context->state->surf_shader_prog_gl_id = progs->gl_ids[prog_index];
    glUseProgram(rendering_context->state->surf_shader_prog_gl_id);
}

void ZFW_SetSurfaceShaderProgUniform(const zfw_s_rendering_context* const rendering_context, const char* const name, const zfw_s_shader_prog_uniform_value val) {
    assert(rendering_context && ZFW_IsRenderingContextValid(rendering_context));
    assert(name);
    assert(rendering_context->state->surf_shader_prog_gl_id != 0 && "Surface shader program must be set before modifying uniforms!");

    const int loc = glGetUniformLocation(rendering_context->state->surf_shader_prog_gl_id, name);
    assert(loc != -1 && "Failed to get location of shader uniform!");

    switch (val.type) {
        case zfw_ek_shader_prog_uniform_value_type_int:
            glUniform1i(loc, val.as_int);
            break;

        case zfw_ek_shader_prog_uniform_value_type_float:
            glUniform1f(loc, val.as_float);
            break;

        case zfw_ek_shader_prog_uniform_value_type_v2:
            glUniform2f(loc, val.as_v2.x, val.as_v2.y);
            break;

        case zfw_ek_shader_prog_uniform_value_type_v3:
            glUniform3f(loc, val.as_v3.x, val.as_v3.y, val.as_v3.z);
            break;

        case zfw_ek_shader_prog_uniform_value_type_v4:
            glUniform4f(loc, val.as_v4.x, val.as_v4.y, val.as_v4.z, val.as_v4.w);
            break;

        case zfw_ek_shader_prog_uniform_value_type_mat4x4:
            glUniformMatrix4fv(loc, 1, false, &val.as_mat4x4[0][0]);
            break;
    }
}

void ZFW_RenderSurface(const zfw_s_rendering_context* const rendering_context, const int surf_index) {
    assert(rendering_context && ZFW_IsRenderingContextValid(rendering_context));
    assert(rendering_context->state->surf_shader_prog_gl_id != 0 && "Surface shader program must be set before rendering a surface!");
    assert(surf_index >= 0 && surf_index < rendering_context->basis->surfs.cnt);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rendering_context->basis->surfs.fb_tex_gl_ids[surf_index]);

    glBindVertexArray(rendering_context->basis->surf_renderable.vert_array_gl_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rendering_context->basis->surf_renderable.elem_buf_gl_id);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
}
