#include <zgl/zgl_rendering.h>

#include <bgfx/bgfx.h>
#include <zgl/zgl_platform.h>

namespace zf {
    // ============================================================
    // @section: Types and Globals

    enum t_state : t_i32 {
        ec_state_inactive,
        ec_state_active_but_not_rendering,
        ec_state_active_and_rendering
    };

    struct {
        t_state state;
        t_v2_i resolution_cache;
        t_rendering_resource_group perm_resource_group;
    } g_state;

    enum t_rendering_resource_type : t_i32 {
        ec_rendering_resource_type_texture,
        ec_rendering_resource_type_shader_prog
    };

    struct t_rendering_resource {
        t_rendering_resource_type type;

        union {
            struct {
                bgfx::TextureHandle bgfx_hdl;
                t_v2_i size;
            } texture;

            struct {
                bgfx::ProgramHandle bgfx_hdl;
            } shader_prog;
        } type_data;

        t_rendering_resource *next;
    };

    extern const t_u8 g_batch_triangle_vert_shader_default_src_raw[];
    extern const t_i32 g_batch_triangle_vert_shader_default_src_len;

    extern const t_u8 g_batch_triangle_frag_shader_default_src_raw[];
    extern const t_i32 g_batch_triangle_frag_shader_default_src_len;

    const t_i32 g_batch_vert_limit = 1024;
    const t_i32 g_frame_vert_limit = 8192; // @todo: This should definitely be modifiable if the user wants.

    struct t_rendering_basis {
        bgfx::DynamicVertexBufferHandle vert_buf_bgfx_hdl;
        bgfx::ProgramHandle shader_prog_bgfx_hdl;
        bgfx::UniformHandle texture_sampler_uniform_bgfx_hdl;

        const t_rendering_resource *px_texture;
    };

    struct t_rendering_context {
        const t_rendering_basis *basis;

        t_i32 frame_vert_cnt;

        struct {
            t_static_array<t_batch_vertex, g_batch_vert_limit> verts;
            t_i32 vert_cnt;

            const t_rendering_resource *texture;
        } batch_state;
    };

    // ============================================================


    // ============================================================
    // @section: Functions

    // @todo: Placeholder!
    static bgfx::ProgramHandle f_rendering_create_bgfx_shader_prog(const t_array_rdonly<t_u8> vert_shader_bin, const t_array_rdonly<t_u8> frag_shader_bin) {
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

    t_rendering_basis *f_rendering_startup_module(mem::t_arena *const arena, t_rendering_resource_group **const o_perm_resource_group) {
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

        const auto fb_size_cache = platform::f_get_window_framebuffer_size_cache();

        bgfx_init.resolution.width = static_cast<uint32_t>(fb_size_cache.x);
        bgfx_init.resolution.height = static_cast<uint32_t>(fb_size_cache.y);

        g_state.resolution_cache = fb_size_cache;

        bgfx_init.platformData.nwh = platform::f_get_native_window_handle();
        bgfx_init.platformData.ndt = platform::f_get_native_display_handle();
        bgfx_init.platformData.type = bgfx::NativeWindowHandleType::Default;

        if (!bgfx::init(bgfx_init)) {
            ZF_FATAL();
        }

        g_state.perm_resource_group = {.arena = arena};
        *o_perm_resource_group = &g_state.perm_resource_group;

        //
        // Basis Setup
        //
        const auto basis = mem::f_arena_push_item<t_rendering_basis>(arena);

        {
            bgfx::VertexLayout vert_layout;
            vert_layout.begin().add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float).add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float).add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float).end();

            basis->vert_buf_bgfx_hdl = bgfx::createDynamicVertexBuffer(static_cast<uint32_t>(g_frame_vert_limit), vert_layout);

