#include <zgl/zgl_batch_renderer.h>

#include <zgl/zgl_platform.h>

namespace zf {
    extern const t_u8 g_test_vs_raw[];
    extern const t_len g_test_vs_len;

    extern const t_u8 g_test_fs_raw[];
    extern const t_len g_test_fs_len;

    [[nodiscard]] t_b8 CreateBatchRenderer(s_batch_renderer_resources &o_resources) {
    }

    void s_batch_renderer::Flush() {
    }

#if 0
    struct s_batch_vert {
        s_v2 pos;
        s_color_rgba32f blend;
        s_v2 uv;

        s_batch_vert() = default;
        s_batch_vert(const s_v2 pos, const s_color_rgba32f blend, const s_v2 uv) : pos(pos), blend(blend), uv(uv) {}
    };

    constexpr s_static_array<t_i32, 3> g_batch_vert_attr_component_cnts = {2, 4, 2}; // This has to match the number of components per attribute above.

    constexpr t_len g_batch_vert_component_cnt = ZF_SIZE_OF(s_batch_vert) / ZF_SIZE_OF(t_f32);

    static_assert([]() {
        t_len sum = 0;

        for (t_len i = 0; i < g_batch_vert_attr_component_cnts.g_len; i++) {
            sum += g_batch_vert_attr_component_cnts[i];
        }

        return sum == g_batch_vert_component_cnt;
    }());

    constexpr t_len g_frame_vert_limit = 8192;

    s_batch_renderer_resources CreateRenderingBasis(s_mem_arena &mem_arena, s_mem_arena &temp_mem_arena) {
        s_batch_renderer_resources basis = {
            .arena = CreateGFXResourceArena(mem_arena),
        };

        if (!CreateMesh(g_frame_vert_limit, basis.arena, basis.mesh)) {
            ZF_FATAL();
        }

        if (!CreateShaderProg({g_test_vs_raw, g_test_vs_len}, {g_test_fs_raw, g_test_fs_len}, basis.arena, basis.shader_proh)) {
            ZF_FATAL();
        }

        if (!CreateUniform(s_cstr_literal("u_tex"), basis.arena, temp_mem_arena, basis.texture_sampler_uniform)) {
            ZF_FATAL();
        }

        {
            const s_static_array<t_u8, 4> px_texture_rgba = {255, 255, 255, 255};

            if (!CreateTexture({{1, 1}, px_texture_rgba}, basis.arena, basis.px_texture)) {
                ZF_FATAL();
            }
        }

        return basis;
    }

    void ReleaseRenderingBasis(s_batch_renderer_resources &basis) {
        DestroyGFXResources(basis.arena);
        basis = {};
    }

    struct s_frame_state {
        s_ptr<const s_batch_renderer_resources> basis;

        s_render_instr_seq instr_seq;

        s_static_array<s_batch_vert, g_frame_vert_limit> verts;
        t_len frame_verts_offs = 0;
        t_len frame_verts_offs = 0;

        s_ptr<const s_gfx_resource> batch_texture_resource;
    };

    s_frame_state &internal::BeginFrame(const s_batch_renderer_resources &basis, s_mem_arena &mem_arena) {
        auto &state = Alloc<s_frame_state>(mem_arena);

        state.basis = &basis;
        state.instr_seq = {mem_arena};

        return state;
    }

    static void Flush(s_frame_state &rs) {
        if (rs.batch_verts.IsEmpty()) {
            return;
        }

        const auto verts = rs.batch_verts.ToArray().SliceFrom();
        const s_array<t_f32> verts_f32 = {reinterpret_cast<t_f32 *>(verts.Ptr().Raw()), verts.SizeInBytes() / ZF_SIZE_OF(t_f32)};
        rs.instr_seq.SubmitMeshUpdate(*rs.basis->mesh, verts_f32);

        rs.instr_seq.SubmitTextureSet(rs.batch_texture_resource ? *rs.batch_texture_resource : *rs.basis->px_texture, *rs.basis->texture_sampler_uniform);

        rs.instr_seq.SubmitMeshDraw(*rs.basis->mesh, *rs.basis->shader_proh, static_cast<t_i32>(rs.batch_verts.Len()));

        rs.batch_verts.Clear();

        rs.batch_texture_resource = nullptr;
    }

    void internal::EndFrame(s_frame_state &rs, s_mem_arena &temp_mem_arena) {
        Flush(rs);
        rs.instr_seq.Exec(temp_mem_arena);
    }

    void DrawTriangle(s_frame_state &rs, const s_static_array<s_v2, 3> &pts, const s_static_array<s_color_rgba32f, 3> &pt_colors) {
        if (rs.batch_verts.Len() + pts.g_len > rs.batch_verts.Cap() || rs.batch_texture_resource) {
            Flush(rs);
        }

        rs.batch_verts.Append({pts[0], pt_colors[0], {}});
        rs.batch_verts.Append({pts[1], pt_colors[1], {}});
        rs.batch_verts.Append({pts[2], pt_colors[2], {}});
    }

    void DrawRect(s_frame_state &rs, const s_rect_f rect, const s_color_rgba32f color_topleft, const s_color_rgba32f color_topright, const s_color_rgba32f color_bottomright, const s_color_rgba32f color_bottomleft) {
        if (rs.batch_verts.Len() + 6 > rs.batch_verts.Cap() || rs.batch_texture_resource) {
            Flush(rs);
        }

        rs.batch_verts.Append({rect.TopLeft(), color_topleft, {0.0f, 0.0f}});
        rs.batch_verts.Append({rect.TopRight(), color_topright, {1.0f, 0.0f}});
        rs.batch_verts.Append({rect.BottomRight(), color_bottomright, {1.0f, 1.0f}});

        rs.batch_verts.Append({rect.BottomRight(), color_bottomright, {1.0f, 1.0f}});
        rs.batch_verts.Append({rect.BottomLeft(), color_bottomleft, {0.0f, 1.0f}});
        rs.batch_verts.Append({rect.TopLeft(), color_topleft, {0.0f, 0.0f}});
    }

    void DrawTexture(s_frame_state &rs, const s_v2 pos, const s_gfx_resource &texture_resource) {
        if (rs.batch_verts.Len() + 6 > rs.batch_verts.Cap() || (!rs.batch_verts.IsEmpty() && &texture_resource != rs.batch_texture_resource)) {
            Flush(rs);
        }

        rs.batch_texture_resource = &texture_resource;

        const auto texture_size = TextureSize(texture_resource);
        const s_rect_f rect = {pos, texture_size.ToV2()};

        rs.batch_verts.Append({rect.TopLeft(), zf::colors::g_white, {0.0f, 0.0f}});
        rs.batch_verts.Append({rect.TopRight(), zf::colors::g_white, {1.0f, 0.0f}});
        rs.batch_verts.Append({rect.BottomRight(), zf::colors::g_white, {1.0f, 1.0f}});

        rs.batch_verts.Append({rect.BottomRight(), zf::colors::g_white, {1.0f, 1.0f}});
        rs.batch_verts.Append({rect.BottomLeft(), zf::colors::g_white, {0.0f, 1.0f}});
        rs.batch_verts.Append({rect.TopLeft(), zf::colors::g_white, {0.0f, 0.0f}});
    }
#endif
}
