#include <zgl/zgl_rendering.h>

#include <zgl/zgl_platform.h>
#include <zgl/zgl_gfx.h>

namespace zf {
    struct s_batch_vert {
        s_v2 vert_coord;
        s_color_rgba32f blend;
    };

    constexpr s_static_array<t_i32, 2> g_batch_vert_attr_component_cnts = {2, 4}; // This has to match the number of components per attribute above.

    constexpr t_len g_batch_vert_component_cnt = ZF_SIZE_OF(s_batch_vert) / ZF_SIZE_OF(t_f32);

    static_assert([]() {
        t_len sum = 0;

        for (t_len i = 0; i < g_batch_vert_attr_component_cnts.g_len; i++) {
            sum += g_batch_vert_attr_component_cnts[i];
        }

        return sum == g_batch_vert_component_cnt;
    }());

    constexpr t_len g_batch_vert_limit = 1024;

    constexpr s_cstr_literal g_batch_vert_shader_src = R"(#version 330 core

layout (location = 0) in vec2 a_vert;
layout (location = 1) in vec4 a_blend;

out vec4 v_blend;

uniform mat4 u_proj;

void main() {
    gl_Position = u_proj * vec4(a_vert, 0.0, 1.0);
    v_blend = a_blend;
}
)";

    constexpr s_cstr_literal g_batch_frag_shader_src = R"(#version 330 core

in vec4 v_blend;

out vec4 o_frag_color;

void main() {
    o_frag_color = v_blend;
}
)";

    s_rendering_basis CreateRenderingBasis(s_mem_arena &mem_arena, s_mem_arena &temp_mem_arena) {
        s_rendering_basis basis = {
            .gfx_res_arena = CreateGFXResourceArena(mem_arena),
        };

        {
            // clang-format off
            constexpr s_static_array<t_f32, 18> verts = {
                0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f,
            };
            // clang-format on

            constexpr s_static_array<t_i32, 2> vert_attr_component_cnts = {2, 4};

            basis.batch_mesh = &CreateMesh(verts.ToNonstatic().Ptr(), verts.ToNonstatic().Len(), true, vert_attr_component_cnts, basis.gfx_res_arena);
        }

        if (!CreateShaderProg(g_batch_vert_shader_src, g_batch_frag_shader_src, basis.gfx_res_arena, temp_mem_arena, basis.batch_shader_prog)) {
            ZF_FATAL();
        }

        return basis;
    }

    struct s_rendering_state {
        s_ptr<const s_rendering_basis> basis;

        s_ptr<s_mem_arena> mem_arena;

        s_render_instr_seq instr_seq;

        s_static_list<s_batch_vert, g_batch_vert_limit> batch_verts;
    };

    s_rendering_state &BeginRendering(const s_rendering_basis &basis, s_mem_arena &mem_arena) {
        auto &state = Alloc<s_rendering_state>(mem_arena);

        state.basis = &basis;
        state.mem_arena = &mem_arena;
        state.instr_seq = {mem_arena};

        state.instr_seq.SubmitClear(s_color_rgb8(0, 255, 0));

        return state;
    }

    static void Flush(s_rendering_state &rs) {
        const auto verts = rs.batch_verts.ToArray();
        const s_array<t_f32> verts_f32 = {reinterpret_cast<t_f32 *>(verts.Ptr().Raw()), verts.SizeInBytes() / ZF_SIZE_OF(t_f32)};
        rs.instr_seq.SubmitMeshUpdate(*rs.basis->batch_mesh, verts_f32);

        rs.instr_seq.SubmitShaderProgSet(*rs.basis->batch_shader_prog);

        const s_v2_i fb_size_cache = WindowFramebufferSizeCache();

        auto proj_mat = CreateIdentityMatrix();
        proj_mat.elems[0][0] = 1.0f / (static_cast<t_f32>(fb_size_cache.x) / 2.0f);
        proj_mat.elems[1][1] = -1.0f / (static_cast<t_f32>(fb_size_cache.y) / 2.0f);
        proj_mat.elems[3][0] = -1.0f;
        proj_mat.elems[3][1] = 1.0f;

        rs.instr_seq.SubmitShaderProgUniformSet(s_cstr_literal("u_proj"), proj_mat);

        rs.instr_seq.SubmitMeshDraw(*rs.basis->batch_mesh);
    }

    void EndRendering(s_rendering_state &rs, s_mem_arena &temp_mem_arena) {
        Flush(rs);
        rs.instr_seq.Exec(temp_mem_arena);
        internal::SwapWindowBuffers();
    }

    void DrawPoly(s_rendering_state &rs, const s_array_rdonly<s_v2> pts, const s_color_rgba32f color) {
        ZF_ASSERT(pts.Len() <= g_batch_vert_limit);

        if (rs.batch_verts.Len() + pts.Len() > rs.batch_verts.Cap()) {
            Flush(rs);
        }

        for (t_len i = 0; i < pts.Len(); i++) {
            rs.batch_verts.Append({pts[i], color});
        }
    }
}
