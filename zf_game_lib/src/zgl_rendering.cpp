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

    constexpr s_cstr_literal g_batch_vert_shader_src = R"(#version 330 core

layout (location = 0) in vec2 a_vert;
layout (location = 1) in vec4 a_blend;

out vec4 v_blend;

void main() {
    gl_Position = vec4(a_vert, 0.0, 1.0);
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

    void DrawTriangle(s_rendering_state &rs, const s_v2 a, const s_v2 b, const s_v2 c, const s_color_rgba32f color) {
        // clang-format off
        rs.verts = {
            a.x, a.y, color.R(), color.G(), color.B(), color.A(),
            b.x, b.y, color.R(), color.G(), color.B(), color.A(),
            c.x, c.y, color.R(), color.G(), color.B(), color.A(),
        };
        // clang-format on

        rs.instr_seq.SubmitMeshUpdate(*rs.basis.batch_mesh, rs.verts);
        rs.instr_seq.SubmitShaderProgSet(*rs.basis.batch_shader_prog);
        rs.instr_seq.SubmitMeshDraw(*rs.basis.batch_mesh);
    }
}
