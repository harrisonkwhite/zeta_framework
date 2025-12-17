#include <zgl/zgl_renderer.h>

#include <bgfx/bgfx.h>
#include <zgl/zgl_platform.h>

namespace zf::renderer {
    enum e_state {
        ek_uninitted,
        ek_state_initted,
        ek_state_rendering
    };

    struct {
        e_state state = ek_uninitted;
        s_v2_i resolution_cache;
    } g_state;

    void Init() {
        ZF_ASSERT(g_state.state == ek_uninitted);

        const auto fb_size_cache = WindowFramebufferSizeCache();

        bgfx::Init init = {};
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

        g_state.state = ek_state_initted;
    }

    void Shutdown() {
        ZF_ASSERT(g_state.state == ek_state_initted);

        bgfx::shutdown();
        g_state = {};
    }

    // ============================================================
    // @section: Resources
    // ============================================================
    struct s_resource {
    public:
        e_resource_type type = ek_resource_type_invalid;
        s_ptr<s_resource> next = nullptr;

        auto &Mesh() {
            ZF_ASSERT(type == ek_resource_type_mesh);
            return type_data.mesh;
        }

        auto &Mesh() const {
            ZF_ASSERT(type == ek_resource_type_mesh);
            return type_data.mesh;
        }

        auto &ShaderProg() {
            ZF_ASSERT(type == ek_resource_type_shader_prog);
            return type_data.shader_prog;
        }

        auto &ShaderProg() const {
            ZF_ASSERT(type == ek_resource_type_shader_prog);
            return type_data.shader_prog;
        }

        auto &Uniform() {
            ZF_ASSERT(type == ek_resource_type_uniform);
            return type_data.uniform;
        }

        auto &Uniform() const {
            ZF_ASSERT(type == ek_resource_type_uniform);
            return type_data.uniform;
        }

        auto &Texture() {
            ZF_ASSERT(type == ek_resource_type_texture);
            return type_data.texture;
        }

        auto &Texture() const {
            ZF_ASSERT(type == ek_resource_type_texture);
            return type_data.texture;
        }

    private:
        union {
            struct {
                bgfx::DynamicVertexBufferHandle vert_buf_bgfx_hdl;
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

    void DestroyResources(s_resource_arena &arena) {
        ZF_ASSERT(g_state.state == ek_state_initted);

        s_ptr<const s_resource> resource = arena.head;

        while (resource) {
            switch (resource->type) {
            case ek_resource_type_mesh:
                bgfx::destroy(resource->Mesh().vert_buf_bgfx_hdl);
                break;

            case ek_resource_type_shader_prog:
                bgfx::destroy(resource->ShaderProg().bgfx_hdl);
                break;

            case ek_resource_type_uniform:
                bgfx::destroy(resource->Uniform().bgfx_hdl);
                break;

            case ek_resource_type_texture:
                bgfx::destroy(resource->Texture().bgfx_hdl);
                break;
            }

            resource = resource->next;
        }

        arena = {};
    }

    static s_resource &PushGFXResource(const e_resource_type type, s_resource_arena &arena) {
        ZF_ASSERT(type != ek_resource_type_invalid);

        s_resource &resource = Alloc<s_resource>(*arena.mem_arena);

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

    t_b8 CreateMesh(const t_i32 vert_cnt, s_resource_arena &arena, s_ptr<s_resource> &o_resource) {
        ZF_ASSERT(g_state.state == ek_state_initted);

        bgfx::VertexLayout layout = {};
        layout.begin().add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float).add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float).add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float).end(); // @todo: Allow customs!

        const auto vert_buf_bgfx_hdl = bgfx::createDynamicVertexBuffer(static_cast<uint32_t>(vert_cnt), layout);

        if (!bgfx::isValid(vert_buf_bgfx_hdl)) {
            return false;
        }

        o_resource = &PushGFXResource(ek_resource_type_mesh, arena);
        o_resource->Mesh().vert_buf_bgfx_hdl = vert_buf_bgfx_hdl;

        return true;
    }

    void UpdateMeshVerts(const s_resource &mesh, const t_len vert_begin, const s_array_rdonly<t_f32> vert_data) {
        ZF_ASSERT(g_state.state == ek_state_initted);

        const auto mem = bgfx::makeRef(vert_data.Ptr(), static_cast<uint32_t>(vert_data.SizeInBytes()));
        bgfx::update(mesh.Mesh().vert_buf_bgfx_hdl, static_cast<uint32_t>(vert_begin), mem);
    }

