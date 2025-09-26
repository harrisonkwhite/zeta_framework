#include "zf_rendering.h"

namespace zf {
#if 0
    static const char g_batch_vert_shader_src[] = "#version 430 core\n"
        "\n"
        "layout (location = 0) in vec2 a_vert;\n"
        "layout (location = 1) in vec2 a_pos;\n"
        "layout (location = 2) in vec2 a_size;\n"
        "layout (location = 3) in float a_rot;\n"
        "layout (location = 4) in vec2 a_tex_coord;\n"
        "layout (location = 5) in vec4 a_blend;\n"
        "\n"
        "out vec2 v_tex_coord;\n"
        "out vec4 v_blend;\n"
        "\n"
        "uniform mat4 u_view;\n"
        "uniform mat4 u_proj;\n"
        "\n"
        "void main() {\n"
        "    float rot_cos = cos(a_rot);\n"
        "    float rot_sin = -sin(a_rot);\n"
        "\n"
        "    mat4 model = mat4(\n"
        "        vec4(a_size.x * rot_cos, a_size.x * rot_sin, 0.0, 0.0),\n"
        "        vec4(a_size.y * -rot_sin, a_size.y * rot_cos, 0.0, 0.0),\n"
        "        vec4(0.0, 0.0, 1.0, 0.0),\n"
        "        vec4(a_pos.x, a_pos.y, 0.0, 1.0)\n"
        "    );\n"
        "\n"
        "    gl_Position = u_proj * u_view * model * vec4(a_vert, 0.0, 1.0);\n"
        "    v_tex_coord = a_tex_coord;\n"
        "    v_blend = a_blend;\n"
        "}\n";

    static const char g_batch_frag_shader_src[] = "#version 430 core\n"
        "\n"
        "in vec2 v_tex_coord;\n"
        "in vec4 v_blend;\n"
        "\n"
        "out vec4 o_frag_color;\n"
        "\n"
        "uniform sampler2D u_tex;\n"
        "\n"
        "void main() {\n"
        "    vec4 tex_color = texture(u_tex, v_tex_coord);\n"
        "    o_frag_color = tex_color * v_blend;\n"
        "}\n";

    static c_array<const t_u16> GenBatchRenderableElems(c_mem_arena& mem_arena) {
        const auto elems = PushArrayToMemArena<t_u16>(mem_arena, g_batch_slot_elem_cnt * g_batch_slot_cnt);

        if (elems.IsEmpty()) {
            ZF_LOG_ERROR("Failed to reserve memory for batch renderable elements!");
            return {};
        }

        for (t_s32 i = 0; i < g_batch_slot_cnt; i++) {
            elems[(i * 6) + 0] = (i * 4) + 0;
            elems[(i * 6) + 1] = (i * 4) + 1;
            elems[(i * 6) + 2] = (i * 4) + 2;
            elems[(i * 6) + 3] = (i * 4) + 2;
            elems[(i * 6) + 4] = (i * 4) + 3;
            elems[(i * 6) + 5] = (i * 4) + 0;
        }

        return elems.View();
    }

    [[nodiscard]] static bool GenRenderableOfType(s_renderable& renderable, s_gl_resource_arena& gl_res_arena, const e_renderable type, c_mem_arena& temp_mem_arena) {
        switch (type) {
            case ek_renderable_batch: {
                const auto verts = c_array<const float>{nullptr, g_batch_slot_vert_len * g_batch_slot_vert_cnt * g_batch_slot_cnt};

                const auto elems = GenBatchRenderableElems(temp_mem_arena);

                if (elems.IsEmpty()) {
                    return {};
                }

                const s_static_array<t_s32, 6> vert_attr_lens = {{2, 2, 2, 1, 2, 4}};

                return GenRenderable(renderable, gl_res_arena, verts, elems, vert_attr_lens.Nonstatic());
            }

            case ek_renderable_surface: {
                const s_static_array<float, 16> verts = {{
                    0.0f, 1.0f, 0.0f, 0.0f,
                    1.0f, 1.0f, 1.0f, 0.0f,
                    1.0f, 0.0f, 1.0f, 1.0f,
                    0.0f, 0.0f, 0.0f, 1.0f
                }};

                const s_static_array<t_u16, 6> elems = {{
                    0, 1, 2,
                    2, 3, 0
                }};

                const s_static_array<t_s32, 2> vert_attr_lens = {{2, 2}};

                return GenRenderable(renderable, gl_res_arena, verts.Nonstatic(), elems.Nonstatic(), vert_attr_lens.Nonstatic());
            }

            default:
                assert(false && "Unhandled renderable case!");
                return {};
        }
    }

