#include <zgl/zgl_gfx_core.h>

#include <bgfx/bgfx.h>
#include <zgl/zgl_platform.h>

namespace zf {
    // ============================================================
    // @section: Types and Globals

    enum e_state : t_i32 {
        ek_state_inactive,
        ek_state_active_but_not_rendering,
        ek_state_active_and_rendering
    };

    struct {
        e_state state;
        s_v2_i resolution_cache;
        zf_rendering_resource_group perm_resource_group;
    } g_state;

    enum e_resource_type : t_i32 {
        ek_resource_type_texture,
        ek_resource_type_shader_prog
    };

    struct s_gfx_resource {
        e_resource_type type;

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
            ZF_ASSERT(type == ek_resource_type_texture);
            return type_data.texture;
        }

        auto &Texture() const {
            ZF_ASSERT(type == ek_resource_type_texture);
            return type_data.texture;
        }

        auto &ShaderProg() {
            ZF_ASSERT(type == ek_resource_type_shader_prog);
            return type_data.shader_prog;
        }

        auto &ShaderProg() const {
            ZF_ASSERT(type == ek_resource_type_shader_prog);
            return type_data.shader_prog;
        }
    };

    extern const t_u8 g_batch_triangle_vert_shader_default_src_raw[];
    extern const t_i32 g_batch_triangle_vert_shader_default_src_len;

    extern const t_u8 g_batch_triangle_frag_shader_default_src_raw[];
    extern const t_i32 g_batch_triangle_frag_shader_default_src_len;

    const t_i32 g_batch_vert_limit = 1024;
    const t_i32 g_frame_vert_limit = 8192; // @todo: This should definitely be modifiable if the user wants.

    struct s_rendering_basis {
        bgfx::DynamicVertexBufferHandle vert_buf_bgfx_hdl;
        bgfx::ProgramHandle shader_prog_bgfx_hdl;
        bgfx::UniformHandle texture_sampler_uniform_bgfx_hdl;

        const s_gfx_resource *px_texture;
    };

    struct s_rendering_context {
        const s_rendering_basis *basis;

        t_i32 frame_vert_cnt;

        struct {
            s_static_array<s_batch_vert, g_batch_vert_limit> verts;
            t_i32 vert_cnt;

            const s_gfx_resource *texture;
        } batch_state;
    };

    // ============================================================


    // ============================================================
    // @section: Functions

    // @todo: Placeholder!
    static bgfx::ProgramHandle CreateBGFXShaderProg(const s_array_rdonly<t_u8> vert_shader_bin, const s_array_rdonly<t_u8> frag_shader_bin) {
        const bgfx::Memory *const vert_shader_bgfx_mem = bgfx::copy(vert_shader_bin.raw, static_cast<uint32_t>(vert_shader_bin.len));
        const bgfx::ShaderHandle vert_shader_bgfx_hdl = bgfx::createShader(vert_shader_bgfx_mem);

        if (!bgfx::isValid(vert_shader_bgfx_hdl)) {
            return BGFX_INVALID_HANDLE;
        }

        const bgfx::Memory *const frag_shader_bgfx_mem = bgfx::copy(frag_shader_bin.raw, static_cast<uint32_t>(frag_shader_bin.len));
        const bgfx::ShaderHandle frag_shader_bgfx_hdl = bgfx::createShader(frag_shader_bgfx_mem);

        if (!bgfx::isValid(frag_shader_bgfx_hdl)) {
            return BGFX_INVALID_HANDLE;
        }

        return bgfx::createProgram(vert_shader_bgfx_hdl, frag_shader_bgfx_hdl, true);
    }

    s_rendering_basis *StartupGFX(s_arena *const arena, zf_rendering_resource_group **const o_perm_resource_group) {
        ZF_ASSERT(g_state.state == ek_state_inactive);

        g_state = {
            .state = ek_state_active_but_not_rendering,
        };

        //
        // BGFX Setup
        //
        bgfx::Init bgfx_init = {};

        bgfx_init.type = bgfx::RendererType::Count;

        bgfx_init.resolution.reset = BGFX_RESET_VSYNC;

        const auto fb_size_cache = platform::get_window_framebuffer_size_cache();

        bgfx_init.resolution.width = static_cast<uint32_t>(fb_size_cache.x);
        bgfx_init.resolution.height = static_cast<uint32_t>(fb_size_cache.y);

        g_state.resolution_cache = fb_size_cache;

        bgfx_init.platformData.nwh = platform::get_native_window_handle();
        bgfx_init.platformData.ndt = platform::get_native_display_handle();
        bgfx_init.platformData.type = bgfx::NativeWindowHandleType::Default;

        if (!bgfx::init(bgfx_init)) {
            ZF_FATAL();
        }

        g_state.perm_resource_group = {.arena = arena};
        *o_perm_resource_group = &g_state.perm_resource_group;

        //
        // Rendering Basis Setup
        //
        const auto rendering_basis = ArenaPushItem<s_rendering_basis>(arena);

        {
            bgfx::VertexLayout vert_layout;
            vert_layout.begin().add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float).add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float).add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float).end();

            rendering_basis->vert_buf_bgfx_hdl = bgfx::createDynamicVertexBuffer(static_cast<uint32_t>(g_frame_vert_limit), vert_layout);

            if (!bgfx::isValid(rendering_basis->vert_buf_bgfx_hdl)) {
                ZF_FATAL();
            }
        }

        rendering_basis->shader_prog_bgfx_hdl = CreateBGFXShaderProg({g_batch_triangle_vert_shader_default_src_raw, g_batch_triangle_vert_shader_default_src_len}, {g_batch_triangle_frag_shader_default_src_raw, g_batch_triangle_frag_shader_default_src_len});

        if (!bgfx::isValid(rendering_basis->shader_prog_bgfx_hdl)) {
            ZF_FATAL();
        }

        rendering_basis->texture_sampler_uniform_bgfx_hdl = bgfx::createUniform("u_tex", bgfx::UniformType::Sampler);

        if (!bgfx::isValid(rendering_basis->texture_sampler_uniform_bgfx_hdl)) {
            ZF_FATAL();
        }

        const s_static_array<t_u8, 4> px_texture_rgba = {{255, 255, 255, 255}};
        rendering_basis->px_texture = CreateTexture({{1, 1}, AsNonstatic(px_texture_rgba)}, &g_state.perm_resource_group);

        return rendering_basis;
    }

    void ShutdownGFX(const s_rendering_basis *const rendering_basis) {
        ZF_ASSERT(g_state.state == ek_state_active_but_not_rendering);

        bgfx::destroy(rendering_basis->texture_sampler_uniform_bgfx_hdl);
        bgfx::destroy(rendering_basis->shader_prog_bgfx_hdl);
        bgfx::destroy(rendering_basis->vert_buf_bgfx_hdl);

        DestroyGFXResourceGroup(&g_state.perm_resource_group);

        bgfx::shutdown();

        g_state = {};
    }

    s_v2_i TextureSize(const s_gfx_resource *const texture) {
        ZF_ASSERT(g_state.state != ek_state_inactive);
        return texture->Texture().size;
    }

    void DestroyGFXResourceGroup(zf_rendering_resource_group *const group) {
        ZF_ASSERT(g_state.state == ek_state_active_but_not_rendering);

        s_gfx_resource *resource = group->head;

        while (resource) {
            switch (resource->type) {
            case ek_resource_type_texture:
                bgfx::destroy(resource->Texture().bgfx_hdl);
                break;

            case ek_resource_type_shader_prog:
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

    static s_gfx_resource *AddToResourceGroup(zf_rendering_resource_group *const group, const e_resource_type type) {
        ZF_ASSERT(g_state.state == ek_state_active_but_not_rendering);

        const auto resource = mem_push_item_zeroed<s_gfx_resource>(group->arena);

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

    s_gfx_resource *CreateTexture(const s_texture_data_rdonly texture_data, zf_rendering_resource_group *const group) {
        ZF_ASSERT(g_state.state == ek_state_active_but_not_rendering);

        const uint64_t sampler_flags = BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
        const auto texture_bgfx_hdl = bgfx::createTexture2D(static_cast<uint16_t>(texture_data.size_in_pxs.x), static_cast<uint16_t>(texture_data.size_in_pxs.y), false, 1, bgfx::TextureFormat::RGBA8, sampler_flags, bgfx::copy(texture_data.rgba_px_data.raw, static_cast<uint32_t>(ArraySizeInBytes(texture_data.rgba_px_data))));

        if (!bgfx::isValid(texture_bgfx_hdl)) {
            ZF_FATAL();
        }

        const auto resource = AddToResourceGroup(group, ek_resource_type_texture);
        resource->Texture().bgfx_hdl = texture_bgfx_hdl;
        resource->Texture().size = texture_data.size_in_pxs;
        return resource;
    }

    s_gfx_resource *CreateShaderProg(const s_array_rdonly<t_u8> vert_shader_compiled_bin, const s_array_rdonly<t_u8> frag_shader_compiled_bin, zf_rendering_resource_group *const group) {
        const bgfx::Memory *const vert_shader_bgfx_mem = bgfx::copy(vert_shader_compiled_bin.raw, static_cast<uint32_t>(vert_shader_compiled_bin.len));
        const bgfx::ShaderHandle vert_shader_bgfx_hdl = bgfx::createShader(vert_shader_bgfx_mem);

        if (!bgfx::isValid(vert_shader_bgfx_hdl)) {
            ZF_FATAL();
        }

        const bgfx::Memory *const frag_shader_bgfx_mem = bgfx::copy(frag_shader_compiled_bin.raw, static_cast<uint32_t>(frag_shader_compiled_bin.len));
        const bgfx::ShaderHandle frag_shader_bgfx_hdl = bgfx::createShader(frag_shader_bgfx_mem);

        if (!bgfx::isValid(frag_shader_bgfx_hdl)) {
            ZF_FATAL();
        }

        const bgfx::ProgramHandle prog_bgfx_hdl = bgfx::createProgram(vert_shader_bgfx_hdl, frag_shader_bgfx_hdl, true);

        if (!bgfx::isValid(prog_bgfx_hdl)) {
            ZF_FATAL();
        }

        const auto resource = AddToResourceGroup(group, ek_resource_type_shader_prog);
        resource->ShaderProg().bgfx_hdl = prog_bgfx_hdl;
        return resource;
    }

    s_rendering_context *zf_rendering_begin_frame(const s_rendering_basis *const rendering_basis, const s_color_rgb8 clear_col, s_arena *const rendering_context_arena) {
        ZF_ASSERT(g_state.state == ek_state_active_but_not_rendering);

        const auto fb_size_cache = platform::get_window_framebuffer_size_cache();

        if (g_state.resolution_cache != fb_size_cache) {
            bgfx::reset(static_cast<uint32_t>(fb_size_cache.x), static_cast<uint32_t>(fb_size_cache.y), BGFX_RESET_VSYNC);
            g_state.resolution_cache = fb_size_cache;
        }

        bgfx::setViewMode(0, bgfx::ViewMode::Sequential);

        const auto view_mat = IdentityMatrix();

        auto proj_mat = IdentityMatrix();
        proj_mat.elems[0][0] = 1.0f / (static_cast<t_f32>(fb_size_cache.x) / 2.0f);
        proj_mat.elems[1][1] = -1.0f / (static_cast<t_f32>(fb_size_cache.y) / 2.0f);
        proj_mat.elems[3][0] = -1.0f;
        proj_mat.elems[3][1] = 1.0f;

        bgfx::setViewTransform(0, &view_mat, &proj_mat);

        bgfx::setViewClear(0, BGFX_CLEAR_COLOR, ColorToHex(clear_col));

        bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(fb_size_cache.x), static_cast<uint16_t>(fb_size_cache.y));

        bgfx::touch(0);

        g_state.state = ek_state_active_and_rendering;

        const auto rendering_context = mem_push_item_zeroed<s_rendering_context>(rendering_context_arena);
        rendering_context->basis = rendering_basis;

        return rendering_context;
    }

    static void FlushBatch(s_rendering_context *const rc) {
        ZF_ASSERT(g_state.state == ek_state_active_and_rendering);

        if (rc->batch_state.vert_cnt == 0) {
            return;
        }

        if (rc->frame_vert_cnt + rc->batch_state.vert_cnt > g_frame_vert_limit) {
            ZF_FATAL();
        }

        const auto verts = ArraySlice(AsNonstatic(rc->batch_state.verts), 0, rc->batch_state.vert_cnt);
        const auto verts_bgfx_mem = bgfx::copy(verts.raw, static_cast<uint32_t>(ArraySizeInBytes(verts)));
        bgfx::update(rc->basis->vert_buf_bgfx_hdl, static_cast<uint32_t>(rc->frame_vert_cnt), verts_bgfx_mem);

        const auto texture_bgfx_hdl = rc->batch_state.texture ? rc->batch_state.texture->Texture().bgfx_hdl : rc->basis->px_texture->Texture().bgfx_hdl;
        bgfx::setTexture(0, rc->basis->texture_sampler_uniform_bgfx_hdl, texture_bgfx_hdl);

        bgfx::setVertexBuffer(0, rc->basis->vert_buf_bgfx_hdl, static_cast<uint32_t>(rc->frame_vert_cnt), static_cast<uint32_t>(rc->batch_state.vert_cnt));

        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);

        bgfx::submit(0, rc->basis->shader_prog_bgfx_hdl);

        rc->frame_vert_cnt += rc->batch_state.vert_cnt;

        ClearItem(&rc->batch_state, 0);
    }

    void zf_rendering_end_frame(s_rendering_context *const rendering_context) {
        ZF_ASSERT(g_state.state == ek_state_active_and_rendering);

        FlushBatch(rendering_context);

        bgfx::frame();

        g_state.state = ek_state_active_but_not_rendering;
    }

    void SubmitTrianglesToBatch(s_rendering_context *const rc, const s_array_rdonly<s_batch_triangle> triangles, const s_gfx_resource *const texture) {
        ZF_ASSERT(g_state.state == ek_state_active_and_rendering);
        ZF_ASSERT(triangles.len > 0);
        ZF_ASSERT(!texture || texture->type == ek_resource_type_texture);

        const t_i32 num_verts_to_submit = 3 * triangles.len;

        if (num_verts_to_submit > g_batch_vert_limit) {
            ZF_FATAL();
        }

        if (texture != rc->batch_state.texture || rc->batch_state.vert_cnt + num_verts_to_submit > g_batch_vert_limit) {
            FlushBatch(rc);
            rc->batch_state.texture = texture;
        }

        for (t_i32 i = 0; i < triangles.len; i++) {
            const t_i32 offs = rc->batch_state.vert_cnt;
            rc->batch_state.verts[offs + (3 * i) + 0] = triangles[i].verts[0];
            rc->batch_state.verts[offs + (3 * i) + 1] = triangles[i].verts[1];
            rc->batch_state.verts[offs + (3 * i) + 2] = triangles[i].verts[2];
        }

        rc->batch_state.vert_cnt += num_verts_to_submit;
    }

    // ============================================================
}
