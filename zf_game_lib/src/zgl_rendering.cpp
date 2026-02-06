#include <zgl/zgl_rendering.h>

namespace zgl {
    extern const zcl::t_u8 g_vertex_shader_default_src_raw[];
    extern const zcl::t_i32 g_vertex_shader_default_src_len;

    extern const zcl::t_u8 g_fragment_shader_default_src_raw[];
    extern const zcl::t_i32 g_fragment_shader_default_src_len;

    extern const zcl::t_u8 g_fragment_shader_str_src_raw[];
    extern const zcl::t_i32 g_fragment_shader_str_src_len;

    extern const zcl::t_u8 g_fragment_shader_blend_src_raw[];
    extern const zcl::t_i32 g_fragment_shader_blend_src_len;

    struct t_rendering_basis {
        t_gfx_resource_group *perm_resource_group;

        t_gfx_resource *vertex_buf;

        zcl::t_static_array<t_gfx_resource *, ekm_renderer_builtin_shader_prog_id_cnt> shader_progs;

        zcl::t_static_array<t_gfx_resource *, ekm_renderer_builtin_uniform_id_cnt> uniforms;

        t_gfx_resource *px_texture;
    };

    constexpr zcl::t_i32 k_batch_vertex_limit = 1024;

    struct t_rendering_state {
        zcl::t_b8 pass_active;
        zcl::t_i32 pass_index;

        zcl::t_i32 vertex_cnt_flushed;

        const t_gfx_resource *shader_prog;

        struct {
            zcl::t_static_array<t_gfx_vertex, k_batch_vertex_limit> vertices;
            zcl::t_i32 vertex_cnt;

            const t_gfx_resource *texture;
        } batch;
    };

    t_rendering_basis *internal::RenderingBasisCreate(const zcl::t_i32 frame_vertex_limit, const t_gfx_ticket_mut gfx_ticket, zcl::t_arena *const arena, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(frame_vertex_limit > 0);

        const auto basis = zcl::ArenaPush<t_rendering_basis>(arena);

        basis->perm_resource_group = GFXResourceGroupCreate(gfx_ticket, arena);

        basis->vertex_buf = VertexBufCreate(gfx_ticket, frame_vertex_limit, basis->perm_resource_group);

        for (zcl::t_i32 i = 0; i < ekm_renderer_builtin_shader_prog_id_cnt; i++) {
            zcl::t_array_rdonly<zcl::t_u8> vertex_shader_compiled_bin;
            zcl::t_array_rdonly<zcl::t_u8> fragment_shader_compiled_bin;

            switch (static_cast<t_renderer_builtin_shader_prog_id>(i)) {
                case ek_renderer_builtin_shader_prog_id_default: {
                    vertex_shader_compiled_bin = {g_vertex_shader_default_src_raw, g_vertex_shader_default_src_len};
                    fragment_shader_compiled_bin = {g_fragment_shader_default_src_raw, g_fragment_shader_default_src_len};
                    break;
                }

                case ek_renderer_builtin_shader_prog_id_str: {
                    vertex_shader_compiled_bin = {g_vertex_shader_default_src_raw, g_vertex_shader_default_src_len};
                    fragment_shader_compiled_bin = {g_fragment_shader_str_src_raw, g_fragment_shader_str_src_len};
                    break;
                }

                case ek_renderer_builtin_shader_prog_id_blend: {
                    vertex_shader_compiled_bin = {g_vertex_shader_default_src_raw, g_vertex_shader_default_src_len};
                    fragment_shader_compiled_bin = {g_fragment_shader_blend_src_raw, g_fragment_shader_blend_src_len};
                    break;
                }

                default: {
                    ZCL_UNREACHABLE();
                }
            }

            basis->shader_progs[i] = ShaderProgCreate(gfx_ticket, vertex_shader_compiled_bin, fragment_shader_compiled_bin, basis->perm_resource_group);
        }

        for (zcl::t_i32 i = 0; i < ekm_renderer_builtin_uniform_id_cnt; i++) {
            switch (static_cast<t_renderer_builtin_uniform_id>(i)) {
                case ek_renderer_builtin_uniform_id_sampler: {
                    basis->uniforms[i] = UniformCreate(gfx_ticket, ZCL_STR_LITERAL("u_texture"), ek_uniform_type_sampler, basis->perm_resource_group, temp_arena);
                    break;
                }

                case ek_renderer_builtin_uniform_id_blend: {
                    basis->uniforms[i] = UniformCreate(gfx_ticket, ZCL_STR_LITERAL("u_blend"), ek_uniform_type_v4, basis->perm_resource_group, temp_arena);
                    break;
                }

                default: {
                    ZCL_UNREACHABLE();
                }
            }
        }

        const zcl::t_static_array<zcl::t_color_rgba8, 1> px_texture_pixels = {{{255, 255, 255, 255}}};

        const zcl::t_texture_data_rdonly px_texture_data = {
            .dims = {1, 1},
            .format = zcl::ek_texture_format_rgba8,
            .pixels = {.rgba8 = zcl::ArrayToNonstatic(&px_texture_pixels)},
        };

        basis->px_texture = TextureCreate(gfx_ticket, px_texture_data, basis->perm_resource_group);

        return basis;
    }

