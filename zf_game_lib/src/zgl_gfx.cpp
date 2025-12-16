#include <zgl/zgl_gfx.h>

#include <bgfx/bgfx.h>
#include <zgl/zgl_platform.h>

namespace zf {
    constexpr s_color_rgb8 g_bg_color_default = {109, 187, 255};
    s_v2_i g_resolution;

    t_b8 g_initted;

    void InitGFX() {
        ZF_ASSERT(!g_initted);

        const auto fb_size_cache = WindowFramebufferSizeCache();

        bgfx::Init init;
        init.type = bgfx::RendererType::Count;

        init.resolution.reset = BGFX_RESET_VSYNC;

        g_resolution = fb_size_cache;
        init.resolution.width = static_cast<uint32_t>(g_resolution.x);
        init.resolution.height = static_cast<uint32_t>(g_resolution.y);

        init.platformData.nwh = internal::NativeWindowHandle();
        init.platformData.ndt = internal::NativeDisplayHandle();
        init.platformData.type = bgfx::NativeWindowHandleType::Default;

        if (!bgfx::init(init)) {
            ZF_FATAL();
        }

        g_initted = true;
    }

    void ShutdownGFX() {
        ZF_ASSERT(g_initted);
        bgfx::shutdown();
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

    private:
        union {
            struct {
                bgfx::DynamicVertexBufferHandle vert_buf_bgfx_hdl;
                t_len verts_len;
            } mesh;

            struct {
                bgfx::ProgramHandle bgfx_hdl;
            } shader_prog;
        } type_data = {};
    };

    static s_gfx_resource &PushGFXResource(const e_gfx_resource_type type, s_gfx_resource_arena &arena) {
        ZF_ASSERT(type != ek_gfx_resource_type_invalid);

        s_gfx_resource &res = Alloc<s_gfx_resource>(*arena.mem_arena);

        if (!arena.head) {
            arena.head = &res;
            arena.tail = &res;
        } else {
            arena.tail->next = &res;
            arena.tail = &res;
        }

        res.type = type;

        return res;
    }

