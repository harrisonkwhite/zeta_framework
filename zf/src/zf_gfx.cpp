#include <zf/zf_gfx.h>

namespace zf::gfx {
    static t_size CalcStride(const s_array_rdonly<t_s32> vert_attr_lens) {
        t_size stride = 0;

        for (t_size i = 0; i < vert_attr_lens.len; i++) {
            stride += ZF_SIZE_OF(t_f32) * static_cast<t_size>(vert_attr_lens[i]);
        }

        return stride;
    }

    static s_gl_mesh MakeGLMesh(const t_f32* const verts_raw, const t_size verts_len, const s_array_rdonly<t_u16> elems, const s_array_rdonly<t_s32> vert_attr_lens) {
        s_gl_mesh mesh = {};

        glGenVertexArrays(1, &mesh.vert_arr_gl_id);
        glBindVertexArray(mesh.vert_arr_gl_id);

        glGenBuffers(1, &mesh.vert_buf_gl_id);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vert_buf_gl_id);
        glBufferData(GL_ARRAY_BUFFER, ZF_SIZE_OF(t_f32) * verts_len, verts_raw, GL_DYNAMIC_DRAW);

        glGenBuffers(1, &mesh.elem_buf_gl_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.elem_buf_gl_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(ArraySizeInBytes(elems)), elems.buf_raw, GL_STATIC_DRAW);

        const t_size stride = CalcStride(vert_attr_lens);
        t_s32 offs = 0;

        for (t_size i = 0; i < vert_attr_lens.len; i++) {
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

            const auto src_raw = StrRaw(src);
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
                        glGetShaderInfoLog(shader_gl_id, static_cast<GLsizei>(log_chrs.len), nullptr, log_chrs.buf_raw);
                        ZF_LOG_ERROR_SPECIAL("OpenGL Shader Compilation", "%s", log_chrs.buf_raw);
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

        ZF_DEFER({ glDeleteShader(vert_gl_id); });

        const t_gl_id frag_gl_id = shader_gen_func(frag_src, true);

        if (!frag_gl_id) {
            return 0;
        }

        ZF_DEFER({ glDeleteShader(frag_gl_id); });

        // Set up the shader program.
        const t_gl_id prog_gl_id = glCreateProgram();
        glAttachShader(prog_gl_id, vert_gl_id);
        glAttachShader(prog_gl_id, frag_gl_id);
        glLinkProgram(prog_gl_id);

        return prog_gl_id;
    }

    static inline s_v2<t_s32> GLTextureSizeLimit() {
        t_s32 size;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
        return { size, size };
    }

    static t_gl_id MakeGLTexture(const s_rgba_texture_data_rdonly& tex_data) {
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

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_data.size_in_pxs.x, tex_data.size_in_pxs.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data.px_data.buf_raw);

