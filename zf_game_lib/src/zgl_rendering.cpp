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
            case ek_renderer_builtin_shader_prog_id_default:
                vertex_shader_compiled_bin = {g_vertex_shader_default_src_raw, g_vertex_shader_default_src_len};
                fragment_shader_compiled_bin = {g_fragment_shader_default_src_raw, g_fragment_shader_default_src_len};
                break;

            case ek_renderer_builtin_shader_prog_id_str:
                vertex_shader_compiled_bin = {g_vertex_shader_default_src_raw, g_vertex_shader_default_src_len};
                fragment_shader_compiled_bin = {g_fragment_shader_str_src_raw, g_fragment_shader_str_src_len};
                break;

            case ek_renderer_builtin_shader_prog_id_blend:
                vertex_shader_compiled_bin = {g_vertex_shader_default_src_raw, g_vertex_shader_default_src_len};
                fragment_shader_compiled_bin = {g_fragment_shader_blend_src_raw, g_fragment_shader_blend_src_len};
                break;

            default:
                ZCL_UNREACHABLE();
            }

            basis->shader_progs[i] = ShaderProgCreate(gfx_ticket, vertex_shader_compiled_bin, fragment_shader_compiled_bin, basis->perm_resource_group);
        }

        for (zcl::t_i32 i = 0; i < ekm_renderer_builtin_uniform_id_cnt; i++) {
            switch (static_cast<t_renderer_builtin_uniform_id>(i)) {
            case ek_renderer_builtin_uniform_id_sampler:
                basis->uniforms[i] = UniformCreate(gfx_ticket, ZCL_STR_LITERAL("u_texture"), ek_uniform_type_sampler, basis->perm_resource_group, temp_arena);
                break;

            case ek_renderer_builtin_uniform_id_blend:
                basis->uniforms[i] = UniformCreate(gfx_ticket, ZCL_STR_LITERAL("u_blend"), ek_uniform_type_v4, basis->perm_resource_group, temp_arena);
                break;

            default:
                ZCL_UNREACHABLE();
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

    t_rendering_context internal::RendererBegin(const t_rendering_basis *const rendering_basis, const t_gfx_ticket_mut gfx_ticket, zcl::t_arena *const rendering_state_arena) {
        FrameBegin(gfx_ticket);

        return {
            .basis = rendering_basis,
            .state = zcl::ArenaPush<t_rendering_state>(rendering_state_arena),
            .gfx_ticket = gfx_ticket,
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
        ZCL_ASSERT(rect.width > 0.0f && rect.height > 0.0f);

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
        zcl::t_arena quad_pts_arena = zcl::ArenaCreateWrapping(zcl::ToBytes(&quad_pts));
        const zcl::t_poly_mut quad_poly = zcl::PolyCreateQuadRotated(pos, size, origin, rot, &quad_pts_arena);

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
        zcl::t_arena quad_pts_arena = zcl::ArenaCreateWrapping(zcl::ToBytes(&quad_pts));
        const zcl::t_v2 quad_size = zcl::CalcCompwiseProd(zcl::V2IToF(zcl::RectGetSize(src_rect_to_use)), scale);
        const zcl::t_poly_mut quad_poly = zcl::PolyCreateQuadRotated(pos, quad_size, origin, rot, &quad_pts_arena);

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

    zcl::t_array_mut<zcl::t_v2> RendererCalcStrChrOffsets(const zcl::t_str_rdonly str, const zcl::t_font_arrangement &font_arrangement, const zcl::t_v2 origin, zcl::t_arena *const arena) {
        ZCL_ASSERT(zcl::StrCheckValidUTF8(str));
        ZCL_ASSERT(OriginCheckValid(origin));

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

        const auto result = zcl::ArenaPushArray<zcl::t_v2>(arena, str_meta.len);

        const zcl::t_f32 offs_y = static_cast<zcl::t_f32>(-(str_meta.line_cnt * font_arrangement.line_height)) * origin.y;

        zcl::t_i32 chr_index = 0;
        zcl::t_v2 chr_offs_pen = {}; // The position of the current character.
        zcl::t_i32 line_begin_chr_index = 0;
        zcl::t_i32 line_len = 0;
        zcl::t_code_point code_pt_last;

        const auto offs_x_applier = [&]() {
            if (line_len > 0) {
                const auto line_width = chr_offs_pen.x;

                for (zcl::t_i32 i = line_begin_chr_index; i < chr_index; i++) {
                    result[i].x -= line_width * origin.x;
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
                offs_x_applier();

                chr_offs_pen.x = 0.0f;
                chr_offs_pen.y += static_cast<zcl::t_f32>(font_arrangement.line_height);

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
                    chr_offs_pen.x += static_cast<zcl::t_f32>(*kerning);
                }
            }

            result[chr_index] = chr_offs_pen + zcl::V2IToF(glyph_info->offs);
            result[chr_index].y += offs_y;

            chr_offs_pen.x += static_cast<zcl::t_f32>(glyph_info->adv);

            line_len++;
        }

        offs_x_applier();

        return result;
    }

    zcl::t_array_mut<zcl::t_poly_mut> RendererCalcStrChrColliders(const zcl::t_str_rdonly str, const t_font &font, const zcl::t_v2 pos, zcl::t_arena *const arena, zcl::t_arena *const temp_arena, const zcl::t_v2 origin, const zcl::t_f32 rot, const zcl::t_v2 scale) {
        ZCL_ASSERT(zcl::StrCheckValidUTF8(str));
        ZCL_ASSERT(zcl::OriginCheckValid(origin));

        if (zcl::StrCheckEmpty(str)) {
            return {}; // @todo: This should probably have the correct position, just no size.
        }

        const auto chr_offsets = RendererCalcStrChrOffsets(str, font.arrangement, origin, temp_arena);

        const auto result = zcl::ArenaPushArray<zcl::t_poly_mut>(arena, chr_offsets.len);

        zcl::t_i32 chr_index = 0;

        ZCL_STR_WALK (str, step) {
            ZCL_DEFER({ chr_index++; });

            if (step.code_pt == ' ' || step.code_pt == '\n') {
                continue;
            }

            zcl::t_font_glyph_info *glyph_info;

            if (!zcl::HashMapFind(&font.arrangement.code_pts_to_glyph_infos, step.code_pt, &glyph_info)) {
                ZCL_ASSERT(false && "Unsupported code point!");
                continue;
            }

            const zcl::t_v2 chr_pos = pos
                + zcl::CalcLengthDir(chr_offsets[chr_index].x * scale.x, rot)
                + zcl::CalcLengthDir(chr_offsets[chr_index].y * scale.y, rot + (zcl::k_pi / 2.0f));

            result[chr_index] = zcl::PolyCreateQuadRotated(chr_pos, zcl::CalcCompwiseProd(zcl::V2IToF(zcl::RectGetSize(glyph_info->atlas_rect)), scale), {}, rot, arena);
        }

        return result;
    }

    void RendererSubmitStr(const t_rendering_context rc, const zcl::t_str_rdonly str, const t_font &font, const zcl::t_v2 pos, const zcl::t_color_rgba32f color, zcl::t_arena *const temp_arena, const zcl::t_v2 origin, const zcl::t_f32 rot, const zcl::t_v2 scale) {
        ZCL_ASSERT(zcl::StrCheckValidUTF8(str));
        ZCL_ASSERT(zcl::OriginCheckValid(origin));

        if (zcl::StrCheckEmpty(str)) {
            return;
        }

        RendererSetShaderProg(rc, rc.basis->shader_progs[ek_renderer_builtin_shader_prog_id_str]);

        const auto chr_offsets = RendererCalcStrChrOffsets(str, font.arrangement, origin, temp_arena);

        zcl::t_i32 chr_index = 0;

        ZCL_STR_WALK (str, step) {
            ZCL_DEFER({ chr_index++; });

            if (step.code_pt == ' ' || step.code_pt == '\n') {
                continue;
            }

            zcl::t_font_glyph_info *glyph_info;

            if (!zcl::HashMapFind(&font.arrangement.code_pts_to_glyph_infos, step.code_pt, &glyph_info)) {
                ZCL_ASSERT(false && "Unsupported code point!");
                continue;
            }

            const zcl::t_v2 chr_pos = pos
                + zcl::CalcLengthDir(chr_offsets[chr_index].x * scale.x, rot)
                + zcl::CalcLengthDir(chr_offsets[chr_index].y * scale.y, rot + (zcl::k_pi / 2.0f));

            zcl::t_static_array<zcl::t_v2, 4> quad_pts;
            zcl::t_arena quad_pts_arena = zcl::ArenaCreateWrapping(zcl::ToBytes(&quad_pts));
            const zcl::t_poly_mut quad_poly = zcl::PolyCreateQuadRotated(chr_pos, zcl::CalcCompwiseProd(zcl::V2IToF(zcl::RectGetSize(glyph_info->atlas_rect)), scale), {}, rot, &quad_pts_arena);

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
