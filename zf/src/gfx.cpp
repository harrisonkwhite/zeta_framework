#include <zf/rendering.h>

namespace zf {
    static size_t CalcStride(const c_array<const int> vert_attr_lens) {
        size_t stride = 0;

        for (int i = 0; i < vert_attr_lens.Len(); i++) {
            stride += sizeof(int) * vert_attr_lens[i];
        }

        return stride;
    }

    static s_gl_mesh MakeGLMesh(const float* const verts_raw, const int verts_len, const c_array<const unsigned short> elems, const c_array<const int> vert_attr_lens) {
        s_gl_mesh mesh;

        glGenVertexArrays(1, &mesh.vert_arr_gl_id);
        glBindVertexArray(mesh.vert_arr_gl_id);

        glGenBuffers(1, &mesh.vert_buf_gl_id);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vert_buf_gl_id);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * verts_len, verts_raw, GL_DYNAMIC_DRAW);

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

    static t_gl_id MakeGLShaderProg(const s_str_view vert_src, const s_str_view frag_src, c_mem_arena& temp_mem_arena) {
        ZF_ASSERT(vert_src.IsTerminated());
        ZF_ASSERT(frag_src.IsTerminated());

        // Generate the individual shaders.
        const auto shader_gen_func = [&temp_mem_arena](const s_str_view src, const bool is_frag) -> bool {
            const t_gl_id shader_gl_id = glCreateShader(is_frag ? GL_FRAGMENT_SHADER : GL_VERTEX_SHADER);

            const auto src_raw = src.Raw();
            glShaderSource(shader_gl_id, 1, &src_raw, nullptr);

            glCompileShader(shader_gl_id);

            int success;
            glGetShaderiv(shader_gl_id, GL_COMPILE_STATUS, &success);

            if (!success) {
                // Try getting the OpenGL compile error message.
                int log_chr_cnt;
                glGetShaderiv(shader_gl_id, GL_INFO_LOG_LENGTH, &log_chr_cnt);

                if (log_chr_cnt > 1) {
                    c_array<char> log_chrs;

                    if (log_chrs.Init(temp_mem_arena, log_chr_cnt)) {
                        glGetShaderInfoLog(shader_gl_id, log_chrs.Len(), nullptr, log_chrs.Raw());
                        ZF_LOG_ERROR_SPECIAL("OpenGL Shader Compilation", "%s", log_chrs.Raw());
                    } else {
                        ZF_LOG_ERROR("Failed to reserve memory for OpenGL shader compilation error log!");
                    }
                } else {
                    ZF_LOG_ERROR("OpenGL shader compilation failed, but no error message available!");
                }

                glDeleteShader(shader_gl_id);

                return 0;
            }

            return shader_gl_id;
        };

        const t_gl_id vert_gl_id = shader_gen_func(vert_src, false);

        if (!vert_gl_id) {
            return 0;
        }

        const t_gl_id frag_gl_id = shader_gen_func(frag_src, true);

        if (!frag_gl_id) {
            glDeleteShader(vert_gl_id);
            return 0;
        }

        // Set up the shader program.
        const t_gl_id prog_gl_id = glCreateProgram();
        glAttachShader(prog_gl_id, vert_gl_id);
        glAttachShader(prog_gl_id, frag_gl_id);
        glLinkProgram(prog_gl_id);

        // Dispose the shaders, they're no longer needed.
        glDeleteShader(frag_gl_id);
        glDeleteShader(vert_gl_id);

        return prog_gl_id;
    }

    static inline s_v2<int> GLTextureSizeLimit() {
        int size;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
        return { size, size };
    }

    static t_gl_id GenGLTextureFromRGBA(const c_rgba_texture& rgba_tex) {
        const s_v2<int> tex_size_limit = GLTextureSizeLimit();

        if (rgba_tex.SizeInPixels().x > tex_size_limit.x || rgba_tex.SizeInPixels().y > tex_size_limit.y) {
            ZF_LOG_ERROR("Texture size (%d, %d) exceeds OpenGL limits (%d, %d)!", rgba_tex.SizeInPixels().x, rgba_tex.SizeInPixels().y, tex_size_limit.x, tex_size_limit.y);
            return 0;
        }

        t_gl_id tex_gl_id;
        glGenTextures(1, &tex_gl_id);

        glBindTexture(GL_TEXTURE_2D, tex_gl_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rgba_tex.SizeInPixels().x, rgba_tex.SizeInPixels().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_tex.PixelData().Raw());

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

    s_gfx_resource_handle c_gfx_resource_arena::AddMesh(const float* const verts_raw, const int verts_len, const c_array<const unsigned short> elems, const c_array<const int> vert_attr_lens) {
        ZF_ASSERT(verts_len > 0);
        ZF_ASSERT(!elems.IsEmpty());
        ZF_ASSERT(!vert_attr_lens.IsEmpty());

        if (m_hdls_taken == m_hdls.Len()) {
            return {};
        }

        auto& hdl = m_hdls[m_hdls_taken];
        hdl.type = ec_gfx_resource_type::mesh;
        hdl.mesh = MakeGLMesh(verts_raw, verts_len, elems, vert_attr_lens);

        m_hdls_taken++;

        return hdl;
    }

    s_gfx_resource_handle c_gfx_resource_arena::AddShaderProg(const s_str_view vert_src, const s_str_view frag_src, c_mem_arena& temp_mem_arena) {
        ZF_ASSERT(vert_src.IsTerminated());
        ZF_ASSERT(frag_src.IsTerminated());

        if (m_hdls_taken == m_hdls.Len()) {
            return {};
        }

        const t_gl_id prog_gl_id = MakeGLShaderProg(vert_src, frag_src, temp_mem_arena);

        if (!prog_gl_id) {
            return {};
        }

        auto& hdl = m_hdls[m_hdls_taken];
        hdl.type = ec_gfx_resource_type::shader_prog;
        hdl.shader_prog.gl_id = prog_gl_id;

        m_hdls_taken++;

        return hdl;
    }

    s_gfx_resource_handle c_gfx_resource_arena::AddTexture(const c_rgba_texture& rgba_tex) {
        ZF_ASSERT(rgba_tex.IsLoaded());

        if (m_hdls_taken == m_hdls.Len()) {
            return {};
        }

        const t_gl_id tex_gl_id = GenGLTextureFromRGBA(rgba_tex);

        if (!tex_gl_id) {
            return {};
        }

        auto& hdl = m_hdls[m_hdls_taken];
        hdl.type = ec_gfx_resource_type::texture;
        hdl.tex.gl_id = tex_gl_id;

        m_hdls_taken++;

        return hdl;
    }

    bool c_texture_group::Load(c_mem_arena& mem_arena, c_gfx_resource_arena& lifetime, const int cnt, bool (* const rgba_tex_loader_func)(c_rgba_texture& rgba_tex, const int index)) {
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
            c_rgba_texture rgba_tex;

            if (!rgba_tex_loader_func(rgba_tex, i)) {
                ZF_BANANA_ERROR();
                return false;
            }

            if (!rgba_tex.IsLoaded()) {
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