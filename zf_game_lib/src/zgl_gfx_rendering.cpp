#include <zgl/zgl_gfx_private.h>

namespace zgl {
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

    static struct {
        t_phase phase;

        t_gfx_resource_group *perm_resource_group;

        t_gfx_resource *vert_buf;

        t_gfx_resource *shader_prog_default;
        t_gfx_resource *shader_prog_blend;

        t_gfx_resource *sampler_uniform;
        t_gfx_resource *blend_uniform;

        t_gfx_resource *px_texture;

        zcl::t_v2_i frame_size_cache;

        struct {
            zcl::t_b8 pass_active;
            zcl::t_i32 pass_index; // Maps directly to BGFX view ID.

            zcl::t_i32 frame_vert_cnt;

            struct {
                zcl::t_static_array<t_vertex, k_batch_vert_limit> verts;
                zcl::t_i32 vert_cnt;

                const t_gfx_resource *shader_prog;
                const t_gfx_resource *texture;
            } batch_state;
        } frame_state;
    } g_state;

    void GFXInitRendering(zcl::t_arena *const arena, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(g_state.phase == ek_phase_inactive);

        g_state.perm_resource_group = GFXResourceGroupCreate(arena);

        g_state.vert_buf = VertexBufCreate(k_frame_vert_limit, arena);

        g_state.shader_prog_default = ShaderProgCreate({g_vert_shader_default_src_raw, g_vert_shader_default_src_len}, {g_frag_shader_default_src_raw, g_frag_shader_default_src_len}, g_state.perm_resource_group);
        g_state.shader_prog_blend = ShaderProgCreate({g_vert_shader_default_src_raw, g_vert_shader_default_src_len}, {g_frag_shader_blend_src_raw, g_frag_shader_blend_src_len}, g_state.perm_resource_group);

        g_state.sampler_uniform = UniformCreate(ZCL_STR_LITERAL("u_texture"), ek_uniform_type_sampler, g_state.perm_resource_group, temp_arena);
        g_state.blend_uniform = UniformCreate(ZCL_STR_LITERAL("u_blend"), ek_uniform_type_v4, g_state.perm_resource_group, temp_arena);

        const zcl::t_static_array<zcl::t_u8, 4> batch_px_texture_rgba = {{255, 255, 255, 255}};
        g_state.px_texture = TextureCreate({{1, 1}, zcl::ArrayToNonstatic(&batch_px_texture_rgba)}, g_state.perm_resource_group);
    }

    void GFXShutdownRendering() {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
    }

    void FrameBegin(const t_platform_ticket_rdonly platform_ticket) {
        ZCL_ASSERT(g_state.phase == ek_phase_active_but_not_midframe);

        g_state.phase = ek_phase_active_and_midframe;

        FrameSetSize(WindowGetFramebufferSizeCache(platform_ticket));
    }

    static void FrameFlush() {
        ZCL_ASSERT(g_state.phase == ek_phase_active_and_midframe);
        ZCL_ASSERT(g_state.frame_state.pass_active);

        if (g_state.frame_state.batch_state.vert_cnt == 0) {
            return;
        }

        if (g_state.frame_state.frame_vert_cnt + g_state.frame_state.batch_state.vert_cnt > k_frame_vert_limit) {
            ZCL_FATAL();
        }

        const auto verts = zcl::ArraySlice(zcl::ArrayToNonstatic(&g_state.frame_state.batch_state.verts), 0, g_state.frame_state.batch_state.vert_cnt);
        VertexBufWrite(g_state.vert_buf, );

        const auto texture = g_state.frame_state.batch_state.texture ? g_state.frame_state.batch_state.texture : g_state.px_texture;
        const t_gfx_resource *const shader_prog = g_state.frame_state.batch_state.shader_prog ? g_state.frame_state.batch_state.shader_prog : g_state.shader_prog_default;

        FrameSubmit(g_state.frame_state.pass_index, g_state.vert_buf, g_state.frame_state.frame_vert_cnt, g_state.frame_state.frame_vert_cnt + g_state.frame_state.batch_state.vert_cnt, texture, shader_prog, g_state.sampler_uniform);

        g_state.frame_state.frame_vert_cnt += g_state.frame_state.batch_state.vert_cnt;

        zcl::ZeroClearItem(&g_state.frame_state.batch_state);
    }

