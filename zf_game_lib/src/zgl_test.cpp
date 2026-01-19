#include <zgl/zgl_test.h>

#include <bgfx/bgfx.h>

#define BGFX_CONFIG_MAX_VIEWS k_frame_pass_limit

namespace zgl {
    enum t_uniform_type {
        ek_uniform_type_sampler,
        ek_uniform_type_v4,
        ek_uniform_type_mat4x4
    };

    enum t_gfx_resource_type : zcl::t_i32 {
        ek_gfx_resource_type_invalid,
        ek_gfx_resource_type_vertex_buf,
        ek_gfx_resource_type_texture,
        ek_gfx_resource_type_shader_prog,
        ek_gfx_resource_type_uniform
    };

    struct t_gfx_resource {
        t_gfx_resource_type type;

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
    };

    enum t_module_state : zcl::t_i32 {
        ek_module_state_inactive,
        ek_module_state_active_but_not_midframe,
        ek_module_state_active_and_midframe
    };

    static t_module_state g_module_state;

    void GFXStartup(const zcl::t_v2_i frame_size, void *const window_native_hdl, void *const display_native_hdl, zcl::t_arena *const arena, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(g_module_state == ek_module_state_inactive);

        g_module_state = ek_module_state_active_but_not_midframe;

        bgfx::Init bgfx_init = {};

        bgfx_init.type = bgfx::RendererType::Count;

        bgfx_init.resolution.reset = BGFX_RESET_VSYNC;

        bgfx_init.resolution.width = static_cast<zcl::t_u32>(frame_size.x);
        bgfx_init.resolution.height = static_cast<zcl::t_u32>(frame_size.y);

        bgfx_init.platformData.nwh = window_native_hdl;
        bgfx_init.platformData.ndt = display_native_hdl;
        bgfx_init.platformData.type = bgfx::NativeWindowHandleType::Default;

        if (!bgfx::init(bgfx_init)) {
            ZCL_FATAL();
        }
    }

    void GFXShutdown() {
        ZCL_ASSERT(g_module_state == ek_module_state_active_but_not_midframe);

        bgfx::shutdown();

        g_module_state = ek_module_state_inactive;
    }

