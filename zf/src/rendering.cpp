#include <zf/rendering.h>

namespace zf {
    static constexpr auto g_batch_vert_shader_src = s_str_view::FromRawTerminated(R"(#version 460 core

layout (location = 0) in vec2 a_vert;
layout (location = 1) in vec2 a_pos;
layout (location = 2) in vec2 a_size;
layout (location = 3) in float a_rot;
layout (location = 4) in vec2 a_tex_coord;
layout (location = 5) in vec4 a_blend;

out vec2 v_tex_coord;
out vec4 v_blend;

uniform mat4 u_view;
uniform mat4 u_proj;

void main() {
    float rot_cos = cos(a_rot);
    float rot_sin = -sin(a_rot);

    mat4 model = mat4(
        vec4(a_size.x * rot_cos, a_size.x * rot_sin, 0.0, 0.0),
        vec4(a_size.y * -rot_sin, a_size.y * rot_cos, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(a_pos.x, a_pos.y, 0.0, 1.0)
    );

    gl_Position = u_proj * u_view * model * vec4(a_vert, 0.0, 1.0);
    v_tex_coord = a_tex_coord;
    v_blend = a_blend;
}
)");

    static constexpr auto g_batch_frag_shader_src = s_str_view::FromRawTerminated(R"(#version 460 core

in vec2 v_tex_coord;
in vec4 v_blend;

out vec4 o_frag_color;

uniform sampler2D u_tex;

void main() {
    vec4 tex_color = texture(u_tex, v_tex_coord);
    o_frag_color = tex_color * v_blend;
}
)");

    static s_gfx_resource_handle MakeBatchMesh(c_gfx_resource_arena& gfx_res_arena, c_mem_arena& temp_mem_arena) {
        const int verts_len = g_batch_vert_component_cnt * g_batch_slot_vert_cnt * g_batch_slot_cnt;

        c_array<unsigned short> elems;

        const bool elems_make_failed = [&elems, &temp_mem_arena]() {
            if (!elems.Init(temp_mem_arena, g_batch_slot_elem_cnt * g_batch_slot_cnt)) {
                ZF_LOG_ERROR("Failed to reserve memory for batch renderable elements!");
                return false;
            }

            for (int i = 0; i < g_batch_slot_cnt; i++) {
                elems[(i * 6) + 0] = (i * 4) + 0;
                elems[(i * 6) + 1] = (i * 4) + 1;
                elems[(i * 6) + 2] = (i * 4) + 2;
                elems[(i * 6) + 3] = (i * 4) + 2;
                elems[(i * 6) + 4] = (i * 4) + 3;
                elems[(i * 6) + 5] = (i * 4) + 0;
            }

            return true;
        }();

        return gfx_res_arena.AddMesh(nullptr, verts_len, elems, g_batch_vert_attr_lens);
    }

    bool s_rendering_basis::Init(c_gfx_resource_arena& gfx_res_arena, c_mem_arena& temp_mem_arena) {
        // Generate the batch mesh.
        batch_mesh_hdl = MakeBatchMesh(gfx_res_arena, temp_mem_arena);

        if (!batch_mesh_hdl.IsValid()) {
            return false;
        }

        // Generate batch shader program.
        batch_shader_prog_hdl = gfx_res_arena.AddShaderProg(g_batch_vert_shader_src, g_batch_frag_shader_src, temp_mem_arena);

        if (!batch_shader_prog_hdl.IsValid()) {
            return false;
        }

        // Generate the pixel texture.
        {
            const s_static_array<t_byte, 4> px_rgba = {
                {255, 255, 255, 255}
            };

            px_tex_hdl = gfx_res_arena.AddTexture({ {1, 1}, px_rgba });

            if (!px_tex_hdl.IsValid()) {
                return false;
            }
        }

        return true;

#if 0
        if (!InitTextureGroup(&basis->builtin_textures, eks_builtin_texture_cnt, BuiltinTextureRGBAGenerator, mem_arena, gl_res_arena, temp_mem_arena)) {
            LOG_ERROR("Failed to generate built-in textures for rendering basis!");
            return false;
        }

        {
            const s_shader_prog_gen_info gen_infos[] = {
                [ek_builtin_shader_prog_batch] = {
                    .holds_srcs = true,
                    .vert_src = g_batch_vert_shader_src,
                    .frag_src = g_batch_frag_shader_src
                },
                [ek_builtin_shader_prog_surface_default] = {
                    .holds_srcs = true,
                    .vert_src = g_surface_default_vert_shader_src,
                    .frag_src = g_surface_default_frag_shader_src
                },
                [ek_builtin_shader_prog_surface_blend] = {
                    .holds_srcs = true,
                    .vert_src = g_surface_blend_vert_shader_src,
                    .frag_src = g_surface_blend_frag_shader_src
                }
            };

            if (!InitShaderProgGroup(&basis->builtin_shader_progs, (s_shader_prog_gen_info_array_view)ARRAY_FROM_STATIC(gen_infos), gl_res_arena, temp_mem_arena)) {
                LOG_ERROR("Failed to generate built-in shader programs for rendering basis!");
                return false;
            }
        }

        for (t_s32 i = 0; i < eks_renderable_cnt; i++) {
            s_renderable* const renderable = STATIC_ARRAY_ELEM(basis->renderables, i);

            *renderable = GenRenderableOfType(gl_res_arena, i, temp_mem_arena);

            if (IS_ZERO(*renderable)) {
                LOG_ERROR("Failed to generate renderable of index %d for rendering basis!", i);
                return false;
            }
        }
#endif
    }
}
