#include <zcl/zcl_untitled.h>

#include <glad/glad.h>

namespace zf::gfx {
    using t_gl_id = GLuint;

    struct s_context {
        s_resource_arena resource_arena = {};
    };

    struct s_resource {
    public:
        auto &Mesh() const {
            return type_data.mesh;
        }

    private:
        union {
            struct {
                t_gl_id vert_arr_gl_id;
                t_gl_id vert_buf_gl_id;
                t_gl_id elem_buf_gl_id;
            } mesh;

            struct {
                t_gl_id gl_id;
            } shader_prog;

            struct {
                t_gl_id gl_id;
            } texture;
        } type_data;
    };

    [[nodiscard]] static t_b8 PushResource(const s_resource_arena &arena, s_ptr<s_resource> &o_res) {
    }

    [[nodiscard]] t_b8 CreateMesh(const s_context &context, const s_ptr<const t_f32> verts_ptr, const t_len verts_len, const s_array_rdonly<t_u16> elems, const s_array_rdonly<t_i32> vert_attr_lens, s_ptr<s_resource> &o_mesh, s_ptr<s_resource_arena> arena) {
        if (!PushResource(arena ? *arena : context.resource_arena, o_mesh)) {
            return false;
        }

        glGenVertexArrays(1, &o_mesh.Mesh().vert_arr_gl_id);
        glBindVertexArray(o_mesh.Mesh().vert_arr_gl_id);

        glGenBuffers(1, &o_mesh.Mesh().vert_buf_gl_id);
        glBindBuffer(GL_ARRAY_BUFFER, o_mesh.Mesh().vert_buf_gl_id);
        glBufferData(GL_ARRAY_BUFFER, ZF_SIZE_OF(t_f32) * verts_len, verts_ptr, GL_DYNAMIC_DRAW);

        glGenBuffers(1, &o_mesh.Mesh().elem_buf_gl_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, o_mesh.Mesh().elem_buf_gl_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(elems.SizeInBytes()), elems.Ptr(), GL_STATIC_DRAW);

        const t_len stride = [vert_attr_lens]() {
            t_len res = 0;

            for (t_len i = 0; i < vert_attr_lens.Len(); i++) {
                res += ZF_SIZE_OF(t_f32) * static_cast<t_len>(vert_attr_lens[i]);
            }

            return res;
        }();

        t_i32 offs = 0;

        for (t_len i = 0; i < vert_attr_lens.Len(); i++) {
            const t_i32 attr_len = vert_attr_lens[i];

            glVertexAttribPointer(static_cast<GLuint>(i), attr_len, GL_FLOAT, false, static_cast<GLsizei>(stride), reinterpret_cast<void *>(ZF_SIZE_OF(t_f32) * offs));
            glEnableVertexAttribArray(static_cast<GLuint>(i));

            offs += attr_len;
        }

        glBindVertexArray(0);

        return gl_ids;
    }

    void SetViewport(const s_context &context) {
        glViewport();
    }

    void Clear(const s_context &context, const s_color_rgb24f col) {
        glClearColor(col.R(), col.G(), col.B(), col.A());
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void RenderMesh(const s_context &context, const s_resource &mesh, const s_resource &shader_prog, const s_ptr<const s_resource> tex) {
        //
        // Submitting Vertex Data to GPU
        //
        glBindVertexArray(rc.basis->batch_mesh_gl_ids.vert_arr);
        glBindBuffer(GL_ARRAY_BUFFER, rc.basis->batch_mesh_gl_ids.vert_buf);

        {
            const t_len write_size = ZF_SIZE_OF(t_batch_slot) * rc.state->batch_slots_used_cnt;
            glBufferSubData(GL_ARRAY_BUFFER, 0, write_size, rc.state->batch_slots.raw);
        }

        //
        // Rendering the Batch
        //
        glUseProgram(rc.basis->batch_shader_prog_gl_id);

        const t_i32 view_uniform_loc = glGetUniformLocation(rc.basis->batch_shader_prog_gl_id, "u_view");
        glUniformMatrix4fv(view_uniform_loc, 1, false, reinterpret_cast<const t_f32 *>(&rc.state->view_mat));

        const auto proj_mat = CreateOrthographicMatrix(0.0f, static_cast<t_f32>(rc.framebuffer_size_cache.x), static_cast<t_f32>(rc.framebuffer_size_cache.y), 0.0f, -1.0f, 1.0f);
        const t_i32 proj_uniform_loc = glGetUniformLocation(rc.basis->batch_shader_prog_gl_id, "u_proj");

        glUniformMatrix4fv(proj_uniform_loc, 1, false, reinterpret_cast<const t_f32 *>(&proj_mat));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rc.state->tex_gl_id);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rc.basis->batch_mesh_gl_ids.elem_buf);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(g_batch_slot_elem_cnt * rc.state->batch_slots_used_cnt), GL_UNSIGNED_SHORT, nullptr);

        glUseProgram(0);
    }
}
