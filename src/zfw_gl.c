#include "zfw_gl.h"

#include <stb_image.h>

zfw_s_gl_resource_arena ZFW_GenGLResourceArena(s_mem_arena* const mem_arena, const int res_limit) {
    assert(mem_arena && IsMemArenaValid(mem_arena));
    assert(res_limit > 0);

    zfw_t_gl_id* const gl_ids = MEM_ARENA_PUSH_TYPE_CNT(mem_arena, zfw_t_gl_id, res_limit);

    if (!gl_ids) {
        LOG_ERROR("Failed to reserve memory for OpenGL resource IDs!");
        return (zfw_s_gl_resource_arena){0};
    }

    zfw_e_gl_resource_type* const res_types = MEM_ARENA_PUSH_TYPE_CNT(mem_arena, zfw_e_gl_resource_type, res_limit);

    if (!res_types) {
        LOG_ERROR("Failed to reserve memory for OpenGL resource types!");
        return (zfw_s_gl_resource_arena){0};
    }

    return (zfw_s_gl_resource_arena){
        .ids = gl_ids,
        .res_types = res_types,
        .res_limit = res_limit
    };
}

void ZFW_CleanGLResourceArena(zfw_s_gl_resource_arena* const res_arena) {
    assert(res_arena);

    for (int i = 0; i < res_arena->res_used; i++) {
        const zfw_t_gl_id gl_id = res_arena->ids[i];

        if (!gl_id) {
            continue;
        }

        switch ((zfw_e_gl_resource_type)i) {
            case zfw_ek_gl_resource_type_texture:
                glDeleteTextures(1, &gl_id);
                break;

            case zfw_ek_gl_resource_type_shader_prog:
                glDeleteProgram(gl_id);
                break;

            case zfw_ek_gl_resource_type_vert_array:
                glDeleteVertexArrays(1, &gl_id);
                break;

            case zfw_ek_gl_resource_type_vert_buf:
            case zfw_ek_gl_resource_type_elem_buf:
                glDeleteBuffers(1, &gl_id);
                break;

            case zfw_ek_gl_resource_type_framebuffer:
                glDeleteFramebuffers(1, &gl_id);
                break;
        }
    }
}

zfw_t_gl_id* ZFW_ReserveGLIDs(zfw_s_gl_resource_arena* const res_arena, const int cnt, const zfw_e_gl_resource_type res_type) {
    assert(res_arena);
    assert(cnt > 0);
    assert(res_type >= 0 && res_type < zfw_eks_gl_resource_type_cnt);

    if (res_arena->res_used + cnt > res_arena->res_limit) {
        LOG_ERROR("OpenGL resource arena is full! Cannot reserve %d more IDs!", cnt);
        return NULL;
    }

    const int res_used_prev = res_arena->res_used;
    res_arena->res_used += cnt;
    return res_arena->ids + res_used_prev;
}

static zfw_t_gl_id GenGLTextureFromRGBAPixelData(const t_u8* const rgba_px_data, const zfw_s_vec_2d_s32 tex_size) {
    assert(rgba_px_data);
    assert(tex_size.x > 0 && tex_size.y > 0);

    zfw_t_gl_id tex_gl_id;
    glGenTextures(1, &tex_gl_id);

    glBindTexture(GL_TEXTURE_2D, tex_gl_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_size.x, tex_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_px_data);

    return tex_gl_id;
}

zfw_s_texture_info ZFW_GenTextureInfoFromFile(const char* const file_path, s_mem_arena* const mem_arena) {
    assert(file_path);
    assert(mem_arena && IsMemArenaValid(mem_arena));

    zfw_s_vec_2d_s32 tex_size = {0};
    unsigned char* const rgba_px_data = stbi_load(file_path, &tex_size.x, &tex_size.y, NULL, 4);

    if (!rgba_px_data) {
        LOG_ERROR("Failed to load pixel data from file \"%s\"!", file_path);
        LOG_ERROR_SPECIAL("STB", "%s", stbi_failure_reason());
        return (zfw_s_texture_info){0};
    }

    return (zfw_s_texture_info){
        .rgba_px_data = rgba_px_data,
        .tex_size = tex_size
    };
}