        return tex_gl_id;
    }

    t_b8 AreResourcesEqual(const s_resource_handle& a, const s_resource_handle& b) {
        if (a.type == b.type) {
            switch (a.type) {
            case ek_resource_type_invalid:
                return true;

            case ek_resource_type_mesh:
                return a.raw.mesh.vert_arr_gl_id == b.raw.mesh.vert_arr_gl_id
                    && a.raw.mesh.vert_buf_gl_id == b.raw.mesh.vert_buf_gl_id
                    && a.raw.mesh.elem_buf_gl_id == b.raw.mesh.elem_buf_gl_id;

            case ek_resource_type_shader_prog:
                return a.raw.shader_prog.gl_id == b.raw.shader_prog.gl_id;

            case ek_resource_type_texture:
                return a.raw.tex.gl_id == b.raw.tex.gl_id;
            }
        }

        return false;
    }

    void ReleaseResource(const s_resource_handle& hdl) {
        switch (hdl.type) {
        case ek_resource_type_invalid:
            ZF_ASSERT(false);
            break;

        case ek_resource_type_mesh:
            glDeleteBuffers(1, &hdl.raw.mesh.elem_buf_gl_id);
            glDeleteBuffers(1, &hdl.raw.mesh.vert_buf_gl_id);
            glDeleteVertexArrays(1, &hdl.raw.mesh.vert_arr_gl_id);
            break;

        case ek_resource_type_shader_prog:
            glDeleteProgram(hdl.raw.shader_prog.gl_id);
            break;

        case ek_resource_type_texture:
            glDeleteTextures(1, &hdl.raw.tex.gl_id);
            break;
        }
    }

    t_b8 MakeResourceArena(s_mem_arena& mem_arena, const t_size cap, s_resource_arena& o_res_arena) {
        return MakeList(mem_arena, cap, o_res_arena.hdls);
    }

    void ReleaseResources(s_resource_arena& res_arena) {
        for (t_size i = 0; i < res_arena.hdls.len; i++) {
            ReleaseResource(res_arena.hdls[i]);
        }
    }

    s_resource_handle MakeMesh(s_resource_arena& res_arena, const t_f32* const verts_raw, const t_size verts_len, const s_array_rdonly<t_u16> elems, const s_array_rdonly<t_s32> vert_attr_lens) {
        ZF_ASSERT(verts_len > 0);
        ZF_ASSERT(!IsArrayEmpty(elems));
        ZF_ASSERT(!IsArrayEmpty(vert_attr_lens));

        if (IsListFull(res_arena.hdls)) {
            return {};
        }

        const auto gl_mesh = MakeGLMesh(verts_raw, verts_len, elems, vert_attr_lens);
        return ListAppend(res_arena.hdls, MakeMeshHandle(gl_mesh));
    }

    s_resource_handle MakeShaderProg(s_resource_arena& res_arena, const s_str_rdonly vert_src, const s_str_rdonly frag_src, s_mem_arena& temp_mem_arena) {
        ZF_ASSERT(IsStrTerminated(vert_src));
        ZF_ASSERT(IsStrTerminated(frag_src));

        if (IsListFull(res_arena.hdls)) {
            return {};
        }

        const t_gl_id prog_gl_id = MakeGLShaderProg(vert_src, frag_src, temp_mem_arena);

        if (!prog_gl_id) {
            return {};
        }

        return ListAppend(res_arena.hdls, MakeShaderProgHandle(prog_gl_id));
    }

    s_resource_handle MakeTexture(s_resource_arena& res_arena, const s_rgba_texture_data_rdonly& tex_data) {
        if (IsListFull(res_arena.hdls)) {
            return {};
        }

        const t_gl_id tex_gl_id = MakeGLTexture(tex_data);

        if (!tex_gl_id) {
            return {};
        }

        return ListAppend(res_arena.hdls, MakeTextureHandle(tex_gl_id));
    }

    [[nodiscard]] static t_b8 MakeFontAtlasTextureHandles(const s_array_rdonly<t_font_atlas_rgba> atlas_rgbas, s_mem_arena& mem_arena, s_resource_arena& res_arena, s_array<s_resource_handle> o_hdls) {
        if (!MakeArray(mem_arena, atlas_rgbas.len, o_hdls)) {
            return false;
        }

        for (t_size i = 0; i < atlas_rgbas.len; i++) {
            o_hdls[i] = MakeTexture(res_arena, {g_font_atlas_size, atlas_rgbas[i]});

            if (!IsResourceHandleValid(o_hdls[i])) {
                return false;
            }
        }

        return true;
    }

    t_b8 LoadFontAssetFromRaw(const s_str_rdonly file_path, const t_s32 height, const s_array_rdonly<t_s32> codepoints_no_dups, s_mem_arena& mem_arena, s_resource_arena& res_arena, s_mem_arena& temp_mem_arena, s_font_asset& o_asset) {
        o_asset = {};

        s_array<t_font_atlas_rgba> atlas_rgbas;

        if (!LoadFontFromRaw(file_path, height, codepoints_no_dups, mem_arena, temp_mem_arena, temp_mem_arena, o_asset.arrangement, atlas_rgbas)) {
            return false;
        }

        return MakeFontAtlasTextureHandles(atlas_rgbas, mem_arena, res_arena, o_asset.atlas_tex_hdls);
    }

    t_b8 LoadFontAssetFromPacked(const s_str_rdonly file_path, s_mem_arena& mem_arena, s_resource_arena& res_arena, s_mem_arena& temp_mem_arena, s_font_asset& o_asset) {
        o_asset = {};

        s_array<t_font_atlas_rgba> atlas_rgbas;

        if (!UnpackFont(file_path, mem_arena, temp_mem_arena, o_asset.arrangement, atlas_rgbas)) {
            return false;
        }

        return MakeFontAtlasTextureHandles(atlas_rgbas, mem_arena, res_arena, o_asset.atlas_tex_hdls);
    }
}