            if (!bgfx::isValid(basis->vert_buf_bgfx_hdl)) {
                ZF_FATAL();
            }
        }

        basis->shader_prog_bgfx_hdl = f_rendering_create_bgfx_shader_prog({g_batch_triangle_vert_shader_default_src_raw, g_batch_triangle_vert_shader_default_src_len}, {g_batch_triangle_frag_shader_default_src_raw, g_batch_triangle_frag_shader_default_src_len});

        if (!bgfx::isValid(basis->shader_prog_bgfx_hdl)) {
            ZF_FATAL();
        }

        basis->texture_sampler_uniform_bgfx_hdl = bgfx::createUniform("u_tex", bgfx::UniformType::Sampler);

        if (!bgfx::isValid(basis->texture_sampler_uniform_bgfx_hdl)) {
            ZF_FATAL();
        }

        const t_static_array<t_u8, 4> px_texture_rgba = {{255, 255, 255, 255}};
        basis->px_texture = f_rendering_create_texture({{1, 1}, f_array_get_as_nonstatic(px_texture_rgba)}, &g_state.perm_resource_group);

        return basis;
    }

    void f_rendering_shutdown_module(const t_rendering_basis *const basis) {
        ZF_ASSERT(g_state.state == ec_state_active_but_not_rendering);

        bgfx::destroy(basis->texture_sampler_uniform_bgfx_hdl);
        bgfx::destroy(basis->shader_prog_bgfx_hdl);
        bgfx::destroy(basis->vert_buf_bgfx_hdl);

        f_rendering_destroy_resource_group(&g_state.perm_resource_group);

        bgfx::shutdown();

        g_state = {};
    }

    t_v2_i f_rendering_get_texture_size(const t_rendering_resource *const texture) {
        ZF_ASSERT(g_state.state != ec_state_inactive);
        return texture->type_data.texture.size;
    }

    void f_rendering_destroy_resource_group(t_rendering_resource_group *const group) {
        ZF_ASSERT(g_state.state == ec_state_active_but_not_rendering);

        t_rendering_resource *resource = group->head;

        while (resource) {
            switch (resource->type) {
            case ec_rendering_resource_type_texture:
                bgfx::destroy(resource->type_data.texture.bgfx_hdl);
                break;

            case ec_rendering_resource_type_shader_prog:
                bgfx::destroy(resource->type_data.shader_prog.bgfx_hdl);
                break;

            default:
                ZF_UNREACHABLE();
            }

            t_rendering_resource *const next = resource->next;
            *resource = {};
            resource = next;
        }

        *group = {};
    }

    static t_rendering_resource *f_rendering_add_to_resource_group(t_rendering_resource_group *const group, const t_rendering_resource_type type) {
        ZF_ASSERT(g_state.state == ec_state_active_but_not_rendering);

        const auto resource = mem::f_arena_push_item_zeroed<t_rendering_resource>(group->arena);

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

    t_rendering_resource *f_rendering_create_texture(const gfx::t_texture_data_rdonly texture_data, t_rendering_resource_group *const group) {
        ZF_ASSERT(g_state.state == ec_state_active_but_not_rendering);

        const uint64_t sampler_flags = BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
        const auto texture_bgfx_hdl = bgfx::createTexture2D(static_cast<uint16_t>(texture_data.size_in_pxs.x), static_cast<uint16_t>(texture_data.size_in_pxs.y), false, 1, bgfx::TextureFormat::RGBA8, sampler_flags, bgfx::copy(texture_data.rgba_px_data.raw, static_cast<uint32_t>(f_array_get_size_in_bytes(texture_data.rgba_px_data))));

        if (!bgfx::isValid(texture_bgfx_hdl)) {
            ZF_FATAL();
        }

        const auto resource = f_rendering_add_to_resource_group(group, ec_rendering_resource_type_texture);
        resource->type_data.texture.bgfx_hdl = texture_bgfx_hdl;
        resource->type_data.texture.size = texture_data.size_in_pxs;
        return resource;
    }

    t_rendering_resource *f_rendering_create_shader_prog(const t_array_rdonly<t_u8> vert_shader_compiled_bin, const t_array_rdonly<t_u8> frag_shader_compiled_bin, t_rendering_resource_group *const group) {
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

        const auto resource = f_rendering_add_to_resource_group(group, ec_rendering_resource_type_shader_prog);
        resource->type_data.shader_prog.bgfx_hdl = prog_bgfx_hdl;
        return resource;
    }

    t_rendering_context *f_rendering_begin_frame(const t_rendering_basis *const basis, const gfx::t_color_rgb8 clear_col, mem::t_arena *const context_arena) {
        ZF_ASSERT(g_state.state == ec_state_active_but_not_rendering);

        const auto fb_size_cache = platform::f_get_window_framebuffer_size_cache();

        if (g_state.resolution_cache != fb_size_cache) {
            bgfx::reset(static_cast<uint32_t>(fb_size_cache.x), static_cast<uint32_t>(fb_size_cache.y), BGFX_RESET_VSYNC);
            g_state.resolution_cache = fb_size_cache;
        }

        bgfx::setViewMode(0, bgfx::ViewMode::Sequential);

        const auto view_mat = f_math_create_identity_matrix();

        auto proj_mat = f_math_create_identity_matrix();
        proj_mat.elems[0][0] = 1.0f / (static_cast<t_f32>(fb_size_cache.x) / 2.0f);
        proj_mat.elems[1][1] = -1.0f / (static_cast<t_f32>(fb_size_cache.y) / 2.0f);
        proj_mat.elems[3][0] = -1.0f;
        proj_mat.elems[3][1] = 1.0f;

        bgfx::setViewTransform(0, &view_mat, &proj_mat);

        bgfx::setViewClear(0, BGFX_CLEAR_COLOR, gfx::f_convert_color_to_hex(clear_col));

        bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(fb_size_cache.x), static_cast<uint16_t>(fb_size_cache.y));

        bgfx::touch(0);

        g_state.state = ec_state_active_and_rendering;

        const auto context = mem::f_arena_push_item_zeroed<t_rendering_context>(context_arena);
        context->basis = basis;

        return context;
    }

    static void f_rendering_flush_batch(t_rendering_context *const context) {
        ZF_ASSERT(g_state.state == ec_state_active_and_rendering);

        if (context->batch_state.vert_cnt == 0) {
            return;
        }

        if (context->frame_vert_cnt + context->batch_state.vert_cnt > g_frame_vert_limit) {
            ZF_FATAL();
        }

        const auto verts = f_array_slice(f_array_get_as_nonstatic(context->batch_state.verts), 0, context->batch_state.vert_cnt);
        const auto verts_bgfx_mem = bgfx::copy(verts.raw, static_cast<uint32_t>(f_array_get_size_in_bytes(verts)));
        bgfx::update(context->basis->vert_buf_bgfx_hdl, static_cast<uint32_t>(context->frame_vert_cnt), verts_bgfx_mem);

        const auto texture_bgfx_hdl = context->batch_state.texture ? context->batch_state.texture->type_data.texture.bgfx_hdl : context->basis->px_texture->type_data.texture.bgfx_hdl;
        bgfx::setTexture(0, context->basis->texture_sampler_uniform_bgfx_hdl, texture_bgfx_hdl);

        bgfx::setVertexBuffer(0, context->basis->vert_buf_bgfx_hdl, static_cast<uint32_t>(context->frame_vert_cnt), static_cast<uint32_t>(context->batch_state.vert_cnt));

        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);

        bgfx::submit(0, context->basis->shader_prog_bgfx_hdl);

        context->frame_vert_cnt += context->batch_state.vert_cnt;

        mem::f_clear_item(&context->batch_state, 0);
    }

    void f_rendering_end_frame(t_rendering_context *const context) {
        ZF_ASSERT(g_state.state == ec_state_active_and_rendering);

        f_rendering_flush_batch(context);

        bgfx::frame();

        g_state.state = ec_state_active_but_not_rendering;
    }

    void f_rendering_submit_triangle(t_rendering_context *const context, const t_array_rdonly<t_batch_triangle> triangles, const t_rendering_resource *const texture) {
        ZF_ASSERT(g_state.state == ec_state_active_and_rendering);
        ZF_ASSERT(triangles.len > 0);
        ZF_ASSERT(!texture || texture->type == ec_rendering_resource_type_texture);

        const t_i32 num_verts_to_submit = 3 * triangles.len;

        if (num_verts_to_submit > g_batch_vert_limit) {
            ZF_FATAL();
        }

        if (texture != context->batch_state.texture || context->batch_state.vert_cnt + num_verts_to_submit > g_batch_vert_limit) {
            f_rendering_flush_batch(context);
            context->batch_state.texture = texture;
        }

        for (t_i32 i = 0; i < triangles.len; i++) {
            const t_i32 offs = context->batch_state.vert_cnt;
            context->batch_state.verts[offs + (3 * i) + 0] = triangles[i].verts[0];
            context->batch_state.verts[offs + (3 * i) + 1] = triangles[i].verts[1];
            context->batch_state.verts[offs + (3 * i) + 2] = triangles[i].verts[2];
        }

        context->batch_state.vert_cnt += num_verts_to_submit;
    }

    // ============================================================
}
