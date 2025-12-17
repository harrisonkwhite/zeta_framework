#include <zgl/zgl_renderer.h>

#include <bgfx/bgfx.h>
#include <zgl/zgl_platform.h>

namespace zf {
    enum e_renderer_state {
        ek_renderer_state_uninitted,
        ek_renderer_state_initted,
        ek_renderer_state_rendering
    };

    struct s_batch_vert {
        s_v2 pos;
        s_color_rgba32f blend;
        s_v2 uv;
    };

    extern const t_u8 g_batch_vert_shader_src_raw[];
    extern const t_len g_batch_vert_shader_src_len;

    extern const t_u8 g_batch_frag_shader_src_raw[];
    extern const t_len g_batch_frag_shader_src_len;

    struct s_frame_builder {
        bgfx::DynamicVertexBufferHandle vert_buf_bgfx_hdl = BGFX_INVALID_HANDLE;
        bgfx::ProgramHandle shader_prog_bgfx_hdl = BGFX_INVALID_HANDLE;
        bgfx::UniformHandle texture_sampler_uniform_bgfx_hdl = BGFX_INVALID_HANDLE;

        s_ptr<s_gfx_resource> px_texture;

        struct {
            s_array<s_batch_vert> verts;
            t_i32 vert_offs = 0;
            t_i32 vert_cnt = 0;

            s_ptr<const s_gfx_resource> texture = nullptr;
        } batch_state;
    };

    struct {
        e_renderer_state state = ek_renderer_state_uninitted;
        s_v2_i resolution_cache;
        s_gfx_resource_arena perm_resource_arena;
    } g_state;

    static bgfx::ProgramHandle CreateShaderProg(const s_array_rdonly<t_u8> vert_shader_bin, const s_array_rdonly<t_u8> frag_shader_bin) {
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

    void InitRenderer(s_mem_arena &mem_arena, const t_i32 frame_vert_limit) {
        ZF_ASSERT(g_state.state == ek_renderer_state_uninitted);

        g_state.state = ek_renderer_state_initted;

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

#if 0
        {
            bgfx::VertexLayout layout = {};
            layout.begin().add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float).add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float).add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float).end();

            g_state.batch_resources.vert_buf_bgfx_hdl = bgfx::createDynamicVertexBuffer(static_cast<uint32_t>(frame_vert_limit), layout);

            if (!bgfx::isValid(g_state.batch_resources.vert_buf_bgfx_hdl)) {
                ZF_FATAL();
            }
        }

        g_state.batch_resources.shader_prog_bgfx_hdl = CreateShaderProg({g_batch_vert_shader_src_raw, g_batch_vert_shader_src_len}, {g_batch_frag_shader_src_raw, g_batch_frag_shader_src_len});

        if (!bgfx::isValid(g_state.batch_resources.shader_prog_bgfx_hdl)) {
            ZF_FATAL();
        }

        g_state.batch_resources.texture_sampler_uniform_bgfx_hdl = bgfx::createUniform("u_tex", bgfx::UniformType::Sampler);

        if (!bgfx::isValid(g_state.batch_resources.texture_sampler_uniform_bgfx_hdl)) {
            ZF_FATAL();
        }

        const s_static_array<t_u8, 4> px_texture_rgba = {255, 255, 255, 255};

        if (!CreateTexture({{1, 1}, px_texture_rgba}, g_state.batch_resources.px_texture)) {
            ZF_FATAL();
        }

        g_state.batch_state.verts = AllocArray<s_batch_vert>(frame_vert_limit, mem_arena);
