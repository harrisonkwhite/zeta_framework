#include <zgl/zgl_gfx_core.h>

#include <bgfx/bgfx.h>
#include <zgl/zgl_platform.h>

namespace zf {
    // ============================================================
    // @section: Types and Declarations
    // ============================================================
    enum e_state {
        ek_state_uninitted,
        ek_state_initted,
        ek_state_rendering
    };

    struct {
        e_state state = ek_state_uninitted;
        s_v2_i resolution_cache = {};
        s_gfx_resource_arena perm_resource_arena = {};
    } g_state;

    enum e_gfx_resource_type : t_i32 {
        ek_gfx_resource_type_texture,
        ek_gfx_resource_type_shader_prog
    };

    struct s_gfx_resource {
        e_gfx_resource_type type;

        union {
            struct {
                bgfx::TextureHandle bgfx_hdl;
                s_v2_i size;
            } texture;

            struct {
                bgfx::ProgramHandle bgfx_hdl;
            } shader_prog;
        } type_data;

        s_ptr<s_gfx_resource> next;

        auto &Texture() {
            ZF_ASSERT(type == ek_gfx_resource_type_texture);
            return type_data.texture;
        }

        auto &Texture() const {
            ZF_ASSERT(type == ek_gfx_resource_type_texture);
            return type_data.texture;
        }

        auto &ShaderProg() {
            ZF_ASSERT(type == ek_gfx_resource_type_shader_prog);
            return type_data.shader_prog;
        }

        auto &ShaderProg() const {
            ZF_ASSERT(type == ek_gfx_resource_type_shader_prog);
            return type_data.shader_prog;
        }
    };

    static bgfx::ProgramHandle CreateBGFXShaderProg(const s_array_rdonly<t_u8> vert_shader_bin, const s_array_rdonly<t_u8> frag_shader_bin);
    static s_ptr<s_gfx_resource> PushGFXResource(const e_gfx_resource_type type, s_gfx_resource_arena &arena);

    extern const t_u8 g_batch_triangle_vert_shader_default_src_raw[];
    extern const t_i32 g_batch_triangle_vert_shader_default_src_len;

    extern const t_u8 g_batch_triangle_frag_shader_default_src_raw[];
    extern const t_i32 g_batch_triangle_frag_shader_default_src_len;

    constexpr t_i32 g_batch_vert_limit_per_frame = 8192; // @todo: This should definitely be modifiable if the user wants.

    struct s_rendering_basis {
        bgfx::DynamicVertexBufferHandle vert_buf_bgfx_hdl;
        bgfx::ProgramHandle shader_prog_bgfx_hdl;
        bgfx::UniformHandle texture_sampler_uniform_bgfx_hdl;

        const s_gfx_resource &px_texture;
    };

    struct s_rendering_context {
        const s_rendering_basis &basis;

        struct {
            s_static_array<s_rendering_vert, g_batch_vert_limit_per_frame> verts;
            t_i32 vert_offs;
            t_i32 vert_cnt;

            s_ptr<const s_gfx_resource> texture;
        } state;
    };

    static s_rendering_basis &CreateRenderingBasis(s_mem_arena &mem_arena, s_gfx_resource_arena &resource_arena);
    static void Flush(s_rendering_context &rc);

    // ============================================================
    // @section: General
    // ============================================================
    s_rendering_basis &StartupGFXModule(s_mem_arena &mem_arena) {
        ZF_ASSERT(g_state.state == ek_state_uninitted);

        g_state.state = ek_state_initted;

        bgfx::Init init = {};

        init.type = bgfx::RendererType::Count;

        init.resolution.reset = BGFX_RESET_VSYNC;

        const auto fb_size_cache = WindowFramebufferSizeCache();

        init.resolution.width = static_cast<uint32_t>(fb_size_cache.x);
        init.resolution.height = static_cast<uint32_t>(fb_size_cache.y);

        g_state.resolution_cache = fb_size_cache;

        init.platformData.nwh = NativeWindowHandle();
        init.platformData.ndt = NativeDisplayHandle();
        init.platformData.type = bgfx::NativeWindowHandleType::Default;

        if (!bgfx::init(init)) {
            ZF_FATAL();
        }

        g_state.perm_resource_arena = {.mem_arena = &mem_arena};

        return CreateRenderingBasis(mem_arena, g_state.perm_resource_arena);
    }

