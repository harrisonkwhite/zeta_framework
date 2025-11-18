#include <zf/zf_gfx.h>

namespace zf {
    static t_size CalcStride(const s_array<const t_s32> vert_attr_lens) {
        t_size stride = 0;

        for (t_size i = 0; i < vert_attr_lens.Len(); i++) {
            stride += ZF_SIZE_OF(t_s32) * static_cast<t_size>(vert_attr_lens[i]);
        }

        return stride;
    }

    static us_gl_mesh MakeGLMesh(const t_f32* const verts_raw, const t_size verts_len, const s_array<const t_u16> elems, const s_array<const t_s32> vert_attr_lens) {
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

    static t_gl_id MakeGLShaderProg(const s_str_rdonly vert_src, const s_str_rdonly frag_src, s_mem_arena& temp_mem_arena) {
        // @todo: Improve error logging here. Should be in return value.
        
        ZF_ASSERT(IsStrTerminated(vert_src));
        ZF_ASSERT(IsStrTerminated(frag_src));

        // Generate the individual shaders.
        const auto shader_gen_func = [&temp_mem_arena](const s_str_rdonly src, const t_b8 is_frag) -> t_gl_id {
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
                    s_array<char> log_chrs;

                    if (MakeArray(temp_mem_arena, log_chr_cnt, log_chrs)) {
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

    static t_gl_id MakeGLTexture(const s_rgba_texture_data& tex_data) {
        const s_v2<t_s32> tex_size_limit = GLTextureSizeLimit();

        if (tex_data.size_in_pxs.x > tex_size_limit.x || tex_data.size_in_pxs.y > tex_size_limit.y) {
            ZF_LOG_ERROR("Texture size (%d, %d) exceeds OpenGL limits (%d, %d)!", tex_data.size_in_pxs.x, tex_data.size_in_pxs.y, tex_size_limit.x, tex_size_limit.y);
            return 0;
        }

        t_gl_id tex_gl_id;
        glGenTextures(1, &tex_gl_id);

        glBindTexture(GL_TEXTURE_2D, tex_gl_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_data.size_in_pxs.x, tex_data.size_in_pxs.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data.px_data.Raw());

        return tex_gl_id;
    }

    t_b8 AreGFXResourcesEqual(const s_gfx_resource_handle& a, const s_gfx_resource_handle& b) {
        if (a.Type() == b.Type()) {
            switch (a.Type()) {
            case ec_gfx_resource_type::invalid:
                return true;

            case ec_gfx_resource_type::mesh:
                return a.Mesh().vert_arr_gl_id == b.Mesh().vert_arr_gl_id
                    && a.Mesh().vert_buf_gl_id == b.Mesh().vert_buf_gl_id
                    && a.Mesh().elem_buf_gl_id == b.Mesh().elem_buf_gl_id;

            case ec_gfx_resource_type::shader_prog:
                return a.ShaderProg().gl_id == b.ShaderProg().gl_id;

            case ec_gfx_resource_type::texture:
                return a.Texture().gl_id == b.Texture().gl_id;
            }
        }

        return false;
    }

    void ReleaseGFXResource(const s_gfx_resource_handle hdl) {
        switch (hdl.Type()) {
        case zf::ec_gfx_resource_type::invalid:
            ZF_ASSERT(false);
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

    t_b8 MakeGFXResourceArena(s_mem_arena& mem_arena, const t_size cap, s_gfx_resource_arena& o_res_arena) {
        return MakeList(mem_arena, cap, o_res_arena.hdls);
    }

    void ReleaseGFXResourceArena(s_gfx_resource_arena& res_arena) {
        for (t_size i = 0; i < res_arena.hdls.len; i++) {
            ReleaseGFXResource(res_arena.hdls[i]);
        }
    }

    s_gfx_resource_handle MakeMesh(s_gfx_resource_arena& gfx_res_arena, const t_f32* const verts_raw, const t_size verts_len, const s_array<const t_u16> elems, const s_array<const t_s32> vert_attr_lens) {
        ZF_ASSERT(verts_len > 0);
        ZF_ASSERT(!elems.IsEmpty());
        ZF_ASSERT(!vert_attr_lens.IsEmpty());

        if (IsListFull(gfx_res_arena.hdls)) {
            return {};
        }

        return ListAppend(gfx_res_arena.hdls, {MakeGLMesh(verts_raw, verts_len, elems, vert_attr_lens)});
    }

    s_gfx_resource_handle MakeShaderProg(s_gfx_resource_arena& gfx_res_arena, const s_str_rdonly vert_src, const s_str_rdonly frag_src, s_mem_arena& temp_mem_arena) {
        ZF_ASSERT(IsStrTerminated(vert_src));
        ZF_ASSERT(IsStrTerminated(frag_src));

        if (IsListFull(gfx_res_arena.hdls)) {
            return {};
        }

        const t_gl_id prog_gl_id = MakeGLShaderProg(vert_src, frag_src, temp_mem_arena);

        if (!prog_gl_id) {
            return {};
        }

        return ListAppend(gfx_res_arena.hdls, {us_gl_shader_prog(prog_gl_id)});
    }

    s_gfx_resource_handle MakeTexture(s_gfx_resource_arena& gfx_res_arena, const s_rgba_texture_data& tex_data) {
        if (IsListFull(gfx_res_arena.hdls)) {
            return {};
        }

        const t_gl_id tex_gl_id = MakeGLTexture(tex_data);

        if (!tex_gl_id) {
            return {};
        }

        return ListAppend(gfx_res_arena.hdls, {us_gl_texture(tex_gl_id)});
    }
}
