#pragma once

#include <zc.h>
#include <glad/glad.h>

namespace zf {
    using t_gl_id = GLuint;

    struct s_mesh_gl_ids {
        t_gl_id vert_arr;
        t_gl_id vert_buf;
        t_gl_id elem_buf;
    };

    s_mesh_gl_ids MakeGLMesh(const t_f32* const verts_raw, const t_size verts_len, const s_array_rdonly<t_u16> elems, const s_array_rdonly<t_s32> vert_attr_lens);
    void ReleaseGLMesh(const s_mesh_gl_ids& gl_ids);

    t_gl_id MakeGLShaderProg(const s_str_rdonly vert_src, const s_str_rdonly frag_src, s_mem_arena& temp_mem_arena);

    t_gl_id MakeGLTexture(const s_rgba_texture_data_rdonly& tex_data);

    inline s_v2<t_s32> GLTextureSizeLimit() {
        t_s32 size;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
        return { size, size };
    }
}