    void internal::RenderingBasisDestroy(t_rendering_basis *const basis, const t_gfx_ticket_mut gfx_ticket) {
        GFXResourceGroupDestroy(gfx_ticket, basis->perm_resource_group);
        *basis = {};
    }

    t_gfx_resource *RendererGetBuiltinShaderProg(const t_rendering_basis *const rb, const t_renderer_builtin_shader_prog_id id) {
        return rb->shader_progs[id];
    }

    t_gfx_resource *RendererGetBuiltinUniform(const t_rendering_basis *const rb, const t_renderer_builtin_uniform_id id) {
        return rb->uniforms[id];
    }

    t_rendering_context internal::RendererBegin(const t_rendering_basis *const rendering_basis, const t_gfx_ticket_mut gfx_ticket, const zcl::t_v2_i screen_size, zcl::t_arena *const rendering_state_arena) {
        FrameBegin(gfx_ticket);

        return {
            .basis = rendering_basis,
            .state = zcl::ArenaPush<t_rendering_state>(rendering_state_arena),
            .gfx_ticket = gfx_ticket,
            .screen_size = screen_size,
        };
    }

    static void RendererFlush(const t_rendering_context rc) {
        ZCL_ASSERT(rc.state->pass_active);

        if (rc.state->batch.vertex_cnt == 0) {
            return;
        }

        if (rc.state->vertex_cnt_flushed + rc.state->batch.vertex_cnt > internal::VertexBufGetVertexCount(rc.gfx_ticket, rc.basis->vertex_buf)) {
            ZCL_FATAL();
        }

        const auto vertices = zcl::ArraySlice(zcl::ArrayToNonstatic(&rc.state->batch.vertices), 0, rc.state->batch.vertex_cnt);
        internal::VertexBufWrite(rc.gfx_ticket, rc.basis->vertex_buf, rc.state->vertex_cnt_flushed, vertices);

        const auto texture = rc.state->batch.texture ? rc.state->batch.texture : rc.basis->px_texture;
        const t_gfx_resource *const shader_prog = rc.state->shader_prog ? rc.state->shader_prog : rc.basis->shader_progs[ek_renderer_builtin_shader_prog_id_default];

        internal::FrameSubmit(rc.gfx_ticket, rc.state->pass_index, rc.basis->vertex_buf, rc.state->vertex_cnt_flushed, rc.state->vertex_cnt_flushed + rc.state->batch.vertex_cnt, texture, shader_prog, rc.basis->uniforms[ek_renderer_builtin_uniform_id_sampler]);

        rc.state->vertex_cnt_flushed += rc.state->batch.vertex_cnt;

        zcl::ZeroClearItem(&rc.state->batch);
    }

    void internal::RendererEnd(const t_rendering_context rc) {
        ZCL_ASSERT(!rc.state->pass_active);

        FrameEnd(rc.gfx_ticket);
    }

    void RendererPassBegin(const t_rendering_context rc, const zcl::t_v2_i size, const zcl::t_mat4x4 &view_mat, const zcl::t_b8 clear, const zcl::t_color_rgba32f clear_col) {
        ZCL_ASSERT(!rc.state->pass_active);

        rc.state->pass_active = true;
        ZCL_REQUIRE(rc.state->pass_index < internal::k_frame_pass_limit && "Trying to begin a new frame pass, but the limit has been reached!");

        internal::FramePassConfigure(rc.gfx_ticket, rc.state->pass_index, size, view_mat, clear, clear_col);
    }

