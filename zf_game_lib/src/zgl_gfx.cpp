#include <zgl/zgl_gfx.h>

#include <bgfx/bgfx.h>
#include <zgl/zgl_platform.h>

namespace zf {
    constexpr s_color_rgb8 g_bg_color_default = {109, 187, 255};

    struct {
        t_b8 initted;
        s_v2_i resolution_cache;
    } g_state;

    void InitGFX() {
        ZF_ASSERT(!g_state.initted);

        const auto fb_size_cache = WindowFramebufferSizeCache();

        bgfx::Init init;
        init.type = bgfx::RendererType::Count;

        init.resolution.reset = BGFX_RESET_VSYNC;
        init.resolution.width = static_cast<uint32_t>(fb_size_cache.x);
        init.resolution.height = static_cast<uint32_t>(fb_size_cache.y);
        g_state.resolution_cache = fb_size_cache;

        init.platformData.nwh = internal::NativeWindowHandle();
        init.platformData.ndt = internal::NativeDisplayHandle();
        init.platformData.type = bgfx::NativeWindowHandleType::Default;

        if (!bgfx::init(init)) {
            ZF_FATAL();
        }

        g_state.initted = true;
    }

    void ShutdownGFX() {
        ZF_ASSERT(g_state.initted);

        bgfx::shutdown();
        g_state = {};
    }

    // ============================================================
    // @section: Resources
    // ============================================================
    struct s_gfx_resource {
    public:
        e_gfx_resource_type type = ek_gfx_resource_type_invalid;
        s_ptr<s_gfx_resource> next = nullptr;

        auto &Mesh() {
            ZF_ASSERT(type == ek_gfx_resource_type_mesh);
            return type_data.mesh;
        }

        auto &Mesh() const {
            ZF_ASSERT(type == ek_gfx_resource_type_mesh);
            return type_data.mesh;
        }

        auto &ShaderProg() {
            ZF_ASSERT(type == ek_gfx_resource_type_shader_prog);
            return type_data.shader_prog;
        }

        auto &ShaderProg() const {
            ZF_ASSERT(type == ek_gfx_resource_type_shader_prog);
            return type_data.shader_prog;
        }

        auto &Uniform() {
            ZF_ASSERT(type == ek_gfx_resource_type_uniform);
            return type_data.uniform;
        }

        auto &Uniform() const {
            ZF_ASSERT(type == ek_gfx_resource_type_uniform);
            return type_data.uniform;
        }

        auto &Texture() {
            ZF_ASSERT(type == ek_gfx_resource_type_texture);
            return type_data.texture;
        }

        auto &Texture() const {
            ZF_ASSERT(type == ek_gfx_resource_type_texture);
            return type_data.texture;
        }

    private:
        union {
            struct {
                bgfx::DynamicVertexBufferHandle vert_buf_bgfx_hdl;
                t_len verts_len;
            } mesh;

            struct {
                bgfx::ProgramHandle bgfx_hdl;
            } shader_prog;

            struct {
                bgfx::UniformHandle bgfx_hdl;
            } uniform;

            struct {
                bgfx::TextureHandle bgfx_hdl;
                s_v2_i size;
            } texture;
        } type_data = {};
    };

    void DestroyGFXResources(s_gfx_resource_arena &arena) {
        s_ptr<const s_gfx_resource> resource = arena.head;

        while (resource) {
            switch (resource->type) {
            case ek_gfx_resource_type_mesh:
                bgfx::destroy(resource->Mesh().vert_buf_bgfx_hdl);
                break;

            case ek_gfx_resource_type_shader_prog:
                bgfx::destroy(resource->ShaderProg().bgfx_hdl);
                break;

            case ek_gfx_resource_type_uniform:
                bgfx::destroy(resource->Uniform().bgfx_hdl);
                break;

            case ek_gfx_resource_type_texture:
                bgfx::destroy(resource->Texture().bgfx_hdl);
                break;
            }

            resource = resource->next;
        }

        arena = {};
    }

    static s_gfx_resource &PushGFXResource(const e_gfx_resource_type type, s_gfx_resource_arena &arena) {
        ZF_ASSERT(type != ek_gfx_resource_type_invalid);

        s_gfx_resource &resource = Alloc<s_gfx_resource>(*arena.mem_arena);

        if (!arena.head) {
            arena.head = &resource;
            arena.tail = &resource;
        } else {
            arena.tail->next = &resource;
            arena.tail = &resource;
        }

        resource.type = type;

        return resource;
    }

