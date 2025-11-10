#include <zf/gfx.h>

#include <zf/rendering.h>

namespace zf {
    static t_size CalcStride(const c_array<const t_s32> vert_attr_lens) {
        t_size stride = 0;

        for (t_size i = 0; i < vert_attr_lens.Len(); i++) {
            stride += ZF_SIZE_OF(t_s32) * static_cast<t_size>(vert_attr_lens[i]);
        }

        return stride;
    }

    static us_gl_mesh MakeGLMesh(const t_f32* const verts_raw, const t_size verts_len, const c_array<const t_u16> elems, const c_array<const t_s32> vert_attr_lens) {
        us_gl_mesh mesh = {};

        glGenVertexArrays(1, &mesh.vert_arr_gl_id);
        glBindVertexArray(mesh.vert_arr_gl_id);

        glGenBuffers(1, &mesh.vert_buf_gl_id);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vert_buf_gl_id);
        glBufferData(GL_ARRAY_BUFFER, ZF_SIZE_OF(t_f32) * verts_len, verts_raw, GL_DYNAMIC_DRAW);

        glGenBuffers(1, &mesh.elem_buf_gl_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.elem_buf_gl_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(elems.SizeInBytes()), elems.Raw(), GL_STATIC_DRAW);

        const t_size stride = CalcStride(vert_attr_lens);
        t_s32 offs = 0;

        for (t_size i = 0; i < vert_attr_lens.Len(); i++) {
            const t_s32 attr_len = vert_attr_lens[i];

            glVertexAttribPointer(static_cast<GLuint>(i), attr_len, GL_FLOAT, false, static_cast<GLsizei>(stride), reinterpret_cast<void*>(ZF_SIZE_OF(t_s32) * offs));
            glEnableVertexAttribArray(static_cast<GLuint>(i));

            offs += attr_len;
        }

        glBindVertexArray(0);

        return mesh;
    }

    static t_gl_id MakeGLShaderProg(const s_str_view vert_src, const s_str_view frag_src, c_mem_arena& temp_mem_arena) {
        ZF_ASSERT(vert_src.IsTerminated());
        ZF_ASSERT(frag_src.IsTerminated());

        // Generate the individual shaders.
        const auto shader_gen_func = [&temp_mem_arena](const s_str_view src, const t_b8 is_frag) -> t_gl_id {
            const t_gl_id shader_gl_id = glCreateShader(is_frag ? GL_FRAGMENT_SHADER : GL_VERTEX_SHADER);

            const auto src_raw = src.Raw();
            glShaderSource(shader_gl_id, 1, &src_raw, nullptr);

            glCompileShader(shader_gl_id);

            t_s32 success;
            glGetShaderiv(shader_gl_id, GL_COMPILE_STATUS, &success);

            if (!success) {
                // Try getting the OpenGL compile error message.
                t_s32 log_chr_cnt;
                glGetShaderiv(shader_gl_id, GL_INFO_LOG_LENGTH, &log_chr_cnt);

                if (log_chr_cnt > 1) {
                    c_array<char> log_chrs;

                    if (log_chrs.Init(temp_mem_arena, log_chr_cnt)) {
                        glGetShaderInfoLog(shader_gl_id, static_cast<GLsizei>(log_chrs.Len()), nullptr, log_chrs.Raw());
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

    static inline s_v2<t_s32> GLTextureSizeLimit() {
        t_s32 size;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
        return { size, size };
    }

    static t_gl_id GenGLTextureFromRGBA(const c_rgba_texture& rgba_tex) {
        const s_v2<t_s32> tex_size_limit = GLTextureSizeLimit();

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

    t_b8 c_gfx_resource_arena::Init(c_mem_arena& mem_arena, const t_size cap) {
        ZF_ASSERT(cap > 0);

        m_hdls_taken = 0;
        return m_hdls.Init(mem_arena, cap);
    }

    void c_gfx_resource_arena::Release() {
        for (t_size i = 0; i < m_hdls_taken; i++) {
            const auto hdl = m_hdls[i];

            switch (hdl.Type()) {
            case zf::ec_gfx_resource_type::invalid:
                ZF_ASSERT_MSG(false, "There shouldn't be an invalid resource type here!");
                break;

            case ec_gfx_resource_type::mesh:
                glDeleteBuffers(1, &hdl.Mesh().elem_buf_gl_id);
                glDeleteBuffers(1, &hdl.Mesh().vert_buf_gl_id);
                glDeleteVertexArrays(1, &hdl.Mesh().vert_arr_gl_id);
                break;

            case ec_gfx_resource_type::shader_prog:
                glDeleteProgram(hdl.ShaderProg().gl_id);
                break;

            case ec_gfx_resource_type::texture:
                glDeleteTextures(1, &hdl.Texture().gl_id);
                break;
            }
        }
    }

    c_gfx_resource_handle c_gfx_resource_arena::AddMesh(const t_f32* const verts_raw, const t_size verts_len, const c_array<const t_u16> elems, const c_array<const t_s32> vert_attr_lens) {
        ZF_ASSERT(verts_len > 0);
        ZF_ASSERT(!elems.IsEmpty());
        ZF_ASSERT(!vert_attr_lens.IsEmpty());

        if (m_hdls_taken == m_hdls.Len()) {
            return {};
        }

        auto& hdl = m_hdls[m_hdls_taken];
        hdl = {MakeGLMesh(verts_raw, verts_len, elems, vert_attr_lens)};
        m_hdls_taken++;
        return hdl;
    }

    c_gfx_resource_handle c_gfx_resource_arena::AddShaderProg(const s_str_view vert_src, const s_str_view frag_src, c_mem_arena& temp_mem_arena) {
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
        hdl = {us_gl_shader_prog(prog_gl_id)};
        m_hdls_taken++;
        return hdl;
    }

    c_gfx_resource_handle c_gfx_resource_arena::AddTexture(const c_rgba_texture& rgba_tex) {
        ZF_ASSERT(rgba_tex.IsLoaded());

        if (m_hdls_taken == m_hdls.Len()) {
            return {};
        }

        const t_gl_id tex_gl_id = GenGLTextureFromRGBA(rgba_tex);

        if (!tex_gl_id) {
            return {};
        }

        auto& hdl = m_hdls[m_hdls_taken];
        hdl = {us_gl_texture(tex_gl_id)};
        m_hdls_taken++;
        return hdl;
    }

    t_b8 s_texture::LoadFromRGBA(const c_rgba_texture& rgba_tex, c_gfx_resource_arena& gfx_res_arena) {
        const c_gfx_resource_handle new_hdl = gfx_res_arena.AddTexture(rgba_tex);

        if (!new_hdl.IsValid()) {
            return false;
        }

        hdl = new_hdl;
        size = rgba_tex.SizeInPixels();

        return true;
    }

    t_b8 s_texture::LoadFromRaw(const s_str_view file_path, c_gfx_resource_arena& gfx_res_arena, c_mem_arena& temp_mem_arena) {
        c_rgba_texture rgba_tex;

        if (!rgba_tex.LoadFromRaw(temp_mem_arena, file_path)) {
            return false;
        }

        return LoadFromRGBA(rgba_tex, gfx_res_arena);
    }

    t_b8 s_texture::LoadFromPacked(const s_str_view file_path, c_gfx_resource_arena& gfx_res_arena) {
        return false;
    }
}
