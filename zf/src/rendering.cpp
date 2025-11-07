#include <zf/rendering.h>

namespace zf {
    static size_t CalcStride(const c_array<const int> vert_attr_lens) {
        size_t stride = 0;

        for (int i = 0; i < vert_attr_lens.Len(); i++) {
            stride += sizeof(int) * vert_attr_lens[i];
        }

        return stride;
    }

    static s_gl_mesh MakeGLMesh(const c_array<const float> verts, const c_array<const unsigned short> elems, const c_array<const int> vert_attr_lens) {
        s_gl_mesh mesh;

        glGenVertexArrays(1, &mesh.vert_arr_gl_id);
        glBindVertexArray(mesh.vert_arr_gl_id);

        glGenBuffers(1, &mesh.vert_buf_gl_id);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vert_buf_gl_id);
        glBufferData(GL_ARRAY_BUFFER, verts.SizeInBytes(), verts.Raw(), GL_DYNAMIC_DRAW);

        glGenBuffers(1, &mesh.elem_buf_gl_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.elem_buf_gl_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, elems.SizeInBytes(), elems.Raw(), GL_STATIC_DRAW);

        const size_t stride = CalcStride(vert_attr_lens);
        int offs = 0;

        for (int i = 0; i < vert_attr_lens.Len(); i++) {
            const int attr_len = vert_attr_lens[i];

            glVertexAttribPointer(i, attr_len, GL_FLOAT, false, stride, reinterpret_cast<void*>(sizeof(int) * offs));
            glEnableVertexAttribArray(i);

            offs += attr_len;
        }

        glBindVertexArray(0);

        return mesh;
    }

    static inline s_v2<int> GLTextureSizeLimit() {
        int size;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
        return {size, size};
    }

    static t_gl_id GenGLTextureFromRGBA(const s_rgba_texture& rgba_tex) {
        const s_v2<int> tex_size_limit = GLTextureSizeLimit();

        if (rgba_tex.size_in_pxs.x > tex_size_limit.x || rgba_tex.size_in_pxs.y > tex_size_limit.y) {
            ZF_LOG_ERROR("Texture size (%d, %d) exceeds OpenGL limits (%d, %d)!", rgba_tex.size_in_pxs.x, rgba_tex.size_in_pxs.y, tex_size_limit.x, tex_size_limit.y);
            return 0;
        }

        t_gl_id tex_gl_id;
        glGenTextures(1, &tex_gl_id);

        glBindTexture(GL_TEXTURE_2D, tex_gl_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rgba_tex.size_in_pxs.x, rgba_tex.size_in_pxs.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_tex.px_data.Raw());

        return tex_gl_id;
    }

    bool c_gfx_resource_arena::Init(c_mem_arena& mem_arena, const int cap) {
        ZF_ASSERT(cap > 0);

        m_hdls_taken = 0;
        return m_hdls.Init(mem_arena, cap);
    }

    void c_gfx_resource_arena::Release() {
        for (int i = 0; i < m_hdls_taken; i++) {
            const auto hdl = m_hdls[i];

            switch (hdl.type) {
            case ec_gfx_resource_type::mesh:
                glDeleteBuffers(1, &hdl.mesh.elem_buf_gl_id);
                glDeleteBuffers(1, &hdl.mesh.vert_buf_gl_id);
                glDeleteVertexArrays(1, &hdl.mesh.vert_arr_gl_id);
                break;

            case ec_gfx_resource_type::shader_prog:
                glDeleteProgram(hdl.shader_prog.gl_id);
                break;

            case ec_gfx_resource_type::texture:
                glDeleteTextures(1, &hdl.tex.gl_id);
                break;
            }
        }
    }

    s_gfx_resource_handle c_gfx_resource_arena::AddTexture(const s_rgba_texture& rgba_tex) {
        if (m_hdls_taken == m_hdls.Len()) {
            return {};
        }

        const t_gl_id tex_gl_id = GenGLTextureFromRGBA(rgba_tex);

        if (!tex_gl_id) {
            return {};
        }

        auto& hdl = m_hdls[m_hdls_taken];

        hdl = {
            .type = ec_gfx_resource_type::texture,
            .tex = {.gl_id = tex_gl_id}
        };

        m_hdls_taken++;

        return hdl;
    }

    bool c_texture_group::Load(c_mem_arena& mem_arena, c_gfx_resource_arena& lifetime, const int cnt, bool (* const rgba_tex_loader_func)(s_rgba_texture& rgba_tex, const int index)) {
        ZF_ASSERT(cnt > 0);
        ZF_ASSERT(rgba_tex_loader_func);

        if (!m_hdls.Init(mem_arena, cnt)) {
            ZF_BANANA_ERROR();
            return false;
        }

        if (!m_sizes.Init(mem_arena, cnt)) {
            ZF_BANANA_ERROR();
            return false;
        }

        for (int i = 0; i < cnt; i++) {
            s_rgba_texture rgba_tex;

            if (!rgba_tex_loader_func(rgba_tex, i)) {
                ZF_BANANA_ERROR();
                return false;
            }

            if (!rbga_tex.IsValid()) {
                ZF_BANANA_ERROR();
                return false;
            }

            m_hdls[i] = lifetime.AddTexture(rgba_tex);

            if (!m_hdls[i].IsValid()) {
                ZF_BANANA_ERROR();
                return false;
            }
        }

        return true;
    }
}
