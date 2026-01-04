#include <zgl/zgl_rendering.h>

#include <bgfx/bgfx.h>
#include <zgl/zgl_platform.h>

namespace zf::rendering {
    // ============================================================
    // @section: Types and Globals

    enum State : I32 {
        ec_state_inactive,
        ec_state_active_but_not_rendering,
        ec_state_active_and_rendering
    };

    struct {
        State state;
        s_v2_i resolution_cache;
        ResourceGroup perm_resource_group;
    } g_state;

    enum ResourceType : I32 {
        ec_resource_type_texture,
        ec_resource_type_shader_prog
    };

    struct Resource {
        ResourceType type;

        union {
            struct {
                bgfx::TextureHandle bgfx_hdl;
                s_v2_i size;
            } texture;

            struct {
                bgfx::ProgramHandle bgfx_hdl;
            } shader_prog;
        } type_data;

        Resource *next;
    };

    extern const U8 g_batch_triangle_vert_shader_default_src_raw[];
    extern const I32 g_batch_triangle_vert_shader_default_src_len;

    extern const U8 g_batch_triangle_frag_shader_default_src_raw[];
    extern const I32 g_batch_triangle_frag_shader_default_src_len;

    const I32 g_batch_vert_limit = 1024;
    const I32 g_frame_vert_limit = 8192; // @todo: This should definitely be modifiable if the user wants.

    struct Basis {
        bgfx::DynamicVertexBufferHandle vert_buf_bgfx_hdl;
        bgfx::ProgramHandle shader_prog_bgfx_hdl;
        bgfx::UniformHandle texture_sampler_uniform_bgfx_hdl;

        const Resource *px_texture;
    };

    struct Context {
        const Basis *basis;

        I32 frame_vert_cnt;

        struct {
            s_static_array<BatchVertex, g_batch_vert_limit> verts;
            I32 vert_cnt;

            const Resource *texture;
        } batch_state;
    };

    // ============================================================


    // ============================================================
    // @section: Functions

    // @todo: Placeholder!
    static bgfx::ProgramHandle create_bgfx_shader_prog(const s_array_rdonly<U8> vert_shader_bin, const s_array_rdonly<U8> frag_shader_bin) {
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

    Basis *startup_module(s_arena *const arena, ResourceGroup **const o_perm_resource_group) {
        ZF_ASSERT(g_state.state == ec_state_inactive);

        g_state = {
            .state = ec_state_active_but_not_rendering,
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
        // Basis Setup
        //
        const auto basis = ArenaPushItem<Basis>(arena);

        {
            bgfx::VertexLayout vert_layout;
            vert_layout.begin().add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float).add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float).add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float).end();

            basis->vert_buf_bgfx_hdl = bgfx::createDynamicVertexBuffer(static_cast<uint32_t>(g_frame_vert_limit), vert_layout);

            if (!bgfx::isValid(basis->vert_buf_bgfx_hdl)) {
                ZF_FATAL();
            }
        }

        basis->shader_prog_bgfx_hdl = create_bgfx_shader_prog({g_batch_triangle_vert_shader_default_src_raw, g_batch_triangle_vert_shader_default_src_len}, {g_batch_triangle_frag_shader_default_src_raw, g_batch_triangle_frag_shader_default_src_len});

        if (!bgfx::isValid(basis->shader_prog_bgfx_hdl)) {
            ZF_FATAL();
        }

        basis->texture_sampler_uniform_bgfx_hdl = bgfx::createUniform("u_tex", bgfx::UniformType::Sampler);

        if (!bgfx::isValid(basis->texture_sampler_uniform_bgfx_hdl)) {
            ZF_FATAL();
        }

        const s_static_array<U8, 4> px_texture_rgba = {{255, 255, 255, 255}};
        basis->px_texture = create_texture({{1, 1}, AsNonstatic(px_texture_rgba)}, &g_state.perm_resource_group);

