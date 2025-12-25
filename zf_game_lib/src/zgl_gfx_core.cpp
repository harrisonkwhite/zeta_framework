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

        // @todo: These 2 below shouldn't need to leak out.
        s_v2_i resolution_cache = {};
        s_gfx_resource_arena perm_resource_arena = {};
    } g_state;

    struct s_gfx_resource {
    public:
        e_gfx_resource_type type = ek_gfx_resource_type_invalid;
        s_ptr<s_gfx_resource> next = nullptr;

        s_gfx_resource() = default;
        s_gfx_resource(const s_gfx_resource &) = delete;

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
                bgfx::TextureHandle bgfx_hdl;
                s_v2_i size;
            } texture;
        } type_data = {};
    };

    static bgfx::ProgramHandle CreateBGFXShaderProg(const s_array_rdonly<t_u8> vert_shader_bin, const s_array_rdonly<t_u8> frag_shader_bin);

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

        s_rendering_basis(bgfx::DynamicVertexBufferHandle vb_hdl, bgfx::ProgramHandle prog_hdl, bgfx::UniformHandle tex_sampler_uniform_hdl, s_gfx_resource &px_tex)
            : vert_buf_bgfx_hdl(vb_hdl), shader_prog_bgfx_hdl(prog_hdl), texture_sampler_uniform_bgfx_hdl(tex_sampler_uniform_hdl), px_texture(px_tex) {}

        s_rendering_basis(const s_rendering_basis &) = delete;
    };

    struct s_rendering_context {
        const s_rendering_basis &basis;

        struct {
            s_static_array<s_batch_vert, g_batch_vert_limit_per_frame> verts;
            t_i32 vert_offs;
            t_i32 vert_cnt;

            s_ptr<const s_gfx_resource> texture;
        } batch_state = {};

        s_rendering_context(const s_rendering_basis &basis) : basis(basis) {}
        s_rendering_context(const s_rendering_context &) = delete;
    };

    static s_rendering_basis &CreateRenderingBasis(s_mem_arena &mem_arena, s_gfx_resource_arena &resource_arena);
    static void Flush(s_rendering_context &rc);

    // ============================================================
    // @section: General
    // ============================================================
    s_rendering_basis &InitGFX(s_mem_arena &mem_arena) {
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

        g_state.perm_resource_arena = {mem_arena};

        return CreateRenderingBasis(mem_arena, g_state.perm_resource_arena);
    }

    void ShutdownGFX(s_rendering_basis &rendering_state) {
        ZF_ASSERT(g_state.state == ek_state_initted);

        bgfx::destroy(rendering_state.texture_sampler_uniform_bgfx_hdl);
        bgfx::destroy(rendering_state.shader_prog_bgfx_hdl);
        bgfx::destroy(rendering_state.vert_buf_bgfx_hdl);

        g_state.perm_resource_arena.Release();

        bgfx::shutdown();

        g_state.state = ek_state_uninitted;
    }

    // ============================================================
    // @section: Resources
    // ============================================================
    s_v2_i TextureSize(const s_gfx_resource &texture) {
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
                break;

            case ek_gfx_resource_type_texture:
                bgfx::destroy(resource->Texture().bgfx_hdl);
                break;
            }

            const auto next = resource->next;
            *resource = {};
            resource = next;
        }

        *this = {};
    }

    t_b8 s_gfx_resource_arena::AddTexture(const s_texture_data_rdonly texture_data, s_ptr<s_gfx_resource> &o_resource) {
        ZF_ASSERT(IsInitted());
        ZF_ASSERT(g_state.state == ek_state_initted);

        const auto texture_bgfx_hdl = bgfx::createTexture2D(static_cast<uint16_t>(texture_data.SizeInPixels().x), static_cast<uint16_t>(texture_data.SizeInPixels().y), false, 1, bgfx::TextureFormat::RGBA8, 0, bgfx::copy(texture_data.RGBAPixelData().Ptr(), static_cast<uint32_t>(texture_data.RGBAPixelData().SizeInBytes())));

        if (!bgfx::isValid(texture_bgfx_hdl)) {
            return false;
        }

        o_resource = &Add(ek_gfx_resource_type_texture);
        o_resource->Texture().bgfx_hdl = texture_bgfx_hdl;
        o_resource->Texture().size = texture_data.SizeInPixels();

        return true;
    }

    s_gfx_resource &s_gfx_resource_arena::Add(const e_gfx_resource_type type) {
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

    s_gfx_resource_arena &PermGFXResourceArena() {
        ZF_ASSERT(g_state.state != ek_state_uninitted);
        return g_state.perm_resource_arena;
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

        s_ptr<s_gfx_resource> px_texture;
        const s_static_array<t_u8, 4> px_texture_rgba = {255, 255, 255, 255};

        if (!px_texture_resource_arena.AddTexture({{1, 1}, px_texture_rgba}, px_texture)) {
            ZF_FATAL();
        }

        return Alloc<s_rendering_basis>(mem_arena, vert_buf_bgfx_hdl, shader_prog_bgfx_hdl, texture_sampler_uniform_bgfx_hdl, *px_texture);
    }

    s_rendering_context &internal::BeginFrame(const s_rendering_basis &rendering_basis, const s_color_rgb24f clear_col, s_mem_arena &mem_arena) {
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

    void internal::EndFrame(s_rendering_context &rc) {
        ZF_ASSERT(g_state.state == ek_state_rendering);

        Flush(rc);

        bgfx::frame();

        g_state.state = ek_state_initted;
    }

    static void Flush(s_rendering_context &rc) {
        if (rc.batch_state.vert_cnt == 0) {
            return;
        }

        const auto verts = rc.batch_state.verts.ToNonstatic().Slice(rc.batch_state.vert_offs, rc.batch_state.vert_offs + rc.batch_state.vert_cnt);
        const auto verts_bgfx_ref = bgfx::makeRef(verts.Ptr(), static_cast<uint32_t>(verts.SizeInBytes()));
        bgfx::update(rc.basis.vert_buf_bgfx_hdl, static_cast<uint32_t>(rc.batch_state.vert_offs), verts_bgfx_ref);

        const auto texture_bgfx_hdl = rc.batch_state.texture ? rc.batch_state.texture->Texture().bgfx_hdl : rc.basis.px_texture.Texture().bgfx_hdl;
        bgfx::setTexture(0, rc.basis.texture_sampler_uniform_bgfx_hdl, texture_bgfx_hdl);

        bgfx::setVertexBuffer(0, rc.basis.vert_buf_bgfx_hdl, static_cast<uint32_t>(rc.batch_state.vert_offs), static_cast<uint32_t>(rc.batch_state.vert_cnt));

        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);

        bgfx::submit(0, rc.basis.shader_prog_bgfx_hdl);

        rc.batch_state.vert_offs += rc.batch_state.vert_cnt;
        rc.batch_state.vert_cnt = 0;
    }

    void SubmitTriangles(s_rendering_context &rc, const s_array_rdonly<s_batch_triangle> triangles, const s_ptr<const s_gfx_resource> texture) {
        ZF_ASSERT(g_state.state == ek_state_rendering);

        if (texture != rc.batch_state.texture) {
            Flush(rc);
            rc.batch_state.texture = texture;
        }

        const t_i32 num_verts_to_submit = 3 * triangles.Len();

        if (rc.batch_state.vert_offs + rc.batch_state.vert_cnt + num_verts_to_submit > rc.batch_state.verts.g_len) {
            ZF_FATAL();
        }

        for (t_i32 i = 0; i < triangles.Len(); i++) {
            const t_i32 offs = rc.batch_state.vert_offs + rc.batch_state.vert_cnt;
            rc.batch_state.verts[offs + (3 * i) + 0] = triangles[i].verts[0];
            rc.batch_state.verts[offs + (3 * i) + 1] = triangles[i].verts[1];
            rc.batch_state.verts[offs + (3 * i) + 2] = triangles[i].verts[2];
        }

        rc.batch_state.vert_cnt += num_verts_to_submit;
    }
}
