#include <zgl/zgl_gfx.h>

#include <zgl/zgl_platform.h>
#include <bgfx/bgfx.h>

#define BGFX_CONFIG_MAX_VIEWS k_frame_pass_limit

namespace zgl {
    struct t_gfx_resource_group {
        zcl::t_arena *arena;
        t_gfx_resource *head;
        t_gfx_resource *tail;
    };

    enum t_gfx_resource_type : zcl::t_i32 {
        ek_gfx_resource_type_invalid,
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

        t_gfx_resource *next;
    };

    struct t_gfx {
        t_platform *platform;

        zcl::t_v2_i resolution_cache;

        t_gfx_resource_group *perm_resource_group;

        bgfx::DynamicVertexBufferHandle vert_buf_bgfx_hdl; // @todo: Consider making an internal resource type for this for consistency.

        const t_gfx_resource *shader_prog_default;
        const t_gfx_resource *shader_prog_blend;

        const t_gfx_resource *sampler_uniform;
        const t_gfx_resource *blend_uniform;

        const t_gfx_resource *px_texture;
    };

    extern const zcl::t_u8 g_vert_shader_default_src_raw[];
    extern const zcl::t_i32 g_vert_shader_default_src_len;

    extern const zcl::t_u8 g_frag_shader_default_src_raw[];
    extern const zcl::t_i32 g_frag_shader_default_src_len;

    extern const zcl::t_u8 g_frag_shader_blend_src_raw[];
    extern const zcl::t_i32 g_frag_shader_blend_src_len;

    constexpr zcl::t_i32 k_batch_vert_limit = 1024;
    constexpr zcl::t_i32 k_frame_vert_limit = 8192; // @todo: This should definitely be modifiable if the user wants.

    enum t_module_state : zcl::t_i32 {
        ek_module_state_inactive,
        ek_module_state_active_but_not_midframe,
        ek_module_state_active_and_midframe
    };

    static t_module_state g_module_state;

    struct t_frame_context {
        t_gfx *gfx;

        zcl::t_b8 pass_active;
        zcl::t_i32 pass_index; // Maps directly to BGFX view ID.

        zcl::t_i32 frame_vert_cnt;

        struct {
            zcl::t_static_array<t_vertex, k_batch_vert_limit> verts;
            zcl::t_i32 vert_cnt;

            const t_gfx_resource *shader_prog;
            const t_gfx_resource *texture;
        } batch_state;
    };

