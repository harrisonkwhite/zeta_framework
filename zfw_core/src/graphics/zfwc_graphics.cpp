#include "zfwc_graphics.h"
#include "cu_mem.h"
#include "zfwc_math.h"

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

bool InitGLResourceArena(s_gl_resource_arena& res_arena, c_mem_arena& mem_arena, const t_s32 res_limit) {
    assert(res_limit > 0);

    res_arena.ids = PushArrayToMemArena<t_gl_id>(mem_arena, res_limit);

    if (res_arena.ids.IsEmpty()) {
        //LOG_ERROR("Failed to reserve memory for OpenGL resource IDs!");
        return false;
    }

    res_arena.res_types = PushArrayToMemArena<e_gl_resource_type>(mem_arena, res_limit);

    if (res_arena.res_types.IsEmpty()) {
        //LOG_ERROR("Failed to reserve memory for OpenGL resource types!");
        return false;
    }

    res_arena.res_limit = res_limit;

    return true;
}

void CleanGLResourceArena(s_gl_resource_arena& res_arena) {
    for (t_s32 i = 0; i < res_arena.res_used; i++) {
        const t_gl_id gl_id = res_arena.ids[i];

        if (!gl_id) {
            continue;
        }

        const e_gl_resource_type res_type = res_arena.res_types[i];

        switch (res_type) {
            case ek_gl_resource_type_texture:
                glDeleteTextures(1, &gl_id);
                break;

            case ek_gl_resource_type_shader_prog:
                glDeleteProgram(gl_id);
                break;

            case ek_gl_resource_type_vert_array:
                glDeleteVertexArrays(1, &gl_id);
                break;

            case ek_gl_resource_type_vert_buf:
            case ek_gl_resource_type_elem_buf:
                glDeleteBuffers(1, &gl_id);
                break;

            case ek_gl_resource_type_framebuffer:
                glDeleteFramebuffers(1, &gl_id);
                break;

            default:
                assert(false && "Unhandled OpenGL resource type case!");
                break;
        }
    }
}

t_gl_id PushToGLResourceArena(s_gl_resource_arena& res_arena, const e_gl_resource_type res_type) {
    return 0;
}

c_array<t_gl_id> PushArrayToGLResourceArena(s_gl_resource_arena& res_arena, t_s32 cnt, e_gl_resource_type res_type) {
    if (res_arena.res_used + cnt > res_arena.res_limit) {
        //LOG_ERROR("OpenGL resource arena is full!");
        return {};
    }

    const t_s32 res_used_prev = res_arena.res_used;
    res_arena.res_used += cnt;
    return {res_arena.ids.Raw() + res_used_prev, cnt};
}

static size_t CalcStride(const c_array<const t_s32> vert_attr_lens) {
    t_s32 stride = 0;

    for (t_s32 i = 0; i < vert_attr_lens.Len(); i++) {
        stride += sizeof(float) * vert_attr_lens[i];
    }

    return stride;
}

static s_renderable GenRenderable(s_gl_resource_arena& gl_res_arena, const c_array<const float> verts, const c_array<const t_u16> elems, const c_array<const t_s32> vert_attr_lens) {
    t_gl_id va_gl_id = PushToGLResourceArena(gl_res_arena, ek_gl_resource_type_vert_array);

    if (!va_gl_id) {
        //LOG_ERROR("Failed to reserve OpenGL vertex array ID for renderable!");
        return {};
    }

    t_gl_id vb_gl_id = PushToGLResourceArena(gl_res_arena, ek_gl_resource_type_vert_buf);

    if (!vb_gl_id) {
        //LOG_ERROR("Failed to reserve OpenGL vertex buffer ID for renderable!");
        return {};
    }

    t_gl_id eb_gl_id = PushToGLResourceArena(gl_res_arena, ek_gl_resource_type_elem_buf);

    if (!eb_gl_id) {
        //LOG_ERROR("Failed to reserve OpenGL element buffer ID for renderable!");
        return {};
    }

    glGenVertexArrays(1, &va_gl_id);
    glBindVertexArray(va_gl_id);

    glGenBuffers(1, &vb_gl_id);
    glBindBuffer(GL_ARRAY_BUFFER, vb_gl_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(*verts.Raw()) * verts.Len(), verts.Raw(), GL_DYNAMIC_DRAW);

    glGenBuffers(1, &eb_gl_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eb_gl_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*elems.Raw()) * elems.Len(), elems.Raw(), GL_STATIC_DRAW);

    const size_t stride = CalcStride(vert_attr_lens);
    t_s32 offs = 0;

    for (t_s32 i = 0; i < vert_attr_lens.Len(); i++) {
        const auto attr_len = vert_attr_lens[i];

        glVertexAttribPointer(i, attr_len, GL_FLOAT, false, stride, reinterpret_cast<void*>(sizeof(float) * offs));
        glEnableVertexAttribArray(i);

        offs += attr_len;
    }

    glBindVertexArray(0);

    return {
        .vert_array_gl_id = va_gl_id,
        .vert_buf_gl_id = vb_gl_id,
        .elem_buf_gl_id = eb_gl_id
    };
}

