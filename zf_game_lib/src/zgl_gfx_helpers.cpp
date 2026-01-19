#include <zgl/zgl_gfx_private.h>

namespace zgl {
    struct t_gfx_resource_group {
        zcl::t_arena *arena;
        t_gfx_resource *head;
        t_gfx_resource *tail;
    };

    struct t_frame_basis {
        t_gfx_resource_group *perm_resource_group;

        const t_gfx_resource *vert_buf;

        const t_gfx_resource *shader_prog_default;
        const t_gfx_resource *shader_prog_blend;

        const t_gfx_resource *sampler_uniform;
        const t_gfx_resource *blend_uniform;

        const t_gfx_resource *px_texture;

        zcl::t_v2_i size;
    };

    constexpr zcl::t_i32 k_batch_vert_limit = 1024;
    constexpr zcl::t_i32 k_frame_vert_limit = 8192; // @todo: This should definitely be modifiable if the user wants.

    extern const zcl::t_u8 g_vert_shader_default_src_raw[];
    extern const zcl::t_i32 g_vert_shader_default_src_len;

    extern const zcl::t_u8 g_frag_shader_default_src_raw[];
    extern const zcl::t_i32 g_frag_shader_default_src_len;

    extern const zcl::t_u8 g_frag_shader_blend_src_raw[];
    extern const zcl::t_i32 g_frag_shader_blend_src_len;

    enum t_phase : zcl::t_i32 {
        ek_phase_inactive,
        ek_phase_active_but_not_midframe,
        ek_phase_active_and_midframe
    };

    static t_phase g_phase;

    t_gfx *GFXStartup(const t_platform_ticket_rdonly platform_ticket, zcl::t_arena *const arena, zcl::t_arena *const temp_arena) {
    }

    void GFXShutdown(t_gfx *const gfx) {
    }

    t_gfx_resource *TextureCreateFromExternal(t_gfx *const gfx, const zcl::t_str_rdonly file_path, t_gfx_resource_group *const group, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(g_module_state == ek_phase_active_but_not_midframe);

        zcl::t_texture_data_mut texture_data;

        if (!zcl::TextureLoadFromExternal(file_path, temp_arena, temp_arena, &texture_data)) {
            ZCL_FATAL();
        }

        return TextureCreate(gfx, texture_data, group);
    }

    t_gfx_resource *TextureCreateFromPacked(t_gfx *const gfx, const zcl::t_str_rdonly file_path, t_gfx_resource_group *const group, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(g_module_state == ek_phase_active_but_not_midframe);

        zcl::t_file_stream file_stream;

        if (!zcl::FileOpen(file_path, zcl::t_file_access_mode::ek_file_access_mode_read, temp_arena, &file_stream)) {
            ZCL_FATAL();
        }

        zcl::t_texture_data_mut texture_data;

        if (!zcl::DeserializeTexture(file_stream, temp_arena, &texture_data)) {
            ZCL_FATAL();
        }

        zcl::FileClose(&file_stream);

        return TextureCreate(gfx, texture_data, group);
    }

    t_gfx_resource *ShaderProgCreateFromPacked(t_gfx *const gfx, const zcl::t_str_rdonly vert_shader_file_path, const zcl::t_str_rdonly frag_shader_file_path, t_gfx_resource_group *const group, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(g_module_state == ek_phase_active_but_not_midframe);

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

        return ShaderProgCreate(gfx, vert_shader_compiled_bin, frag_shader_compiled_bin, group);
    }

    void internal::FrameBegin(t_gfx *const gfx, t_frame_basis *const basis, const t_platform_ticket_rdonly platform_ticket, zcl::t_arena *const context_arena) {
        ZCL_ASSERT(g_module_state == ek_phase_active_but_not_midframe);

        g_module_state = ek_phase_active_and_midframe;

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
        ZCL_ASSERT(g_module_state == ek_phase_active_and_midframe);
        ZCL_ASSERT(context.state->pass_active);

        if (context.state->batch_state.vert_cnt == 0) {
            return;
        }

        if (context.state->frame_vert_cnt + context.state->batch_state.vert_cnt > k_frame_vert_limit) {
            ZCL_FATAL();
        }

        const auto verts = zcl::ArraySlice(zcl::ArrayToNonstatic(&context.state->batch_state.verts), 0, context.state->batch_state.vert_cnt);
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
        ZCL_ASSERT(g_module_state == ek_phase_active_and_midframe);
        ZCL_ASSERT(!context.state->pass_active);

        bgfx::frame();

        g_module_state = ek_phase_active_but_not_midframe;
    }