zfw_s_texture_group ZFW_GenTextures(const int tex_cnt, const zfw_t_gen_texture_info_func gen_tex_info_func, zfw_s_gl_resource_arena* const gl_res_arena, s_mem_arena* const mem_arena, s_mem_arena* const temp_mem_arena) {
    assert(tex_cnt > 0);
    assert(gen_tex_info_func);
    assert(gl_res_arena);
    assert(mem_arena && IsMemArenaValid(mem_arena));
    assert(temp_mem_arena && IsMemArenaValid(temp_mem_arena));

    zfw_t_gl_id* const gl_ids = ZFW_ReserveGLIDs(gl_res_arena, tex_cnt, zfw_ek_gl_resource_type_texture);

    if (!gl_ids) {
        LOG_ERROR("Failed to reserve OpenGL texture IDs!");
        return (zfw_s_texture_group){0};
    }

    zfw_s_vec_2d_s32* const sizes = MEM_ARENA_PUSH_TYPE_CNT(mem_arena, zfw_s_vec_2d_s32, tex_cnt);

    if (!sizes) {
        LOG_ERROR("Failed to reserve memory for texture sizes!");
        return (zfw_s_texture_group){0};
    }

    for (int i = 0; i < tex_cnt; i++) {
        const zfw_s_texture_info tex_info = gen_tex_info_func(i, temp_mem_arena);

        if (IS_ZERO(tex_info)) {
            // TODO: Log error.
            return (zfw_s_texture_group){0};
        }

        assert(tex_info.rgba_px_data && tex_info.tex_size.x > 0 && tex_info.tex_size.y > 0);

        gl_ids[i] = GenGLTextureFromRGBAPixelData(tex_info.rgba_px_data, tex_info.tex_size);
        sizes[i] = tex_info.tex_size;
    }

    return (zfw_s_texture_group){
        .gl_ids = gl_ids,
        .sizes = sizes,
        .cnt = tex_cnt
    };
}

zfw_s_rect_edges ZFW_TextureCoords(const zfw_s_rect_s32 src_rect, const zfw_s_vec_2d_s32 tex_size) {
    assert(ZFW_IsSrcRectValid(src_rect, tex_size));
    assert(tex_size.x > 0 && tex_size.y > 0);

    return (zfw_s_rect_edges){
        .left = (float)src_rect.x / tex_size.x,
        .top = (float)src_rect.y / tex_size.y,
        .right = (float)(src_rect.x + src_rect.width) / tex_size.x,
        .bottom = (float)(src_rect.y + src_rect.height) / tex_size.y
    };
}

static zfw_t_gl_id GenShaderFromSrc(const char* const src, const bool frag, s_mem_arena* const temp_mem_arena) {
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

static zfw_t_gl_id GenShaderProg(const char* const vs_src, const char* const fs_src, s_mem_arena* const temp_mem_arena) {
    assert(vs_src);
    assert(fs_src);
    assert(temp_mem_arena && IsMemArenaValid(temp_mem_arena));

    const zfw_t_gl_id vs_gl_id = GenShaderFromSrc(vs_src, false, temp_mem_arena);

    if (!vs_gl_id) {
        LOG_ERROR("Failed to generate vertex shader from source!");
        return 0;
    }

    const zfw_t_gl_id fs_gl_id = GenShaderFromSrc(fs_src, true, temp_mem_arena);

    if (!fs_gl_id) {
        LOG_ERROR("Failed to generate fragment shader from source!");
        glDeleteShader(vs_gl_id);
        return 0;
    }

    const zfw_t_gl_id prog_gl_id = glCreateProgram();
    glAttachShader(prog_gl_id, vs_gl_id);
    glAttachShader(prog_gl_id, fs_gl_id);
    glLinkProgram(prog_gl_id);

    glDeleteShader(fs_gl_id);
    glDeleteShader(vs_gl_id);

    return prog_gl_id;
}

zfw_s_shader_prog_group ZFW_GenShaderProgs(const int prog_cnt, const zfw_t_gen_shader_prog_info_func gen_prog_info_func, zfw_s_gl_resource_arena* const gl_res_arena, s_mem_arena* const temp_mem_arena) {
    zfw_t_gl_id* const gl_ids = ZFW_ReserveGLIDs(gl_res_arena, prog_cnt, zfw_ek_gl_resource_type_shader_prog);

    if (!gl_ids) {
        LOG_ERROR("Failed to reserve OpenGL shader program IDs for built-in shader programs!");
        return (zfw_s_shader_prog_group){0};
    }

    for (int i = 0; i < prog_cnt; i++) {
        const zfw_s_shader_prog_info prog_info = gen_prog_info_func(i, temp_mem_arena);

        gl_ids[i] = GenShaderProg(prog_info.vs_src, prog_info.fs_src, temp_mem_arena);

        if (!gl_ids[i]) {
            LOG_ERROR("Failed to generate shader program with index %d!", i);
            return (zfw_s_shader_prog_group){0};
        }
    }

    return (zfw_s_shader_prog_group){
        .gl_ids = gl_ids,
        .cnt = prog_cnt
    };
}

static size_t Stride(const int* const vert_attr_lens, const int vert_attr_cnt) {
    assert(vert_attr_lens);
    assert(vert_attr_cnt > 0);

    int stride = 0;

    for (int i = 0; i < vert_attr_cnt; i++) {
        assert(vert_attr_lens[i] > 0);
        stride += sizeof(float) * vert_attr_lens[i];
    }

    return stride;
}

void ZFW_GenRenderable(zfw_t_gl_id* const va_gl_id, zfw_t_gl_id* const vb_gl_id, zfw_t_gl_id* const eb_gl_id, const float* const vert_buf, const size_t vert_buf_size, const unsigned short* const elem_buf, const size_t elem_buf_size, const int* const vert_attr_lens, const int vert_attr_cnt) {
    assert(va_gl_id && !(*va_gl_id));
    assert(vb_gl_id && !(*vb_gl_id));
    assert(eb_gl_id && !(*eb_gl_id));
    assert(vert_buf_size > 0);
    assert(elem_buf && elem_buf_size > 0);
    assert(vert_attr_lens && vert_attr_cnt > 0);

    glGenVertexArrays(1, va_gl_id);
    glBindVertexArray(*va_gl_id);

    glGenBuffers(1, vb_gl_id);
    glBindBuffer(GL_ARRAY_BUFFER, *vb_gl_id);
    glBufferData(GL_ARRAY_BUFFER, vert_buf_size, vert_buf, GL_DYNAMIC_DRAW);

    glGenBuffers(1, eb_gl_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *eb_gl_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, elem_buf_size, elem_buf, GL_STATIC_DRAW);

    const GLsizei stride = Stride(vert_attr_lens, vert_attr_cnt);
    int offs = 0;

    for (int i = 0; i < vert_attr_cnt; i++) {
        assert(vert_attr_lens[i] > 0);

        glVertexAttribPointer(i, vert_attr_lens[i], GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * offs));
        glEnableVertexAttribArray(i);

        offs += vert_attr_lens[i];
    }

    glBindVertexArray(0);
}

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

