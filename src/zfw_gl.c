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

zfw_t_gl_id ZFW_GenGLTextureFromRGBAPixelData(const t_u8* const rgba_px_data, const zfw_s_vec_2d_s32 tex_size) {
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

zfw_s_texture_group ZFW_GenTexturesFromFiles(zfw_s_gl_resource_arena* const gl_res_arena, s_mem_arena* const mem_arena, const int tex_cnt, const char* const* const file_paths) {
    assert(gl_res_arena);
    assert(mem_arena && IsMemArenaValid(mem_arena));
    assert(tex_cnt > 0);
    assert(file_paths);

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
        if (!ZFW_GenGLTextureFromFile(&gl_ids[i], &sizes[i], file_paths[i])) {
            LOG_ERROR("Failed to generate OpenGL texture from file \"%s\"!", file_paths[i]);
            return (zfw_s_texture_group){0};
        }
    }

    return (zfw_s_texture_group){
        .gl_ids = gl_ids,
        .sizes = sizes,
        .cnt = tex_cnt
    };
}

bool ZFW_GenGLTextureFromFile(zfw_t_gl_id* const tex_gl_id, zfw_s_vec_2d_s32* const tex_size, const char* const file_path) {
    assert(tex_gl_id && !(*tex_gl_id));
    assert(tex_size && IS_ZERO(*tex_size));
    assert(file_path);

    unsigned char* const px_data = stbi_load(file_path, &tex_size->x, &tex_size->y, NULL, 4);

    if (!px_data) {
        LOG_ERROR("Failed to load pixel data for texture from file \"%s\"!", file_path);
        LOG_ERROR_SPECIAL("STB", "%s", stbi_failure_reason());
        return false;
    }

    *tex_gl_id = ZFW_GenGLTextureFromRGBAPixelData(px_data, *tex_size);

    return true;
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

zfw_t_gl_id ZFW_GenShaderFromSrc(const char* const src, const bool frag, s_mem_arena* const temp_mem_arena) {
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

zfw_t_gl_id ZFW_GenShaderProg(const char* const vs_src, const char* const fs_src, s_mem_arena* const temp_mem_arena) {
    assert(vs_src);
    assert(fs_src);
    assert(temp_mem_arena && IsMemArenaValid(temp_mem_arena));

    const zfw_t_gl_id vs_gl_id = ZFW_GenShaderFromSrc(vs_src, false, temp_mem_arena);

    if (!vs_gl_id) {
        LOG_ERROR("Failed to generate vertex shader from source!");
        return 0;
    }

    const zfw_t_gl_id fs_gl_id = ZFW_GenShaderFromSrc(fs_src, true, temp_mem_arena);

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