    static bool BuiltinTextureRGBALoader(s_rgba_texture& rgba, const t_s32 tex_index, c_mem_arena& mem_arena) {
        switch (static_cast<e_builtin_texture>(tex_index)) {
            case ek_builtin_texture_pixel: {
                rgba.px_data = PushArrayToMemArena<t_u8>(mem_arena, 4);

                if (rgba.px_data.IsEmpty()) {
                    return false;
                }

                rgba.px_data[0] = 255;
                rgba.px_data[1] = 255;
                rgba.px_data[2] = 255;
                rgba.px_data[3] = 255;

                rgba.tex_size = {1, 1};

                return true;
            }

            default:
                assert(false && "Unhandled built-in texture case!");
                return {};
        }
    }

    static void WriteBatchSlot(t_batch_slot& slot, const s_batch_slot_write_info& write_info) {
        const s_static_array<s_v2, 4> vert_coords = {{
            {0.0f - write_info.origin.x, 0.0f - write_info.origin.y},
            {1.0f - write_info.origin.x, 0.0f - write_info.origin.y},
            {1.0f - write_info.origin.x, 1.0f - write_info.origin.y},
            {0.0f - write_info.origin.x, 1.0f - write_info.origin.y}
        }};

        const s_static_array<s_v2, 4> tex_coords = {{
            {write_info.tex_coords.left, write_info.tex_coords.top},
            {write_info.tex_coords.right, write_info.tex_coords.top},
            {write_info.tex_coords.right, write_info.tex_coords.bottom},
            {write_info.tex_coords.left, write_info.tex_coords.bottom}
        }};

        for (t_s32 i = 0; i < slot.Len(); i++) {
            slot[i] = {
                vert_coords[i],
                write_info.pos,
                write_info.size,
                write_info.rot,
                tex_coords[i],
                write_info.blend
            };
        }
    }
#endif