    void RendererPassBeginOffscreen(const t_rendering_context rc, const t_gfx_resource *const texture_target, const zcl::t_mat4x4 &view_mat, const zcl::t_b8 clear, const zcl::t_color_rgba32f clear_col) {
        ZCL_ASSERT(!rc.state->pass_active);

        rc.state->pass_active = true;
        ZCL_REQUIRE(rc.state->pass_index < internal::k_frame_pass_limit && "Trying to begin a new frame pass, but the limit has been reached!");

        internal::FramePassConfigureOffscreen(rc.gfx_ticket, rc.state->pass_index, texture_target, view_mat, clear, clear_col);
    }

    void RendererPassEnd(const t_rendering_context rc) {
        ZCL_ASSERT(rc.state->pass_active);

        RendererFlush(rc);

        rc.state->pass_active = false;
        rc.state->pass_index++;
    }

    void RendererSetShaderProg(const t_rendering_context rc, const t_gfx_resource *const prog) {
        if (prog != rc.state->shader_prog) {
            RendererFlush(rc);
            rc.state->shader_prog = prog;
        }
    }

    void RendererSubmit(const t_rendering_context rc, const zcl::t_array_rdonly<t_gfx_triangle> triangles, const t_gfx_resource *const texture) {
        ZCL_ASSERT(rc.state->pass_index != -1 && "A pass must be set before submitting primitives!");
        ZCL_ASSERT(triangles.len > 0);

        const zcl::t_i32 num_verts_to_submit = 3 * triangles.len;

        if (num_verts_to_submit > k_batch_vertex_limit) {
            ZCL_FATAL();
        }

#ifdef ZCL_DEBUG
        for (zcl::t_i32 i = 0; i < triangles.len; i++) {
            ZCL_ASSERT(GFXTriangleCheckValid(triangles[i]));
        }
#endif

        if (texture != rc.state->batch.texture || rc.state->batch.vertex_cnt + num_verts_to_submit > k_batch_vertex_limit) {
            RendererFlush(rc);
            rc.state->batch.texture = texture;
        }

        for (zcl::t_i32 i = 0; i < triangles.len; i++) {
            const zcl::t_i32 offs = rc.state->batch.vertex_cnt;
            rc.state->batch.vertices[offs + (3 * i) + 0] = triangles[i].vertices[0];
            rc.state->batch.vertices[offs + (3 * i) + 1] = triangles[i].vertices[1];
            rc.state->batch.vertices[offs + (3 * i) + 2] = triangles[i].vertices[2];
        }

        rc.state->batch.vertex_cnt += num_verts_to_submit;
    }

    void RendererSubmitRect(const t_rendering_context rc, const zcl::t_rect_f rect, const zcl::t_color_rgba32f color_topleft, const zcl::t_color_rgba32f color_topright, const zcl::t_color_rgba32f color_bottomright, const zcl::t_color_rgba32f color_bottomleft) {
        ZCL_ASSERT(rect.width >= 0.0f && rect.height >= 0.0f);

        const zcl::t_static_array<t_gfx_triangle, 2> triangles = {{
            {
                .vertices = {{
                    {.pos = zcl::RectGetTopLeft(rect), .blend = color_topleft, .uv = {0.0f, 0.0f}},
                    {.pos = zcl::RectGetTopRight(rect), .blend = color_topright, .uv = {1.0f, 0.0f}},
                    {.pos = zcl::RectGetBottomLeft(rect), .blend = color_bottomleft, .uv = {1.0f, 1.0f}},
                }},
            },
            {
                .vertices = {{
                    {.pos = zcl::RectGetBottomLeft(rect), .blend = color_bottomleft, .uv = {1.0f, 1.0f}},
                    {.pos = zcl::RectGetTopRight(rect), .blend = color_topright, .uv = {0.0f, 1.0f}},
                    {.pos = zcl::RectGetBottomRight(rect), .blend = color_bottomright, .uv = {0.0f, 0.0f}},
                }},
            },
        }};

        RendererSubmit(rc, zcl::ArrayToNonstatic(&triangles), nullptr);
    }

