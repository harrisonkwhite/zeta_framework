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
    }

    void c_renderer::Clear(const s_v4<float> col) {
        glClearColor(col.x, col.y, col.z, col.w);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void c_renderer::SetViewMatrix(const s_matrix_4x4& mat) {
        Flush();
        m_view_mat = mat;
    }

    void c_renderer::Draw(const s_gfx_resource_handle tex_hdl, const s_rect<float> tex_coords, s_v2<float> pos, s_v2<float> size, s_v2<float> origin, const float rot, const s_v4<float> blend) {
        if (m_slots_used_cnt == 0) {
            // This is the first draw to the batch, so set the texture associated with the batch to the one we're trying to render.
            m_batch_tex_hdl = tex_hdl;
        } else if (m_slots_used_cnt == g_batch_slot_cnt || !tex_hdl.Equals(m_batch_tex_hdl)) {
            // Flush the batch and then try this same render operation again but on a fresh batch.
            Flush();
            Draw(tex_hdl, tex_coords, pos, size, origin, rot, blend);
            return;
        }

        // Write the vertex data to the next slot.
        const s_static_array<s_v2<float>, 4> slot_vert_coords = {{
            {0.0f - origin.x, 0.0f - origin.y},
            {1.0f - origin.x, 0.0f - origin.y},
            {1.0f - origin.x, 1.0f - origin.y},
            {0.0f - origin.x, 1.0f - origin.y}
        }};

        const s_static_array<s_v2<float>, 4> slot_tex_coords = {{
            {tex_coords.Left(), tex_coords.Top()},
            {tex_coords.Right(), tex_coords.Top()},
            {tex_coords.Right(), tex_coords.Bottom()},
            {tex_coords.Left(), tex_coords.Bottom()}
        }};

        t_batch_slot& slot = m_slots[m_slots_used_cnt];

        for (int i = 0; i < slot.Len(); i++) {
            slot[i] = {
                .vert_coord = slot_vert_coords[i],
                .pos = pos,
                .size = size,
                .rot = rot,
                .tex_coord = slot_tex_coords[i],
                .blend = blend
            };
        }

        // Update the count - we've used a slot!
        m_slots_used_cnt++;
    }

    void DrawTexture(const s_texture tex, const s_v2<float> pos, const s_rect<int> src_rect, const s_v2<float> origin, const s_v2<float> scale, const float rot, const s_v4<float> blend) {

    }

    void c_renderer::Flush() {
        if (m_slots_used_cnt == 0) {
            // Nothing to flush!
            return;
        }

#if 0
        //
        // Submitting Vertex Data to GPU
        //
        glBindVertexArray(m_basis.batch_mesh_hdl.mesh.vert_arr_gl_id);
        glBindBuffer(GL_ARRAY_BUFFER, m_basis.batch_mesh_hdl.mesh.vert_buf_gl_id);

        {
            const size_t write_size = sizeof(t_batch_slot) * m_slots_used_cnt;
            glBufferSubData(GL_ARRAY_BUFFER, 0, write_size, m_slots.Raw());
        }

        //
        // Rendering the Batch
        //
        const t_gl_id prog_gl_id = m_basis.batch_shader_prog_hdl.shader_prog.gl_id;

        glUseProgram(prog_gl_id);

        const int view_uniform_loc = glGetUniformLocation(prog_gl_id, "u_view");
        glUniformMatrix4fv(view_uniform_loc, 1, false, reinterpret_cast<float*>(m_view_mat.elems.buf_raw.buf_raw));

        const s_rect<int> viewport = GLViewport();
        const auto proj_mat = s_matrix_4x4::Orthographic(0.0f, viewport.width, viewport.height, 0.0f, -1.0f, 1.0f);

        const int proj_uniform_loc = glGetUniformLocation(prog_gl_id, "u_proj");
        glUniformMatrix4fv(proj_uniform_loc, 1, false, (const t_r32*)proj_mat.elems);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_batch_tex_hdl.tex.gl_id);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_basis.batch_mesh_hdl.mesh.elem_buf_gl_id);
        glDrawElements(GL_TRIANGLES, g_batch_slot_elem_cnt * m_slots_used_cnt, GL_UNSIGNED_SHORT, nullptr);

        glUseProgram(0);

        m_slots_used_cnt = 0;
        m_batch_tex_hdl = {};
#endif
    }
}