    s_gfx_resource &CreateMesh(const t_len verts_len, s_gfx_resource_arena &arena) {
        bgfx::VertexLayout layout;
        layout.begin().add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float).add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float).end(); // @todo: Allow customs!

        auto &res = PushGFXResource(ek_gfx_resource_type_mesh, arena);
        res.Mesh().vert_buf_bgfx_hdl = bgfx::createDynamicVertexBuffer(static_cast<uint32_t>(verts_len), layout);
        res.Mesh().verts_len = verts_len;

        return res;
    }

    t_b8 CreateShaderProg(const s_array_rdonly<t_u8> vert_shader_bin, const s_array_rdonly<t_u8> frag_shader_bin, s_gfx_resource_arena &arena, s_ptr<s_gfx_resource> &o_res) {
        const bgfx::Memory *const vert_shader_mem = bgfx::makeRef(vert_shader_bin.Ptr(), static_cast<uint32_t>(vert_shader_bin.Len()));
        const bgfx::ShaderHandle vert_shader_hdl = bgfx::createShader(vert_shader_mem);

        if (!bgfx::isValid(vert_shader_hdl)) {
            return false;
        }

        const bgfx::Memory *const frag_shader_mem = bgfx::makeRef(frag_shader_bin.Ptr(), static_cast<uint32_t>(frag_shader_bin.Len()));
        const bgfx::ShaderHandle frag_shader_hdl = bgfx::createShader(frag_shader_mem);

        if (!bgfx::isValid(frag_shader_hdl)) {
            return false;
        }

        const bgfx::ProgramHandle prog_hdl = bgfx::createProgram(vert_shader_hdl, frag_shader_hdl, true);

        if (!bgfx::isValid(prog_hdl)) {
            return false;
        }

        o_res = &PushGFXResource(ek_gfx_resource_type_shader_prog, arena);
        o_res->ShaderProg().bgfx_hdl = prog_hdl;

        return true;
    }

    // ============================================================
    // @section: Rendering
    // ============================================================
    enum e_render_instr_type {
        ek_render_instr_type_invalid,
        ek_render_instr_type_mesh_update,
        ek_render_instr_type_mesh_draw
    };

    struct s_render_instr {
    public:
        e_render_instr_type type = ek_render_instr_type_invalid;

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
                s_ptr<const s_gfx_resource> mesh;
                s_array_rdonly<t_f32> verts;
            } mesh_update;

            struct {
                s_ptr<const s_gfx_resource> mesh;
                s_ptr<const s_gfx_resource> prog;
            } mesh_draw;
        } type_data = {};
    };

    struct s_render_instr_seq::s_render_instr_block {
        s_static_list<s_render_instr, 32> instrs;
        s_ptr<s_render_instr_block> next;
    };

    void s_render_instr_seq::SubmitMeshUpdate(const s_gfx_resource &mesh, const s_array_rdonly<t_f32> verts) {
        s_render_instr instr;
        instr.type = ek_render_instr_type_mesh_update;
        instr.MeshUpdate().mesh = &mesh;
        instr.MeshUpdate().verts = verts;

        Submit(instr);
    }

    void s_render_instr_seq::SubmitMeshDraw(const s_gfx_resource &mesh, const s_gfx_resource &prog) {
        s_render_instr instr;
        instr.type = ek_render_instr_type_mesh_draw;
        instr.MeshDraw().mesh = &mesh;
        instr.MeshDraw().prog = &prog;

        Submit(instr);
    }

    void s_render_instr_seq::Exec(s_mem_arena &temp_mem_arena) {
        ZF_ASSERT(g_initted);

        const auto fb_size_cache = WindowFramebufferSizeCache();

        if (g_resolution != fb_size_cache) {
            bgfx::reset(static_cast<uint32_t>(fb_size_cache.x), static_cast<uint32_t>(fb_size_cache.y), BGFX_RESET_VSYNC);
            g_resolution = fb_size_cache;
        }

        //
        //
        //
        bgfx::setViewMode(0, bgfx::ViewMode::Sequential);

        const auto view_mat = CreateIdentityMatrix();

        auto proj_mat = CreateIdentityMatrix();
        proj_mat.elems[0][0] = 1.0f / (static_cast<t_f32>(fb_size_cache.x) / 2.0f);
        proj_mat.elems[1][1] = -1.0f / (static_cast<t_f32>(fb_size_cache.y) / 2.0f);
        proj_mat.elems[3][0] = -1.0f;
        proj_mat.elems[3][1] = 1.0f;

        bgfx::setViewTransform(0, &view_mat, &proj_mat);

        bgfx::setViewClear(0, BGFX_CLEAR_COLOR, ColorToHex(g_bg_color_default));

        bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(fb_size_cache.x), static_cast<uint16_t>(fb_size_cache.y));

        bgfx::touch(0);

        //
        //
        //
        s_ptr<s_render_instr_block> block = blocks_head;

        while (block) {
            for (t_len i = 0; i < block->instrs.Len(); i++) {
                const auto &instr = block->instrs[i];

                switch (instr.type) {
                case ek_render_instr_type_mesh_update: {
                    const auto &mu = instr.MeshUpdate();

                    const auto mem = bgfx::makeRef(mu.verts.Ptr(), static_cast<uint32_t>(mu.verts.SizeInBytes()));
                    bgfx::update(mu.mesh->Mesh().vert_buf_bgfx_hdl, 0, mem);

                    break;
                }

                case ek_render_instr_type_mesh_draw: {
                    const auto &md = instr.MeshDraw();

                    bgfx::setVertexBuffer(0, md.mesh->Mesh().vert_buf_bgfx_hdl, 0, static_cast<uint32_t>(md.mesh->Mesh().verts_len));
                    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);
                    bgfx::submit(0, md.prog->ShaderProg().bgfx_hdl);

                    break;
                }
                }
            }

            block = block->next;
        }

        bgfx::frame();
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