    void RendererSubmitRectRotated(const t_rendering_context rc, const zcl::t_v2 pos, const zcl::t_v2 size, const zcl::t_v2 origin, const zcl::t_f32 rot, const zcl::t_color_rgba32f color_topleft, const zcl::t_color_rgba32f color_topright, const zcl::t_color_rgba32f color_bottomright, const zcl::t_color_rgba32f color_bottomleft) {
        ZCL_ASSERT(OriginCheckValid(origin));

        zcl::t_static_array<zcl::t_v2, 4> quad_pts;

        zcl::t_arena *const quad_pts_arena = zcl::ArenaCreateWrapping(zcl::ToBytes(&quad_pts));
        ZCL_DEFER({ zcl::ArenaDestroy(quad_pts_arena); });

        const zcl::t_poly_mut quad_poly = zcl::PolyCreateQuadRotated(pos, size, origin, rot, quad_pts_arena);

        const zcl::t_static_array<t_gfx_triangle, 2> triangles = {{
            {
                .vertices = {{
                    {.pos = quad_poly.pts[0], .blend = color_topleft, .uv = {}},
                    {.pos = quad_poly.pts[1], .blend = color_topright, .uv = {}},
                    {.pos = quad_poly.pts[3], .blend = color_bottomleft, .uv = {}},
                }},
            },
            {
                .vertices = {{
                    {.pos = quad_poly.pts[3], .blend = color_bottomleft, .uv = {}},
                    {.pos = quad_poly.pts[1], .blend = color_topright, .uv = {}},
                    {.pos = quad_poly.pts[2], .blend = color_bottomright, .uv = {}},
                }},
            },
        }};

        RendererSubmit(rc, zcl::ArrayToNonstatic(&triangles));
    }

    void RendererSubmitRectOutlineOpaque(const t_rendering_context rc, const zcl::t_rect_f rect, const zcl::t_f32 color_r, const zcl::t_f32 color_g, const zcl::t_f32 color_b, const zcl::t_f32 innerness, const zcl::t_f32 thickness) {
        ZCL_ASSERT(thickness >= 0.0f);
        ZCL_ASSERT(innerness >= -1.0f && innerness <= 1.0f);

        const zcl::t_f32 innerness_perc = (innerness + 1.0f) / 2.0f;
        const zcl::t_f32 innerness_perc_inv = 1.0f - innerness_perc;

        const auto color = zcl::ColorCreateRGBA32F(color_r, color_g, color_b, 1.0f);

        RendererSubmitRect(rc, zcl::RectCreateF(rect.x - (thickness * innerness_perc_inv), rect.y - (thickness * innerness_perc_inv), rect.width - (thickness * innerness), thickness), color);
        RendererSubmitRect(rc, zcl::RectCreateF(rect.x + rect.width - (thickness * innerness_perc), rect.y - (thickness * innerness_perc_inv), thickness, rect.height + (thickness * -innerness)), color);
        RendererSubmitRect(rc, zcl::RectCreateF(rect.x + (thickness * innerness_perc), rect.y + rect.height - (thickness * innerness_perc), rect.width + (thickness * -innerness), thickness), color);
        RendererSubmitRect(rc, zcl::RectCreateF(rect.x - (thickness * innerness_perc_inv), rect.y + (thickness * innerness_perc), thickness, rect.height + (thickness * -innerness)), color);
    }

    void RendererSubmitLineSegment(const t_rendering_context rc, const zcl::t_v2 pos_begin, const zcl::t_v2 pos_end, const zcl::t_color_rgba32f color, const zcl::t_f32 thickness) {
        ZCL_ASSERT(thickness >= 0.0f);

        const zcl::t_v2 size = {zcl::CalcDist(pos_begin, pos_end), thickness};
        const zcl::t_f32 rot = zcl::CalcDirRads(pos_begin, pos_end);
        RendererSubmitRectRotated(rc, pos_begin, size, {0.0f, 0.5f}, rot, color);
    }

    void RendererSubmitPolyOutlineOpaque(const t_rendering_context rc, const zcl::t_poly_rdonly poly, const zcl::t_f32 color_r, const zcl::t_f32 color_g, const zcl::t_f32 color_b, const zcl::t_f32 thickness) {
        ZCL_ASSERT(thickness >= 0.0f);

        for (zcl::t_i32 i = 0; i < poly.pts.len; i++) {
            const zcl::t_v2 a = poly.pts[i];
            const zcl::t_v2 b = poly.pts[(i + 1) % poly.pts.len];
            const zcl::t_v2 offs = zcl::CalcDir(b, a) * thickness * 0.5f;

            RendererSubmitLineSegment(rc, a + offs, b + offs, {color_r, color_g, color_b, 1.0f}, thickness);
        }
    }