    void GFXResourceDestroy(t_gfx_resource *const resource) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_but_not_midframe);

        switch (resource->type) {
        case ek_gfx_resource_type_texture:
            if (resource->type_data.texture.is_target) {
                bgfx::destroy(resource->type_data.texture.target_fb_bgfx_hdl);
            } else {
                bgfx::destroy(resource->type_data.texture.nontarget_texture_bgfx_hdl);
            }

            break;

        case ek_gfx_resource_type_shader_prog:
            bgfx::destroy(resource->type_data.shader_prog.bgfx_hdl);
            break;

        case ek_gfx_resource_type_uniform:
            bgfx::destroy(resource->type_data.uniform.bgfx_hdl);
            break;

        default:
            ZCL_UNREACHABLE();
        }

        *resource = {};
    }

    t_gfx_resource *TextureCreate(const zcl::t_texture_data_rdonly texture_data, zcl::t_arena *const arena) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_but_not_midframe);

        const auto flags = BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
        const auto texture_bgfx_hdl = bgfx::createTexture2D(static_cast<zcl::t_u16>(texture_data.size_in_pxs.x), static_cast<zcl::t_u16>(texture_data.size_in_pxs.y), false, 1, bgfx::TextureFormat::RGBA8, flags, bgfx::copy(texture_data.rgba_px_data.raw, static_cast<zcl::t_u32>(zcl::ArrayGetSizeInBytes(texture_data.rgba_px_data))));

        if (!bgfx::isValid(texture_bgfx_hdl)) {
            ZCL_FATAL();
        }

        const auto resource = zcl::ArenaPushItem<t_gfx_resource>(arena);
        resource->type = ek_gfx_resource_type_texture;
        resource->type_data.texture.nontarget_texture_bgfx_hdl = texture_bgfx_hdl;
        resource->type_data.texture.size = texture_data.size_in_pxs;

        return resource;
    }

    static bgfx::FrameBufferHandle BGFXCreateFramebuffer(const zcl::t_v2_i size) {
        return bgfx::createFrameBuffer(static_cast<zcl::t_u16>(size.x), static_cast<zcl::t_u16>(size.y), bgfx::TextureFormat::RGBA8, BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP);
    }

    t_gfx_resource *TextureCreateTarget(const zcl::t_v2_i size, zcl::t_arena *const arena) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_but_not_midframe);
        ZCL_ASSERT(size.x > 0 && size.y > 0);

        const bgfx::FrameBufferHandle fb_bgfx_hdl = BGFXCreateFramebuffer(size);

        if (!bgfx::isValid(fb_bgfx_hdl)) {
            ZCL_FATAL();
        }

        const auto resource = zcl::ArenaPushItem<t_gfx_resource>(arena);
        resource->type = ek_gfx_resource_type_texture;
        resource->type_data.texture.is_target = true;
        resource->type_data.texture.target_fb_bgfx_hdl = fb_bgfx_hdl;
        resource->type_data.texture.size = size;

        return resource;
    }

    void TextureResizeTarget(t_gfx_resource *const texture, const zcl::t_v2_i size) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_but_not_midframe);
        ZCL_ASSERT(texture->type == ek_gfx_resource_type_texture && texture->type_data.texture.is_target);
        ZCL_ASSERT(size.x > 0 && size.y > 0);
        ZCL_ASSERT(size != texture->type_data.texture.size);

        const bgfx::FrameBufferHandle fb_bgfx_hdl = BGFXCreateFramebuffer(size);

        if (!bgfx::isValid(fb_bgfx_hdl)) {
            ZCL_FATAL();
        }

        texture->type_data.texture.target_fb_bgfx_hdl = fb_bgfx_hdl;
        texture->type_data.texture.size = size;
    }

    zcl::t_v2_i TextureGetSize(const t_gfx_resource *const texture) {
        ZCL_ASSERT(g_module_state != ek_module_state_inactive);
        ZCL_ASSERT(texture->type == ek_gfx_resource_type_texture);

        return texture->type_data.texture.size;
    }

    t_gfx_resource *ShaderProgCreate(const zcl::t_array_rdonly<zcl::t_u8> vert_shader_compiled_bin, const zcl::t_array_rdonly<zcl::t_u8> frag_shader_compiled_bin, zcl::t_arena *const arena) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_but_not_midframe);

        const bgfx::Memory *const vert_shader_bgfx_mem = bgfx::copy(vert_shader_compiled_bin.raw, static_cast<zcl::t_u32>(vert_shader_compiled_bin.len));
        const bgfx::ShaderHandle vert_shader_bgfx_hdl = bgfx::createShader(vert_shader_bgfx_mem);

        if (!bgfx::isValid(vert_shader_bgfx_hdl)) {
            ZCL_FATAL();
        }

        const bgfx::Memory *const frag_shader_bgfx_mem = bgfx::copy(frag_shader_compiled_bin.raw, static_cast<zcl::t_u32>(frag_shader_compiled_bin.len));
        const bgfx::ShaderHandle frag_shader_bgfx_hdl = bgfx::createShader(frag_shader_bgfx_mem);

        if (!bgfx::isValid(frag_shader_bgfx_hdl)) {
            ZCL_FATAL();
        }

        const bgfx::ProgramHandle prog_bgfx_hdl = bgfx::createProgram(vert_shader_bgfx_hdl, frag_shader_bgfx_hdl, true);

        if (!bgfx::isValid(prog_bgfx_hdl)) {
            ZCL_FATAL();
        }

        const auto resource = zcl::ArenaPushItem<t_gfx_resource>(arena);
        resource->type = ek_gfx_resource_type_shader_prog;
        resource->type_data.shader_prog.bgfx_hdl = prog_bgfx_hdl;

        return resource;
    }

    t_gfx_resource *UniformCreate(const zcl::t_str_rdonly name, const t_uniform_type type, zcl::t_arena *const arena, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_but_not_midframe);

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

        const auto resource = zcl::ArenaPushItem<t_gfx_resource>(arena);
        resource->type = ek_gfx_resource_type_uniform;
        resource->type_data.uniform.bgfx_hdl = bgfx_hdl;
        resource->type_data.uniform.type = type;

        return resource;
    }

    t_uniform_type UniformGetType(const t_gfx_resource *const uniform) {
        ZCL_ASSERT(g_module_state != ek_module_state_inactive);
        ZCL_ASSERT(uniform->type == ek_gfx_resource_type_uniform);

        return uniform->type_data.uniform.type;
    }