    void ShutdownGFXModule(s_rendering_basis &rendering_state) {
        ZF_ASSERT(g_state.state == ek_state_initted);

        bgfx::destroy(rendering_state.texture_sampler_uniform_bgfx_hdl);
        bgfx::destroy(rendering_state.shader_prog_bgfx_hdl);
        bgfx::destroy(rendering_state.vert_buf_bgfx_hdl);

        ReleaseGFXResources(g_state.perm_resource_arena);

        bgfx::shutdown();

        g_state.state = ek_state_uninitted;
    }

    // ============================================================
    // @section: Resources
    // ============================================================
    static bgfx::ProgramHandle CreateBGFXShaderProg(const s_array_rdonly<t_u8> vert_shader_bin, const s_array_rdonly<t_u8> frag_shader_bin) {
        const bgfx::Memory *const vert_shader_bgfx_mem = bgfx::makeRef(vert_shader_bin.Ptr(), static_cast<uint32_t>(vert_shader_bin.Len()));
        const bgfx::ShaderHandle vert_shader_bgfx_hdl = bgfx::createShader(vert_shader_bgfx_mem);

        if (!bgfx::isValid(vert_shader_bgfx_hdl)) {
            return BGFX_INVALID_HANDLE;
        }

        const bgfx::Memory *const frag_shader_bgfx_mem = bgfx::makeRef(frag_shader_bin.Ptr(), static_cast<uint32_t>(frag_shader_bin.Len()));
        const bgfx::ShaderHandle frag_shader_bgfx_hdl = bgfx::createShader(frag_shader_bgfx_mem);

        if (!bgfx::isValid(frag_shader_bgfx_hdl)) {
            return BGFX_INVALID_HANDLE;
        }

        return bgfx::createProgram(vert_shader_bgfx_hdl, frag_shader_bgfx_hdl, true);
    }

    s_v2_i TextureSize(const s_gfx_resource &texture) {
        ZF_ASSERT(g_state.state != ek_state_uninitted);
        return texture.Texture().size;
    }

#if 0
    s_v2_i GetTextureSize(const s_gfx_resource &texture) {
        ZF_ASSERT(g_state.state != ek_state_uninitted);
        return texture.Texture().size;
    }

    static bgfx::ProgramHandle CreateBGFXShaderProg(const s_array_rdonly<t_u8> vert_shader_bin, const s_array_rdonly<t_u8> frag_shader_bin) {
        const bgfx::Memory *const vert_shader_bgfx_mem = bgfx::makeRef(vert_shader_bin.Ptr(), static_cast<uint32_t>(vert_shader_bin.Len()));
        const bgfx::ShaderHandle vert_shader_bgfx_hdl = bgfx::createShader(vert_shader_bgfx_mem);

        if (!bgfx::isValid(vert_shader_bgfx_hdl)) {
            return BGFX_INVALID_HANDLE;
        }

        const bgfx::Memory *const frag_shader_bgfx_mem = bgfx::makeRef(frag_shader_bin.Ptr(), static_cast<uint32_t>(frag_shader_bin.Len()));
        const bgfx::ShaderHandle frag_shader_bgfx_hdl = bgfx::createShader(frag_shader_bgfx_mem);

        if (!bgfx::isValid(frag_shader_bgfx_hdl)) {
            return BGFX_INVALID_HANDLE;
        }

        return bgfx::createProgram(vert_shader_bgfx_hdl, frag_shader_bgfx_hdl, true);
    }