    static zcl::t_rect_f TextureUVRectCalc(const zcl::t_rect_i src_rect, const zcl::t_v2_i texture_size) {
        ZCL_ASSERT(texture_size.x > 0 && texture_size.y > 0);
        ZCL_ASSERT(src_rect.x >= 0 && src_rect.y >= 0 && src_rect.width > 0 && src_rect.height > 0 && zcl::RectGetRight(src_rect) <= texture_size.x && zcl::RectGetBottom(src_rect) <= texture_size.y);

        return {
            static_cast<zcl::t_f32>(src_rect.x) / static_cast<zcl::t_f32>(texture_size.x),
            static_cast<zcl::t_f32>(src_rect.y) / static_cast<zcl::t_f32>(texture_size.y),
            static_cast<zcl::t_f32>(src_rect.width) / static_cast<zcl::t_f32>(texture_size.x),
            static_cast<zcl::t_f32>(src_rect.height) / static_cast<zcl::t_f32>(texture_size.y),
        };
    }

    void RendererSubmitTexture(const t_rendering_context rc, const t_gfx_resource *const texture, const zcl::t_v2 pos, const zcl::t_rect_i src_rect, const zcl::t_v2 origin, const zcl::t_f32 rot, const zcl::t_v2 scale, const zcl::t_color_rgba32f blend) {
        const auto texture_size = TextureGetSize(rc.gfx_ticket, texture);

        zcl::t_rect_i src_rect_to_use;

        if (zcl::CheckEqual(src_rect, {})) {
            src_rect_to_use = {0, 0, texture_size.x, texture_size.y};
        } else {
            ZCL_ASSERT(src_rect.x >= 0 && src_rect.y >= 0 && zcl::RectGetRight(src_rect) <= texture_size.x && zcl::RectGetBottom(src_rect) <= texture_size.y);
            src_rect_to_use = src_rect;
        }

        const zcl::t_rect_f uv_rect = TextureUVRectCalc(src_rect_to_use, texture_size);

        zcl::t_static_array<zcl::t_v2, 4> quad_pts;

        zcl::t_arena *const quad_pts_arena = zcl::ArenaCreateWrapping(zcl::ToBytes(&quad_pts));
        ZCL_DEFER({ zcl::ArenaDestroy(quad_pts_arena); });

        const zcl::t_v2 quad_size = zcl::CalcCompwiseProd(zcl::V2IToF(zcl::RectGetSize(src_rect_to_use)), scale);
        const zcl::t_poly_mut quad_poly = zcl::PolyCreateQuadRotated(pos, quad_size, origin, rot, quad_pts_arena);

        const zcl::t_static_array<t_gfx_triangle, 2> triangles = {{
            {
                .vertices = {{
                    {.pos = quad_poly.pts[0], .blend = zcl::k_color_white, .uv = zcl::RectGetTopLeft(uv_rect)},
                    {.pos = quad_poly.pts[1], .blend = zcl::k_color_white, .uv = zcl::RectGetTopRight(uv_rect)},
                    {.pos = quad_poly.pts[3], .blend = zcl::k_color_white, .uv = zcl::RectGetBottomLeft(uv_rect)},
                }},
            },
            {
                .vertices = {{
                    {.pos = quad_poly.pts[3], .blend = zcl::k_color_white, .uv = zcl::RectGetBottomLeft(uv_rect)},
                    {.pos = quad_poly.pts[1], .blend = zcl::k_color_white, .uv = zcl::RectGetTopRight(uv_rect)},
                    {.pos = quad_poly.pts[2], .blend = zcl::k_color_white, .uv = zcl::RectGetBottomRight(uv_rect)},
                }},
            },
        }};

        RendererSubmit(rc, zcl::ArrayToNonstatic(&triangles), texture);
    }

    struct t_str_render_info_rdonly {
        zcl::t_array_rdonly<zcl::t_v2> chr_offsets;
        zcl::t_v2 size;
    };