    t_gfx *GFXStartup(t_platform *const platform, zcl::t_arena *const arena, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(g_module_state == ek_module_state_inactive);

        g_module_state = ek_module_state_active_but_not_midframe;

        const auto gfx = zcl::ArenaPushItem<t_gfx>(arena);

        gfx->platform = platform;

        //
        // BGFX Setup
        //
        bgfx::Init bgfx_init = {};

        bgfx_init.type = bgfx::RendererType::Count;

        bgfx_init.resolution.reset = BGFX_RESET_VSYNC;

        const auto fb_size_cache = WindowGetFramebufferSizeCache(platform);

        bgfx_init.resolution.width = static_cast<uint32_t>(fb_size_cache.x);
        bgfx_init.resolution.height = static_cast<uint32_t>(fb_size_cache.y);

        gfx->resolution_cache = fb_size_cache;

        bgfx_init.platformData.nwh = WindowGetNativeHandle(platform);
        bgfx_init.platformData.ndt = DisplayGetNativeHandle(platform);
        bgfx_init.platformData.type = bgfx::NativeWindowHandleType::Default;

        if (!bgfx::init(bgfx_init)) {
            ZCL_FATAL();
        }

        gfx->perm_resource_group = GFXResourceGroupCreate(gfx, arena);

        //
        // Frame Basis Setup
        //
        {
            bgfx::VertexLayout vert_layout;
            vert_layout.begin().add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float).add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float).add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float).end();

            gfx->vert_buf_bgfx_hdl = bgfx::createDynamicVertexBuffer(static_cast<uint32_t>(k_frame_vert_limit), vert_layout);

            if (!bgfx::isValid(gfx->vert_buf_bgfx_hdl)) {
                ZCL_FATAL();
            }
        }

        // @todo: This half-complete GFX strangeness can be fixed by bringing back the frame basis object and returning that here. Only the game loop needs it.

        gfx->shader_prog_default = ShaderProgCreate({g_vert_shader_default_src_raw, g_vert_shader_default_src_len}, {g_frag_shader_default_src_raw, g_frag_shader_default_src_len}, gfx->perm_resource_group);
        gfx->shader_prog_blend = ShaderProgCreate({g_vert_shader_default_src_raw, g_vert_shader_default_src_len}, {g_frag_shader_blend_src_raw, g_frag_shader_blend_src_len}, gfx->perm_resource_group);

        gfx->sampler_uniform = UniformCreate(ZCL_STR_LITERAL("u_texture"), ek_uniform_type_sampler, gfx->perm_resource_group, temp_arena);
        gfx->blend_uniform = UniformCreate(ZCL_STR_LITERAL("u_blend"), ek_uniform_type_v4, gfx->perm_resource_group, temp_arena);

        const zcl::t_static_array<zcl::t_u8, 4> batch_px_texture_rgba = {{255, 255, 255, 255}};
        gfx->px_texture = TextureCreate({{1, 1}, zcl::ArrayToNonstatic(&batch_px_texture_rgba)}, gfx->perm_resource_group);

        return gfx;
    }

    void GFXShutdown(t_gfx *const gfx) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_but_not_midframe);

        bgfx::destroy(gfx->vert_buf_bgfx_hdl);

        GFXResourceGroupDestroy(gfx->perm_resource_group);

        bgfx::shutdown();

        g_module_state = {};
    }

    // @temp: Remove once fonts are reworked.
    zcl::t_arena *GFXResourceGroupGetArena(t_gfx_resource_group *const group) {
        return group->arena;
    }

    t_gfx_resource_group *GFXResourceGroupCreate(t_gfx *const gfx, zcl::t_arena *const arena) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_but_not_midframe);

        const auto result = zcl::ArenaPushItem<t_gfx_resource_group>(arena);
        result->arena = arena;

        return result;
    }

    void GFXResourceGroupDestroy(t_gfx_resource_group *const group) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_but_not_midframe);

        t_gfx_resource *resource = group->head;

        while (resource) {
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

            t_gfx_resource *const resource_next = resource->next;
            *resource = {};
            resource = resource_next;
        }

        *group = {};
    }

    static t_gfx_resource *GFXResourceGroupAdd(t_gfx_resource_group *const group, const t_gfx_resource_type type) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_but_not_midframe);

        const auto resource = zcl::ArenaPushItem<t_gfx_resource>(group->arena);

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

    t_gfx_resource *TextureCreate(const zcl::t_texture_data_rdonly texture_data, t_gfx_resource_group *const group) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_but_not_midframe);

        const uint64_t flags = BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
        const auto texture_bgfx_hdl = bgfx::createTexture2D(static_cast<uint16_t>(texture_data.size_in_pxs.x), static_cast<uint16_t>(texture_data.size_in_pxs.y), false, 1, bgfx::TextureFormat::RGBA8, flags, bgfx::copy(texture_data.rgba_px_data.raw, static_cast<uint32_t>(zcl::ArrayGetSizeInBytes(texture_data.rgba_px_data))));

        if (!bgfx::isValid(texture_bgfx_hdl)) {
            ZCL_FATAL();
        }

        const auto resource = GFXResourceGroupAdd(group, ek_gfx_resource_type_texture);
        resource->type_data.texture.nontarget_texture_bgfx_hdl = texture_bgfx_hdl;
        resource->type_data.texture.size = texture_data.size_in_pxs;
        return resource;
    }

    t_gfx_resource *TextureCreateFromExternal(const zcl::t_str_rdonly file_path, t_gfx_resource_group *const group, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_but_not_midframe);

        zcl::t_texture_data_mut texture_data;

        if (!zcl::TextureLoadFromExternal(file_path, temp_arena, temp_arena, &texture_data)) {
            ZCL_FATAL();
        }

        return TextureCreate(texture_data, group);
    }

    t_gfx_resource *TextureCreateFromPacked(const zcl::t_str_rdonly file_path, t_gfx_resource_group *const group, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_but_not_midframe);

        zcl::t_file_stream file_stream;

        if (!zcl::FileOpen(file_path, zcl::t_file_access_mode::ek_file_access_mode_read, temp_arena, &file_stream)) {
            ZCL_FATAL();
        }

        zcl::t_texture_data_mut texture_data;

        if (!zcl::DeserializeTexture(file_stream, temp_arena, &texture_data)) {
            ZCL_FATAL();
        }

        zcl::FileClose(&file_stream);

        return TextureCreate(texture_data, group);
    }

    static bgfx::FrameBufferHandle BGFXCreateFramebuffer(const zcl::t_v2_i size) {
        return bgfx::createFrameBuffer(static_cast<uint16_t>(size.x), static_cast<uint16_t>(size.y), bgfx::TextureFormat::RGBA8, BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP);
    }

    t_gfx_resource *TextureCreateTarget(t_gfx *const gfx, const zcl::t_v2_i size, t_gfx_resource_group *const group) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_but_not_midframe);
        ZCL_ASSERT(size.x > 0 && size.y > 0);

        const bgfx::FrameBufferHandle fb_bgfx_hdl = BGFXCreateFramebuffer(size);

        if (!bgfx::isValid(fb_bgfx_hdl)) {
            ZCL_FATAL();
        }

        const auto resource = GFXResourceGroupAdd(group, ek_gfx_resource_type_texture);
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

    t_gfx_resource *ShaderProgCreate(const zcl::t_array_rdonly<zcl::t_u8> vert_shader_compiled_bin, const zcl::t_array_rdonly<zcl::t_u8> frag_shader_compiled_bin, t_gfx_resource_group *const group) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_but_not_midframe);

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

        const auto resource = GFXResourceGroupAdd(group, ek_gfx_resource_type_shader_prog);
        resource->type_data.shader_prog.bgfx_hdl = prog_bgfx_hdl;
        return resource;
    }

    t_gfx_resource *ShaderProgCreateFromPacked(const zcl::t_str_rdonly vert_shader_file_path, const zcl::t_str_rdonly frag_shader_file_path, t_gfx_resource_group *const group, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_but_not_midframe);

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

        return ShaderProgCreate(vert_shader_compiled_bin, frag_shader_compiled_bin, group);
    }

    t_gfx_resource *UniformCreate(const zcl::t_str_rdonly name, const t_uniform_type type, t_gfx_resource_group *const group, zcl::t_arena *const temp_arena) {
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

        const auto resource = GFXResourceGroupAdd(group, ek_gfx_resource_type_uniform);
        resource->type_data.uniform.bgfx_hdl = bgfx_hdl;
        resource->type_data.uniform.type = type;
        return resource;
    }

    t_uniform_type UniformGetType(const t_gfx_resource *const uniform) {
        ZCL_ASSERT(g_module_state != ek_module_state_inactive);
        ZCL_ASSERT(uniform->type == ek_gfx_resource_type_uniform);

        return uniform->type_data.uniform.type;
    }

    t_frame_context *FrameBegin(t_gfx *const gfx, zcl::t_arena *const context_arena) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_but_not_midframe);

        g_module_state = ek_module_state_active_and_midframe;

        const auto fb_size_cache = WindowGetFramebufferSizeCache(gfx->platform);

        if (gfx->resolution_cache != fb_size_cache) {
            bgfx::reset(static_cast<uint32_t>(fb_size_cache.x), static_cast<uint32_t>(fb_size_cache.y), BGFX_RESET_VSYNC);
            gfx->resolution_cache = fb_size_cache;
        }

        const auto context = zcl::ArenaPushItem<t_frame_context>(context_arena);
        context->gfx = gfx;

        return context;
    }

    static void FrameFlush(t_frame_context *const context) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_and_midframe);
        ZCL_ASSERT(context->pass_active);

        if (context->batch_state.vert_cnt == 0) {
            return;
        }

        if (context->frame_vert_cnt + context->batch_state.vert_cnt > k_frame_vert_limit) {
            ZCL_FATAL();
        }

        const auto verts = zcl::ArraySlice(ArrayToNonstatic(&context->batch_state.verts), 0, context->batch_state.vert_cnt);
        const auto verts_bgfx_mem = bgfx::copy(verts.raw, static_cast<uint32_t>(zcl::ArrayGetSizeInBytes(verts)));
        bgfx::update(context->gfx->vert_buf_bgfx_hdl, static_cast<uint32_t>(context->frame_vert_cnt), verts_bgfx_mem);

        FrameSetUniformSampler(context, context->gfx->sampler_uniform, context->batch_state.texture ? context->batch_state.texture : context->gfx->px_texture);

        bgfx::setVertexBuffer(0, context->gfx->vert_buf_bgfx_hdl, static_cast<uint32_t>(context->frame_vert_cnt), static_cast<uint32_t>(context->batch_state.vert_cnt));

        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);

        const t_gfx_resource *const prog = context->batch_state.shader_prog ? context->batch_state.shader_prog : context->gfx->shader_prog_default;
        bgfx::submit(static_cast<bgfx::ViewId>(context->pass_index), prog->type_data.shader_prog.bgfx_hdl);

        context->frame_vert_cnt += context->batch_state.vert_cnt;

        zcl::ZeroClearItem(&context->batch_state);
    }

    void FrameEnd(t_frame_context *const context) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_and_midframe);
        ZCL_ASSERT(!context->pass_active);

        bgfx::frame();

        g_module_state = ek_module_state_active_but_not_midframe;
    }

    static void BGFXViewConfigure(const bgfx::ViewId view_id, const zcl::t_v2_i size, const zcl::t_mat4x4 &view_mat, const zcl::t_b8 clear, const zcl::t_color_rgba32f clear_col, const bgfx::FrameBufferHandle fb_hdl) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_and_midframe);
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

    void FramePassBegin(t_frame_context *const context, const zcl::t_v2_i size, const zcl::t_mat4x4 &view_mat, const zcl::t_b8 clear, const zcl::t_color_rgba32f clear_col) {
        ZCL_ASSERT(!context->pass_active);

        context->pass_active = true;
        ZCL_REQUIRE(context->pass_index < k_frame_pass_limit && "Trying to begin a new frame pass, but the limit has been reached!");

        BGFXViewConfigure(static_cast<bgfx::ViewId>(context->pass_index), size, view_mat, clear, clear_col, BGFX_INVALID_HANDLE);
    }

    void FramePassBeginOffscreen(t_frame_context *const context, const t_gfx_resource *const texture_target, const zcl::t_mat4x4 &view_mat, const zcl::t_b8 clear, const zcl::t_color_rgba32f clear_col) {
        ZCL_ASSERT(!context->pass_active);
        ZCL_ASSERT(texture_target->type == ek_gfx_resource_type_texture && texture_target->type_data.texture.is_target);

        context->pass_active = true;
        ZCL_REQUIRE(context->pass_index < k_frame_pass_limit && "Trying to begin a new frame pass, but the limit has been reached!");

        BGFXViewConfigure(static_cast<bgfx::ViewId>(context->pass_index), texture_target->type_data.texture.size, view_mat, clear, clear_col, texture_target->type_data.texture.target_fb_bgfx_hdl);
    }

    void FramePassEnd(t_frame_context *const context) {
        ZCL_ASSERT(context->pass_active);

        FrameFlush(context);

        context->pass_active = false;
        context->pass_index++;
    }

    zcl::t_b8 FramePassCheckActive(const t_frame_context *const context) {
        return context->pass_active;
    }

    zcl::t_i32 FramePassGetIndex(const t_frame_context *const context) {
        return context->pass_index;
    }

    void FrameSetShaderProg(t_frame_context *const context, const t_gfx_resource *const prog) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_and_midframe);
        ZCL_ASSERT(!prog || prog->type == ek_gfx_resource_type_shader_prog);

        if (prog != context->batch_state.shader_prog) {
            FrameFlush(context);
            context->batch_state.shader_prog = prog;
        }
    }

    const t_gfx_resource *FrameGetBuiltinShaderProgDefault(t_frame_context *const context) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_and_midframe);
        return context->gfx->shader_prog_default;
    }

    const t_gfx_resource *FrameGetBuiltinShaderProgBlend(t_frame_context *const context) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_and_midframe);
        return context->gfx->shader_prog_blend;
    }

    const t_gfx_resource *FrameGetBuiltinUniformBlend(t_frame_context *const context) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_and_midframe);
        return context->gfx->blend_uniform;
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

    static void FrameSetUniform(t_frame_context *const context, const t_gfx_resource *const uniform, const t_uniform_data &uniform_data) {
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

    void FrameSetUniformSampler(t_frame_context *const context, const t_gfx_resource *const uniform, const t_gfx_resource *const sampler_texture) {
        const t_uniform_data uniform_data = {
            .type = ek_uniform_type_sampler,
            .type_data = {.sampler = {.texture = sampler_texture}},
        };

        FrameSetUniform(context, uniform, uniform_data);
    }

    void FrameSetUniformV4(t_frame_context *const context, const t_gfx_resource *const uniform, const zcl::t_v4 v4) {
        const t_uniform_data uniform_data = {
            .type = ek_uniform_type_v4,
            .type_data = {.v4 = {.ptr = &v4}},
        };

        FrameSetUniform(context, uniform, uniform_data);
    }

    void FrameSetUniformMat4x4(t_frame_context *const context, const t_gfx_resource *const uniform, const zcl::t_mat4x4 &mat4x4) {
        const t_uniform_data uniform_data = {
            .type = ek_uniform_type_mat4x4,
            .type_data = {.mat4x4 = {.ptr = &mat4x4}},
        };

        FrameSetUniform(context, uniform, uniform_data);
    }

    void FrameSubmitTriangles(t_frame_context *const context, const zcl::t_array_rdonly<t_triangle> triangles, const t_gfx_resource *const texture) {
        ZCL_ASSERT(g_module_state == ek_module_state_active_and_midframe);
        ZCL_ASSERT(context->pass_index != -1 && "A pass must be set before submitting primitives!");
        ZCL_ASSERT(triangles.len > 0);
        ZCL_ASSERT(!texture || texture->type == ek_gfx_resource_type_texture);

        const zcl::t_i32 num_verts_to_submit = 3 * triangles.len;

        if (num_verts_to_submit > k_batch_vert_limit) {
            ZCL_FATAL();
        }

#ifdef ZCL_DEBUG
        for (zcl::t_i32 i = 0; i < triangles.len; i++) {
            ZCL_ASSERT(TriangleValid(triangles[i]));
        }
#endif

        if (texture != context->batch_state.texture || context->batch_state.vert_cnt + num_verts_to_submit > k_batch_vert_limit) {
            FrameFlush(context);
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
