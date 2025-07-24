#include "zfw_graphics.h"

static bool AttachFramebufferTexture(const zfw_t_gl_id fb_gl_id, const zfw_t_gl_id tex_gl_id, const zfw_s_vec_2d_i tex_size) {
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

bool ZFW_InitRenderSurfaces(zfw_s_render_surfaces* const surfs, zfw_s_mem_arena* const mem_arena, const int cnt, const zfw_s_vec_2d_i size) {
    assert(surfs && ZFW_IS_ZERO(*surfs));
    assert(cnt > 0);
    assert(size.x > 0 && size.y > 0);

    const size_t mem_arena_init_offs = mem_arena->offs;

    surfs->fb_gl_ids = ZFW_MEM_ARENA_PUSH_TYPE_MANY(mem_arena, zfw_t_gl_id, cnt);

    if (!surfs->fb_gl_ids) {
        goto error;
    }

    surfs->fb_tex_gl_ids = ZFW_MEM_ARENA_PUSH_TYPE_MANY(mem_arena, zfw_t_gl_id, cnt);

    if (!surfs->fb_tex_gl_ids) {
        goto error;
    }

    glGenFramebuffers(cnt, surfs->fb_gl_ids);
    glGenTextures(cnt, surfs->fb_tex_gl_ids);

    for (int i = 0; i < cnt; i++) {
        if (!AttachFramebufferTexture(surfs->fb_gl_ids[i], surfs->fb_tex_gl_ids[i], size)) {
            goto error;
        }
    }

    surfs->cnt = cnt;
    surfs->size = size;

    return true;

error:
    if (surfs->fb_tex_gl_ids) {
        glDeleteTextures(cnt, surfs->fb_tex_gl_ids);
    }

    if (surfs->fb_gl_ids) {
        glDeleteFramebuffers(cnt, surfs->fb_gl_ids);
    }

    ZFW_RewindMemArena(mem_arena, mem_arena_init_offs);

    ZFW_ZERO_OUT(*surfs);

    return false;
}

void ZFW_CleanRenderSurfaces(zfw_s_render_surfaces* const surfs) {
    assert(surfs && ZFW_IsRenderSurfacesValid(surfs));

    glDeleteTextures(surfs->cnt, surfs->fb_tex_gl_ids);
    glDeleteFramebuffers(surfs->cnt, surfs->fb_gl_ids);

    ZFW_ZERO_OUT(*surfs);
}

bool ZFWResizeRenderSurfaces(zfw_s_render_surfaces* const surfs, const zfw_s_vec_2d_i size) {
    assert(surfs && ZFW_IsRenderSurfacesValid(surfs));
    assert(size.x > 0 && size.y > 0);

    // TODO: I don't like the error handling in this function.

    surfs->size = size;

    // Delete old textures.
    glDeleteTextures(surfs->cnt, surfs->fb_tex_gl_ids);

    // Generate new ones with the given size.
    glGenTextures(surfs->cnt, surfs->fb_tex_gl_ids);

    for (int i = 0; i < surfs->cnt; i++) {
        if (!AttachFramebufferTexture(surfs->fb_gl_ids[i], surfs->fb_tex_gl_ids[i], size)) {
            return false;
        }
    }

    return true;
}