    void s_gfx_resource_arena::Release() {
        ZF_ASSERT(IsInitted());
        ZF_ASSERT(g_state.state == ek_state_initted);

        auto resource = m_head;

        while (resource) {
            switch (resource->type) {
            case ek_gfx_resource_type_invalid:
                ZF_UNREACHABLE();

            case ek_gfx_resource_type_texture:
                bgfx::destroy(resource->Texture().bgfx_hdl);
                break;

            case ek_gfx_resource_type_shader_prog:
                bgfx::destroy(resource->ShaderProg().bgfx_hdl);
                break;
            }

            const auto next = resource->next;
            *resource = {};
            resource = next;
        }

        *this = {};
    }

    s_gfx_resource &s_gfx_resource_arena::AddTexture(const s_texture_data_rdonly texture_data) {
        ZF_ASSERT(IsInitted());
        ZF_ASSERT(g_state.state == ek_state_initted);

        const auto texture_bgfx_hdl = bgfx::createTexture2D(static_cast<uint16_t>(texture_data.SizeInPixels().x), static_cast<uint16_t>(texture_data.SizeInPixels().y), false, 1, bgfx::TextureFormat::RGBA8, 0, bgfx::copy(texture_data.RGBAPixelData().Ptr(), static_cast<uint32_t>(texture_data.RGBAPixelData().SizeInBytes())));

        if (!bgfx::isValid(texture_bgfx_hdl)) {
            ZF_FATAL();
        }

        auto &res = Push(ek_gfx_resource_type_texture);
        res.Texture().bgfx_hdl = texture_bgfx_hdl;
        res.Texture().size = texture_data.SizeInPixels();
        return res;
    }

    s_gfx_resource &s_gfx_resource_arena::AddShaderProg(const s_array_rdonly<t_u8> vert_shader_compiled_bin, const s_array_rdonly<t_u8> frag_shader_compiled_bin) {
        const bgfx::Memory *const vert_shader_bgfx_mem = bgfx::makeRef(vert_shader_compiled_bin.Ptr(), static_cast<uint32_t>(vert_shader_compiled_bin.Len()));
        const bgfx::ShaderHandle vert_shader_bgfx_hdl = bgfx::createShader(vert_shader_bgfx_mem);

        if (!bgfx::isValid(vert_shader_bgfx_hdl)) {
            ZF_FATAL();
        }

        const bgfx::Memory *const frag_shader_bgfx_mem = bgfx::makeRef(frag_shader_compiled_bin.Ptr(), static_cast<uint32_t>(frag_shader_compiled_bin.Len()));
        const bgfx::ShaderHandle frag_shader_bgfx_hdl = bgfx::createShader(frag_shader_bgfx_mem);

        if (!bgfx::isValid(frag_shader_bgfx_hdl)) {
            ZF_FATAL();
        }

        const bgfx::ProgramHandle prog_bgfx_hdl = bgfx::createProgram(vert_shader_bgfx_hdl, frag_shader_bgfx_hdl, true);

        if (!bgfx::isValid(prog_bgfx_hdl)) {
            ZF_FATAL();
        }

        auto &res = Push(ek_gfx_resource_type_shader_prog);
        res.ShaderProg().bgfx_hdl = prog_bgfx_hdl;
        return res;
    }

    s_gfx_resource &s_gfx_resource_arena::Push(const e_gfx_resource_type type) {
        ZF_ASSERT(g_state.state == ek_state_initted);

        auto &resource = Alloc<s_gfx_resource>(*m_mem_arena);

        if (!m_head) {
            m_head = &resource;
            m_tail = &resource;
        } else {
            m_tail->next = &resource;
            m_tail = &resource;
        }

        resource.type = type;

        return resource;
    }
#endif

    s_gfx_resource_arena &PermGFXResourceArena() {
        ZF_ASSERT(g_state.state != ek_state_uninitted);
        return g_state.perm_resource_arena;
    }

    void ReleaseGFXResources(s_gfx_resource_arena &arena) {
        ZF_ASSERT(g_state.state == ek_state_initted);

        auto resource = arena.head;

        while (resource) {
            switch (resource->type) {
            case ek_gfx_resource_type_texture:
                bgfx::destroy(resource->Texture().bgfx_hdl);
                break;

            case ek_gfx_resource_type_shader_prog:
                bgfx::destroy(resource->ShaderProg().bgfx_hdl);
                break;

            default:
                ZF_UNREACHABLE();
            }

            const auto next = resource->next;
            *resource = {};
            resource = next;
        }

        arena = {};
    }