    t_b8 CreateShaderProg(const s_array_rdonly<t_u8> vert_shader_bin, const s_array_rdonly<t_u8> frag_shader_bin, s_resource_arena &arena, s_ptr<s_resource> &o_resource) {
        ZF_ASSERT(g_state.state == ek_state_initted);

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

        o_resource = &PushGFXResource(ek_resource_type_shader_prog, arena);
        o_resource->ShaderProg().bgfx_hdl = prog_bgfx_hdl;

        return true;
    }

    t_b8 CreateUniform(const s_str_rdonly name, s_resource_arena &arena, s_mem_arena &temp_mem_arena, s_ptr<s_resource> &o_resource) {
        ZF_ASSERT(g_state.state == ek_state_initted);

        const s_str_rdonly name_terminated = AllocStrCloneButAddTerminator(name, temp_mem_arena);

        const bgfx::UniformHandle uniform_bgfx_hdl = bgfx::createUniform(name_terminated.Cstr(), bgfx::UniformType::Sampler); // @todo: Allow different uniform types. BGFX enum on this is strange?

        if (!bgfx::isValid(uniform_bgfx_hdl)) {
            return false;
        }

        o_resource = &PushGFXResource(ek_resource_type_uniform, arena);
        o_resource->Uniform().bgfx_hdl = uniform_bgfx_hdl;

        return true;
    }

    t_b8 CreateTexture(const s_texture_data_rdonly texture_data, s_resource_arena &arena, s_ptr<s_resource> &o_resource) {
        ZF_ASSERT(g_state.state == ek_state_initted);

        const auto texture_bgfx_hdl = bgfx::createTexture2D(static_cast<uint16_t>(texture_data.SizeInPixels().x), static_cast<uint16_t>(texture_data.SizeInPixels().y), false, 1, bgfx::TextureFormat::RGBA8, 0, bgfx::copy(texture_data.RGBAPixelData().Ptr(), static_cast<uint32_t>(texture_data.RGBAPixelData().SizeInBytes())));

        if (!bgfx::isValid(texture_bgfx_hdl)) {
            return false;
        }

        o_resource = &PushGFXResource(ek_resource_type_texture, arena);
        o_resource->Texture().bgfx_hdl = texture_bgfx_hdl;
        o_resource->Texture().size = texture_data.SizeInPixels();

        return true;
    }

    s_v2_i TextureSize(const s_resource &texture_resource) {
        ZF_ASSERT(g_state.state != ek_uninitted);
        return texture_resource.Texture().size;
    }

    // ============================================================
    // @section: Rendering
    // ============================================================
    void BeginFrame(const s_color_rgb24f clear_col) {
        ZF_ASSERT(g_state.state == ek_state_initted);

        const auto fb_size_cache = WindowFramebufferSizeCache();

        if (g_state.resolution_cache != fb_size_cache) {
            bgfx::reset(static_cast<uint32_t>(fb_size_cache.x), static_cast<uint32_t>(fb_size_cache.y), BGFX_RESET_VSYNC);
            g_state.resolution_cache = fb_size_cache;
        }

        bgfx::setViewMode(0, bgfx::ViewMode::Sequential);

        const auto view_mat = CreateIdentityMatrix();

        auto proj_mat = CreateIdentityMatrix();
        proj_mat.elems[0][0] = 1.0f / (static_cast<t_f32>(fb_size_cache.x) / 2.0f);
        proj_mat.elems[1][1] = -1.0f / (static_cast<t_f32>(fb_size_cache.y) / 2.0f);
        proj_mat.elems[3][0] = -1.0f;
        proj_mat.elems[3][1] = 1.0f;

        bgfx::setViewTransform(0, &view_mat, &proj_mat);

        bgfx::setViewClear(0, BGFX_CLEAR_COLOR, ColorToHex(clear_col));

        bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(fb_size_cache.x), static_cast<uint16_t>(fb_size_cache.y));

        bgfx::touch(0);

        g_state.state = ek_state_rendering;
    }

    void CompleteFrame() {
        ZF_ASSERT(g_state.state == ek_state_rendering);

        bgfx::frame();
        g_state.state = ek_state_initted;
    }

    void DrawMesh(const s_resource &mesh, const t_len vert_begin, const t_len vert_cnt, const s_resource &shader_prog, const s_resource &texture, const s_resource &texture_sampler_uniform) {
        ZF_ASSERT(g_state.state == ek_state_rendering);

        bgfx::setTexture(0, texture_sampler_uniform.Uniform().bgfx_hdl, texture.Texture().bgfx_hdl);
        bgfx::setVertexBuffer(0, mesh.Mesh().vert_buf_bgfx_hdl, static_cast<uint32_t>(vert_begin), static_cast<uint32_t>(vert_cnt));
        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);
        bgfx::submit(0, shader_prog.ShaderProg().bgfx_hdl);
    }
}