    zcl::t_v2_i FrameGetSize(const t_frame_context context) {
        return context.basis->size;
    }

    static void BGFXViewConfigure(const bgfx::ViewId view_id, const zcl::t_v2_i size, const zcl::t_mat4x4 &view_mat, const zcl::t_b8 clear, const zcl::t_color_rgba32f clear_col, const bgfx::FrameBufferHandle fb_hdl) {
        ZCL_ASSERT(g_module_state == ek_phase_active_and_midframe);
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
        ZCL_ASSERT(g_module_state == ek_phase_active_and_midframe);
        ZCL_ASSERT(!prog || prog->type == ek_gfx_resource_type_shader_prog);

        if (prog != context.state->batch_state.shader_prog) {
            FrameFlush(context);
            context.state->batch_state.shader_prog = prog;
        }
    }

    const t_gfx_resource *FrameGetBuiltinShaderProgDefault(const t_frame_context context) {
        ZCL_ASSERT(g_module_state == ek_phase_active_and_midframe);
        return context.basis->shader_prog_default;
    }

    const t_gfx_resource *FrameGetBuiltinShaderProgBlend(const t_frame_context context) {
        ZCL_ASSERT(g_module_state == ek_phase_active_and_midframe);
        return context.basis->shader_prog_blend;
    }

    const t_gfx_resource *FrameGetBuiltinUniformBlend(const t_frame_context context) {
        ZCL_ASSERT(g_module_state == ek_phase_active_and_midframe);
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
        ZCL_ASSERT(g_module_state == ek_phase_active_and_midframe);
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
        ZCL_ASSERT(g_module_state == ek_phase_active_and_midframe);
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
}

#if 0
    #include <zgl/zgl_gfx.h>

namespace zgl {
    void FrameSubmitRectRotated(const t_frame_context context, const zcl::t_v2 pos, const zcl::t_v2 size, const zcl::t_v2 origin, const zcl::t_f32 rot, const zcl::t_color_rgba32f color_topleft, const zcl::t_color_rgba32f color_topright, const zcl::t_color_rgba32f color_bottomright, const zcl::t_color_rgba32f color_bottomleft) {
        ZCL_ASSERT(OriginCheckValid(origin));

        zcl::t_static_array<zcl::t_v2, 4> quad_pts;
        zcl::t_arena quad_pts_arena = zcl::ArenaCreateWrapping(zcl::ToBytes(&quad_pts));
        const zcl::t_poly_mut quad_poly = zcl::PolyCreateQuadRotated(pos, size, origin, rot, &quad_pts_arena);

        const zcl::t_static_array<t_frame_triangle, 2> triangles = {{
            {
                .verts = {{
                    {.pos = quad_poly.pts[0], .blend = color_topleft, .uv = {}},
                    {.pos = quad_poly.pts[1], .blend = color_topright, .uv = {}},
                    {.pos = quad_poly.pts[3], .blend = color_bottomleft, .uv = {}},
                }},
            },
            {
                .verts = {{
                    {.pos = quad_poly.pts[3], .blend = color_bottomleft, .uv = {}},
                    {.pos = quad_poly.pts[1], .blend = color_topright, .uv = {}},
                    {.pos = quad_poly.pts[2], .blend = color_bottomright, .uv = {}},
                }},
            },
        }};

        FrameSubmitTriangles(context, zcl::ArrayToNonstatic(&triangles));
    }

