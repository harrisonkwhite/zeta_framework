#include <zf/zf_rendering.h>

namespace zf {
    static constexpr s_str_rdonly g_batch_vert_shader_src = StrFromRawTerminated(R"(#version 460 core

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

    static constexpr s_str_rdonly g_batch_frag_shader_src = StrFromRawTerminated(R"(#version 460 core

in vec2 v_tex_coord;
in vec4 v_blend;

out vec4 o_frag_color;

uniform sampler2D u_tex;

void main() {
    vec4 tex_color = texture(u_tex, v_tex_coord);
    o_frag_color = tex_color * v_blend;
}
)");

    static c_gfx_resource_handle MakeBatchMesh(c_gfx_resource_arena& gfx_res_arena, s_mem_arena& temp_mem_arena) {
        const t_size verts_len = g_batch_vert_component_cnt * g_batch_slot_vert_cnt * g_batch_slot_cnt;

        s_array<t_u16> elems;

        if (!MakeArray(temp_mem_arena, g_batch_slot_elem_cnt * g_batch_slot_cnt, elems)) {
            ZF_REPORT_FAILURE();
            return {};
        }

        for (t_size i = 0; i < g_batch_slot_cnt; i++) {
            elems[(i * 6) + 0] = static_cast<t_u16>((i * 4) + 0);
            elems[(i * 6) + 1] = static_cast<t_u16>((i * 4) + 1);
            elems[(i * 6) + 2] = static_cast<t_u16>((i * 4) + 2);
            elems[(i * 6) + 3] = static_cast<t_u16>((i * 4) + 2);
            elems[(i * 6) + 4] = static_cast<t_u16>((i * 4) + 3);
            elems[(i * 6) + 5] = static_cast<t_u16>((i * 4) + 0);
        }

        return gfx_res_arena.AddMesh(nullptr, verts_len, elems, g_batch_vert_attr_lens);
    }

    t_b8 s_rendering_basis::Init(c_gfx_resource_arena& gfx_res_arena, s_mem_arena& temp_mem_arena) {
        // Generate the batch mesh.
        batch_mesh_hdl = MakeBatchMesh(gfx_res_arena, temp_mem_arena);

        if (!batch_mesh_hdl.IsValid()) {
            ZF_REPORT_FAILURE();
            return false;
        }

        // Generate batch shader program.
        batch_shader_prog_hdl = gfx_res_arena.AddShaderProg(g_batch_vert_shader_src, g_batch_frag_shader_src, temp_mem_arena);

        if (!batch_shader_prog_hdl.IsValid()) {
            ZF_REPORT_FAILURE();
            return false;
        }

        // Generate the pixel texture.
        s_static_array<t_u8, 4> px_rgba = {
            {255, 255, 255, 255}
        };

        if (!px_tex.Load({{1, 1}, px_rgba}, gfx_res_arena)) {
            ZF_REPORT_FAILURE();
            return false;
        }

        return true;
    }

    t_b8 c_renderer::Init(c_gfx_resource_arena& gfx_res_arena, s_mem_arena& mem_arena, s_mem_arena& temp_mem_arena) {
        if (!m_basis.Init(gfx_res_arena, temp_mem_arena)) {
            ZF_REPORT_FAILURE();
            return false;
        }

        if (!MakeArray(mem_arena, g_batch_slot_cnt, m_batch_slots)) {
            ZF_REPORT_FAILURE();
            return false;
        }

        return true;
    }

    void c_renderer::Clear(const s_color_rgba32f col) const {
        glClearColor(col.R(), col.G(), col.B(), 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void c_renderer::SetViewMatrix(const s_matrix_4x4& mat) {
        Flush();
        m_batch_view_mat = mat;
    }

    void c_renderer::Draw(const c_gfx_resource_handle tex_hdl, const s_rect<t_f32> tex_coords, s_v2<t_f32> pos, s_v2<t_f32> size, s_v2<t_f32> origin, const t_f32 rot, const s_color_rgba32f blend) {
        ZF_ASSERT(tex_hdl.IsValid());

        if (m_batch_slots_used_cnt == 0) {
            // This is the first draw to the batch, so set the texture associated with the batch to the one we're trying to render.
            m_batch_tex_hdl = tex_hdl;
        } else if (m_batch_slots_used_cnt == g_batch_slot_cnt || !tex_hdl.Equals(m_batch_tex_hdl)) {
            // Flush the batch and then try this same render operation again but on a fresh batch.
            Flush();
            Draw(tex_hdl, tex_coords, pos, size, origin, rot, blend);
            return;
        }

        // Write the vertex data to the next slot.
        const s_static_array<s_v2<t_f32>, 4> vert_coords = {{
            {0.0f - origin.x, 0.0f - origin.y},
            {1.0f - origin.x, 0.0f - origin.y},
            {1.0f - origin.x, 1.0f - origin.y},
            {0.0f - origin.x, 1.0f - origin.y}
        }};

        const s_static_array<s_v2<t_f32>, 4> tex_coords_per_vert = {{
            {tex_coords.Left(), tex_coords.Top()},
            {tex_coords.Right(), tex_coords.Top()},
            {tex_coords.Right(), tex_coords.Bottom()},
            {tex_coords.Left(), tex_coords.Bottom()}
        }};

        t_batch_slot& slot = m_batch_slots[m_batch_slots_used_cnt];

        for (t_size i = 0; i < slot.Len(); i++) {
            slot[i] = {
                .vert_coord = vert_coords[i],
                .pos = pos,
                .size = size,
                .rot = rot,
                .tex_coord = tex_coords_per_vert[i],
                .blend = blend
            };
        }

        // Update the count - we've used a slot!
        m_batch_slots_used_cnt++;
    }

    s_rect<t_f32> MakeTextureCoords(const s_rect<t_s32> src_rect, const s_v2<t_s32> tex_size) {
        const s_v2<t_f32> half_texel = {
            0.5f / static_cast<t_f32>(tex_size.x),
            0.5f / static_cast<t_f32>(tex_size.y)
        };

        return {
            (static_cast<t_f32>(src_rect.x) + half_texel.x) / static_cast<t_f32>(tex_size.x),
            (static_cast<t_f32>(src_rect.y) + half_texel.y) / static_cast<t_f32>(tex_size.y),
            (static_cast<t_f32>(src_rect.width) - half_texel.x) / static_cast<t_f32>(tex_size.x),
            (static_cast<t_f32>(src_rect.height) - half_texel.y) / static_cast<t_f32>(tex_size.y)
        };
    }

    void c_renderer::DrawTexture(const s_texture& tex, const s_v2<t_f32> pos, const s_rect<t_s32> src_rect, const s_v2<t_f32> origin, const s_v2<t_f32> scale, const t_f32 rot, const s_color_rgba32f blend) {
        ZF_ASSERT(tex.hdl.IsValid());
        ZF_ASSERT(origin.x >= 0.0f && origin.x <= 1.0f && origin.y >= 0.0f && origin.y <= 1.0f); // @todo: Generic function for this check?
        // @todo: Add more assertions here!

        s_rect<t_s32> src_rect_to_use;

        if (src_rect == s_rect<t_s32>()) {
            // If the source rectangle wasn't set, just go with the whole texture.
            src_rect_to_use = {0, 0, tex.size_cache.x, tex.size_cache.y};
        } else {
            ZF_ASSERT_MSG(src_rect.Left() >= 0 && src_rect.Top() >= 0 && src_rect.Right() <= tex.size_cache.x && src_rect.Top() <= tex.size_cache.y, "Invalid source rectangle!");
            src_rect_to_use = src_rect;
        }

        const s_rect tex_coords = MakeTextureCoords(src_rect_to_use, tex.size_cache);

        const s_v2<t_f32> size = {
            static_cast<t_f32>(src_rect_to_use.width) * scale.x, static_cast<t_f32>(src_rect_to_use.height) * scale.y
        };

        Draw(tex.hdl, tex_coords, pos, size, origin, rot, blend);
    }

    void c_renderer::Flush() {
        if (m_batch_slots_used_cnt == 0) {
            // Nothing to flush!
            return;
        }

        //
        // Submitting Vertex Data to GPU
        //
        glBindVertexArray(m_basis.batch_mesh_hdl.Mesh().vert_arr_gl_id);
        glBindBuffer(GL_ARRAY_BUFFER, m_basis.batch_mesh_hdl.Mesh().vert_buf_gl_id);

        {
            const t_size write_size = ZF_SIZE_OF(t_batch_slot) * m_batch_slots_used_cnt;
            glBufferSubData(GL_ARRAY_BUFFER, 0, write_size, m_batch_slots.Raw());
        }

        //
        // Rendering the Batch
        //
        const t_gl_id prog_gl_id = m_basis.batch_shader_prog_hdl.ShaderProg().gl_id;

        glUseProgram(prog_gl_id);

        const t_s32 view_uniform_loc = glGetUniformLocation(prog_gl_id, "u_view");
        glUniformMatrix4fv(view_uniform_loc, 1, false, m_batch_view_mat.Raw());

        const s_rect<t_s32> viewport = []() {
            s_rect<t_s32> vp;
            glGetIntegerv(GL_VIEWPORT, reinterpret_cast<t_s32*>(&vp));
            return vp;
        }();

        const auto proj_mat = s_matrix_4x4::Orthographic(0.0f, static_cast<t_f32>(viewport.width), static_cast<t_f32>(viewport.height), 0.0f, -1.0f, 1.0f);
        const t_s32 proj_uniform_loc = glGetUniformLocation(prog_gl_id, "u_proj");
        glUniformMatrix4fv(proj_uniform_loc, 1, false, proj_mat.Raw());

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_batch_tex_hdl.Texture().gl_id);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_basis.batch_mesh_hdl.Mesh().elem_buf_gl_id);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(g_batch_slot_elem_cnt * m_batch_slots_used_cnt), GL_UNSIGNED_SHORT, nullptr);

        glUseProgram(0);

        m_batch_slots_used_cnt = 0;
    }
}