    void FrameEnd() {
        ZCL_ASSERT(g_state.phase == ek_phase_active_and_midframe);
        ZCL_ASSERT(!g_state.frame_state.pass_active);

        FrameComplete();

        g_state.phase = ek_phase_active_but_not_midframe;
    }

    zcl::t_v2_i FrameGetSize() {
        return g_state.frame_size_cache;
    }

    void FramePassBegin(const zcl::t_v2_i size, const zcl::t_mat4x4 &view_mat, const zcl::t_b8 clear, const zcl::t_color_rgba32f clear_col) {
        ZCL_ASSERT(!g_state.frame_state.pass_active);

        g_state.frame_state.pass_active = true;
        ZCL_REQUIRE(g_state.frame_state.pass_index < k_frame_pass_limit && "Trying to begin a new frame pass, but the limit has been reached!");

        FramePassConfigure(g_state.frame_state.pass_index, size, view_mat, clear, clear_col);
    }

    void FramePassBeginOffscreen(const t_gfx_resource *const texture_target, const zcl::t_mat4x4 &view_mat, const zcl::t_b8 clear, const zcl::t_color_rgba32f clear_col) {
        ZCL_ASSERT(!g_state.frame_state.pass_active);

        g_state.frame_state.pass_active = true;
        ZCL_REQUIRE(g_state.frame_state.pass_index < k_frame_pass_limit && "Trying to begin a new frame pass, but the limit has been reached!");

        FramePassConfigureOffscreen(g_state.frame_state.pass_index, texture_target, view_mat, clear, clear_col);
    }

    void FramePassEnd() {
        ZCL_ASSERT(g_state.frame_state.pass_active);

        FrameFlush();

        g_state.frame_state.pass_active = false;
        g_state.frame_state.pass_index++;
    }

    zcl::t_b8 FramePassCheckActive() {
        return g_state.frame_state.pass_active;
    }

    zcl::t_i32 FramePassGetIndex() {
        return g_state.frame_state.pass_index;
    }

    void FrameSetShaderProg(const t_gfx_resource *const prog) {
        ZCL_ASSERT(g_state.phase == ek_phase_active_and_midframe);

        if (prog != g_state.frame_state.batch_state.shader_prog) {
            FrameFlush();
            g_state.frame_state.batch_state.shader_prog = prog;
        }
    }

    void FrameSubmitTriangles(const zcl::t_array_rdonly<t_frame_triangle> triangles, const t_gfx_resource *const texture) {
        ZCL_ASSERT(g_state.phase == ek_phase_active_and_midframe);
        ZCL_ASSERT(g_state.frame_state.pass_index != -1 && "A pass must be set before submitting primitives!");
        ZCL_ASSERT(triangles.len > 0);

        const zcl::t_i32 num_verts_to_submit = 3 * triangles.len;

        if (num_verts_to_submit > k_batch_vert_limit) {
            ZCL_FATAL();
        }

#ifdef ZCL_DEBUG
        for (zcl::t_i32 i = 0; i < triangles.len; i++) {
            ZCL_ASSERT(FrameTriangleCheckValid(triangles[i]));
        }
#endif

        if (texture != g_state.frame_state.batch_state.texture || g_state.frame_state.batch_state.vert_cnt + num_verts_to_submit > k_batch_vert_limit) {
            FrameFlush();
            g_state.frame_state.batch_state.texture = texture;
        }

        for (zcl::t_i32 i = 0; i < triangles.len; i++) {
            const zcl::t_i32 offs = g_state.frame_state.batch_state.vert_cnt;
            g_state.frame_state.batch_state.verts[offs + (3 * i) + 0] = triangles[i].verts[0];
            g_state.frame_state.batch_state.verts[offs + (3 * i) + 1] = triangles[i].verts[1];
            g_state.frame_state.batch_state.verts[offs + (3 * i) + 2] = triangles[i].verts[2];
        }

        g_state.frame_state.batch_state.vert_cnt += num_verts_to_submit;
    }
}