    struct t_str_render_info_mut {
        zcl::t_array_mut<zcl::t_v2> chr_offsets;
        zcl::t_v2 size;

        operator t_str_render_info_rdonly() const {
            return {
                .chr_offsets = chr_offsets,
                .size = size,
            };
        }
    };

    static t_str_render_info_mut CalcStrRenderInfo(const zcl::t_str_rdonly str, const zcl::t_font_arrangement &font_arrangement, const zcl::t_v2 origin, zcl::t_arena *const arena) {
        ZCL_ASSERT(zcl::StrCheckValidUTF8(str));
        ZCL_ASSERT(zcl::OriginCheckValid(origin));

        if (zcl::StrCheckEmpty(str)) {
            return {};
        }

        struct t_str_meta {
            zcl::t_i32 len;
            zcl::t_i32 line_cnt;
        };

        const t_str_meta str_meta = [str]() {
            t_str_meta result = {.line_cnt = 1};

            ZCL_STR_WALK (str, step) {
                result.len++;

                if (step.code_pt == '\n') {
                    result.line_cnt++;
                }
            }

            return result;
        }();

        const auto chr_offsets = zcl::ArenaPushArray<zcl::t_v2>(arena, str_meta.len);

        zcl::t_f32 top_pen = zcl::k_f32_inf_pos;
        zcl::t_f32 width_pen = 0.0f;
        zcl::t_f32 bottom_pen = 0.0f;

        {
            zcl::t_i32 chr_index = 0;
            zcl::t_v2 chr_offs_pen = {};
            zcl::t_f32 line_width_pen = 0.0f;
            zcl::t_i32 line_begin_chr_index = 0;

            zcl::t_code_point code_pt_last;

            ZCL_STR_WALK (str, step) {
                ZCL_DEFER({
                    chr_index++;
                    code_pt_last = step.code_pt;
                });

                if (step.code_pt == '\n') {
                    chr_offs_pen.x = 0;
                    chr_offs_pen.y += static_cast<zcl::t_f32>(font_arrangement.line_height);

                    for (zcl::t_i32 i = line_begin_chr_index; i < chr_index; i++) {
                        chr_offsets[i].x -= line_width_pen * origin.x;
                    }

                    line_width_pen = 0.0f;
                    line_begin_chr_index = chr_index + 1;

                    continue;
                }

                zcl::t_font_glyph_info *glyph_info;

                if (!zcl::HashMapFind(&font_arrangement.code_pts_to_glyph_infos, step.code_pt, &glyph_info)) {
                    ZCL_FATAL();
                }

                if (chr_index > 0 && font_arrangement.has_kernings) {
                    zcl::t_i32 *kerning;

                    if (zcl::HashMapFind(&font_arrangement.code_pt_pairs_to_kernings, {code_pt_last, step.code_pt}, &kerning)) {
                        chr_offs_pen.x += static_cast<zcl::t_f32>(*kerning);
                    }
                }

                chr_offsets[chr_index] = chr_offs_pen + zcl::V2IToF(glyph_info->offs);

                chr_offs_pen.x += static_cast<zcl::t_f32>(glyph_info->adv);

                // @speed
                line_width_pen = zcl::CalcMax(line_width_pen, chr_offsets[chr_index].x + static_cast<zcl::t_f32>(glyph_info->atlas_rect.width));

                // @speed
                top_pen = zcl::CalcMin(top_pen, chr_offsets[chr_index].y);
                width_pen = zcl::CalcMax(width_pen, line_width_pen); // @todo: Need to drop excess horizontal padding too.
                bottom_pen = zcl::CalcMax(bottom_pen, chr_offsets[chr_index].y + static_cast<zcl::t_f32>(glyph_info->atlas_rect.height));
            }

            for (zcl::t_i32 i = line_begin_chr_index; i < str_meta.len; i++) {
                chr_offsets[i].x -= line_width_pen * origin.x;
            }

            for (zcl::t_i32 i = 0; i < str_meta.len; i++) {
                chr_offsets[i].y -= top_pen;
                chr_offsets[i].y -= (bottom_pen - top_pen) * origin.y;
            }
        }

        return {
            .chr_offsets = chr_offsets,
            .size = {width_pen, bottom_pen - top_pen},
        };
    }

