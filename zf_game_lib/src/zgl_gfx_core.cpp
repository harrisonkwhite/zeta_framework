#include <zgl/zgl_gfx.h>

#include <bgfx/bgfx.h>
#include <zgl/zgl_platform.h>

#define BGFX_CONFIG_MAX_VIEWS k_frame_pass_limit

namespace zgl::gfx {
    enum t_module_phase : zcl::t_i32 {
        ek_module_phase_inactive,
        ek_module_phase_active_but_not_midframe,
        ek_module_phase_active_and_midframe
    };

    static struct {
        t_module_phase phase;

        // @todo: Try and drop both of these.
        zcl::t_v2_i resolution_cache;
        t_resource_group perm_resource_group; // @todo: Probably not needed as global anymore.
    } g_module_state;

    enum t_resource_type : zcl::t_i32 {
        ek_resource_type_invalid,
        ek_resource_type_texture,
        ek_resource_type_shader_prog,
        ek_resource_type_uniform
    };

    struct t_resource {
        t_resource_type type;

        union {
            struct {
                zcl::t_b8 is_target;
                bgfx::TextureHandle nontarget_texture_bgfx_hdl;
                bgfx::FrameBufferHandle target_fb_bgfx_hdl;
                zcl::t_v2_i size;
            } texture;

            struct {
                bgfx::ProgramHandle bgfx_hdl;
            } shader_prog;

            struct {
                bgfx::UniformHandle bgfx_hdl;
                t_uniform_type type;
            } uniform;
        } type_data;

        t_resource *next;
    };

    extern const zcl::t_u8 g_vert_shader_default_src_raw[];
    extern const zcl::t_i32 g_vert_shader_default_src_len;

    extern const zcl::t_u8 g_frag_shader_default_src_raw[];
    extern const zcl::t_i32 g_frag_shader_default_src_len;

    extern const zcl::t_u8 g_frag_shader_blend_src_raw[];
    extern const zcl::t_i32 g_frag_shader_blend_src_len;

    const zcl::t_i32 g_batch_vert_limit = 1024;
    const zcl::t_i32 g_frame_vert_limit = 8192; // @todo: This should definitely be modifiable if the user wants.

    struct t_frame_basis {
        bgfx::DynamicVertexBufferHandle vert_buf_bgfx_hdl;

        const t_resource *shader_prog_default;
        const t_resource *shader_prog_blend;

        const t_resource *sampler_uniform;
        const t_resource *blend_uniform;

        const t_resource *px_texture;
    };

    struct t_frame_context {
        const t_frame_basis *basis;

        zcl::t_b8 pass_active;
        zcl::t_i32 pass_index; // Maps directly to BGFX view ID.

        zcl::t_i32 frame_vert_cnt;

        struct {
            zcl::t_static_array<t_vertex, g_batch_vert_limit> verts;
            zcl::t_i32 vert_cnt;

            const t_resource *shader_prog;
            const t_resource *texture;
        } batch_state;
    };

