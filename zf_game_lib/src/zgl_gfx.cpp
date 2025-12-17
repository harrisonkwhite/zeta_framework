#include <zgl/zgl_gfx.h>

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
        s_v2_i resolution_cache;
        s_gfx_resource_arena perm_resource_arena;
    } g_state;

    static bgfx::ProgramHandle CreateBGFXShaderProg(const s_array_rdonly<t_u8> vert_shader_bin, const s_array_rdonly<t_u8> frag_shader_bin);
    static s_gfx_resource &PushGFXResource(const e_gfx_resource_type type, s_gfx_resource_arena &arena);

    struct s_gfx_resource {
    public:
        e_gfx_resource_type type = ek_gfx_resource_type_invalid;
        s_ptr<s_gfx_resource> next = nullptr;

        auto &Texture() {
            ZF_ASSERT(type == ek_gfx_resource_type_texture);
            return type_data.texture;
        }

        auto &Texture() const {
            ZF_ASSERT(type == ek_gfx_resource_type_texture);
            return type_data.texture;
        }

        auto &Font() {
            ZF_ASSERT(type == ek_gfx_resource_type_font);
            return type_data.font;
        }

        auto &Font() const {
            ZF_ASSERT(type == ek_gfx_resource_type_font);
            return type_data.font;
        }

    private:
        union {
            struct {
                bgfx::TextureHandle bgfx_hdl;
                s_v2_i size;
            } texture;

            struct {
                s_font_arrangement arrangement;
                s_array<s_ptr<s_gfx_resource>> atlases;
            } font;
        } type_data = {};
    };

    // @todo: "Batch" is probably not the right name for this.
    struct s_batch_vert {
        s_v2 pos;
        s_color_rgba32f blend;
        s_v2 uv;
    };

    extern const t_u8 g_batch_vert_shader_src_raw[];
    extern const t_len g_batch_vert_shader_src_len;

    extern const t_u8 g_batch_frag_shader_src_raw[];
    extern const t_len g_batch_frag_shader_src_len;

    constexpr t_i32 g_batch_vert_limit_per_frame = 8192;

    struct s_rendering_basis {
        bgfx::DynamicVertexBufferHandle vert_buf_bgfx_hdl;
        bgfx::ProgramHandle shader_prog_bgfx_hdl;
        bgfx::UniformHandle texture_sampler_uniform_bgfx_hdl;

        const s_gfx_resource &px_texture;

        s_rendering_basis(bgfx::DynamicVertexBufferHandle vb_hdl, bgfx::ProgramHandle prog_hdl, bgfx::UniformHandle tex_sampler_uniform_hdl, s_gfx_resource &px_tex)
            : vert_buf_bgfx_hdl(vb_hdl), shader_prog_bgfx_hdl(prog_hdl), texture_sampler_uniform_bgfx_hdl(tex_sampler_uniform_hdl), px_texture(px_tex) {}
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
    };

    static s_rendering_basis &CreateRenderingBasis(s_mem_arena &mem_arena, s_gfx_resource_arena &resource_arena);
    static void Flush(s_rendering_context &rc);
    static s_array<s_batch_vert> ReserveBatchVerts(s_rendering_context &rc, const t_i32 cnt, const s_ptr<const s_gfx_resource> texture);

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

        init.platformData.nwh = internal::NativeWindowHandle();
        init.platformData.ndt = internal::NativeDisplayHandle();
        init.platformData.type = bgfx::NativeWindowHandleType::Default;

        if (!bgfx::init(init)) {
            ZF_FATAL();
        }

        g_state.perm_resource_arena = CreateGFXResourceArena(mem_arena);

        return CreateRenderingBasis(mem_arena, g_state.perm_resource_arena);
    }

    void ShutdownGFX(s_rendering_basis &rendering_state) {
        ZF_ASSERT(g_state.state == ek_state_initted);

        bgfx::destroy(rendering_state.texture_sampler_uniform_bgfx_hdl);
        bgfx::destroy(rendering_state.shader_prog_bgfx_hdl);
        bgfx::destroy(rendering_state.vert_buf_bgfx_hdl);

        DestroyGFXResources(g_state.perm_resource_arena);

        bgfx::shutdown();

        g_state = {};
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

    void DestroyGFXResources(s_gfx_resource_arena &arena) {
        ZF_ASSERT(g_state.state == ek_state_initted);

        const auto destroy_resource = [](const auto self, s_gfx_resource &resource) -> void {
            switch (resource.type) {
            case ek_gfx_resource_type_invalid: break;

            case ek_gfx_resource_type_texture:
                bgfx::destroy(resource.Texture().bgfx_hdl);
                break;

            case ek_gfx_resource_type_font:
                for (t_len i = 0; i < resource.Font().atlases.Len(); i++) {
                    self(self, *resource.Font().atlases[i]);
                }

                break;
            }

            resource = {};
        };

        s_ptr<s_gfx_resource> resource = arena.head;

        while (resource) {
            const auto next = resource->next;
            destroy_resource(destroy_resource, *resource);
            resource = next;
        }

        arena = {};
    }

    t_b8 CreateTextureResource(const s_texture_data_rdonly texture_data, s_ptr<s_gfx_resource> &o_resource, const s_ptr<s_gfx_resource_arena> arena) {
        ZF_ASSERT(g_state.state == ek_state_initted);

        const auto texture_bgfx_hdl = bgfx::createTexture2D(static_cast<uint16_t>(texture_data.SizeInPixels().x), static_cast<uint16_t>(texture_data.SizeInPixels().y), false, 1, bgfx::TextureFormat::RGBA8, 0, bgfx::copy(texture_data.RGBAPixelData().Ptr(), static_cast<uint32_t>(texture_data.RGBAPixelData().SizeInBytes())));

        if (!bgfx::isValid(texture_bgfx_hdl)) {
            return false;
        }

        o_resource = &PushGFXResource(ek_gfx_resource_type_texture, arena ? *arena : g_state.perm_resource_arena);
        o_resource->Texture().bgfx_hdl = texture_bgfx_hdl;
        o_resource->Texture().size = texture_data.SizeInPixels();

        return true;
    }

    s_v2_i TextureSize(const s_gfx_resource &texture) {
        ZF_ASSERT(g_state.state != ek_state_uninitted);
        return texture.Texture().size;
    }

    [[nodiscard]] t_b8 CreateFontResource(const s_font_arrangement &arrangement, const s_array<t_font_atlas_rgba> atlas_rgbas, s_ptr<s_gfx_resource> &o_resource, const s_ptr<s_gfx_resource_arena> arena) {
        o_resource = &PushGFXResource(ek_gfx_resource_type_font, *arena);
        o_resource->Font().arrangement = arrangement;

        o_resource->Font().atlases = AllocArray<s_ptr<s_gfx_resource>>(atlas_rgbas.Len(), *arena->mem_arena);

        for (t_len i = 0; i < atlas_rgbas.Len(); i++) {
            if (!CreateTextureResource({g_font_atlas_size, atlas_rgbas[i]}, o_resource->Font().atlases[i], arena)) {
                return false;
            }
        }

        return true;
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

        const auto shader_prog_bgfx_hdl = CreateBGFXShaderProg({g_batch_vert_shader_src_raw, g_batch_vert_shader_src_len}, {g_batch_frag_shader_src_raw, g_batch_frag_shader_src_len});

        if (!bgfx::isValid(shader_prog_bgfx_hdl)) {
            ZF_FATAL();
        }

        const auto texture_sampler_uniform_bgfx_hdl = bgfx::createUniform("u_tex", bgfx::UniformType::Sampler);

        if (!bgfx::isValid(texture_sampler_uniform_bgfx_hdl)) {
            ZF_FATAL();
        }

        s_ptr<s_gfx_resource> px_texture;
        const s_static_array<t_u8, 4> px_texture_rgba = {255, 255, 255, 255};

        if (!CreateTextureResource({{1, 1}, px_texture_rgba}, px_texture, &px_texture_resource_arena)) {
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

    static s_array<s_batch_vert> ReserveBatchVerts(s_rendering_context &rc, const t_i32 cnt, const s_ptr<const s_gfx_resource> texture) {
        ZF_ASSERT(cnt >= 0);

        if (texture != rc.batch_state.texture) {
            Flush(rc);
            rc.batch_state.texture = texture;
        }

        if (rc.batch_state.vert_offs + rc.batch_state.vert_cnt + cnt > rc.batch_state.verts.g_len) {
            ZF_FATAL();
        }

        rc.batch_state.vert_cnt += cnt;

        return rc.batch_state.verts.ToNonstatic().Slice(rc.batch_state.vert_offs + rc.batch_state.vert_cnt - cnt, rc.batch_state.vert_offs + rc.batch_state.vert_cnt);
    }

    void DrawTriangle(s_rendering_context &rc, const s_static_array<s_v2, 3> &pts, const s_static_array<s_color_rgba32f, 3> &pt_colors) {
        ZF_ASSERT(g_state.state == ek_state_rendering);

        const auto verts = ReserveBatchVerts(rc, 3, nullptr);
        verts[0] = {pts[0], pt_colors[0], {}};
        verts[1] = {pts[1], pt_colors[1], {}};
        verts[2] = {pts[2], pt_colors[2], {}};
    }

    void DrawRect(s_rendering_context &rc, const s_rect_f rect, const s_color_rgba32f color_topleft, const s_color_rgba32f color_topright, const s_color_rgba32f color_bottomright, const s_color_rgba32f color_bottomleft) {
        ZF_ASSERT(g_state.state == ek_state_rendering);

        const auto verts = ReserveBatchVerts(rc, 6, nullptr);

        verts[0] = {rect.TopLeft(), color_topleft, {0.0f, 0.0f}};
        verts[1] = {rect.TopRight(), color_topright, {1.0f, 0.0f}};
        verts[2] = {rect.BottomRight(), color_bottomright, {1.0f, 1.0f}};

        verts[3] = {rect.BottomRight(), color_bottomright, {1.0f, 1.0f}};
        verts[4] = {rect.BottomLeft(), color_bottomleft, {0.0f, 1.0f}};
        verts[5] = {rect.TopLeft(), color_topleft, {0.0f, 0.0f}};
    }

    void DrawTexture(s_rendering_context &rc, const s_v2 pos, const s_gfx_resource &texture) {
        ZF_ASSERT(g_state.state == ek_state_rendering);

        const auto verts = ReserveBatchVerts(rc, 6, &texture);

        const s_rect_f rect = {pos, texture.Texture().size.ToV2()};

        verts[0] = {rect.TopLeft(), colors::g_white, {0.0f, 0.0f}};
        verts[1] = {rect.TopRight(), colors::g_white, {1.0f, 0.0f}};
        verts[2] = {rect.BottomRight(), colors::g_white, {1.0f, 1.0f}};

        verts[3] = {rect.BottomRight(), colors::g_white, {1.0f, 1.0f}};
        verts[4] = {rect.BottomLeft(), colors::g_white, {0.0f, 1.0f}};
        verts[5] = {rect.TopLeft(), colors::g_white, {0.0f, 0.0f}};
    }
}
