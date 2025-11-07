#include <zf/rendering.h>

namespace zf {
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
            case ec_gfx_resource_type::texture:
                glDeleteTextures(1, &hdl.gl_id);
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
            .gl_id = tex_gl_id
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
