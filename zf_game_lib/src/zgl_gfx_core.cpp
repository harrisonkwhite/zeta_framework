#include <zgl/zgl_gfx_core.h>

#include <bgfx/bgfx.h>
#include <zgl/zgl_platform.h>

namespace zf {
    // ============================================================
    // @section: Types and Globals

    enum e_state : t_i32 {
        ek_state_uninitted,
        ek_state_initted,
        ek_state_rendering
    };

    struct {
        e_state state;
        s_v2_i resolution_cache;
        s_gfx_resource_group perm_resource_group;
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

        s_gfx_resource *next;

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

    extern const t_u8 g_batch_triangle_vert_shader_default_src_raw[];
    extern const t_i32 g_batch_triangle_vert_shader_default_src_len;

    extern const t_u8 g_batch_triangle_frag_shader_default_src_raw[];
    extern const t_i32 g_batch_triangle_frag_shader_default_src_len;

    constexpr t_i32 g_batch_vert_limit_per_frame = 8192; // @todo: This should definitely be modifiable if the user wants.

    struct s_rendering_basis {
        bgfx::DynamicVertexBufferHandle vert_buf_bgfx_hdl;
        bgfx::ProgramHandle shader_prog_bgfx_hdl;
        bgfx::UniformHandle texture_sampler_uniform_bgfx_hdl;

        const s_gfx_resource *px_texture;
    };

    struct s_rendering_context {
        const s_rendering_basis *basis;

        struct {
            s_static_array<s_rendering_vert, g_batch_vert_limit_per_frame> verts;
            t_i32 vert_offs;
            t_i32 vert_cnt;

            const s_gfx_resource *texture;
        } state;
    };

    // ============================================================


    static s_rendering_basis *CreateRenderingBasis(c_arena *const arena, s_gfx_resource_group *const px_texture_resource_group);

    s_rendering_basis *StartupGFXModule(c_arena *const arena) {
        ZF_ASSERT(g_state.state == ek_state_uninitted);

        g_state = {};

        g_state.state = ek_state_initted;

        bgfx::Init init;

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

        g_state.perm_resource_group = {.arena = arena};

        return CreateRenderingBasis(arena, &g_state.perm_resource_group);
    }

    void ShutdownGFXModule(const s_rendering_basis *const rendering_basis) {
        ZF_ASSERT(g_state.state == ek_state_initted);

        bgfx::destroy(rendering_basis->texture_sampler_uniform_bgfx_hdl);
        bgfx::destroy(rendering_basis->shader_prog_bgfx_hdl);
        bgfx::destroy(rendering_basis->vert_buf_bgfx_hdl);

        ReleaseGFXResources(&g_state.perm_resource_group);

        bgfx::shutdown();

        g_state.state = ek_state_uninitted;
        g_state = {};
    }


    // ============================================================
    // @section: Resources

    static bgfx::ProgramHandle CreateBGFXShaderProg(const c_array_rdonly<t_u8> vert_shader_bin, const c_array_rdonly<t_u8> frag_shader_bin) {
        const bgfx::Memory *const vert_shader_bgfx_mem = bgfx::makeRef(vert_shader_bin.Raw(), static_cast<uint32_t>(vert_shader_bin.Len()));
        const bgfx::ShaderHandle vert_shader_bgfx_hdl = bgfx::createShader(vert_shader_bgfx_mem);

        if (!bgfx::isValid(vert_shader_bgfx_hdl)) {
            return BGFX_INVALID_HANDLE;
        }

        const bgfx::Memory *const frag_shader_bgfx_mem = bgfx::makeRef(frag_shader_bin.Raw(), static_cast<uint32_t>(frag_shader_bin.Len()));
        const bgfx::ShaderHandle frag_shader_bgfx_hdl = bgfx::createShader(frag_shader_bgfx_mem);

        if (!bgfx::isValid(frag_shader_bgfx_hdl)) {
            return BGFX_INVALID_HANDLE;
        }

        return bgfx::createProgram(vert_shader_bgfx_hdl, frag_shader_bgfx_hdl, true);
    }

    s_v2_i TextureSize(const s_gfx_resource *const texture) {
        ZF_ASSERT(g_state.state != ek_state_uninitted);
        return texture->Texture().size;
    }

