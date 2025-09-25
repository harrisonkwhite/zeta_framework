#include "zf_rendering.h"

namespace zf {
    static t_gl_id GenShaderFromSrc(const c_string_view src, const bool frag, c_mem_arena& temp_mem_arena) {
        const t_gl_id shader_gl_id = glCreateShader(frag ? GL_FRAGMENT_SHADER : GL_VERTEX_SHADER);
        const char* const src_ptr = src.Raw();
        glShaderSource(shader_gl_id, 1, &src_ptr, nullptr);
        glCompileShader(shader_gl_id);

        t_s32 success;
        glGetShaderiv(shader_gl_id, GL_COMPILE_STATUS, &success);

        if (!success) {
            // Try getting the OpenGL compile error message.
            t_s32 log_len;
            glGetShaderiv(shader_gl_id, GL_INFO_LOG_LENGTH, &log_len);

            if (log_len > 1) {
                const auto log = PushArrayToMemArena<char>(temp_mem_arena, log_len);

                if (!log.IsEmpty()) {
                    glGetShaderInfoLog(shader_gl_id, log.Len(), nullptr, log.Raw());
                    //LOG_ERROR_SPECIAL("OpenGL Shader Compilation", "%s", log.Raw());
                } else {
                    //LOG_ERROR("Failed to reserve memory for OpenGL shader compilation error log!");
                }
            }

            glDeleteShader(shader_gl_id);

            return 0;
        }

        return shader_gl_id;
    }

    static bool LoadShaderSrcsFromFile(c_string_view& vert_src, c_string_view& frag_src, const c_string_view file_path, c_mem_arena& mem_arena) {
        assert(vert_src.IsEmpty());
        assert(frag_src.IsEmpty());

        c_file_reader fr;
        fr.DeferClose();

        if (!fr.Open(file_path)) {
            //LOG_ERROR("Failed to open shader program file \"%s\"!", file_path.Raw());
            return false;
        }

        t_s32 vert_src_len;

        if (!fr.ReadItem(vert_src_len)) {
            //LOG_ERROR("Failed to read vertex shader source length from file \"%s\"!", file_path.Raw());
            return false;
        }

        const auto vert_src_chrs = PushArrayToMemArena<char>(mem_arena, vert_src_len);

        if (vert_src_chrs.IsEmpty()) {
            //LOG_ERROR("Failed to reserve memory for vertex shader source from file \"%s\"!", file_path.Raw());
            return false;
        }

        if (fr.Read(vert_src_chrs) < vert_src_chrs.Len()) {
            //LOG_ERROR("Failed to read vertex shader source from file \"%s\"!", file_path.Raw());
            return false;
        }

        t_s32 frag_src_len;

        if (!fr.ReadItem(frag_src_len)) {
            //LOG_ERROR("Failed to read fragment shader source length from file \"%s\"!", file_path.Raw());
            return false;
        }

        const auto frag_src_chrs = PushArrayToMemArena<char>(mem_arena, frag_src_len);

        if (frag_src_chrs.IsEmpty()) {
            //LOG_ERROR("Failed to reserve memory for fragment shader source from file \"%s\"!", file_path.Raw());
            return false;
        }

        if (fr.Read(frag_src_chrs) < frag_src_chrs.Len()) {
            //LOG_ERROR("Failed to read fragment shader source from file \"%s\"!", file_path.Raw());
            return false;
        }

        vert_src = vert_src_chrs.View();
        frag_src = frag_src_chrs.View();

        return true;
    }

    static t_gl_id GenShaderProg(const s_shader_prog_gen_info gen_info, c_mem_arena& temp_mem_arena) {
        // Determine the shader sources.
        c_string_view vert_src;
        c_string_view frag_src;

        if (gen_info.holds_srcs) {
            vert_src = gen_info.vert_src;
            frag_src = gen_info.frag_src;
        } else {
            if (!LoadShaderSrcsFromFile(vert_src, frag_src, gen_info.file_path, temp_mem_arena)) {
                //LOG_ERROR("Failed to load shader sources from file \"%s\"!", gen_info.file_path.Raw());
                return 0;
            }
        }

        // Generate the shaders from the sources.
        const t_gl_id vs_gl_id = GenShaderFromSrc(vert_src, false, temp_mem_arena);

        if (!vs_gl_id) {
            if (gen_info.holds_srcs) {
                //LOG_ERROR("Failed to generate vertex shader from source!");
            } else {
                //LOG_ERROR("Failed to generate vertex shader from file \"%s\"!", gen_info.file_path.Raw());
            }

            return 0;
        }

        const t_gl_id fs_gl_id = GenShaderFromSrc(frag_src, true, temp_mem_arena);

        if (!fs_gl_id) {
            if (gen_info.holds_srcs) {
                //LOG_ERROR("Failed to generate fragment shader from source!");
            } else {
                //LOG_ERROR("Failed to generate fragment shader from file \"%s\"!", gen_info.file_path.Raw());
            }

            glDeleteShader(vs_gl_id);

            return 0;
        }

        // Set up the shader program.
        const t_gl_id prog_gl_id = glCreateProgram();
        glAttachShader(prog_gl_id, vs_gl_id);
        glAttachShader(prog_gl_id, fs_gl_id);
        glLinkProgram(prog_gl_id);

        // Dispose the shaders, they're no longer needed.
        glDeleteShader(fs_gl_id);
        glDeleteShader(vs_gl_id);

        return prog_gl_id;
    }

    bool InitShaderProgGroup(s_shader_prog_group& prog_group, const c_array<const s_shader_prog_gen_info> gen_infos, s_gl_resource_arena& gl_res_arena, c_mem_arena& temp_mem_arena) {
        assert(gen_infos.Len() > 0);

        c_string_view test = "dog";

        const t_s32 prog_cnt = gen_infos.Len();

        const c_array<t_gl_id> gl_ids = PushArrayToGLResourceArena(gl_res_arena, prog_cnt, ek_gl_resource_type_shader_prog);

        if (gl_ids.IsEmpty()) {
            //LOG_ERROR("Failed to reserve OpenGL shader program IDs for shader program group!");
            return false;
        }

        for (t_s32 i = 0; i < prog_cnt; i++) {
            const s_shader_prog_gen_info gen_info = gen_infos[i];

            gl_ids[i] = GenShaderProg(gen_info, temp_mem_arena);

            if (!gl_ids[i]) {
                //LOG_ERROR("Failed to generate shader program with index %d!", i);
                return false;
            }
        }

        prog_group = {
            .gl_ids = gl_ids.View()
        };

        return true;
    }
}
