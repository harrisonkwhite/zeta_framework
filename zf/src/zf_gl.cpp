#include <zf/zf_gl.h>

namespace zf {
    s_mesh_gl_ids MakeGLMesh(const t_f32* const verts_raw, const t_size verts_len, const s_array_rdonly<t_u16> elems, const s_array_rdonly<t_s32> vert_attr_lens) {
        s_mesh_gl_ids gl_ids = {};

        glGenVertexArrays(1, &gl_ids.vert_arr);
        glBindVertexArray(gl_ids.vert_arr);

        glGenBuffers(1, &gl_ids.vert_buf);
        glBindBuffer(GL_ARRAY_BUFFER, gl_ids.vert_buf);
        glBufferData(GL_ARRAY_BUFFER, ZF_SIZE_OF(t_f32) * verts_len, verts_raw, GL_DYNAMIC_DRAW);

        glGenBuffers(1, &gl_ids.elem_buf);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_ids.elem_buf);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(ArraySizeInBytes(elems)), elems.buf_raw, GL_STATIC_DRAW);

        const t_size stride = [vert_attr_lens]() {
            t_size res = 0;

            for (t_size i = 0; i < vert_attr_lens.len; i++) {
                res += ZF_SIZE_OF(t_f32) * static_cast<t_size>(vert_attr_lens[i]);
            }

            return res;
        }();

        t_s32 offs = 0;

        for (t_size i = 0; i < vert_attr_lens.len; i++) {
            const t_s32 attr_len = vert_attr_lens[i];

            glVertexAttribPointer(static_cast<GLuint>(i), attr_len, GL_FLOAT, false, static_cast<GLsizei>(stride), reinterpret_cast<void*>(ZF_SIZE_OF(t_f32) * offs));
            glEnableVertexAttribArray(static_cast<GLuint>(i));

            offs += attr_len;
        }

        glBindVertexArray(0);

        return gl_ids;
    }

    void ReleaseGLMesh(const s_mesh_gl_ids& gl_ids) {
        glDeleteBuffers(1, &gl_ids.elem_buf);
        glDeleteBuffers(1, &gl_ids.vert_buf);
        glDeleteVertexArrays(1, &gl_ids.vert_arr);
    }

    t_gl_id MakeGLShaderProg(const s_str_rdonly vert_src, const s_str_rdonly frag_src, s_mem_arena& temp_mem_arena) {
        s_str vert_src_terminated;
        s_str frag_src_terminated;

        if (!CloneStrButAddTerminator(vert_src, temp_mem_arena, vert_src_terminated)
            || !CloneStrButAddTerminator(frag_src, temp_mem_arena, frag_src_terminated)) {
            return 0;
        }

        // Generate the individual shaders.
        const auto gen_shader = [&temp_mem_arena](const s_str_rdonly src, const t_b8 is_frag) -> t_gl_id {
            const t_gl_id shader_gl_id = glCreateShader(is_frag ? GL_FRAGMENT_SHADER : GL_VERTEX_SHADER);

            const auto src_raw = StrRaw(src);
            glShaderSource(shader_gl_id, 1, &src_raw, nullptr);

            glCompileShader(shader_gl_id);

            t_s32 success;
            glGetShaderiv(shader_gl_id, GL_COMPILE_STATUS, &success);

            if (!success) {
                ZF_DEFER({ glDeleteShader(shader_gl_id); });

                // Try getting the OpenGL compile error message.
                t_s32 log_chr_cnt;
                glGetShaderiv(shader_gl_id, GL_INFO_LOG_LENGTH, &log_chr_cnt);

                if (log_chr_cnt >= 1) {
                    s_array<char> log_chrs;

                    if (MakeArray(temp_mem_arena, log_chr_cnt, log_chrs)) {
                        glGetShaderInfoLog(shader_gl_id, static_cast<GLsizei>(log_chrs.len), nullptr, log_chrs.buf_raw);
                        LogErrorType("OpenGL Shader Compilation", "%", StrFromRaw(log_chrs.buf_raw));
                    } else {
                        LogError("Failed to reserve memory for OpenGL shader compilation error log!");
                    }
                } else {
                    LogError("OpenGL shader compilation failed, but no error message available!");
                }

                return 0;
            }

            return shader_gl_id;
        };

        const t_gl_id vert_gl_id = gen_shader(vert_src_terminated, false);

        if (!vert_gl_id) {
            return 0;
        }

        ZF_DEFER({ glDeleteShader(vert_gl_id); });

        const t_gl_id frag_gl_id = gen_shader(frag_src_terminated, true);

        if (!frag_gl_id) {
            return 0;
        }

        ZF_DEFER({ glDeleteShader(frag_gl_id); });

        // Set up the shader program.
        const t_gl_id prog_gl_id = glCreateProgram();
        glAttachShader(prog_gl_id, vert_gl_id);
        glAttachShader(prog_gl_id, frag_gl_id);
        glLinkProgram(prog_gl_id);

        // @todo: Check for link success.

        return prog_gl_id;
    }

    t_gl_id MakeGLTexture(const s_rgba_texture_data_rdonly& tex_data) {
        const s_v2<t_s32> tex_size_limit = GLTextureSizeLimit();

        if (tex_data.size_in_pxs.x > tex_size_limit.x || tex_data.size_in_pxs.y > tex_size_limit.y) {
            LogError("Texture size % exceeds limits %!", tex_data.size_in_pxs, tex_size_limit);
            ZF_REPORT_ERROR();
            return 0;
        }

        t_gl_id gl_id;
        glGenTextures(1, &gl_id);

        glBindTexture(GL_TEXTURE_2D, gl_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_data.size_in_pxs.x, tex_data.size_in_pxs.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data.px_data.buf_raw);

        return gl_id;
    }
}