#endif
    }

    void ShutdownRenderer(s_frame_builder &frame_builder) {
        ZF_ASSERT(g_state.state == ek_renderer_state_initted);

        bgfx::destroy(frame_builder.texture_sampler_uniform_bgfx_hdl);
        bgfx::destroy(frame_builder.shader_prog_bgfx_hdl);
        bgfx::destroy(frame_builder.vert_buf_bgfx_hdl);

        DestroyGFXResources(g_state.perm_resource_arena);

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

    void DestroyGFXResources(s_gfx_resource_arena &arena) {
        ZF_ASSERT(g_state.state == ek_renderer_state_initted);

        s_ptr<const s_gfx_resource> resource = arena.head;

        while (resource) {
            switch (resource->type) {
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

    t_b8 CreateTexture(const s_texture_data_rdonly texture_data, s_ptr<s_gfx_resource> &o_resource, const s_ptr<s_gfx_resource_arena> arena) {
        ZF_ASSERT(g_state.state == ek_renderer_state_initted);

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
        ZF_ASSERT(g_state.state != ek_renderer_state_uninitted);
        return texture.Texture().size;
    }

    // ============================================================
    // @section: Rendering
    // ============================================================
    void BeginFrame(s_frame_builder &frame_builder, const s_color_rgb24f clear_col) {
        ZF_ASSERT(g_state.state == ek_renderer_state_initted);

        frame_builder.batch_state = {};

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

        g_state.state = ek_renderer_state_rendering;
    }

    static void Flush(s_frame_builder &frame_builder) {
        if (frame_builder.batch_state.vert_cnt == 0) {
            return;
        }

        const auto verts = frame_builder.batch_state.verts.Slice(frame_builder.batch_state.vert_offs, frame_builder.batch_state.vert_offs + frame_builder.batch_state.vert_cnt);
        const auto verts_bgfx_ref = bgfx::makeRef(verts.Ptr(), static_cast<uint32_t>(verts.SizeInBytes()));
        bgfx::update(frame_builder.vert_buf_bgfx_hdl, static_cast<uint32_t>(frame_builder.batch_state.vert_offs), verts_bgfx_ref);

        const auto texture_bgfx_hdl = frame_builder.batch_state.texture ? frame_builder.batch_state.texture->Texture().bgfx_hdl : frame_builder.px_texture->Texture().bgfx_hdl;
        bgfx::setTexture(0, frame_builder.texture_sampler_uniform_bgfx_hdl, texture_bgfx_hdl);

        bgfx::setVertexBuffer(0, frame_builder.vert_buf_bgfx_hdl, static_cast<uint32_t>(frame_builder.batch_state.vert_offs), static_cast<uint32_t>(frame_builder.batch_state.vert_cnt));

        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);

        bgfx::submit(0, frame_builder.shader_prog_bgfx_hdl);

        frame_builder.batch_state.vert_offs += frame_builder.batch_state.vert_cnt;
        frame_builder.batch_state.vert_cnt = 0;
    }

    void EndFrame(s_frame_builder &frame_builder) {
        ZF_ASSERT(g_state.state == ek_renderer_state_rendering);

        Flush(frame_builder);

        bgfx::frame();

        g_state.state = ek_renderer_state_initted;
    }

    static s_array<s_batch_vert> ReserveBatchVerts(s_frame_builder &frame_builder, const t_i32 cnt, const s_ptr<const s_gfx_resource> texture) {
        ZF_ASSERT(cnt >= 0);

        if (texture != frame_builder.batch_state.texture) {
            Flush(frame_builder);
            frame_builder.batch_state.texture = texture;
        }

        if (frame_builder.batch_state.vert_offs + frame_builder.batch_state.vert_cnt + cnt > frame_builder.batch_state.verts.Len()) {
            ZF_FATAL();
        }

        frame_builder.batch_state.vert_cnt += cnt;

        return frame_builder.batch_state.verts.Slice(frame_builder.batch_state.vert_offs + frame_builder.batch_state.vert_cnt - cnt, frame_builder.batch_state.vert_offs + frame_builder.batch_state.vert_cnt);
    }

    void DrawTriangle(s_frame_builder &frame_builder, const s_static_array<s_v2, 3> &pts, const s_static_array<s_color_rgba32f, 3> &pt_colors) {
        ZF_ASSERT(g_state.state == ek_renderer_state_rendering);

        const auto verts = ReserveBatchVerts(frame_builder, 3, nullptr);
        verts[0] = {pts[0], pt_colors[0], {}};
        verts[1] = {pts[1], pt_colors[1], {}};
        verts[2] = {pts[2], pt_colors[2], {}};
    }

    void DrawRect(s_frame_builder &frame_builder, const s_rect_f rect, const s_color_rgba32f color_topleft, const s_color_rgba32f color_topright, const s_color_rgba32f color_bottomright, const s_color_rgba32f color_bottomleft) {
        ZF_ASSERT(g_state.state == ek_renderer_state_rendering);

        const auto verts = ReserveBatchVerts(frame_builder, 6, nullptr);

        verts[0] = {rect.TopLeft(), color_topleft, {0.0f, 0.0f}};
        verts[1] = {rect.TopRight(), color_topright, {1.0f, 0.0f}};
        verts[2] = {rect.BottomRight(), color_bottomright, {1.0f, 1.0f}};

        verts[3] = {rect.BottomRight(), color_bottomright, {1.0f, 1.0f}};
        verts[4] = {rect.BottomLeft(), color_bottomleft, {0.0f, 1.0f}};
        verts[5] = {rect.TopLeft(), color_topleft, {0.0f, 0.0f}};
    }

    void DrawTexture(s_frame_builder &frame_builder, const s_v2 pos, const s_gfx_resource &texture) {
        ZF_ASSERT(g_state.state == ek_renderer_state_rendering);

        const auto verts = ReserveBatchVerts(frame_builder, 6, &texture);

        const s_rect_f rect = {pos, texture.Texture().size.ToV2()};

        verts[0] = {rect.TopLeft(), colors::g_white, {0.0f, 0.0f}};
        verts[1] = {rect.TopRight(), colors::g_white, {1.0f, 0.0f}};
        verts[2] = {rect.BottomRight(), colors::g_white, {1.0f, 1.0f}};

        verts[3] = {rect.BottomRight(), colors::g_white, {1.0f, 1.0f}};
        verts[4] = {rect.BottomLeft(), colors::g_white, {0.0f, 1.0f}};
        verts[5] = {rect.TopLeft(), colors::g_white, {0.0f, 0.0f}};
    }
}