    void FrameSubmitTexture(const t_frame_context context, const t_gfx_resource *const texture, const zcl::t_v2 pos, const zcl::t_rect_i src_rect, const zcl::t_v2 origin, const zcl::t_f32 rot) {
        const auto texture_size = TextureGetSize(context.gfx, texture);

        zcl::t_rect_i src_rect_to_use;

        if (zcl::RectsCheckEqual(src_rect, {})) {
            src_rect_to_use = {0, 0, texture_size.x, texture_size.y};
        } else {
            ZCL_ASSERT(src_rect.x >= 0 && src_rect.y >= 0 && zcl::RectGetRight(src_rect) <= texture_size.x && zcl::RectGetBottom(src_rect) <= texture_size.y);
            src_rect_to_use = src_rect;
        }

        const zcl::t_rect_f uv_rect = zcl::TextureCalcUVRect(src_rect_to_use, texture_size);

        zcl::t_static_array<zcl::t_v2, 4> quad_pts;
        zcl::t_arena quad_pts_arena = zcl::ArenaCreateWrapping(zcl::ToBytes(&quad_pts));
        const zcl::t_poly_mut quad_poly = zcl::PolyCreateQuadRotated(pos, zcl::V2IToF(zcl::RectGetSize(src_rect_to_use)), origin, rot, &quad_pts_arena);

        const zcl::t_static_array<t_frame_triangle, 2> triangles = {{
            {
                .verts = {{
                    {.pos = quad_poly.pts[0], .blend = zcl::k_color_white, .uv = zcl::RectGetTopLeft(uv_rect)},
                    {.pos = quad_poly.pts[1], .blend = zcl::k_color_white, .uv = zcl::RectGetTopRight(uv_rect)},
                    {.pos = quad_poly.pts[3], .blend = zcl::k_color_white, .uv = zcl::RectGetBottomLeft(uv_rect)},
                }},
            },
            {
                .verts = {{
                    {.pos = quad_poly.pts[3], .blend = zcl::k_color_white, .uv = zcl::RectGetBottomLeft(uv_rect)},
                    {.pos = quad_poly.pts[1], .blend = zcl::k_color_white, .uv = zcl::RectGetTopRight(uv_rect)},
                    {.pos = quad_poly.pts[2], .blend = zcl::k_color_white, .uv = zcl::RectGetBottomRight(uv_rect)},
                }},
            },
        }};

        FrameSubmitTriangles(context, zcl::ArrayToNonstatic(&triangles), texture);
    }

    t_font FontCreateFromExternal(t_gfx *const gfx, const zcl::t_str_rdonly file_path, const zcl::t_i32 height, zcl::t_code_point_bitset *const code_pts, t_gfx_resource_group *const resource_group, zcl::t_arena *const temp_arena) {
        zcl::t_font_arrangement arrangement;
        zcl::t_array_mut<zcl::t_font_atlas_rgba> atlas_rgbas;

        if (!zcl::FontLoadFromExternal(file_path, height, code_pts, GFXResourceGroupGetArena(resource_group), temp_arena, temp_arena, &arrangement, &atlas_rgbas)) {
            ZCL_FATAL();
        }

        const zcl::t_array_mut<t_gfx_resource *> atlases = zcl::ArenaPushArray<t_gfx_resource *>(GFXResourceGroupGetArena(resource_group), atlas_rgbas.len);

        for (zcl::t_i32 i = 0; i < atlas_rgbas.len; i++) {
            atlases[i] = TextureCreate(gfx, {zcl::k_font_atlas_size, atlas_rgbas[i]}, resource_group);
        }

        return {
            .arrangement = arrangement,
            .atlases = atlases,
        };
    }

    t_font FontCreateFromPacked(t_gfx *const gfx, const zcl::t_str_rdonly file_path, t_gfx_resource_group *const resource_group, zcl::t_arena *const temp_arena) {
        zcl::t_font_arrangement arrangement;
        zcl::t_array_mut<zcl::t_font_atlas_rgba> atlas_rgbas;

        {
            zcl::t_file_stream file_stream;

            if (!zcl::FileOpen(file_path, zcl::t_file_access_mode::ek_file_access_mode_read, temp_arena, &file_stream)) {
                ZCL_FATAL();
            }


            if (!zcl::DeserializeFont(file_stream, GFXResourceGroupGetArena(resource_group), temp_arena, temp_arena, &arrangement, &atlas_rgbas)) {
                ZCL_FATAL();
            }

            zcl::FileClose(&file_stream);
        }

        const auto atlases = zcl::ArenaPushArray<t_gfx_resource *>(GFXResourceGroupGetArena(resource_group), atlas_rgbas.len);

        for (zcl::t_i32 i = 0; i < atlas_rgbas.len; i++) {
            atlases[i] = TextureCreate(gfx, {zcl::k_font_atlas_size, atlas_rgbas[i]}, resource_group);
        }

        return {
            .arrangement = arrangement,
            .atlases = atlases,
        };
    }