        return basis;
    }

    void shutdown_module(const Basis *const basis) {
        ZF_ASSERT(g_state.state == ec_state_active_but_not_rendering);

        bgfx::destroy(basis->texture_sampler_uniform_bgfx_hdl);
        bgfx::destroy(basis->shader_prog_bgfx_hdl);
        bgfx::destroy(basis->vert_buf_bgfx_hdl);

        destroy_resource_group(&g_state.perm_resource_group);

        bgfx::shutdown();

        g_state = {};
    }

    s_v2_i get_texture_size(const Resource *const texture) {
        ZF_ASSERT(g_state.state != ec_state_inactive);
        return texture->type_data.texture.size;
    }

    void destroy_resource_group(ResourceGroup *const group) {
        ZF_ASSERT(g_state.state == ec_state_active_but_not_rendering);

        Resource *resource = group->head;

        while (resource) {
            switch (resource->type) {
            case ec_resource_type_texture:
                bgfx::destroy(resource->type_data.texture.bgfx_hdl);
                break;

            case ec_resource_type_shader_prog:
                bgfx::destroy(resource->type_data.shader_prog.bgfx_hdl);
                break;

            default:
                ZF_UNREACHABLE();
            }

            Resource *const next = resource->next;
            *resource = {};
            resource = next;
        }

        *group = {};
    }

    static Resource *add_to_resource_group(ResourceGroup *const group, const ResourceType type) {
        ZF_ASSERT(g_state.state == ec_state_active_but_not_rendering);

        const auto resource = mem_push_item_zeroed<Resource>(group->arena);

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

    Resource *create_texture(const gfx::TextureDataRdonly texture_data, ResourceGroup *const group) {
        ZF_ASSERT(g_state.state == ec_state_active_but_not_rendering);

        const uint64_t sampler_flags = BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
        const auto texture_bgfx_hdl = bgfx::createTexture2D(static_cast<uint16_t>(texture_data.size_in_pxs.x), static_cast<uint16_t>(texture_data.size_in_pxs.y), false, 1, bgfx::TextureFormat::RGBA8, sampler_flags, bgfx::copy(texture_data.rgba_px_data.raw, static_cast<uint32_t>(ArraySizeInBytes(texture_data.rgba_px_data))));

        if (!bgfx::isValid(texture_bgfx_hdl)) {
            ZF_FATAL();
        }

        const auto resource = add_to_resource_group(group, ec_resource_type_texture);
        resource->type_data.texture.bgfx_hdl = texture_bgfx_hdl;
        resource->type_data.texture.size = texture_data.size_in_pxs;
        return resource;
    }

    Resource *create_shader_prog(const s_array_rdonly<U8> vert_shader_compiled_bin, const s_array_rdonly<U8> frag_shader_compiled_bin, ResourceGroup *const group) {
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

        const auto resource = add_to_resource_group(group, ec_resource_type_shader_prog);
        resource->type_data.shader_prog.bgfx_hdl = prog_bgfx_hdl;
        return resource;
    }

    Context *begin_frame(const Basis *const basis, const gfx::ColorRGB8 clear_col, s_arena *const context_arena) {
        ZF_ASSERT(g_state.state == ec_state_active_but_not_rendering);

        const auto fb_size_cache = platform::get_window_framebuffer_size_cache();

        if (g_state.resolution_cache != fb_size_cache) {
            bgfx::reset(static_cast<uint32_t>(fb_size_cache.x), static_cast<uint32_t>(fb_size_cache.y), BGFX_RESET_VSYNC);
            g_state.resolution_cache = fb_size_cache;
        }

        bgfx::setViewMode(0, bgfx::ViewMode::Sequential);

        const auto view_mat = IdentityMatrix();

        auto proj_mat = IdentityMatrix();
        proj_mat.elems[0][0] = 1.0f / (static_cast<F32>(fb_size_cache.x) / 2.0f);
        proj_mat.elems[1][1] = -1.0f / (static_cast<F32>(fb_size_cache.y) / 2.0f);
        proj_mat.elems[3][0] = -1.0f;
        proj_mat.elems[3][1] = 1.0f;

        bgfx::setViewTransform(0, &view_mat, &proj_mat);

        bgfx::setViewClear(0, BGFX_CLEAR_COLOR, convert_color_to_hex(clear_col));

        bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(fb_size_cache.x), static_cast<uint16_t>(fb_size_cache.y));

        bgfx::touch(0);

        g_state.state = ec_state_active_and_rendering;

        const auto context = mem_push_item_zeroed<Context>(context_arena);
        context->basis = basis;

        return context;
    }

    static void flush_batch(Context *const context) {
        ZF_ASSERT(g_state.state == ec_state_active_and_rendering);

        if (context->batch_state.vert_cnt == 0) {
            return;
        }

        if (context->frame_vert_cnt + context->batch_state.vert_cnt > g_frame_vert_limit) {
            ZF_FATAL();
        }

        const auto verts = ArraySlice(AsNonstatic(context->batch_state.verts), 0, context->batch_state.vert_cnt);
        const auto verts_bgfx_mem = bgfx::copy(verts.raw, static_cast<uint32_t>(ArraySizeInBytes(verts)));
        bgfx::update(context->basis->vert_buf_bgfx_hdl, static_cast<uint32_t>(context->frame_vert_cnt), verts_bgfx_mem);

        const auto texture_bgfx_hdl = context->batch_state.texture ? context->batch_state.texture->type_data.texture.bgfx_hdl : context->basis->px_texture->type_data.texture.bgfx_hdl;
        bgfx::setTexture(0, context->basis->texture_sampler_uniform_bgfx_hdl, texture_bgfx_hdl);

        bgfx::setVertexBuffer(0, context->basis->vert_buf_bgfx_hdl, static_cast<uint32_t>(context->frame_vert_cnt), static_cast<uint32_t>(context->batch_state.vert_cnt));

        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);

        bgfx::submit(0, context->basis->shader_prog_bgfx_hdl);

        context->frame_vert_cnt += context->batch_state.vert_cnt;

        ClearItem(&context->batch_state, 0);
    }

    void end_frame(Context *const context) {
        ZF_ASSERT(g_state.state == ec_state_active_and_rendering);

        flush_batch(context);

        bgfx::frame();

        g_state.state = ec_state_active_but_not_rendering;
    }

    void submit_triangle(Context *const context, const s_array_rdonly<BatchTriangle> triangles, const Resource *const texture) {
        ZF_ASSERT(g_state.state == ec_state_active_and_rendering);
        ZF_ASSERT(triangles.len > 0);
        ZF_ASSERT(!texture || texture->type == ec_resource_type_texture);

        const I32 num_verts_to_submit = 3 * triangles.len;

        if (num_verts_to_submit > g_batch_vert_limit) {
            ZF_FATAL();
        }

        if (texture != context->batch_state.texture || context->batch_state.vert_cnt + num_verts_to_submit > g_batch_vert_limit) {
            flush_batch(context);
            context->batch_state.texture = texture;
        }

        for (I32 i = 0; i < triangles.len; i++) {
            const I32 offs = context->batch_state.vert_cnt;
            context->batch_state.verts[offs + (3 * i) + 0] = triangles[i].verts[0];
            context->batch_state.verts[offs + (3 * i) + 1] = triangles[i].verts[1];
            context->batch_state.verts[offs + (3 * i) + 2] = triangles[i].verts[2];
        }

        context->batch_state.vert_cnt += num_verts_to_submit;
    }

    // ============================================================
}