    t_frame_basis *module_startup(zcl::t_arena *const arena, zcl::t_arena *const temp_arena, t_resource_group **const o_perm_resource_group) {
        ZCL_ASSERT(g_module_state.phase == ek_module_phase_inactive);

        g_module_state = {.phase = ek_module_phase_active_but_not_midframe};

        //
        // BGFX Setup
        //
        bgfx::Init bgfx_init = {};

        bgfx_init.type = bgfx::RendererType::Count;

        bgfx_init.resolution.reset = BGFX_RESET_VSYNC;

        const auto fb_size_cache = platform::window_get_framebuffer_size_cache();

        bgfx_init.resolution.width = static_cast<uint32_t>(fb_size_cache.x);
        bgfx_init.resolution.height = static_cast<uint32_t>(fb_size_cache.y);

        g_module_state.resolution_cache = fb_size_cache;

        bgfx_init.platformData.nwh = platform::window_get_native_handle();
        bgfx_init.platformData.ndt = platform::display_get_native_handle();
        bgfx_init.platformData.type = bgfx::NativeWindowHandleType::Default;

        if (!bgfx::init(bgfx_init)) {
            ZCL_FATAL();
        }

        g_module_state.perm_resource_group = resource_group_create(arena);
        *o_perm_resource_group = &g_module_state.perm_resource_group;

        //
        // Frame Basis Setup
        //
        const auto frame_basis = zcl::arena_push_item<t_frame_basis>(arena);

        {
            bgfx::VertexLayout vert_layout;
            vert_layout.begin().add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float).add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float).add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float).end();

            frame_basis->vert_buf_bgfx_hdl = bgfx::createDynamicVertexBuffer(static_cast<uint32_t>(g_frame_vert_limit), vert_layout);

            if (!bgfx::isValid(frame_basis->vert_buf_bgfx_hdl)) {
                ZCL_FATAL();
            }
        }

        frame_basis->shader_prog_default = shader_prog_create({g_vert_shader_default_src_raw, g_vert_shader_default_src_len}, {g_frag_shader_default_src_raw, g_frag_shader_default_src_len}, &g_module_state.perm_resource_group);
        frame_basis->shader_prog_blend = shader_prog_create({g_vert_shader_default_src_raw, g_vert_shader_default_src_len}, {g_frag_shader_blend_src_raw, g_frag_shader_blend_src_len}, &g_module_state.perm_resource_group);

        frame_basis->sampler_uniform = uniform_create(ZCL_STR_LITERAL("u_texture"), ek_uniform_type_sampler, &g_module_state.perm_resource_group, temp_arena);
        frame_basis->blend_uniform = uniform_create(ZCL_STR_LITERAL("u_blend"), ek_uniform_type_v4, &g_module_state.perm_resource_group, temp_arena);

        const zcl::t_static_array<zcl::t_u8, 4> batch_px_texture_rgba = {{255, 255, 255, 255}};
        frame_basis->px_texture = texture_create({{1, 1}, zcl::array_to_nonstatic(&batch_px_texture_rgba)}, &g_module_state.perm_resource_group);

        return frame_basis;
    }

    void module_shutdown(const t_frame_basis *const frame_basis) {
        ZCL_ASSERT(g_module_state.phase == ek_module_phase_active_but_not_midframe);

        bgfx::destroy(frame_basis->vert_buf_bgfx_hdl);

        resource_group_destroy(&g_module_state.perm_resource_group);

        bgfx::shutdown();

        g_module_state = {};
    }

    void resource_group_destroy(t_resource_group *const group) {
        ZCL_ASSERT(g_module_state.phase == ek_module_phase_active_but_not_midframe);

        t_resource *resource = group->head;

        while (resource) {
            switch (resource->type) {
            case ek_resource_type_texture:
                if (resource->type_data.texture.is_target) {
                    bgfx::destroy(resource->type_data.texture.target_fb_bgfx_hdl);
                } else {
                    bgfx::destroy(resource->type_data.texture.nontarget_texture_bgfx_hdl);
                }

                break;

            case ek_resource_type_shader_prog:
                bgfx::destroy(resource->type_data.shader_prog.bgfx_hdl);
                break;

            case ek_resource_type_uniform:
                bgfx::destroy(resource->type_data.uniform.bgfx_hdl);
                break;

            default:
                ZCL_UNREACHABLE();
            }

            t_resource *const resource_next = resource->next;
            *resource = {};
            resource = resource_next;
        }

        *group = {};
    }

    static t_resource *resource_group_add(t_resource_group *const group, const t_resource_type type) {
        ZCL_ASSERT(g_module_state.phase == ek_module_phase_active_but_not_midframe);

        const auto resource = zcl::arena_push_item<t_resource>(group->arena);

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

    t_resource *texture_create(const zcl::t_texture_data_rdonly texture_data, t_resource_group *const group) {
        ZCL_ASSERT(g_module_state.phase == ek_module_phase_active_but_not_midframe);

        const uint64_t flags = BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
        const auto texture_bgfx_hdl = bgfx::createTexture2D(static_cast<uint16_t>(texture_data.size_in_pxs.x), static_cast<uint16_t>(texture_data.size_in_pxs.y), false, 1, bgfx::TextureFormat::RGBA8, flags, bgfx::copy(texture_data.rgba_px_data.raw, static_cast<uint32_t>(zcl::array_get_size_in_bytes(texture_data.rgba_px_data))));

        if (!bgfx::isValid(texture_bgfx_hdl)) {
            ZCL_FATAL();
        }

        const auto resource = resource_group_add(group, ek_resource_type_texture);
        resource->type_data.texture.nontarget_texture_bgfx_hdl = texture_bgfx_hdl;
        resource->type_data.texture.size = texture_data.size_in_pxs;
        return resource;
    }

    t_resource *texture_create_from_raw(const zcl::t_str_rdonly file_path, zcl::t_arena *const temp_arena, t_resource_group *const group) {
        ZCL_ASSERT(g_module_state.phase == ek_module_phase_active_but_not_midframe);

        zcl::t_texture_data_mut texture_data;

        if (!zcl::TextureLoadFromRaw(file_path, temp_arena, temp_arena, &texture_data)) {
            ZCL_FATAL();
        }

        return texture_create(texture_data, group);
    }

    t_resource *texture_create_from_packed(const zcl::t_str_rdonly file_path, zcl::t_arena *const temp_arena, t_resource_group *const group) {
        ZCL_ASSERT(g_module_state.phase == ek_module_phase_active_but_not_midframe);

        zcl::t_file_stream file_stream;

        if (!zcl::FileOpen(file_path, zcl::t_file_access_mode::ek_file_access_mode_read, temp_arena, &file_stream)) {
            ZCL_FATAL();
        }

        zcl::t_texture_data_mut texture_data;

        if (!zcl::DeserializeTexture(file_stream, temp_arena, &texture_data)) {
            ZCL_FATAL();
        }

        zcl::FileClose(&file_stream);

        return texture_create(texture_data, group);
    }

    static bgfx::FrameBufferHandle bgfx_create_framebuffer(const zcl::t_v2_i size) {
        return bgfx::createFrameBuffer(static_cast<uint16_t>(size.x), static_cast<uint16_t>(size.y), bgfx::TextureFormat::RGBA8, BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP);
    }

    t_resource *texture_create_target(const zcl::t_v2_i size, t_resource_group *const group) {
        ZCL_ASSERT(g_module_state.phase == ek_module_phase_active_but_not_midframe);
        ZCL_ASSERT(size.x > 0 && size.y > 0);

        const bgfx::FrameBufferHandle fb_bgfx_hdl = bgfx_create_framebuffer(size);

        if (!bgfx::isValid(fb_bgfx_hdl)) {
            ZCL_FATAL();
        }

        const auto resource = resource_group_add(group, ek_resource_type_texture);
        resource->type_data.texture.is_target = true;
        resource->type_data.texture.target_fb_bgfx_hdl = fb_bgfx_hdl;
        resource->type_data.texture.size = size;
        return resource;
    }

    void texture_resize_target(t_resource *const texture, const zcl::t_v2_i size) {
        ZCL_ASSERT(g_module_state.phase == ek_module_phase_active_but_not_midframe);
        ZCL_ASSERT(texture->type == ek_resource_type_texture && texture->type_data.texture.is_target);
        ZCL_ASSERT(size.x > 0 && size.y > 0);
        ZCL_ASSERT(size != texture->type_data.texture.size);

        const bgfx::FrameBufferHandle fb_bgfx_hdl = bgfx_create_framebuffer(size);

        if (!bgfx::isValid(fb_bgfx_hdl)) {
            ZCL_FATAL();
        }

        texture->type_data.texture.target_fb_bgfx_hdl = fb_bgfx_hdl;
        texture->type_data.texture.size = size;
    }

    zcl::t_v2_i texture_get_size(const t_resource *const texture) {
        ZCL_ASSERT(g_module_state.phase != ek_module_phase_inactive);
        ZCL_ASSERT(texture->type == ek_resource_type_texture);

        return texture->type_data.texture.size;
    }

    t_resource *shader_prog_create(const zcl::t_array_rdonly<zcl::t_u8> vert_shader_compiled_bin, const zcl::t_array_rdonly<zcl::t_u8> frag_shader_compiled_bin, t_resource_group *const group) {
        ZCL_ASSERT(g_module_state.phase == ek_module_phase_active_but_not_midframe);

        const bgfx::Memory *const vert_shader_bgfx_mem = bgfx::copy(vert_shader_compiled_bin.raw, static_cast<uint32_t>(vert_shader_compiled_bin.len));
        const bgfx::ShaderHandle vert_shader_bgfx_hdl = bgfx::createShader(vert_shader_bgfx_mem);

        if (!bgfx::isValid(vert_shader_bgfx_hdl)) {
            ZCL_FATAL();
        }

        const bgfx::Memory *const frag_shader_bgfx_mem = bgfx::copy(frag_shader_compiled_bin.raw, static_cast<uint32_t>(frag_shader_compiled_bin.len));
        const bgfx::ShaderHandle frag_shader_bgfx_hdl = bgfx::createShader(frag_shader_bgfx_mem);

        if (!bgfx::isValid(frag_shader_bgfx_hdl)) {
            ZCL_FATAL();
        }

        const bgfx::ProgramHandle prog_bgfx_hdl = bgfx::createProgram(vert_shader_bgfx_hdl, frag_shader_bgfx_hdl, true);

        if (!bgfx::isValid(prog_bgfx_hdl)) {
            ZCL_FATAL();
        }

        const auto resource = resource_group_add(group, ek_resource_type_shader_prog);
        resource->type_data.shader_prog.bgfx_hdl = prog_bgfx_hdl;
        return resource;
    }

    t_resource *shader_prog_create_from_packed(const zcl::t_str_rdonly vert_shader_file_path, const zcl::t_str_rdonly frag_shader_file_path, t_resource_group *const group, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(g_module_state.phase == ek_module_phase_active_but_not_midframe);

        zcl::t_array_mut<zcl::t_u8> vert_shader_compiled_bin;

        {
            zcl::t_file_stream vert_shader_file_stream;

            if (!zcl::FileOpen(vert_shader_file_path, zcl::t_file_access_mode::ek_file_access_mode_read, temp_arena, &vert_shader_file_stream)) {
                ZCL_FATAL();
            }

            if (!zcl::DeserializeShader(vert_shader_file_stream, temp_arena, &vert_shader_compiled_bin)) {
                ZCL_FATAL();
            }

            zcl::FileClose(&vert_shader_file_stream);
        }

        zcl::t_array_mut<zcl::t_u8> frag_shader_compiled_bin;

        {
            zcl::t_file_stream frag_shader_file_stream;

            if (!zcl::FileOpen(frag_shader_file_path, zcl::t_file_access_mode::ek_file_access_mode_read, temp_arena, &frag_shader_file_stream)) {
                ZCL_FATAL();
            }

            if (!zcl::DeserializeShader(frag_shader_file_stream, temp_arena, &frag_shader_compiled_bin)) {
                ZCL_FATAL();
            }

            zcl::FileClose(&frag_shader_file_stream);
        }

        return shader_prog_create(vert_shader_compiled_bin, frag_shader_compiled_bin, group);
    }

    t_resource *uniform_create(const zcl::t_str_rdonly name, const t_uniform_type type, t_resource_group *const group, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(g_module_state.phase == ek_module_phase_active_but_not_midframe);

        const zcl::t_str_rdonly name_terminated = zcl::StrCloneButAddTerminator(name, temp_arena);

        const auto bgfx_type = [type]() -> bgfx::UniformType::Enum {
            switch (type) {
            case ek_uniform_type_sampler: return bgfx::UniformType::Sampler;
            case ek_uniform_type_v4: return bgfx::UniformType::Vec4;
            case ek_uniform_type_mat4x4: return bgfx::UniformType::Mat4;
            }

            ZCL_UNREACHABLE();
        }();

        const bgfx::UniformHandle bgfx_hdl = bgfx::createUniform(zcl::StrToCStr(name_terminated), bgfx_type);

        if (!bgfx::isValid(bgfx_hdl)) {
            ZCL_FATAL();
        }

        const auto resource = resource_group_add(group, ek_resource_type_uniform);
        resource->type_data.uniform.bgfx_hdl = bgfx_hdl;
        resource->type_data.uniform.type = type;
        return resource;
    }

    t_uniform_type uniform_get_type(const t_resource *const uniform) {
        ZCL_ASSERT(g_module_state.phase != ek_module_phase_inactive);
        ZCL_ASSERT(uniform->type == ek_resource_type_uniform);

        return uniform->type_data.uniform.type;
    }

    t_frame_context *frame_begin(const t_frame_basis *const basis, zcl::t_arena *const context_arena) {
        ZCL_ASSERT(g_module_state.phase == ek_module_phase_active_but_not_midframe);

        g_module_state.phase = ek_module_phase_active_and_midframe;

        const auto fb_size_cache = platform::window_get_framebuffer_size_cache();

        if (g_module_state.resolution_cache != fb_size_cache) {
            bgfx::reset(static_cast<uint32_t>(fb_size_cache.x), static_cast<uint32_t>(fb_size_cache.y), BGFX_RESET_VSYNC);
            g_module_state.resolution_cache = fb_size_cache;
        }

        const auto context = zcl::arena_push_item<t_frame_context>(context_arena);
        context->basis = basis;

        return context;
    }

    static void frame_flush(t_frame_context *const context) {
        ZCL_ASSERT(g_module_state.phase == ek_module_phase_active_and_midframe);
        ZCL_ASSERT(context->pass_active);

        if (context->batch_state.vert_cnt == 0) {
            return;
        }

        if (context->frame_vert_cnt + context->batch_state.vert_cnt > g_frame_vert_limit) {
            ZCL_FATAL();
        }

        const auto verts = zcl::array_slice(array_to_nonstatic(&context->batch_state.verts), 0, context->batch_state.vert_cnt);
        const auto verts_bgfx_mem = bgfx::copy(verts.raw, static_cast<uint32_t>(zcl::array_get_size_in_bytes(verts)));
        bgfx::update(context->basis->vert_buf_bgfx_hdl, static_cast<uint32_t>(context->frame_vert_cnt), verts_bgfx_mem);

        frame_set_uniform_sampler(context, context->basis->sampler_uniform, context->batch_state.texture ? context->batch_state.texture : context->basis->px_texture);

        bgfx::setVertexBuffer(0, context->basis->vert_buf_bgfx_hdl, static_cast<uint32_t>(context->frame_vert_cnt), static_cast<uint32_t>(context->batch_state.vert_cnt));

        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);

        const t_resource *const prog = context->batch_state.shader_prog ? context->batch_state.shader_prog : context->basis->shader_prog_default;
        bgfx::submit(static_cast<bgfx::ViewId>(context->pass_index), prog->type_data.shader_prog.bgfx_hdl);

        context->frame_vert_cnt += context->batch_state.vert_cnt;

        zcl::zero_clear_item(&context->batch_state);
    }

    void frame_end(t_frame_context *const context) {
        ZCL_ASSERT(g_module_state.phase == ek_module_phase_active_and_midframe);
        ZCL_ASSERT(!context->pass_active);

        bgfx::frame();

        g_module_state.phase = ek_module_phase_active_but_not_midframe;
    }

    static void bgfx_view_configure(const bgfx::ViewId view_id, const zcl::t_v2_i size, const zcl::t_mat4x4 &view_mat, const zcl::t_b8 clear, const zcl::t_color_rgba32f clear_col, const bgfx::FrameBufferHandle fb_hdl) {
        ZCL_ASSERT(g_module_state.phase == ek_module_phase_active_and_midframe);
        ZCL_ASSERT(view_id >= 0 && view_id < BGFX_CONFIG_MAX_VIEWS);
        ZCL_ASSERT(size.x > 0 && size.y > 0);
        ZCL_ASSERT(!clear || zcl::ColorCheckNormalized(clear_col));

        const auto bgfx_view_id = static_cast<bgfx::ViewId>(view_id);

        bgfx::setViewMode(bgfx_view_id, bgfx::ViewMode::Sequential);

        bgfx::setViewRect(bgfx_view_id, 0, 0, static_cast<uint16_t>(size.x), static_cast<uint16_t>(size.y));

        auto proj_mat = zcl::MatrixCreateIdentity();
        proj_mat.elems[0][0] = 1.0f / (static_cast<zcl::t_f32>(size.x) / 2.0f);
        proj_mat.elems[1][1] = -1.0f / (static_cast<zcl::t_f32>(size.y) / 2.0f);
        proj_mat.elems[3][0] = -1.0f;
        proj_mat.elems[3][1] = 1.0f;

        bgfx::setViewTransform(bgfx_view_id, &view_mat, &proj_mat);

        if (clear) {
            bgfx::setViewClear(bgfx_view_id, BGFX_CLEAR_COLOR, zcl::ColorRGBA8ToHex(zcl::ColorRGBA32FToRGBA8(clear_col)));
        }

        bgfx::setViewFrameBuffer(bgfx_view_id, fb_hdl);

        bgfx::touch(bgfx_view_id);
    }

    void frame_pass_begin(t_frame_context *const context, const zcl::t_v2_i size, const zcl::t_mat4x4 &view_mat, const zcl::t_b8 clear, const zcl::t_color_rgba32f clear_col) {
        ZCL_ASSERT(!context->pass_active);

        context->pass_active = true;
        ZCL_REQUIRE(context->pass_index < k_frame_pass_limit && "Trying to begin a new frame pass, but the limit has been reached!");

        bgfx_view_configure(static_cast<bgfx::ViewId>(context->pass_index), size, view_mat, clear, clear_col, BGFX_INVALID_HANDLE);
    }

    void frame_pass_begin_offscreen(t_frame_context *const context, const t_resource *const texture_target, const zcl::t_mat4x4 &view_mat, const zcl::t_b8 clear, const zcl::t_color_rgba32f clear_col) {
        ZCL_ASSERT(!context->pass_active);
        ZCL_ASSERT(texture_target->type == ek_resource_type_texture && texture_target->type_data.texture.is_target);

        context->pass_active = true;
        ZCL_REQUIRE(context->pass_index < k_frame_pass_limit && "Trying to begin a new frame pass, but the limit has been reached!");

        bgfx_view_configure(static_cast<bgfx::ViewId>(context->pass_index), texture_target->type_data.texture.size, view_mat, clear, clear_col, texture_target->type_data.texture.target_fb_bgfx_hdl);
    }

    void frame_pass_end(t_frame_context *const context) {
        ZCL_ASSERT(context->pass_active);

        frame_flush(context);

        context->pass_active = false;
        context->pass_index++;
    }

    zcl::t_b8 frame_pass_check_active(const t_frame_context *const context) {
        return context->pass_active;
    }

    zcl::t_i32 frame_pass_get_index(const t_frame_context *const context) {
        return context->pass_index;
    }

    void frame_set_shader_prog(t_frame_context *const context, const t_resource *const prog) {
        ZCL_ASSERT(g_module_state.phase == ek_module_phase_active_and_midframe);
        ZCL_ASSERT(!prog || prog->type == ek_resource_type_shader_prog);

        if (prog != context->batch_state.shader_prog) {
            frame_flush(context);
            context->batch_state.shader_prog = prog;
        }
    }

    const t_resource *frame_get_builtin_shader_prog_default(t_frame_context *const context) {
        ZCL_ASSERT(g_module_state.phase == ek_module_phase_active_and_midframe);
        return context->basis->shader_prog_default;
    }

    const t_resource *frame_get_builtin_shader_prog_blend(t_frame_context *const context) {
        ZCL_ASSERT(g_module_state.phase == ek_module_phase_active_and_midframe);
        return context->basis->shader_prog_blend;
    }

    const t_resource *frame_get_builtin_uniform_blend(t_frame_context *const context) {
        ZCL_ASSERT(g_module_state.phase == ek_module_phase_active_and_midframe);
        return context->basis->blend_uniform;
    }

    struct t_uniform_data {
        t_uniform_type type;

        union {
            struct {
                const t_resource *texture;
            } sampler;

            struct {
                const zcl::t_v4 *ptr;
            } v4;

            struct {
                const zcl::t_mat4x4 *ptr;
            } mat4x4;
        } type_data;
    };

    static void frame_set_uniform(t_frame_context *const context, const t_resource *const uniform, const t_uniform_data &uniform_data) {
        ZCL_ASSERT(g_module_state.phase == ek_module_phase_active_and_midframe);
        ZCL_ASSERT(uniform->type == ek_resource_type_uniform);
        ZCL_ASSERT(uniform->type_data.uniform.type == uniform_data.type);

        const auto uniform_bgfx_hdl = uniform->type_data.uniform.bgfx_hdl;

        switch (uniform->type_data.uniform.type) {
        case ek_uniform_type_sampler: {
            const t_resource *const texture = uniform_data.type_data.sampler.texture;
            ZCL_ASSERT(texture->type == ek_resource_type_texture);

            const auto texture_type_data = &texture->type_data.texture;
            const bgfx::TextureHandle bgfx_texture_hdl = texture_type_data->is_target ? bgfx::getTexture(texture_type_data->target_fb_bgfx_hdl) : texture_type_data->nontarget_texture_bgfx_hdl;

            bgfx::setTexture(0, uniform_bgfx_hdl, bgfx_texture_hdl);

            break;
        }

        case ek_uniform_type_v4: {
            bgfx::setUniform(uniform_bgfx_hdl, uniform_data.type_data.v4.ptr);
            break;
        }

        case ek_uniform_type_mat4x4: {
            bgfx::setUniform(uniform_bgfx_hdl, uniform_data.type_data.mat4x4.ptr);
            break;
        }
        }
    }

    void frame_set_uniform_sampler(t_frame_context *const context, const t_resource *const uniform, const t_resource *const sampler_texture) {
        const t_uniform_data uniform_data = {
            .type = ek_uniform_type_sampler,
            .type_data = {.sampler = {.texture = sampler_texture}},
        };

        frame_set_uniform(context, uniform, uniform_data);
    }

    void frame_set_uniform_v4(t_frame_context *const context, const t_resource *const uniform, const zcl::t_v4 v4) {
        const t_uniform_data uniform_data = {
            .type = ek_uniform_type_v4,
            .type_data = {.v4 = {.ptr = &v4}},
        };

        frame_set_uniform(context, uniform, uniform_data);
    }

    void frame_set_uniform_mat4x4(t_frame_context *const context, const t_resource *const uniform, const zcl::t_mat4x4 &mat4x4) {
        const t_uniform_data uniform_data = {
            .type = ek_uniform_type_mat4x4,
            .type_data = {.mat4x4 = {.ptr = &mat4x4}},
        };

        frame_set_uniform(context, uniform, uniform_data);
    }

    void frame_submit_triangles(t_frame_context *const context, const zcl::t_array_rdonly<t_triangle> triangles, const t_resource *const texture) {
        ZCL_ASSERT(g_module_state.phase == ek_module_phase_active_and_midframe);
        ZCL_ASSERT(context->pass_index != -1 && "A pass must be set before submitting primitives!");
        ZCL_ASSERT(triangles.len > 0);
        ZCL_ASSERT(!texture || texture->type == ek_resource_type_texture);

        const zcl::t_i32 num_verts_to_submit = 3 * triangles.len;

        if (num_verts_to_submit > g_batch_vert_limit) {
            ZCL_FATAL();
        }

#ifdef ZCL_DEBUG
        for (zcl::t_i32 i = 0; i < triangles.len; i++) {
            ZCL_ASSERT(triangle_check_valid(triangles[i]));
        }
#endif

        if (texture != context->batch_state.texture || context->batch_state.vert_cnt + num_verts_to_submit > g_batch_vert_limit) {
            frame_flush(context);
            context->batch_state.texture = texture;
        }

        for (zcl::t_i32 i = 0; i < triangles.len; i++) {
            const zcl::t_i32 offs = context->batch_state.vert_cnt;
            context->batch_state.verts[offs + (3 * i) + 0] = triangles[i].verts[0];
            context->batch_state.verts[offs + (3 * i) + 1] = triangles[i].verts[1];
            context->batch_state.verts[offs + (3 * i) + 2] = triangles[i].verts[2];
        }

        context->batch_state.vert_cnt += num_verts_to_submit;
    }
}