    static zcl::t_array_mut<zcl::t_v2> CalcStrChrRenderPositions(const zcl::t_str_rdonly str, const zcl::t_font_arrangement &font_arrangement, const zcl::t_v2 pos, const zcl::t_v2 origin, zcl::t_arena *const arena) {
        ZCL_ASSERT(zcl::StrCheckValidUTF8(str));
        ZCL_ASSERT(OriginCheckValid(origin));

        // Calculate some useful string metadata.
        struct t_str_meta {
            zcl::t_i32 len;
            zcl::t_i32 line_cnt;
        };

        const auto str_meta = [str]() {
            t_str_meta meta = {.line_cnt = 1};

            ZCL_STR_WALK (str, step) {
                meta.len++;

                if (step.code_pt == '\n') {
                    meta.line_cnt++;
                }
            }

            return meta;
        }();

        // Reserve memory for the character positions.
        const auto positions = zcl::ArenaPushArray<zcl::t_v2>(arena, str_meta.len);

        // From the line count we can determine the vertical offset to apply.
        const zcl::t_f32 offs_y = static_cast<zcl::t_f32>(-(str_meta.line_cnt * font_arrangement.line_height)) * origin.y;

        // Calculate the position of each character.
        zcl::t_i32 chr_index = 0;
        zcl::t_v2 chr_pos_pen = {}; // The position of the current character.
        zcl::t_i32 line_begin_chr_index = 0;
        zcl::t_i32 line_len = 0;
        zcl::t_code_point code_pt_last;

        const auto apply_offs_x = [&]() {
            if (line_len > 0) {
                const auto line_width = chr_pos_pen.x;

                for (zcl::t_i32 i = line_begin_chr_index; i < chr_index; i++) {
                    positions[i].x -= line_width * origin.x;
                }
            }
        };

        ZCL_STR_WALK (str, step) {
            ZCL_DEFER({
                chr_index++;
                code_pt_last = step.code_pt;
            });

            if (line_len == 0) {
                line_begin_chr_index = chr_index;
            }

            if (step.code_pt == '\n') {
                apply_offs_x();

                chr_pos_pen.x = 0.0f;
                chr_pos_pen.y += static_cast<zcl::t_f32>(font_arrangement.line_height);

                line_len = 0;

                continue;
            }

            zcl::t_font_glyph_info *glyph_info;

            if (!zcl::HashMapFind(&font_arrangement.code_pts_to_glyph_infos, step.code_pt, &glyph_info)) {
                ZCL_ASSERT(false && "Unsupported code point!");
                continue;
            }

            if (chr_index > 0 && font_arrangement.has_kernings) {
                zcl::t_i32 *kerning;

                if (zcl::HashMapFind(&font_arrangement.code_pt_pairs_to_kernings, {code_pt_last, step.code_pt}, &kerning)) {
                    chr_pos_pen.x += static_cast<zcl::t_f32>(*kerning);
                }
            }

            positions[chr_index] = pos + chr_pos_pen + zcl::V2IToF(glyph_info->offs);
            positions[chr_index].y += offs_y;

            chr_pos_pen.x += static_cast<zcl::t_f32>(glyph_info->adv);

            line_len++;
        }

        apply_offs_x();

        return positions;
    }

    void FrameSubmitStr(const t_frame_context context, const zcl::t_str_rdonly str, const t_font &font, const zcl::t_v2 pos, zcl::t_arena *const temp_arena, const zcl::t_v2 origin, const zcl::t_color_rgba32f blend) {
        ZCL_ASSERT(zcl::StrCheckValidUTF8(str));
        ZCL_ASSERT(zcl::OriginCheckValid(origin));

        if (zcl::StrCheckEmpty(str)) {
            return;
        }

        const zcl::t_array_mut<zcl::t_v2> chr_positions = CalcStrChrRenderPositions(str, font.arrangement, pos, origin, temp_arena);

        zcl::t_i32 chr_index = 0;

        ZCL_STR_WALK (str, step) {
            if (step.code_pt == ' ' || step.code_pt == '\n') {
                chr_index++;
                continue;
            }

            zcl::t_font_glyph_info *glyph_info;

            if (!zcl::HashMapFind(&font.arrangement.code_pts_to_glyph_infos, step.code_pt, &glyph_info)) {
                ZCL_ASSERT(false && "Unsupported code point!");
                continue;
            }

            FrameSubmitTexture(context, font.atlases[glyph_info->atlas_index], chr_positions[chr_index], glyph_info->atlas_rect);

            chr_index++;
        };
    }
}
#endif