    s_gfx_resource_group *PermGFXResourceGroup() {
        ZF_ASSERT(g_state.state != ek_state_uninitted);
        return &g_state.perm_resource_group;
    }

    void ReleaseGFXResources(s_gfx_resource_group *const group) {
        ZF_ASSERT(g_state.state == ek_state_initted);

        s_gfx_resource *resource = group->head;

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

            s_gfx_resource *const next = resource->next;
            *resource = {};
            resource = next;
        }

        *group = {};
    }

    static s_gfx_resource *AddGFXResourceToGroup(s_gfx_resource_group *const group, const e_gfx_resource_type type) {
        ZF_ASSERT(g_state.state == ek_state_initted);

        const auto resource = Alloc<s_gfx_resource>(group->arena);

        if (!group->head) {
            group->head = resource;
            group->tail = resource;
        } else {
            group->tail->next = resource;
            group->tail = resource;
        }

        resource->type = type;

        return resource;
    }

    s_gfx_resource *CreateTextureResource(const s_texture_data_rdonly texture_data, s_gfx_resource_group *const group) {
        ZF_ASSERT(g_state.state == ek_state_initted);

        const auto texture_bgfx_hdl = bgfx::createTexture2D(static_cast<uint16_t>(texture_data.size_in_pxs.x), static_cast<uint16_t>(texture_data.size_in_pxs.y), false, 1, bgfx::TextureFormat::RGBA8, 0, bgfx::copy(texture_data.rgba_px_data.Raw(), static_cast<uint32_t>(texture_data.rgba_px_data.SizeInBytes())));

        if (!bgfx::isValid(texture_bgfx_hdl)) {
            ZF_FATAL();
        }

        const auto resource = AddGFXResourceToGroup(group, ek_gfx_resource_type_texture);
        resource->Texture().bgfx_hdl = texture_bgfx_hdl;
        resource->Texture().size = texture_data.size_in_pxs;
        return resource;
    }

    s_gfx_resource *CreateShaderProgResource(const c_array_rdonly<t_u8> vert_shader_compiled_bin, const c_array_rdonly<t_u8> frag_shader_compiled_bin, s_gfx_resource_group *const group) {
        const bgfx::Memory *const vert_shader_bgfx_mem = bgfx::makeRef(vert_shader_compiled_bin.Raw(), static_cast<uint32_t>(vert_shader_compiled_bin.Len()));
        const bgfx::ShaderHandle vert_shader_bgfx_hdl = bgfx::createShader(vert_shader_bgfx_mem);

        if (!bgfx::isValid(vert_shader_bgfx_hdl)) {
            ZF_FATAL();
        }

        const bgfx::Memory *const frag_shader_bgfx_mem = bgfx::makeRef(frag_shader_compiled_bin.Raw(), static_cast<uint32_t>(frag_shader_compiled_bin.Len()));
        const bgfx::ShaderHandle frag_shader_bgfx_hdl = bgfx::createShader(frag_shader_bgfx_mem);

        if (!bgfx::isValid(frag_shader_bgfx_hdl)) {
            ZF_FATAL();
        }

        const bgfx::ProgramHandle prog_bgfx_hdl = bgfx::createProgram(vert_shader_bgfx_hdl, frag_shader_bgfx_hdl, true);

        if (!bgfx::isValid(prog_bgfx_hdl)) {
            ZF_FATAL();
        }

        const auto resource = AddGFXResourceToGroup(group, ek_gfx_resource_type_shader_prog);
        resource->ShaderProg().bgfx_hdl = prog_bgfx_hdl;
        return resource;
    }

    // ============================================================


    // ============================================================
    // @section: Rendering

