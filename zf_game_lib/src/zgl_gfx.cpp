#include <zgl/zgl_gfx.h>

#include <glad/glad.h>
#include <zgl/zgl_platform.h>

namespace zf {
    using t_gl_id = GLuint;

    t_b8 g_initted;

    void InitGFX() {
        ZF_ASSERT(!g_initted);

        // Load OpenGL function pointers.
        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(internal::GetGLProcAddrFunc()))) {
            ZF_FATAL();
        }

        // Enable blending.
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        g_initted = true;
    }

    void ShutdownGFX() {
        ZF_ASSERT(g_initted);
        g_initted = false;
    }

    // ============================================================
    // @section: Resources
    // ============================================================
    struct s_gfx_resource {
    public:
        e_gfx_resource_type type = ek_gfx_resource_type_invalid;
        s_ptr<s_gfx_resource> next = nullptr;

        auto &Mesh() { return type_data.mesh; }
        auto &Mesh() const { return type_data.mesh; }

        auto &ShaderProg() { return type_data.shader_prog; }
        auto &ShaderProg() const { return type_data.shader_prog; }

        auto &Texture() { return type_data.texture; }
        auto &Texture() const { return type_data.texture; }

    private:
        union {
            struct {
                t_gl_id vert_arr_gl_id;
                t_gl_id vert_buf_gl_id;
                t_len verts_len;
            } mesh;

            struct {
                t_gl_id gl_id;
                s_hash_map<s_str_rdonly, t_i32> uniform_names_to_locs; // @todo: Construct and use this!
            } shader_prog;

            struct {
                t_gl_id gl_id;
                s_v2_i size;
            } texture;
        } type_data = {};
    };

    void DestroyGFXResources(const s_gfx_resource_arena arena) {
        ZF_ASSERT(g_initted);

        auto res = arena.head;

        while (res) {
            switch (res->type) {
            case ek_gfx_resource_type_mesh:
                glDeleteBuffers(1, &res->Mesh().vert_buf_gl_id);
                glDeleteVertexArrays(1, &res->Mesh().vert_arr_gl_id);
                break;

            case ek_gfx_resource_type_shader_prog:
                glDeleteProgram(res->ShaderProg().gl_id);
                break;

            case ek_gfx_resource_type_texture:
                glDeleteTextures(1, &res->Texture().gl_id);
                break;
            }

            res = res->next;
        }
    }

    static s_gfx_resource &PushGFXResource(s_gfx_resource_arena &arena) {
        s_gfx_resource &res = Alloc<s_gfx_resource>(*arena.mem_arena);

        if (!arena.head) {
            arena.head = &res;
            arena.tail = &res;
        } else {
            arena.tail->next = &res;
            arena.tail = &res;
        }

        return res;
    }

    s_gfx_resource &CreateMesh(const s_ptr<const t_f32> verts, const t_len verts_len, const t_b8 verts_dynamic, const s_array_rdonly<t_i32> vert_attr_component_cnts, s_gfx_resource_arena &arena) {
        ZF_ASSERT(g_initted);
        ZF_ASSERT(verts_len > 0);

        auto &res = PushGFXResource(arena);

        res.type = ek_gfx_resource_type_mesh;
        res.Mesh().verts_len = verts_len;

        glGenVertexArrays(1, &res.Mesh().vert_arr_gl_id);
        glBindVertexArray(res.Mesh().vert_arr_gl_id);

        glGenBuffers(1, &res.Mesh().vert_buf_gl_id);
        glBindBuffer(GL_ARRAY_BUFFER, res.Mesh().vert_buf_gl_id);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(ZF_SIZE_OF(t_f32) * verts_len), verts, verts_dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

        const t_len stride = [vert_attr_component_cnts]() {
            t_len res = 0;

            for (t_len i = 0; i < vert_attr_component_cnts.Len(); i++) {
                res += ZF_SIZE_OF(t_f32) * static_cast<t_len>(vert_attr_component_cnts[i]);
            }

            return res;
        }();

        t_i32 offs = 0;

        for (t_len i = 0; i < vert_attr_component_cnts.Len(); i++) {
            const t_i32 comp_cnt = vert_attr_component_cnts[i];

            glVertexAttribPointer(static_cast<GLuint>(i), comp_cnt, GL_FLOAT, false, static_cast<GLsizei>(stride), reinterpret_cast<void *>(ZF_SIZE_OF(t_f32) * offs));
            glEnableVertexAttribArray(static_cast<GLuint>(i));

            offs += comp_cnt;
        }

        ZF_ASSERT(verts_len % offs == 0);

        glBindVertexArray(0);

        return res;
    }

    t_b8 CreateShaderProg(const s_str_rdonly vert_src, const s_str_rdonly frag_src, s_gfx_resource_arena &res_arena, s_mem_arena &temp_mem_arena, s_ptr<s_gfx_resource> &o_res) {
        ZF_ASSERT(g_initted);

        const auto vert_src_terminated = AllocStrCloneButAddTerminator(vert_src, temp_mem_arena);
        const auto frag_src_terminated = AllocStrCloneButAddTerminator(frag_src, temp_mem_arena);

        //
        // Shader Creation
        //
        const auto create_shader = [&temp_mem_arena](const s_str_rdonly src, const t_b8 is_frag) -> t_gl_id {
            const t_gl_id shader_gl_id = glCreateShader(is_frag ? GL_FRAGMENT_SHADER : GL_VERTEX_SHADER);

            const auto src_cstr = src.Cstr();
            glShaderSource(shader_gl_id, 1, &src_cstr, nullptr);

            glCompileShader(shader_gl_id);

            t_i32 success = 0;
            glGetShaderiv(shader_gl_id, GL_COMPILE_STATUS, &success);

            if (!success) {
                ZF_DEFER({ glDeleteShader(shader_gl_id); });

                // Try getting the OpenGL compile error message.
                t_i32 log_chr_cnt = 0;
                glGetShaderiv(shader_gl_id, GL_INFO_LOG_LENGTH, &log_chr_cnt);

                if (log_chr_cnt > 1) {
                    const auto log_chrs = AllocArray<char>(log_chr_cnt, temp_mem_arena);
                    glGetShaderInfoLog(shader_gl_id, static_cast<GLsizei>(log_chrs.Len()), nullptr, log_chrs.Ptr());
                    LogErrorType(s_cstr_literal("OpenGL Shader Compilation"), s_cstr_literal("%"), FormatStr({log_chrs.ToBytes()}));
                } else {
                    LogError(s_cstr_literal("OpenGL shader compilation failed, but no error message available!"));
                }

                return 0;
            }

            return shader_gl_id;
        };

        const t_gl_id vert_gl_id = create_shader(vert_src_terminated, false);

        if (!vert_gl_id) {
            return false;
        }

        ZF_DEFER({ glDeleteShader(vert_gl_id); });

        const t_gl_id frag_gl_id = create_shader(frag_src_terminated, true);

        if (!frag_gl_id) {
            return false;
        }

        ZF_DEFER({ glDeleteShader(frag_gl_id); });

        //
        // Program Creation
        //
        const t_gl_id prog_gl_id = glCreateProgram();

        if (!prog_gl_id) {
            return false;
        }

        glAttachShader(prog_gl_id, vert_gl_id);
        glAttachShader(prog_gl_id, frag_gl_id);

        glLinkProgram(prog_gl_id);

        // Check link result.
        t_i32 link_status = 0;
        glGetProgramiv(prog_gl_id, GL_LINK_STATUS, &link_status);

        if (!link_status) {
            // Linking failed, try getting an error log.
            t_i32 log_chr_cnt = 0;
            glGetProgramiv(prog_gl_id, GL_INFO_LOG_LENGTH, &log_chr_cnt);

            if (log_chr_cnt > 1) {
                const auto log_chrs = AllocArray<char>(log_chr_cnt, temp_mem_arena);
                glGetProgramInfoLog(prog_gl_id, static_cast<GLsizei>(log_chrs.Len()), nullptr, log_chrs.Ptr());
                LogErrorType(s_cstr_literal("OpenGL Program Link"), s_cstr_literal("%"), FormatStr({log_chrs.ToBytes()}));
            } else {
                LogError(s_cstr_literal("OpenGL program link failed, but no error message available!"));
            }

            glDeleteProgram(prog_gl_id);

            return false;
        }

        o_res = &PushGFXResource(res_arena);
        o_res->type = ek_gfx_resource_type_shader_prog;
        o_res->ShaderProg().gl_id = prog_gl_id;

        return true;
    }

    t_b8 CreateTexture(const s_texture_data_rdonly tex_data, s_gfx_resource_arena &arena, s_ptr<s_gfx_resource> &o_res) {
        ZF_ASSERT(g_initted);
        ZF_ASSERT(!tex_data.RGBAPixelData().IsEmpty());

        const auto tex_size_limit = []() -> s_v2_i {
            t_i32 size;
            glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
            return {size, size};
        }();

        if (tex_data.SizeInPixels().x > tex_size_limit.x || tex_data.SizeInPixels().y > tex_size_limit.y) {
            LogError(s_cstr_literal("Texture size % exceeds limits %!"), tex_data.SizeInPixels(), tex_size_limit);
            return false;
        }

        t_gl_id gl_id;
        glGenTextures(1, &gl_id);

        glBindTexture(GL_TEXTURE_2D, gl_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_data.SizeInPixels().x, tex_data.SizeInPixels().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data.RGBAPixelData().Ptr());

        o_res = &PushGFXResource(arena);
        o_res->type = ek_gfx_resource_type_texture;
        o_res->Texture().gl_id = gl_id;
        o_res->Texture().size = tex_data.SizeInPixels();

        return true;
    }

    s_v2_i TextureSize(const s_gfx_resource &res) {
        ZF_ASSERT(res.type == ek_gfx_resource_type_texture);
        return res.Texture().size;
    }

    // ============================================================
    // @section: Rendering
    // ============================================================
    enum e_render_instr_type {
        ek_render_instr_type_invalid,
        ek_render_instr_type_clear,
        ek_render_instr_type_shader_prog_set,
        ek_render_instr_type_shader_prog_uniform_set,
        ek_render_instr_type_mesh_update,
        ek_render_instr_type_mesh_draw
    };

    struct s_render_instr {
    public:
        e_render_instr_type type = ek_render_instr_type_invalid;

        auto &Clear() {
            ZF_ASSERT(type == ek_render_instr_type_clear);
            return type_data.clear;
        }

        auto &Clear() const {
            ZF_ASSERT(type == ek_render_instr_type_clear);
            return type_data.clear;
        }

        auto &ShaderProgSet() {
            ZF_ASSERT(type == ek_render_instr_type_shader_prog_set);
            return type_data.shader_prog_set;
        }

        auto &ShaderProgSet() const {
            ZF_ASSERT(type == ek_render_instr_type_shader_prog_set);
            return type_data.shader_prog_set;
        }

        auto &ShaderProgUniformSet() {
            ZF_ASSERT(type == ek_render_instr_type_shader_prog_uniform_set);
            return type_data.shader_prog_uniform_set;
        }

        auto &ShaderProgUniformSet() const {
            ZF_ASSERT(type == ek_render_instr_type_shader_prog_uniform_set);
            return type_data.shader_prog_uniform_set;
        }

        auto &MeshUpdate() {
            ZF_ASSERT(type == ek_render_instr_type_mesh_update);
            return type_data.mesh_update;
        }

        auto &MeshUpdate() const {
            ZF_ASSERT(type == ek_render_instr_type_mesh_update);
            return type_data.mesh_update;
        }

        auto &MeshDraw() {
            ZF_ASSERT(type == ek_render_instr_type_mesh_draw);
            return type_data.mesh_draw;
        }

        auto &MeshDraw() const {
            ZF_ASSERT(type == ek_render_instr_type_mesh_draw);
            return type_data.mesh_draw;
        }

    private:
        union {
            struct {
                s_color_rgb24f col;
            } clear;

            struct {
                s_ptr<const s_gfx_resource> prog;
            } shader_prog_set;

            struct {
                s_str_rdonly name;
                s_shader_prog_uniform_val val;
            } shader_prog_uniform_set;

            struct {
                s_ptr<const s_gfx_resource> mesh;
                s_array_rdonly<t_f32> verts;
            } mesh_update;

            struct {
                s_ptr<const s_gfx_resource> mesh;
                s_ptr<const s_gfx_resource> tex;
            } mesh_draw;
        } type_data = {};
    };

    struct s_render_instr_seq::s_render_instr_block {
        s_static_list<s_render_instr, 32> instrs;
        s_ptr<s_render_instr_block> next;
    };

    void s_render_instr_seq::SubmitClear(const s_color_rgb24f col) {
        s_render_instr instr;
        instr.type = ek_render_instr_type_clear;
        instr.Clear().col = col;

        Submit(instr);
    }

    void s_render_instr_seq::SubmitShaderProgSet(const s_gfx_resource &prog) {
        s_render_instr instr;
        instr.type = ek_render_instr_type_shader_prog_set;
        instr.ShaderProgSet().prog = &prog;

        Submit(instr);
    }

    void s_render_instr_seq::SubmitShaderProgUniformSet(const s_str_rdonly name, const s_shader_prog_uniform_val &val) {
        s_render_instr instr;
        instr.type = ek_render_instr_type_shader_prog_uniform_set;
        instr.ShaderProgUniformSet().name = name;
        instr.ShaderProgUniformSet().val = val;

        Submit(instr);
    }

    void s_render_instr_seq::SubmitMeshUpdate(const s_gfx_resource &mesh, const s_array_rdonly<t_f32> verts) {
        s_render_instr instr;
        instr.type = ek_render_instr_type_mesh_update;
        instr.MeshUpdate().mesh = &mesh;
        instr.MeshUpdate().verts = verts;

        Submit(instr);
    }

    void s_render_instr_seq::SubmitMeshDraw(const s_gfx_resource &mesh, const s_ptr<const s_gfx_resource> tex) {
        s_render_instr instr;
        instr.type = ek_render_instr_type_mesh_draw;
        instr.MeshDraw().mesh = &mesh;
        instr.MeshDraw().tex = tex;

        Submit(instr);
    }

    void s_render_instr_seq::Exec(s_mem_arena &temp_mem_arena) {
        ZF_ASSERT(g_initted);

        const auto fb_size_cache = WindowFramebufferSizeCache();
        glViewport(0, 0, fb_size_cache.x, fb_size_cache.y); // @todo: Possibly make this into an instruction type?

        s_ptr<const s_gfx_resource> shader_prog_active;

        s_ptr<s_render_instr_block> block = blocks_head;

        while (block) {
            for (t_len i = 0; i < block->instrs.Len(); i++) {
                const auto &instr = block->instrs[i];

                switch (instr.type) {
                case ek_render_instr_type_clear: {
                    const auto &col = instr.Clear().col;
                    glClearColor(col.R(), col.G(), col.B(), 1.0f);
                    glClear(GL_COLOR_BUFFER_BIT);
                    break;
                }

                case ek_render_instr_type_shader_prog_set: {
                    shader_prog_active = instr.ShaderProgSet().prog;
                    glUseProgram(shader_prog_active->ShaderProg().gl_id);
                    break;
                }

                case ek_render_instr_type_shader_prog_uniform_set: {
                    ZF_ASSERT(shader_prog_active);

                    const s_str_rdonly name_terminated = AllocStrCloneButAddTerminator(instr.ShaderProgUniformSet().name, temp_mem_arena);

                    const t_i32 loc = glGetUniformLocation(shader_prog_active->ShaderProg().gl_id, name_terminated.Cstr()); // @todo: Should be using a hash map here!
                    ZF_ASSERT(loc != -1);

                    const auto &val = instr.ShaderProgUniformSet().val;

                    switch (val.Type()) {
                    case ek_shader_prog_uniform_val_type_i32:
                        glUniform1i(loc, val.I32());
                        break;

                    case ek_shader_prog_uniform_val_type_u32:
                        glUniform1ui(loc, val.U32());
                        break;

                    case ek_shader_prog_uniform_val_type_f32:
                        glUniform1f(loc, val.F32());
                        break;

                    case ek_shader_prog_uniform_val_type_v2:
                        glUniform2f(loc, val.V2().x, val.V2().y);
                        break;

                    case ek_shader_prog_uniform_val_type_v3:
                        glUniform3f(loc, val.V3().x, val.V3().y, val.V3().z);
                        break;

                    case ek_shader_prog_uniform_val_type_v4:
                        glUniform4f(loc, val.V4().x, val.V4().y, val.V4().z, val.V4().w);
                        break;

                    case ek_shader_prog_uniform_val_type_mat4x4:
                        glUniformMatrix4fv(loc, 1, false, reinterpret_cast<const t_f32 *>(&val.Mat4x4()));
                        break;
                    }

                    break;
                }

                case ek_render_instr_type_mesh_update: {
                    const auto &mu = instr.MeshUpdate();
                    glBindVertexArray(mu.mesh->Mesh().vert_arr_gl_id);
                    glBindBuffer(GL_ARRAY_BUFFER, mu.mesh->Mesh().vert_buf_gl_id);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, mu.verts.SizeInBytes(), mu.verts.Ptr());

                    break;
                }

                case ek_render_instr_type_mesh_draw: {
                    ZF_ASSERT(shader_prog_active);

                    const auto &md = instr.MeshDraw();

                    if (md.tex) {
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, instr.MeshDraw().tex->Texture().gl_id);
                    }

                    glBindVertexArray(md.mesh->Mesh().vert_arr_gl_id);
                    glBindBuffer(GL_ARRAY_BUFFER, md.mesh->Mesh().vert_buf_gl_id);
                    glDrawArrays(GL_TRIANGLES, 0, md.mesh->Mesh().verts_len);

                    glUseProgram(0);
                    shader_prog_active = nullptr;

                    break;
                }
                }
            }

            block = block->next;
        }
    }

    void s_render_instr_seq::Submit(const s_render_instr instr) {
        if (!blocks_head) {
            blocks_head = &Alloc<s_render_instr_block>(*blocks_mem_arena);
            blocks_tail = blocks_head;
        } else if (blocks_tail->instrs.IsFull()) {
            blocks_tail->next = &Alloc<s_render_instr_block>(*blocks_mem_arena);
            blocks_tail = blocks_tail->next;
        }

        blocks_tail->instrs.Append(instr);
    }
}
