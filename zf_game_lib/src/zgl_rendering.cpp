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

    t_rendering_basis *RenderingBasisCreate(t_gfx *const gfx, zcl::t_arena *const arena, zcl::t_arena *const temp_arena) {
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

    void RenderingBasisDestroy(t_rendering_basis *const basis, t_gfx *const gfx) {
        GFXResourceGroupDestroy(gfx, basis->perm_resource_group);
        *basis = {};
    }

    t_rendering_context RendererBegin(const t_rendering_basis *const rendering_basis, t_gfx *const gfx, zcl::t_arena *const rendering_state_arena) {
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
        VertexBufWrite(rc.gfx, rc.basis->vert_buf, rc.state->frame_vert_cnt, vertices);

        const auto texture = rc.state->batch_state.texture ? rc.state->batch_state.texture : rc.basis->px_texture;
        const t_gfx_resource *const shader_prog = rc.state->batch_state.shader_prog ? rc.state->batch_state.shader_prog : rc.basis->shader_prog_default;

        FrameSubmit(rc.gfx, rc.state->pass_index, rc.basis->vert_buf, rc.state->frame_vert_cnt, rc.state->frame_vert_cnt + rc.state->batch_state.vertex_cnt, texture, shader_prog, rc.basis->sampler_uniform);

        rc.state->frame_vert_cnt += rc.state->batch_state.vertex_cnt;

        zcl::ZeroClearItem(&rc.state->batch_state);
    }

    void RendererEnd(const t_rendering_context rc) {
        RendererFlush(rc);
        FrameEnd(rc.gfx);
    }

    void RendererPassBegin(const t_rendering_context rc, const zcl::t_v2_i size, const zcl::t_mat4x4 &view_mat, const zcl::t_b8 clear, const zcl::t_color_rgba32f clear_col) {
        ZCL_ASSERT(!rc.state->pass_active);

        rc.state->pass_active = true;
        ZCL_REQUIRE(rc.state->pass_index < k_frame_pass_limit && "Trying to begin a new frame pass, but the limit has been reached!");

        FramePassConfigure(rc.gfx, rc.state->pass_index, size, view_mat, clear, clear_col);
    }

    void RendererPassBeginOffscreen(const t_rendering_context rc, const t_gfx_resource *const texture_target, const zcl::t_mat4x4 &view_mat, const zcl::t_b8 clear, const zcl::t_color_rgba32f clear_col) {
        ZCL_ASSERT(!rc.state->pass_active);

        rc.state->pass_active = true;
        ZCL_REQUIRE(rc.state->pass_index < k_frame_pass_limit && "Trying to begin a new frame pass, but the limit has been reached!");

        FramePassConfigureOffscreen(rc.gfx, rc.state->pass_index, texture_target, view_mat, clear, clear_col);
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
}
