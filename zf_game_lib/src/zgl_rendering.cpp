#include <zgl/zgl_rendering.h>

namespace zgl {
    constexpr zcl::t_i32 k_frame_vert_limit = 8192; // @todo: This should definitely be modifiable if the user wants.
    constexpr zcl::t_i32 k_batch_vert_limit = 1024;

    extern const zcl::t_u8 g_vert_shader_default_src_raw[];
    extern const zcl::t_i32 g_vert_shader_default_src_len;

    extern const zcl::t_u8 g_frag_shader_default_src_raw[];
    extern const zcl::t_i32 g_frag_shader_default_src_len;

    extern const zcl::t_u8 g_frag_shader_blend_src_raw[];
    extern const zcl::t_i32 g_frag_shader_blend_src_len;

    struct t_rendering_basis {
        t_gfx_resource_group *perm_resource_group;

        t_gfx_resource *vert_buf;

        t_gfx_resource *shader_prog_default;
        t_gfx_resource *shader_prog_blend;

        t_gfx_resource *sampler_uniform;
        t_gfx_resource *blend_uniform;

        t_gfx_resource *px_texture;
    };

    struct t_rendering_state {
        zcl::t_b8 pass_active;
        zcl::t_i32 pass_index;

        zcl::t_i32 frame_vert_cnt;

        struct {
            zcl::t_static_array<t_vertex, k_batch_vert_limit> verts;
            zcl::t_i32 vertex_cnt;

            const t_gfx_resource *shader_prog;
            const t_gfx_resource *texture;
        } batch_state;
    };

    t_rendering_basis *internal::RenderingBasisCreate(t_gfx *const gfx, zcl::t_arena *const arena, zcl::t_arena *const temp_arena) {
        const auto basis = zcl::ArenaPushItem<t_rendering_basis>(arena);

        basis->perm_resource_group = GFXResourceGroupCreate(gfx, arena);

        basis->vert_buf = VertexBufCreate(gfx, k_frame_vert_limit, basis->perm_resource_group);

        basis->shader_prog_default = ShaderProgCreate(gfx, {g_vert_shader_default_src_raw, g_vert_shader_default_src_len}, {g_frag_shader_default_src_raw, g_frag_shader_default_src_len}, basis->perm_resource_group);
        basis->shader_prog_blend = ShaderProgCreate(gfx, {g_vert_shader_default_src_raw, g_vert_shader_default_src_len}, {g_frag_shader_blend_src_raw, g_frag_shader_blend_src_len}, basis->perm_resource_group);

        basis->sampler_uniform = UniformCreate(gfx, ZCL_STR_LITERAL("u_texture"), ek_uniform_type_sampler, basis->perm_resource_group, temp_arena);
        basis->blend_uniform = UniformCreate(gfx, ZCL_STR_LITERAL("u_blend"), ek_uniform_type_v4, basis->perm_resource_group, temp_arena);

        const zcl::t_static_array<zcl::t_u8, 4> batch_px_texture_rgba = {{255, 255, 255, 255}};
        basis->px_texture = TextureCreate(gfx, {{1, 1}, zcl::ArrayToNonstatic(&batch_px_texture_rgba)}, basis->perm_resource_group);

        return basis;
    }

    void internal::RenderingBasisDestroy(t_rendering_basis *const basis, t_gfx *const gfx) {
        GFXResourceGroupDestroy(gfx, basis->perm_resource_group);
        *basis = {};
    }

    t_rendering_context internal::RendererBegin(const t_rendering_basis *const rendering_basis, t_gfx *const gfx, zcl::t_arena *const rendering_state_arena) {
        FrameBegin(gfx);

        return {
            .basis = rendering_basis,
            .state = zcl::ArenaPushItem<t_rendering_state>(rendering_state_arena),
            .gfx = gfx,
        };
    }

    static void RendererFlush(const t_rendering_context rc) {
        ZCL_ASSERT(rc.state->pass_active);

        if (rc.state->batch_state.vertex_cnt == 0) {
            return;
        }

        if (rc.state->frame_vert_cnt + rc.state->batch_state.vertex_cnt > k_frame_vert_limit) {
            ZCL_FATAL();
        }

        const auto vertices = zcl::ArraySlice(zcl::ArrayToNonstatic(&rc.state->batch_state.verts), 0, rc.state->batch_state.vertex_cnt);
        internal::VertexBufWrite(rc.gfx, rc.basis->vert_buf, rc.state->frame_vert_cnt, vertices);

        const auto texture = rc.state->batch_state.texture ? rc.state->batch_state.texture : rc.basis->px_texture;
        const t_gfx_resource *const shader_prog = rc.state->batch_state.shader_prog ? rc.state->batch_state.shader_prog : rc.basis->shader_prog_default;

        internal::FrameSubmit(rc.gfx, rc.state->pass_index, rc.basis->vert_buf, rc.state->frame_vert_cnt, rc.state->frame_vert_cnt + rc.state->batch_state.vertex_cnt, texture, shader_prog, rc.basis->sampler_uniform);

        rc.state->frame_vert_cnt += rc.state->batch_state.vertex_cnt;

        zcl::ZeroClearItem(&rc.state->batch_state);
    }