    t_b8 CreateMesh(const t_len verts_len, s_gfx_resource_arena &arena, s_ptr<s_gfx_resource> &o_resource) {
        bgfx::VertexLayout layout = {};
        layout.begin().add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float).add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float).add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float).end(); // @todo: Allow customs!

        const auto vert_buf_bgfx_hdl = bgfx::createDynamicVertexBuffer(static_cast<uint32_t>(verts_len), layout);

        if (!bgfx::isValid(vert_buf_bgfx_hdl)) {
            return false;
        }

        o_resource = &PushGFXResource(ek_gfx_resource_type_mesh, arena);
        o_resource->Mesh().vert_buf_bgfx_hdl = vert_buf_bgfx_hdl;
        o_resource->Mesh().verts_len = verts_len;

        return true;
    }

    t_b8 CreateShaderProg(const s_array_rdonly<t_u8> vert_shader_bin, const s_array_rdonly<t_u8> frag_shader_bin, s_gfx_resource_arena &arena, s_ptr<s_gfx_resource> &o_resource) {
        const bgfx::Memory *const vert_shader_bgfx_mem = bgfx::makeRef(vert_shader_bin.Ptr(), static_cast<uint32_t>(vert_shader_bin.Len()));
        const bgfx::ShaderHandle vert_shader_bgfx_hdl = bgfx::createShader(vert_shader_bgfx_mem);

        if (!bgfx::isValid(vert_shader_bgfx_hdl)) {
            return false;
        }

        const bgfx::Memory *const frag_shader_bgfx_mem = bgfx::makeRef(frag_shader_bin.Ptr(), static_cast<uint32_t>(frag_shader_bin.Len()));
        const bgfx::ShaderHandle frag_shader_bgfx_hdl = bgfx::createShader(frag_shader_bgfx_mem);

        if (!bgfx::isValid(frag_shader_bgfx_hdl)) {
            return false;
        }

        const bgfx::ProgramHandle prog_bgfx_hdl = bgfx::createProgram(vert_shader_bgfx_hdl, frag_shader_bgfx_hdl, true);

        if (!bgfx::isValid(prog_bgfx_hdl)) {
            return false;
        }

        o_resource = &PushGFXResource(ek_gfx_resource_type_shader_prog, arena);
        o_resource->ShaderProg().bgfx_hdl = prog_bgfx_hdl;

        return true;
    }

    t_b8 CreateUniform(const s_str_rdonly name, s_gfx_resource_arena &arena, s_mem_arena &temp_mem_arena, s_ptr<s_gfx_resource> &o_resource) {
        const s_str_rdonly name_terminated = AllocStrCloneButAddTerminator(name, temp_mem_arena);

        const bgfx::UniformHandle uniform_bgfx_hdl = bgfx::createUniform(name_terminated.Cstr(), bgfx::UniformType::Sampler); // @todo: Allow different uniform types. BGFX enum on this is strange?

        if (!bgfx::isValid(uniform_bgfx_hdl)) {
            return false;
        }

        o_resource = &PushGFXResource(ek_gfx_resource_type_uniform, arena);
        o_resource->Uniform().bgfx_hdl = uniform_bgfx_hdl;

        return true;
    }

    t_b8 CreateTexture(const s_texture_data_rdonly texture_data, s_gfx_resource_arena &arena, s_ptr<s_gfx_resource> &o_resource) {
        const auto texture_bgfx_hdl = bgfx::createTexture2D(static_cast<uint16_t>(texture_data.SizeInPixels().x), static_cast<uint16_t>(texture_data.SizeInPixels().y), false, 1, bgfx::TextureFormat::RGBA8, 0, bgfx::copy(texture_data.RGBAPixelData().Ptr(), static_cast<uint32_t>(texture_data.RGBAPixelData().SizeInBytes())));

        if (!bgfx::isValid(texture_bgfx_hdl)) {
            return false;
        }

        o_resource = &PushGFXResource(ek_gfx_resource_type_texture, arena);
        o_resource->Texture().bgfx_hdl = texture_bgfx_hdl;
        o_resource->Texture().size = texture_data.SizeInPixels();

        return true;
    }

    s_v2_i TextureSize(const s_gfx_resource &texture_resource) {
        return texture_resource.Texture().size;
    }

    // ============================================================
    // @section: Rendering
    // ============================================================
    enum e_render_instr_type {
        ek_render_instr_type_invalid,
        ek_render_instr_type_mesh_update,
        ek_render_instr_type_texture_set,
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

        auto &TextureSet() {
            ZF_ASSERT(type == ek_render_instr_type_texture_set);
            return type_data.texture_set;
        }

        auto &TextureSet() const {
            ZF_ASSERT(type == ek_render_instr_type_texture_set);
            return type_data.texture_set;
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
                s_ptr<const s_gfx_resource> tex;
                s_ptr<const s_gfx_resource> sampler_uniform;
            } texture_set;

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

    void s_render_instr_seq::SubmitTextureSet(const s_gfx_resource &tex, const s_gfx_resource &sampler_uniform) {
        s_render_instr instr;
        instr.type = ek_render_instr_type_texture_set;
        instr.TextureSet().tex = &tex;
        instr.TextureSet().sampler_uniform = &sampler_uniform;

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
        ZF_ASSERT(g_state.initted);

        //
        // Handling Framebuffer Resize
        //
        const auto fb_size_cache = WindowFramebufferSizeCache();

        if (g_state.resolution_cache != fb_size_cache) {
            bgfx::reset(static_cast<uint32_t>(fb_size_cache.x), static_cast<uint32_t>(fb_size_cache.y), BGFX_RESET_VSYNC);
            g_state.resolution_cache = fb_size_cache;
        }

        //
        // View Setup
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

        //
        // Executing Instructions
        //
        bgfx::touch(0);

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

                case ek_render_instr_type_texture_set: {
                    bgfx::setTexture(0, instr.TextureSet().sampler_uniform->Uniform().bgfx_hdl, instr.TextureSet().tex->Texture().bgfx_hdl);
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