static c_array<const t_u16> GenBatchRenderableElems(c_mem_arena& mem_arena) {
    const auto elems = PushArrayToMemArena<t_u16>(mem_arena, g_batch_slot_elem_cnt * g_batch_slot_cnt);

    if (elems.IsEmpty()) {
        //LOG_ERROR("Failed to reserve memory for batch renderable elements!");
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

static s_renderable GenRenderableOfType(s_gl_resource_arena& gl_res_arena, e_renderable type, c_mem_arena& temp_mem_arena) {
    switch (type) {
        case ek_renderable_batch: {
            const auto verts = c_array<const float>{nullptr, g_batch_slot_vert_len * g_batch_slot_vert_cnt * g_batch_slot_cnt};

            const auto elems = GenBatchRenderableElems(temp_mem_arena);

            if (elems.IsEmpty()) {
                return {};
            }

            const c_static_array<t_s32, 6> vert_attr_lens = {{2, 2, 2, 1, 2, 4}};

            return GenRenderable(gl_res_arena, verts, elems, vert_attr_lens.Nonstatic());
        }

        case ek_renderable_surface: {
            const c_static_array<float, 16> verts = {{
                0.0f, 1.0f, 0.0f, 0.0f,
                1.0f, 1.0f, 1.0f, 0.0f,
                1.0f, 0.0f, 1.0f, 1.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            }};

            const c_static_array<t_u16, 6> elems = {{
                0, 1, 2,
                2, 3, 0
            }};

            const c_static_array<t_s32, 2> vert_attr_lens = {{2, 2}};

            return GenRenderable(gl_res_arena, verts.Nonstatic(), elems.Nonstatic(), vert_attr_lens.Nonstatic());
        }

        default:
            assert(false && "Unhandled renderable case!");
            return {};
    }
}

static s_rgba_texture BuiltinTextureRGBAGenerator(const t_s32 tex_index, c_mem_arena& mem_arena) {
    switch (static_cast<e_builtin_texture>(tex_index)) {
        case ek_builtin_texture_pixel: {
            const auto px_data = PushArrayToMemArena<t_u8>(mem_arena, 4);

            if (px_data.IsEmpty()) {
                return {};
            }

            px_data[0] = 255;
            px_data[1] = 255;
            px_data[2] = 255;
            px_data[3] = 255;

            return {{1, 1}, px_data};
        }

        default:
            assert(false && "Unhandled built-in texture case!");
            return {};
    }
}

bool InitRenderingBasis(s_rendering_basis& basis, s_gl_resource_arena& gl_res_arena, c_mem_arena& mem_arena, c_mem_arena& temp_mem_arena) {
    /*if (!InitTextureGroup(basis.builtin_textures, eks_builtin_texture_cnt, BuiltinTextureRGBAGenerator, mem_arena, gl_res_arena, temp_mem_arena)) {
        //LOG_ERROR("Failed to generate built-in textures for rendering basis!");
        return false;
    }

    {
        s_shader_prog_gen_info gen_infos[eks_builtin_shader_prog_cnt] = {};
        gen_infos[ek_builtin_shader_prog_batch] = (s_shader_prog_gen_info){true, g_batch_vert_shader_src, g_batch_frag_shader_src};
        gen_infos[ek_builtin_shader_prog_surface_default] = {true, g_surface_default_vert_shader_src, g_surface_default_frag_shader_src};
        gen_infos[ek_builtin_shader_prog_surface_blend] = {true, g_surface_blend_vert_shader_src, g_surface_blend_frag_shader_src};

        if (!InitShaderProgGroup(basis.builtin_shader_progs, {gen_infos, eks_builtin_shader_prog_cnt}, gl_res_arena, temp_mem_arena)) {
            //LOG_ERROR("Failed to generate built-in shader programs for rendering basis!");
            return false;
        }
    }

    for (t_s32 i = 0; i < eks_renderable_cnt; i++) {
        basis.renderables[i] = GenRenderableOfType(gl_res_arena, static_cast<e_renderable>(i), temp_mem_arena);

        if (basis.renderables[i].IsZero()) {
            //LOG_ERROR("Failed to generate renderable of index %d for rendering basis!", i);
            return false;
        }
    }*/

    return true;
}

void InitRenderingState(s_rendering_state& state, const s_v2_s32 window_size) {
    state.view_mat = s_matrix_4x4::Identity();
    glViewport(0, 0, window_size.x, window_size.y);
}

void Clear(const s_rendering_context& rendering_context, const u_v4 col) {
    glClearColor(col.r, col.g, col.b, col.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void SetViewMatrix(const s_rendering_context& rendering_context, const s_matrix_4x4& mat) {
    SubmitBatch(rendering_context);
    rendering_context.state.view_mat = mat;
}

static void WriteBatchSlot(t_batch_slot& slot, const s_batch_slot_write_info& write_info) {
    const c_static_array<s_v2, 4> vert_coords = {{
        {0.0f - write_info.origin.x, 0.0f - write_info.origin.y},
        {1.0f - write_info.origin.x, 0.0f - write_info.origin.y},
        {1.0f - write_info.origin.x, 1.0f - write_info.origin.y},
        {0.0f - write_info.origin.x, 1.0f - write_info.origin.y}
    }};

    const c_static_array<s_v2, 4> tex_coords = {{
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

void Render(const s_rendering_context& rendering_context, const s_batch_slot_write_info& write_info) {
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

void SubmitBatch(const s_rendering_context& rendering_context) {
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