    zcl::t_poly_mut CalcStrRenderCollider(const zcl::t_str_rdonly str, const t_font &font, const zcl::t_v2 pos, zcl::t_arena *const arena, zcl::t_arena *const temp_arena, const zcl::t_v2 origin, const zcl::t_f32 rot, const zcl::t_v2 scale) {
        ZCL_ASSERT(zcl::StrCheckValidUTF8(str));
        ZCL_ASSERT(zcl::OriginCheckValid(origin));

        if (zcl::StrCheckEmpty(str)) {
            return {};
        }

        const t_str_render_info_rdonly render_info = CalcStrRenderInfo(str, font.arrangement, origin, temp_arena);

        return zcl::PolyCreateQuadRotated(pos, render_info.size, origin, rot, arena);
    }

    void RendererSubmitStr(const t_rendering_context rc, const zcl::t_str_rdonly str, const t_font &font, const zcl::t_v2 pos, const zcl::t_color_rgba32f color, zcl::t_arena *const temp_arena, const zcl::t_v2 origin, const zcl::t_f32 rot, const zcl::t_v2 scale) {
        ZCL_ASSERT(zcl::StrCheckValidUTF8(str));
        ZCL_ASSERT(zcl::OriginCheckValid(origin));

        if (zcl::StrCheckEmpty(str)) {
            return;
        }

        const t_str_render_info_rdonly render_info = CalcStrRenderInfo(str, font.arrangement, origin, temp_arena);

        RendererSubmitRectRotated(rc, pos, render_info.size, origin, rot, zcl::k_color_red);

        RendererSetShaderProg(rc, rc.basis->shader_progs[ek_renderer_builtin_shader_prog_id_str]);

        zcl::t_i32 chr_index = 0;

        ZCL_STR_WALK (str, step) {
            ZCL_DEFER({ chr_index++; });

            if (step.code_pt == ' ' || step.code_pt == '\n') {
                continue;
            }

            zcl::t_font_glyph_info *glyph_info;

            if (!zcl::HashMapFind(&font.arrangement.code_pts_to_glyph_infos, step.code_pt, &glyph_info)) {
                ZCL_FATAL();
            }

            const zcl::t_v2 chr_pos = pos + zcl::CalcLengthDir(render_info.chr_offsets[chr_index].x * scale.x, rot) + zcl::CalcLengthDir(render_info.chr_offsets[chr_index].y * scale.y, rot + (zcl::k_pi / 2.0f));

            zcl::t_static_array<zcl::t_v2, 4> quad_pts;

            zcl::t_arena *const quad_pts_arena = zcl::ArenaCreateWrapping(zcl::ToBytes(&quad_pts));
            ZCL_DEFER({ zcl::ArenaDestroy(quad_pts_arena); });

            const zcl::t_poly_mut quad_poly = zcl::PolyCreateQuadRotated(chr_pos, zcl::CalcCompwiseProd(zcl::V2IToF(zcl::RectGetSize(glyph_info->atlas_rect)), scale), {}, rot, quad_pts_arena);

            const zcl::t_rect_f uv_rect = TextureUVRectCalc(glyph_info->atlas_rect, zcl::k_font_atlas_texture_size);

            const zcl::t_static_array<t_gfx_triangle, 2> triangles = {{
                {
                    .vertices = {{
                        {.pos = quad_poly.pts[0], .blend = color, .uv = zcl::RectGetTopLeft(uv_rect)},
                        {.pos = quad_poly.pts[1], .blend = color, .uv = zcl::RectGetTopRight(uv_rect)},
                        {.pos = quad_poly.pts[3], .blend = color, .uv = zcl::RectGetBottomLeft(uv_rect)},
                    }},
                },
                {
                    .vertices = {{
                        {.pos = quad_poly.pts[3], .blend = color, .uv = zcl::RectGetBottomLeft(uv_rect)},
                        {.pos = quad_poly.pts[1], .blend = color, .uv = zcl::RectGetTopRight(uv_rect)},
                        {.pos = quad_poly.pts[2], .blend = color, .uv = zcl::RectGetBottomRight(uv_rect)},
                    }},
                },
            }};

            RendererSubmit(rc, zcl::ArrayToNonstatic(&triangles), font.atlas_textures[glyph_info->atlas_index]);
        }

        RendererSetShaderProg(rc, nullptr);
    }
}