    static s_ptr<s_gfx_resource> PushGFXResource(const e_gfx_resource_type type, s_gfx_resource_arena &arena) {
        ZF_ASSERT(g_state.state == ek_state_initted);

        const s_ptr<s_gfx_resource> resource = &Alloc<s_gfx_resource>(*arena.mem_arena);

        if (!arena.head) {
            arena.head = resource;
            arena.tail = resource;
        } else {
            arena.tail->next = resource;
            arena.tail = resource;
        }

        resource->type = type;

        return resource;
    }

    s_ptr<s_gfx_resource> CreateTextureResource(const s_texture_data_rdonly texture_data, s_gfx_resource_arena &arena) {
        ZF_ASSERT(g_state.state == ek_state_initted);

        const auto texture_bgfx_hdl = bgfx::createTexture2D(static_cast<uint16_t>(texture_data.SizeInPixels().x), static_cast<uint16_t>(texture_data.SizeInPixels().y), false, 1, bgfx::TextureFormat::RGBA8, 0, bgfx::copy(texture_data.RGBAPixelData().Ptr(), static_cast<uint32_t>(texture_data.RGBAPixelData().SizeInBytes())));

        if (!bgfx::isValid(texture_bgfx_hdl)) {
            ZF_FATAL();
        }

        auto resource = PushGFXResource(ek_gfx_resource_type_texture, arena);
        resource->Texture().bgfx_hdl = texture_bgfx_hdl;
        resource->Texture().size = texture_data.SizeInPixels();
        return resource;
    }

    s_ptr<s_gfx_resource> CreateShaderProgResource(const s_array_rdonly<t_u8> vert_shader_compiled_bin, const s_array_rdonly<t_u8> frag_shader_compiled_bin, s_gfx_resource_arena &arena) {
        const bgfx::Memory *const vert_shader_bgfx_mem = bgfx::makeRef(vert_shader_compiled_bin.Ptr(), static_cast<uint32_t>(vert_shader_compiled_bin.Len()));
        const bgfx::ShaderHandle vert_shader_bgfx_hdl = bgfx::createShader(vert_shader_bgfx_mem);

        if (!bgfx::isValid(vert_shader_bgfx_hdl)) {
            ZF_FATAL();
        }

        const bgfx::Memory *const frag_shader_bgfx_mem = bgfx::makeRef(frag_shader_compiled_bin.Ptr(), static_cast<uint32_t>(frag_shader_compiled_bin.Len()));
        const bgfx::ShaderHandle frag_shader_bgfx_hdl = bgfx::createShader(frag_shader_bgfx_mem);

        if (!bgfx::isValid(frag_shader_bgfx_hdl)) {
            ZF_FATAL();
        }

        const bgfx::ProgramHandle prog_bgfx_hdl = bgfx::createProgram(vert_shader_bgfx_hdl, frag_shader_bgfx_hdl, true);

        if (!bgfx::isValid(prog_bgfx_hdl)) {
            ZF_FATAL();
        }

        auto resource = PushGFXResource(ek_gfx_resource_type_shader_prog, arena);
        resource->ShaderProg().bgfx_hdl = prog_bgfx_hdl;
        return resource;
    }

