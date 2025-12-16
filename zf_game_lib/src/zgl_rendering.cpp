#include <zgl/zgl_rendering.h>

#include <zgl/zgl_platform.h>

namespace zf {
    extern const t_u8 g_test_vs_raw[];
    extern const t_len g_test_vs_len;

    extern const t_u8 g_test_fs_raw[];
    extern const t_len g_test_fs_len;

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

    constexpr t_len g_batch_vert_limit = 1024;

    s_rendering_basis CreateRenderingBasis(s_mem_arena &mem_arena, s_mem_arena &temp_mem_arena) {
        s_rendering_basis basis = {
            .gfx_res_arena = CreateGFXResourceArena(mem_arena),
        };

        if (!CreateMesh(g_batch_vert_component_cnt * g_batch_vert_limit, basis.gfx_res_arena, basis.batch_mesh_resource)) {
            ZF_FATAL();
        }

        if (!CreateShaderProg({g_test_vs_raw, g_test_vs_len}, {g_test_fs_raw, g_test_fs_len}, basis.gfx_res_arena, basis.batch_shader_prog_resource)) {
            ZF_FATAL();
        }

        if (!CreateUniform(s_cstr_literal("u_tex"), basis.gfx_res_arena, temp_mem_arena, basis.batch_sampler_uniform_resource)) {
            ZF_FATAL();
        }

        {
            const s_static_array<t_u8, 4> px_texture_rgba = {255, 255, 255, 255};

            if (!CreateTexture({{1, 1}, px_texture_rgba}, basis.gfx_res_arena, basis.px_texture)) {
                ZF_FATAL();
            }
        }

        return basis;
    }

    void ReleaseRenderingBasis(s_rendering_basis &basis) {
        DestroyGFXResources(basis.gfx_res_arena);
        basis = {};
    }

    struct s_rendering_state {
        s_ptr<const s_rendering_basis> basis;

        s_ptr<s_mem_arena> mem_arena;

        s_render_instr_seq instr_seq;

        s_static_list<s_batch_vert, g_batch_vert_limit> batch_verts;
        s_ptr<const s_gfx_resource> batch_texture_resource;
    };

    s_rendering_state &internal::BeginRendering(const s_rendering_basis &basis, s_mem_arena &mem_arena) {
        auto &state = Alloc<s_rendering_state>(mem_arena);

        state.basis = &basis;
        state.mem_arena = &mem_arena;
        state.instr_seq = {mem_arena};

        return state;
    }

    static void Flush(s_rendering_state &rs) {
        if (rs.batch_verts.IsEmpty()) {
            return;
        }

        const auto verts = rs.batch_verts.ToArray();
        const s_array<t_f32> verts_f32 = {reinterpret_cast<t_f32 *>(verts.Ptr().Raw()), verts.SizeInBytes() / ZF_SIZE_OF(t_f32)};
        rs.instr_seq.SubmitMeshUpdate(*rs.basis->batch_mesh_resource, verts_f32);

        Log(s_cstr_literal("Dude: %, %"), rs.batch_texture_resource, FormatArray(verts));

        rs.instr_seq.SubmitTextureSet(rs.batch_texture_resource ? *rs.batch_texture_resource : *rs.basis->px_texture, *rs.basis->batch_sampler_uniform_resource);

        rs.instr_seq.SubmitMeshDraw(*rs.basis->batch_mesh_resource, *rs.basis->batch_shader_prog_resource);

        rs.batch_verts.Clear();
    }

    void internal::EndRendering(s_rendering_state &rs, s_mem_arena &temp_mem_arena) {
        Flush(rs);
        rs.instr_seq.Exec(temp_mem_arena);
    }

    void DrawTriangle(s_rendering_state &rs, const s_static_array<s_v2, 3> &pts, const s_static_array<s_color_rgba32f, 3> &pt_colors) {
        if (rs.batch_verts.Len() + pts.g_len > rs.batch_verts.Cap() || rs.batch_texture_resource) {
            Flush(rs);
        }

        rs.batch_texture_resource = nullptr;

        rs.batch_verts.Append({pts[0], pt_colors[0], {}});
        rs.batch_verts.Append({pts[1], pt_colors[1], {}});
        rs.batch_verts.Append({pts[2], pt_colors[2], {}});
    }

    void DrawRect(s_rendering_state &rs, const s_rect_f rect, const s_color_rgba32f color_topleft, const s_color_rgba32f color_topright, const s_color_rgba32f color_bottomright, const s_color_rgba32f color_bottomleft) {
        if (rs.batch_verts.Len() + 6 > rs.batch_verts.Cap() || rs.batch_texture_resource) {
            Flush(rs);
        }

        rs.batch_texture_resource = nullptr;

        rs.batch_verts.Append({rect.TopLeft(), color_topleft, {0.0f, 0.0f}});
        rs.batch_verts.Append({rect.TopRight(), color_topright, {1.0f, 0.0f}});
        rs.batch_verts.Append({rect.BottomRight(), color_bottomright, {1.0f, 1.0f}});

        rs.batch_verts.Append({rect.BottomRight(), color_bottomright, {1.0f, 1.0f}});
        rs.batch_verts.Append({rect.BottomLeft(), color_bottomleft, {0.0f, 1.0f}});
        rs.batch_verts.Append({rect.TopLeft(), color_topleft, {0.0f, 0.0f}});
    }

    void DrawTexture(s_rendering_state &rs, const s_v2 pos, const s_gfx_resource &texture_resource) {
        if (rs.batch_verts.Len() + 6 > rs.batch_verts.Cap()
            || (!rs.batch_verts.IsEmpty() && &texture_resource != rs.batch_texture_resource)) {
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
}