#if 0
    t_frame_context internal::FrameBegin(t_gfx *const gfx, t_frame_basis *const basis, const t_platform_ticket_rdonly platform_ticket, zcl::t_arena *const context_arena) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_but_not_midframe);

        g_module_state = ek_module_state_active_and_midframe;

        const auto fb_size_cache = WindowGetFramebufferSizeCache(platform_ticket);

        if (basis->size != fb_size_cache) {
            bgfx::reset(static_cast<zcl::t_u32>(fb_size_cache.x), static_cast<zcl::t_u32>(fb_size_cache.y), BGFX_RESET_VSYNC);
            basis->size = fb_size_cache;
        }

        return {
            .gfx = gfx,
            .basis = basis,
            .state = zcl::ArenaPushItem<t_frame_state>(context_arena),
        };
    }

    static void FrameFlush(const t_frame_context context) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_and_midframe);
        ZCL_ASSERT(context.state->pass_active);

        if (context.state->batch_state.vert_cnt == 0) {
            return;
        }

        if (context.state->frame_vert_cnt + context.state->batch_state.vert_cnt > k_frame_vert_limit) {
            ZCL_FATAL();
        }

        const auto verts = zcl::ArraySlice(ArrayToNonstatic(&context.state->batch_state.verts), 0, context.state->batch_state.vert_cnt);
        const auto verts_bgfx_mem = bgfx::copy(verts.raw, static_cast<zcl::t_u32>(zcl::ArrayGetSizeInBytes(verts)));
        bgfx::update(context.basis->vert_buf_bgfx_hdl, static_cast<zcl::t_u32>(context.state->frame_vert_cnt), verts_bgfx_mem);

        FrameSetUniformSampler(context, context.basis->sampler_uniform, context.state->batch_state.texture ? context.state->batch_state.texture : context.basis->px_texture);

        bgfx::setVertexBuffer(0, context.basis->vert_buf_bgfx_hdl, static_cast<zcl::t_u32>(context.state->frame_vert_cnt), static_cast<zcl::t_u32>(context.state->batch_state.vert_cnt));

        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);

        const t_gfx_resource *const prog = context.state->batch_state.shader_prog ? context.state->batch_state.shader_prog : context.basis->shader_prog_default;
        bgfx::submit(static_cast<bgfx::ViewId>(context.state->pass_index), prog->type_data.shader_prog.bgfx_hdl);

        context.state->frame_vert_cnt += context.state->batch_state.vert_cnt;

        zcl::ZeroClearItem(&context.state->batch_state);
    }

    void internal::FrameEnd(const t_frame_context context) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_and_midframe);
        ZCL_ASSERT(!context.state->pass_active);

        bgfx::frame();

        g_module_state = ek_module_state_active_but_not_midframe;
    }

    zcl::t_v2_i FrameGetSize(const t_frame_context context) {
        return context.basis->size;
    }

    static void BGFXViewConfigure(const bgfx::ViewId view_id, const zcl::t_v2_i size, const zcl::t_mat4x4 &view_mat, const zcl::t_b8 clear, const zcl::t_color_rgba32f clear_col, const bgfx::FrameBufferHandle fb_hdl) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_and_midframe);
        ZCL_ASSERT(view_id >= 0 && view_id < BGFX_CONFIG_MAX_VIEWS);
        ZCL_ASSERT(size.x > 0 && size.y > 0);
        ZCL_ASSERT(!clear || zcl::ColorCheckNormalized(clear_col));

        const auto bgfx_view_id = static_cast<bgfx::ViewId>(view_id);

        bgfx::setViewMode(bgfx_view_id, bgfx::ViewMode::Sequential);

        bgfx::setViewRect(bgfx_view_id, 0, 0, static_cast<zcl::t_u16>(size.x), static_cast<zcl::t_u16>(size.y));

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

    void FramePassBegin(const t_frame_context context, const zcl::t_v2_i size, const zcl::t_mat4x4 &view_mat, const zcl::t_b8 clear, const zcl::t_color_rgba32f clear_col) {
        ZCL_ASSERT(!context.state->pass_active);

        context.state->pass_active = true;
        ZCL_REQUIRE(context.state->pass_index < k_frame_pass_limit && "Trying to begin a new frame pass, but the limit has been reached!");

        BGFXViewConfigure(static_cast<bgfx::ViewId>(context.state->pass_index), size, view_mat, clear, clear_col, BGFX_INVALID_HANDLE);
    }

    void FramePassBeginOffscreen(const t_frame_context context, const t_gfx_resource *const texture_target, const zcl::t_mat4x4 &view_mat, const zcl::t_b8 clear, const zcl::t_color_rgba32f clear_col) {
        ZCL_ASSERT(!context.state->pass_active);
        ZCL_ASSERT(texture_target->type == ek_gfx_resource_type_texture && texture_target->type_data.texture.is_target);

        context.state->pass_active = true;
        ZCL_REQUIRE(context.state->pass_index < k_frame_pass_limit && "Trying to begin a new frame pass, but the limit has been reached!");

        BGFXViewConfigure(static_cast<bgfx::ViewId>(context.state->pass_index), texture_target->type_data.texture.size, view_mat, clear, clear_col, texture_target->type_data.texture.target_fb_bgfx_hdl);
    }

    void FramePassEnd(const t_frame_context context) {
        ZCL_ASSERT(context.state->pass_active);

        FrameFlush(context);

        context.state->pass_active = false;
        context.state->pass_index++;
    }

    zcl::t_b8 FramePassCheckActive(const t_frame_context context) {
        return context.state->pass_active;
    }

    zcl::t_i32 FramePassGetIndex(const t_frame_context context) {
        return context.state->pass_index;
    }

    void FrameSetShaderProg(const t_frame_context context, const t_gfx_resource *const prog) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_and_midframe);
        ZCL_ASSERT(!prog || prog->type == ek_gfx_resource_type_shader_prog);

        if (prog != context.state->batch_state.shader_prog) {
            FrameFlush(context);
            context.state->batch_state.shader_prog = prog;
        }
    }

    const t_gfx_resource *FrameGetBuiltinShaderProgDefault(const t_frame_context context) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_and_midframe);
        return context.basis->shader_prog_default;
    }

    const t_gfx_resource *FrameGetBuiltinShaderProgBlend(const t_frame_context context) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_and_midframe);
        return context.basis->shader_prog_blend;
    }

    const t_gfx_resource *FrameGetBuiltinUniformBlend(const t_frame_context context) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_and_midframe);
        return context.basis->blend_uniform;
    }

    struct t_uniform_data {
        t_uniform_type type;

        union {
            struct {
                const t_gfx_resource *texture;
            } sampler;

            struct {
                const zcl::t_v4 *ptr;
            } v4;

            struct {
                const zcl::t_mat4x4 *ptr;
            } mat4x4;
        } type_data;
    };

    static void FrameSetUniform(const t_frame_context context, const t_gfx_resource *const uniform, const t_uniform_data &uniform_data) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_and_midframe);
        ZCL_ASSERT(uniform->type == ek_gfx_resource_type_uniform);
        ZCL_ASSERT(uniform->type_data.uniform.type == uniform_data.type);

        const auto uniform_bgfx_hdl = uniform->type_data.uniform.bgfx_hdl;

        switch (uniform->type_data.uniform.type) {
        case ek_uniform_type_sampler: {
            const t_gfx_resource *const texture = uniform_data.type_data.sampler.texture;
            ZCL_ASSERT(texture->type == ek_gfx_resource_type_texture);

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

    void FrameSetUniformSampler(const t_frame_context context, const t_gfx_resource *const uniform, const t_gfx_resource *const sampler_texture) {
        const t_uniform_data uniform_data = {
            .type = ek_uniform_type_sampler,
            .type_data = {.sampler = {.texture = sampler_texture}},
        };

        FrameSetUniform(context, uniform, uniform_data);
    }

    void FrameSetUniformV4(const t_frame_context context, const t_gfx_resource *const uniform, const zcl::t_v4 v4) {
        const t_uniform_data uniform_data = {
            .type = ek_uniform_type_v4,
            .type_data = {.v4 = {.ptr = &v4}},
        };

        FrameSetUniform(context, uniform, uniform_data);
    }

    void FrameSetUniformMat4x4(const t_frame_context context, const t_gfx_resource *const uniform, const zcl::t_mat4x4 &mat4x4) {
        const t_uniform_data uniform_data = {
            .type = ek_uniform_type_mat4x4,
            .type_data = {.mat4x4 = {.ptr = &mat4x4}},
        };

        FrameSetUniform(context, uniform, uniform_data);
    }

    void FrameSubmitTriangles(const t_frame_context context, const zcl::t_array_rdonly<t_frame_triangle> triangles, const t_gfx_resource *const texture) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_and_midframe);
        ZCL_ASSERT(context.state->pass_index != -1 && "A pass must be set before submitting primitives!");
        ZCL_ASSERT(triangles.len > 0);
        ZCL_ASSERT(!texture || texture->type == ek_gfx_resource_type_texture);

        const zcl::t_i32 num_verts_to_submit = 3 * triangles.len;

        if (num_verts_to_submit > k_batch_vert_limit) {
            ZCL_FATAL();
        }

    #ifdef ZCL_DEBUG
        for (zcl::t_i32 i = 0; i < triangles.len; i++) {
            ZCL_ASSERT(FrameTriangleCheckValid(triangles[i]));
        }
    #endif

        if (texture != context.state->batch_state.texture || context.state->batch_state.vert_cnt + num_verts_to_submit > k_batch_vert_limit) {
            FrameFlush(context);
            context.state->batch_state.texture = texture;
        }

        for (zcl::t_i32 i = 0; i < triangles.len; i++) {
            const zcl::t_i32 offs = context.state->batch_state.vert_cnt;
            context.state->batch_state.verts[offs + (3 * i) + 0] = triangles[i].verts[0];
            context.state->batch_state.verts[offs + (3 * i) + 1] = triangles[i].verts[1];
            context.state->batch_state.verts[offs + (3 * i) + 2] = triangles[i].verts[2];
        }

        context.state->batch_state.vert_cnt += num_verts_to_submit;
    }
#endif
}