    // ============================================================
    // @section: Rendering
    // ============================================================
    static s_rendering_basis &CreateRenderingBasis(s_mem_arena &mem_arena, s_gfx_resource_arena &px_texture_resource_arena) {
        bgfx::VertexLayout vert_layout = {};
        vert_layout.begin().add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float).add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float).add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float).end();

        const auto vert_buf_bgfx_hdl = bgfx::createDynamicVertexBuffer(static_cast<uint32_t>(g_batch_vert_limit_per_frame), vert_layout);

        if (!bgfx::isValid(vert_buf_bgfx_hdl)) {
            ZF_FATAL();
        }

        const auto shader_prog_bgfx_hdl = CreateBGFXShaderProg({g_batch_triangle_vert_shader_default_src_raw, g_batch_triangle_vert_shader_default_src_len}, {g_batch_triangle_frag_shader_default_src_raw, g_batch_triangle_frag_shader_default_src_len});

        if (!bgfx::isValid(shader_prog_bgfx_hdl)) {
            ZF_FATAL();
        }

        const auto texture_sampler_uniform_bgfx_hdl = bgfx::createUniform("u_tex", bgfx::UniformType::Sampler);

        if (!bgfx::isValid(texture_sampler_uniform_bgfx_hdl)) {
            ZF_FATAL();
        }

        const s_static_array<t_u8, 4> px_texture_rgba = {{255, 255, 255, 255}};
        const auto px_texture = CreateTextureResource({{1, 1}, px_texture_rgba}, px_texture_resource_arena);

        return Alloc<s_rendering_basis>(mem_arena, vert_buf_bgfx_hdl, shader_prog_bgfx_hdl, texture_sampler_uniform_bgfx_hdl, *px_texture);
    }

    s_rendering_context &BeginRendering(const s_rendering_basis &rendering_basis, const s_color_rgb24f clear_col, s_mem_arena &mem_arena) {
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

        return Alloc<s_rendering_context>(mem_arena, rendering_basis);
    }

    void EndRendering(s_rendering_context &rc) {
        ZF_ASSERT(g_state.state == ek_state_rendering);

        Flush(rc);

        bgfx::frame();

        g_state.state = ek_state_initted;
    }

    static void Flush(s_rendering_context &rc) {
        if (rc.state.vert_cnt == 0) {
            return;
        }

        const auto verts = rc.state.verts.ToNonstatic().Slice(rc.state.vert_offs, rc.state.vert_offs + rc.state.vert_cnt);
        const auto verts_bgfx_ref = bgfx::makeRef(verts.Ptr(), static_cast<uint32_t>(verts.SizeInBytes()));
        bgfx::update(rc.basis.vert_buf_bgfx_hdl, static_cast<uint32_t>(rc.state.vert_offs), verts_bgfx_ref);

        const auto texture_bgfx_hdl = rc.state.texture ? rc.state.texture->Texture().bgfx_hdl : rc.basis.px_texture.Texture().bgfx_hdl;
        bgfx::setTexture(0, rc.basis.texture_sampler_uniform_bgfx_hdl, texture_bgfx_hdl);

        bgfx::setVertexBuffer(0, rc.basis.vert_buf_bgfx_hdl, static_cast<uint32_t>(rc.state.vert_offs), static_cast<uint32_t>(rc.state.vert_cnt));

        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);

        bgfx::submit(0, rc.basis.shader_prog_bgfx_hdl);

        rc.state.vert_offs += rc.state.vert_cnt;
        rc.state.vert_cnt = 0;
    }

    void RenderTriangles(s_rendering_context &rc, const s_array_rdonly<s_render_triangle> triangles, const s_ptr<const s_gfx_resource> texture) {
        ZF_ASSERT(g_state.state == ek_state_rendering);

        if (texture != rc.state.texture) {
            Flush(rc);
            rc.state.texture = texture;
        }

        const t_i32 num_verts_to_submit = 3 * triangles.Len();

        if (rc.state.vert_offs + rc.state.vert_cnt + num_verts_to_submit > rc.state.verts.g_len) {
            ZF_FATAL();
        }

        for (t_i32 i = 0; i < triangles.Len(); i++) {
            const t_i32 offs = rc.state.vert_offs + rc.state.vert_cnt;
            rc.state.verts[offs + (3 * i) + 0] = triangles[i].verts[0];
            rc.state.verts[offs + (3 * i) + 1] = triangles[i].verts[1];
            rc.state.verts[offs + (3 * i) + 2] = triangles[i].verts[2];
        }

        rc.state.vert_cnt += num_verts_to_submit;
    }
}