zfw_s_surface_group ZFW_GenSurfaces(const int surf_cnt, const zfw_s_vec_2d_s32* const surf_sizes, zfw_s_gl_resource_arena* const gl_res_arena, s_mem_arena* const mem_arena) {
    assert(surf_cnt > 0);
    assert(surf_sizes);
    assert(gl_res_arena);
    assert(mem_arena && IsMemArenaValid(mem_arena));

    zfw_t_gl_id* const fb_gl_ids = ZFW_ReserveGLIDs(gl_res_arena, surf_cnt, zfw_ek_gl_resource_type_framebuffer);

    if (!fb_gl_ids) {
        LOG_ERROR("Failed to reserve OpenGL framebuffer IDs!");
        return (zfw_s_surface_group){0};
    }

    zfw_t_gl_id* const fb_tex_gl_ids = ZFW_ReserveGLIDs(gl_res_arena, surf_cnt, zfw_ek_gl_resource_type_texture);

    if (!fb_tex_gl_ids) {
        LOG_ERROR("Failed to reserve OpenGL texture IDs for framebuffers!");
        return (zfw_s_surface_group){0};
    }

    zfw_s_vec_2d_s32* const surf_sizes_pers = MEM_ARENA_PUSH_TYPE_CNT(mem_arena, zfw_s_vec_2d_s32, surf_cnt);

    if (!surf_sizes_pers) {
        LOG_ERROR("Failed to reserve memory for surface sizes!");
        return (zfw_s_surface_group){0};
    }

    glGenFramebuffers(surf_cnt, fb_gl_ids);
    glGenTextures(surf_cnt, fb_tex_gl_ids);

    for (int i = 0; i < surf_cnt; i++) {
        if (!AttachFramebufferTexture(fb_gl_ids[i], fb_tex_gl_ids[i], surf_sizes[i])) {
            LOG_ERROR("Failed to attach framebuffer texture for surface %d!", i);
            return (zfw_s_surface_group){0};
        }
    }

    memcpy(surf_sizes_pers, surf_sizes, sizeof(*surf_sizes) * surf_cnt);

    return (zfw_s_surface_group){
        .fb_gl_ids = fb_gl_ids,
        .fb_tex_gl_ids = fb_tex_gl_ids,
        .sizes = surf_sizes_pers,
        .cnt = surf_cnt
    };
}

bool ZFW_ResizeSurface(zfw_s_surface_group* const surfs, const int surf_index, const zfw_s_vec_2d_s32 size) {
    assert(surfs);
    assert(surf_index >= 0 && surf_index < surfs->cnt);
    assert(size.x > 0 && size.y > 0);
    assert(size.x != surfs->sizes[surf_index].x || size.y != surfs->sizes[surf_index].y);

    // Delete old texture.
    glDeleteTextures(surfs->cnt, &surfs->fb_tex_gl_ids[surf_index]);

    // Generate and attach new texture of the new size.
    glGenTextures(surfs->cnt, &surfs->fb_tex_gl_ids[surf_index]);

    for (int i = 0; i < surfs->cnt; i++) {
        if (!AttachFramebufferTexture(surfs->fb_gl_ids[i], surfs->fb_tex_gl_ids[i], size)) {
            LOG_ERROR("Failed to attach framebuffer texture for surface %d!", i);
            return false;
        }
    }

    return true;
}