    void internal::RendererEnd(const t_rendering_context rc) {
        ZCL_ASSERT(!rc.state->pass_active);

        FrameEnd(rc.gfx);
    }

    void RendererPassBegin(const t_rendering_context rc, const zcl::t_v2_i size, const zcl::t_mat4x4 &view_mat, const zcl::t_b8 clear, const zcl::t_color_rgba32f clear_col) {
        ZCL_ASSERT(!rc.state->pass_active);

        rc.state->pass_active = true;
        ZCL_REQUIRE(rc.state->pass_index < internal::k_frame_pass_limit && "Trying to begin a new frame pass, but the limit has been reached!");

        internal::FramePassConfigure(rc.gfx, rc.state->pass_index, size, view_mat, clear, clear_col);
    }

    void RendererPassBeginOffscreen(const t_rendering_context rc, const t_gfx_resource *const texture_target, const zcl::t_mat4x4 &view_mat, const zcl::t_b8 clear, const zcl::t_color_rgba32f clear_col) {
        ZCL_ASSERT(!rc.state->pass_active);

        rc.state->pass_active = true;
        ZCL_REQUIRE(rc.state->pass_index < internal::k_frame_pass_limit && "Trying to begin a new frame pass, but the limit has been reached!");

        internal::FramePassConfigureOffscreen(rc.gfx, rc.state->pass_index, texture_target, view_mat, clear, clear_col);
    }

    void RendererPassEnd(const t_rendering_context rc) {
        ZCL_ASSERT(rc.state->pass_active);

        RendererFlush(rc);

        rc.state->pass_active = false;
        rc.state->pass_index++;
    }

    void RendererSetShaderProg(const t_rendering_context rc, const t_gfx_resource *const prog) {
        if (prog != rc.state->batch_state.shader_prog) {
            RendererFlush(rc);
            rc.state->batch_state.shader_prog = prog;
        }
    }

    void RendererSubmit(const t_rendering_context rc, const zcl::t_array_rdonly<t_triangle> triangles, const t_gfx_resource *const texture) {
        ZCL_ASSERT(rc.state->pass_index != -1 && "A pass must be set before submitting primitives!");
        ZCL_ASSERT(triangles.len > 0);

        const zcl::t_i32 num_verts_to_submit = 3 * triangles.len;

        if (num_verts_to_submit > k_batch_vert_limit) {
            ZCL_FATAL();
        }

#ifdef ZCL_DEBUG
        for (zcl::t_i32 i = 0; i < triangles.len; i++) {
            ZCL_ASSERT(TriangleCheckValid(triangles[i]));
        }
#endif

        if (texture != rc.state->batch_state.texture || rc.state->batch_state.vertex_cnt + num_verts_to_submit > k_batch_vert_limit) {
            RendererFlush(rc);
            rc.state->batch_state.texture = texture;
        }

        for (zcl::t_i32 i = 0; i < triangles.len; i++) {
            const zcl::t_i32 offs = rc.state->batch_state.vertex_cnt;
            rc.state->batch_state.verts[offs + (3 * i) + 0] = triangles[i].vertices[0];
            rc.state->batch_state.verts[offs + (3 * i) + 1] = triangles[i].vertices[1];
            rc.state->batch_state.verts[offs + (3 * i) + 2] = triangles[i].vertices[2];
        }

        rc.state->batch_state.vertex_cnt += num_verts_to_submit;
    }

    void RendererSubmitRect(const t_rendering_context rc, const zcl::t_rect_f rect, const zcl::t_color_rgba32f color_topleft, const zcl::t_color_rgba32f color_topright, const zcl::t_color_rgba32f color_bottomright, const zcl::t_color_rgba32f color_bottomleft) {
        ZCL_ASSERT(rect.width > 0.0f && rect.height > 0.0f);

        const zcl::t_static_array<t_triangle, 2> triangles = {{
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

        const zcl::t_static_array<t_triangle, 2> triangles = {{
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

    void RendererSubmitTexture(const t_rendering_context rc, const t_gfx_resource *const texture, const zcl::t_v2 pos, const zcl::t_rect_i src_rect, const zcl::t_v2 origin, const zcl::t_f32 rot) {
        const auto texture_size = TextureGetSize(rc.gfx, texture);

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

        const zcl::t_static_array<t_triangle, 2> triangles = {{
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

    void RendererSubmitStr(const t_rendering_context rc, const zcl::t_str_rdonly str, const t_font &font, const zcl::t_v2 pos, zcl::t_arena *const temp_arena, const zcl::t_v2 origin, const zcl::t_color_rgba32f blend) {
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

            RendererSubmitTexture(rc, font.atlases[glyph_info->atlas_index], chr_positions[chr_index], glyph_info->atlas_rect);

            chr_index++;
        };
    }
}