    static s_rendering_basis *CreateRenderingBasis(c_arena *const arena, s_gfx_resource_group *const px_texture_resource_group) {
        const auto res = Alloc<s_rendering_basis>(arena);

        {
            bgfx::VertexLayout vert_layout;
            vert_layout.begin().add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float).add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float).add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float).end();

            res->vert_buf_bgfx_hdl = bgfx::createDynamicVertexBuffer(static_cast<uint32_t>(g_batch_vert_limit_per_frame), vert_layout);

            if (!bgfx::isValid(res->vert_buf_bgfx_hdl)) {
                ZF_FATAL();
            }
        }

        res->shader_prog_bgfx_hdl = CreateBGFXShaderProg({g_batch_triangle_vert_shader_default_src_raw, g_batch_triangle_vert_shader_default_src_len}, {g_batch_triangle_frag_shader_default_src_raw, g_batch_triangle_frag_shader_default_src_len});

        if (!bgfx::isValid(res->shader_prog_bgfx_hdl)) {
            ZF_FATAL();
        }

        res->texture_sampler_uniform_bgfx_hdl = bgfx::createUniform("u_tex", bgfx::UniformType::Sampler);

        if (!bgfx::isValid(res->texture_sampler_uniform_bgfx_hdl)) {
            ZF_FATAL();
        }

        const s_static_array<t_u8, 4> px_texture_rgba = {{255, 255, 255, 255}};
        res->px_texture = CreateTextureResource({{1, 1}, px_texture_rgba}, px_texture_resource_group);

        return res;
    }

    s_rendering_context *BeginRendering(const s_rendering_basis *const rendering_basis, const s_color_rgb8 clear_col, c_arena *const rendering_context_arena) {
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

        const auto rendering_context = Alloc<s_rendering_context>(rendering_context_arena);
        rendering_context->basis = rendering_basis;

        return rendering_context;
    }

    static void Flush(s_rendering_context *const rc) {
        ZF_ASSERT(g_state.state == ek_state_rendering);

        if (rc->state.vert_cnt == 0) {
            return;
        }

        const auto verts = rc->state.verts.AsNonstatic().Slice(rc->state.vert_offs, rc->state.vert_offs + rc->state.vert_cnt);
        const auto verts_bgfx_ref = bgfx::makeRef(verts.Raw(), static_cast<uint32_t>(verts.SizeInBytes()));
        bgfx::update(rc->basis->vert_buf_bgfx_hdl, static_cast<uint32_t>(rc->state.vert_offs), verts_bgfx_ref);

        const auto texture_bgfx_hdl = rc->state.texture ? rc->state.texture->Texture().bgfx_hdl : rc->basis->px_texture->Texture().bgfx_hdl;
        bgfx::setTexture(0, rc->basis->texture_sampler_uniform_bgfx_hdl, texture_bgfx_hdl);

        bgfx::setVertexBuffer(0, rc->basis->vert_buf_bgfx_hdl, static_cast<uint32_t>(rc->state.vert_offs), static_cast<uint32_t>(rc->state.vert_cnt));

        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);

        bgfx::submit(0, rc->basis->shader_prog_bgfx_hdl);

        rc->state.vert_offs += rc->state.vert_cnt;
        rc->state.vert_cnt = 0;
    }

    void EndRendering(s_rendering_context *const rendering_context) {
        ZF_ASSERT(g_state.state == ek_state_rendering);

        Flush(rendering_context);

        bgfx::frame();

        g_state.state = ek_state_initted;
    }

    void RenderTriangles(s_rendering_context *const rc, const c_array_rdonly<s_render_triangle> triangles, const s_gfx_resource *const texture) {
        ZF_ASSERT(g_state.state == ek_state_rendering);
        ZF_ASSERT(triangles.Len() > 0);
        ZF_ASSERT(!texture || texture->type == ek_gfx_resource_type_texture);

        if (texture != rc->state.texture) {
            Flush(rc);
            rc->state.texture = texture;
        }

        const t_i32 num_verts_to_submit = 3 * triangles.Len();

        if (rc->state.vert_offs + rc->state.vert_cnt + num_verts_to_submit > rc->state.verts.g_len) {
            ZF_FATAL();
        }

        for (t_i32 i = 0; i < triangles.Len(); i++) {
            const t_i32 offs = rc->state.vert_offs + rc->state.vert_cnt;
            rc->state.verts[offs + (3 * i) + 0] = triangles[i].verts[0];
            rc->state.verts[offs + (3 * i) + 1] = triangles[i].verts[1];
            rc->state.verts[offs + (3 * i) + 2] = triangles[i].verts[2];
        }

        rc->state.vert_cnt += num_verts_to_submit;
    }

    // ============================================================
}