    void c_renderer::Clear(const s_v4 col) {
        glClearColor(col.x, col.y, col.z, col.w);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void c_renderer::SetViewMatrix(const s_matrix_4x4& mat) {
        SubmitBatch(rendering_context);
        rendering_context.state.view_mat = mat;
    }

    void c_renderer::Render(const s_batch_slot_write_info& write_info) {
        auto& batch_state = rendering_context.state.batch;

        if (batch_state.num_slots_used == 0) {
            batch_state.tex_gl_id = write_info.tex_gl_id;
        } else if (batch_state.num_slots_used == g_batch_slot_cnt || write_info.tex_gl_id != batch_state.tex_gl_id) {
            SubmitBatch(rendering_context);
            Render(rendering_context, write_info);
            return;
        }

        auto& slot = batch_state.slots[batch_state.num_slots_used];
        slot = {};
        WriteBatchSlot(slot, write_info);
        batch_state.num_slots_used++;
    }

    void c_renderer::RenderTexture(const s_rendering_context& rendering_context, const s_texture_group& textures, const t_s32 tex_index, const s_rect_s32 src_rect, const s_v2 pos, const s_v2 origin, const s_v2 scale, const float rot, const s_v4 blend) {
        assert(origin.x >= 0.0f && origin.x <= 1.0f && origin.y >= 0.0f && origin.y <= 1.0f);

        const s_v2_s32 tex_size = textures.sizes[tex_index];

        s_rect_s32 src_rect_to_use;

        if (src_rect == (s_rect_s32){}) {
            src_rect_to_use = {0, 0, tex_size.x, tex_size.y};
        } else {
            src_rect_to_use = src_rect;
            assert(src_rect.x + src_rect.width <= tex_size.x && src_rect.y + src_rect.height <= tex_size.y);
        }

        const s_batch_slot_write_info write_info = {
            .tex_gl_id = textures.gl_ids[tex_index],
            .tex_coords = GenTextureCoords(src_rect_to_use, tex_size),
            .pos = pos,
            .size = {src_rect_to_use.width * scale.x, src_rect_to_use.height * scale.y},
            .origin = origin,
            .rot = rot,
            .blend = blend
        };

        Render(rendering_context, write_info);
    }

    void c_renderer::RenderRect(const s_rendering_context& rendering_context, const s_rect rect, const s_v4 color) {
        RenderTexture(rendering_context.basis.builtin_textures, ek_builtin_texture_pixel, {}, rect.Pos(), {}, rect.Size(), 0, color);
    }

    static inline s_rect InnerRect(const s_rect rect, const float outline_thickness) {
        return s_rect{
            rect.x + outline_thickness,
            rect.y + outline_thickness,
            rect.width - (outline_thickness * 2.0f),
            rect.height - (outline_thickness * 2.0f)
        };
    }

    void c_renderer::RenderRectWithOutline(const s_rendering_context& rendering_context, const s_rect rect, const s_v4 fill_color, const s_v4 outline_color, const float outline_thickness) {
        // Top Outline
        RenderRect(rendering_context, {rect.x, rect.y, rect.width - outline_thickness, outline_thickness}, outline_color);

        // Right Outline
        RenderRect(rendering_context, {rect.x + rect.width - outline_thickness, rect.y, outline_thickness, rect.height - outline_thickness}, outline_color);

        // Bottom Outline
        RenderRect(rendering_context, {rect.x + outline_thickness, rect.y + rect.height - outline_thickness, rect.width - outline_thickness, outline_thickness}, outline_color);

        // Left Outline
        RenderRect(rendering_context, {rect.x, rect.y + outline_thickness, outline_thickness, rect.height - outline_thickness}, outline_color);

        // Inside
        RenderRect(rendering_context, InnerRect(rect, outline_thickness), fill_color);
    }

    void RenderRectWithOutlineAndOpaqueFill(const s_rendering_context& rendering_context, const s_rect rect, const s_v3 fill_color, const s_v4 outline_color, const float outline_thickness) {
        // Outline
        RenderRect(rendering_context, rect, outline_color);

        // Inside
        RenderRect(rendering_context, InnerRect(rect, outline_thickness), s_v4{fill_color.x, fill_color.y, fill_color.z, 1.0f});
    }

    void c_renderer::RenderBarHor(const s_rendering_context& rendering_context, const s_rect rect, const float perc, const s_v4 front_color, const s_v4 bg_color) {
        assert(perc >= 0.0f && perc <= 1.0f);

        const float front_rect_width = rect.width * perc;

        if (front_rect_width > 0.0f) {
            RenderRect(rendering_context, s_rect{rect.x, rect.y, front_rect_width, rect.height}, front_color);
        }

        const float bg_rect_x = rect.x + front_rect_width;
        const float bg_rect_width = rect.width - front_rect_width;

        if (bg_rect_width > 0.0f) {
            RenderRect(rendering_context, s_rect{bg_rect_x, rect.y, bg_rect_width, rect.height}, bg_color);
        }
    }

    void c_renderer::RenderBarVertical(const s_rendering_context& rendering_context, const s_rect rect, const float perc, const s_v4 front_color, const s_v4 bg_color) {
        assert(perc >= 0.0f && perc <= 1.0f);

        const float front_rect_height = rect.height * perc;

        if (front_rect_height > 0.0f) {
            RenderRect(rendering_context, s_rect{rect.x, rect.y, rect.width, front_rect_height}, front_color);
        }

        const float bg_rect_y = rect.y + front_rect_height;
        const float bg_rect_height = rect.height - front_rect_height;

        if (bg_rect_height > 0.0f) {
            RenderRect(rendering_context, s_rect{rect.x, bg_rect_y, rect.width, bg_rect_height}, bg_color);
        }
    }

    void c_renderer::RenderLine(const s_rendering_context& rendering_context, s_v2 a, s_v2 b, s_v4 blend, float width) {
        const s_v2 diff{b.x - a.x, b.y - a.y};
        const float len = sqrtf((diff.x * diff.x) + (diff.y * diff.y));

        RenderTexture(rendering_context, rendering_context.basis.builtin_textures, ek_builtin_texture_pixel, {}, a, s_v2{0.0f, 0.5f}, s_v2{len, width}, atan2f(-diff.y, diff.x), blend);
    }

    void c_renderer::Flush() {
        if (rendering_context.state.batch.num_slots_used == 0) {
            return;
        }

        const auto& renderable = rendering_context.basis.renderables[ek_renderable_batch];

        glBindVertexArray(renderable.vert_array_gl_id);
        glBindBuffer(GL_ARRAY_BUFFER, renderable.vert_buf_gl_id);

        const size_t write_size = sizeof(t_batch_slot) * rendering_context.state.batch.num_slots_used;
        glBufferSubData(GL_ARRAY_BUFFER, 0, write_size, rendering_context.state.batch.slots);

        const t_gl_id prog_gl_id = rendering_context.basis.builtin_shader_progs.gl_ids[ek_builtin_shader_prog_batch];

        glUseProgram(prog_gl_id);

        const t_s32 view_uniform_loc = glGetUniformLocation(prog_gl_id, "u_view");
        glUniformMatrix4fv(view_uniform_loc, 1, false, &rendering_context.state.view_mat.elems[0][0]);

        const s_rect_s32 viewport = GLViewport();
        const s_matrix_4x4 proj_mat = s_matrix_4x4::Orthographic(0.0f, viewport.width, viewport.height, 0.0f, -1.0f, 1.0f);

        const t_s32 proj_uniform_loc = glGetUniformLocation(prog_gl_id, "u_proj");
        glUniformMatrix4fv(proj_uniform_loc, 1, false, &proj_mat.elems[0][0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rendering_context.state.batch.tex_gl_id);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderable.elem_buf_gl_id);
        glDrawElements(GL_TRIANGLES, g_batch_slot_elem_cnt * rendering_context.state.batch.num_slots_used, GL_UNSIGNED_SHORT, nullptr);

        glUseProgram(0);

        rendering_context.state.batch.num_slots_used = 0;
        rendering_context.state.batch.tex_gl_id = 0;
    }
}
