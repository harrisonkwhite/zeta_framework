#include "zfw_graphics.h"

static zfw_s_batch_shader_prog LoadBatchShaderProg() {
    static_assert(ZFW_GL_VERSION_MAJOR == 4 && ZFW_GL_VERSION_MINOR == 3, "Invalid OpenGL version for the below scripts!");

    const char* const vert_shader_src = "#version 430 core\n"
        "layout (location = 0) in vec2 a_vert;\n"
        "layout (location = 1) in vec2 a_pos;\n"
        "layout (location = 2) in vec2 a_size;\n"
        "layout (location = 3) in float a_rot;\n"
        "layout (location = 4) in vec2 a_tex_coord;\n"
        "layout (location = 5) in vec4 a_blend;\n"
        "out vec2 v_tex_coord;\n"
        "out vec4 v_blend;\n"
        "uniform mat4 u_view;\n"
        "uniform mat4 u_proj;\n"
        "void main() {\n"
        "    float rot_cos = cos(a_rot);\n"
        "    float rot_sin = -sin(a_rot);\n"
        "    mat4 model = mat4(\n"
        "        vec4(a_size.x * rot_cos, a_size.x * rot_sin, 0.0, 0.0),\n"
        "        vec4(a_size.y * -rot_sin, a_size.y * rot_cos, 0.0, 0.0),\n"
        "        vec4(0.0, 0.0, 1.0, 0.0),\n"
        "        vec4(a_pos.x, a_pos.y, 0.0, 1.0));\n"
        "    gl_Position = u_proj * u_view * model * vec4(a_vert, 0.0, 1.0);\n"
        "    v_tex_coord = a_tex_coord;\n"
        "    v_blend = a_blend;\n"
        "}";

    const char* const frag_shader_src = "#version 430 core\n"
        "in vec2 v_tex_coord;\n"
        "in vec4 v_blend;\n"
        "out vec4 o_frag_color;\n"
        "uniform sampler2D u_tex;\n"
        "void main() {\n"
        "    vec4 tex_color = texture(u_tex, v_tex_coord);\n"
        "    o_frag_color = tex_color * v_blend;\n"
        "}";

    zfw_s_batch_shader_prog prog = {0};
    prog.gl_id = ZFW_CreateShaderProgFromSrcs(vert_shader_src, frag_shader_src);
    assert(prog.gl_id != 0);

    prog.view_uniform_loc = glGetUniformLocation(prog.gl_id, "u_view");
    prog.proj_uniform_loc = glGetUniformLocation(prog.gl_id, "u_proj");

    return prog;
}

static size_t Stride(const int* const vert_attr_lens, const int vert_attr_cnt) {
    assert(vert_attr_lens);
    assert(vert_attr_cnt > 0);

    int stride = 0;

    for (int i = 0; i < vert_attr_cnt; i++) {
        assert(vert_attr_lens[i] > 0);
        stride += sizeof(float) * vert_attr_lens[i];
    }

    return stride;
}

static zfw_s_gl_ids GenBatch(const float* const vert_buf, const size_t vert_buf_size, const unsigned short* const elem_buf, const size_t elem_buf_size, const int* const vert_attr_lens, const int vert_attr_cnt) {
    assert(vert_buf_size > 0);
    assert(elem_buf && elem_buf_size > 0);
    assert(vert_attr_lens && vert_attr_cnt > 0);

    zfw_s_gl_ids gl_ids = {0};

    glGenVertexArrays(1, &gl_ids.vert_array_gl_id);
    glBindVertexArray(gl_ids.vert_array_gl_id);

    glGenBuffers(1, &gl_ids.vert_buf_gl_id);
    glBindBuffer(GL_ARRAY_BUFFER, gl_ids.vert_buf_gl_id);
    glBufferData(GL_ARRAY_BUFFER, vert_buf_size, vert_buf, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &gl_ids.elem_buf_gl_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_ids.elem_buf_gl_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, elem_buf_size, elem_buf, GL_STATIC_DRAW);

    const GLsizei stride = Stride(vert_attr_lens, vert_attr_cnt);
    int offs = 0;

    for (int i = 0; i < vert_attr_cnt; i++) {
        assert(vert_attr_lens[i] > 0);

        glVertexAttribPointer(i, vert_attr_lens[i], GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * offs));
        glEnableVertexAttribArray(i);

        offs += vert_attr_lens[i];
    }

    return gl_ids;
}

static void WriteBatchSlot(zfw_t_batch_slot* const slot, const zfw_s_batch_slot_write_info* const write_info) {
    assert(slot && ZFW_IS_ZERO(*slot));
    assert(write_info && ZFW_IsBatchSlotWriteInfoValid(write_info));

    const zfw_s_vec_2d vert_coords[] = {
        {0.0f - write_info->origin.x, 0.0f - write_info->origin.y},
        {1.0f - write_info->origin.x, 0.0f - write_info->origin.y},
        {1.0f - write_info->origin.x, 1.0f - write_info->origin.y},
        {0.0f - write_info->origin.x, 1.0f - write_info->origin.y}
    };

    ZFW_CHECK_STATIC_ARRAY_LEN(vert_coords, ZFW_STATIC_ARRAY_LEN(*slot));

    const zfw_s_vec_2d tex_coords[] = {
        {write_info->tex_coords.left, write_info->tex_coords.top},
        {write_info->tex_coords.right, write_info->tex_coords.top},
        {write_info->tex_coords.right, write_info->tex_coords.bottom},
        {write_info->tex_coords.left, write_info->tex_coords.bottom}
    };

    ZFW_CHECK_STATIC_ARRAY_LEN(tex_coords, ZFW_STATIC_ARRAY_LEN(*slot));

    for (int i = 0; i < ZFW_STATIC_ARRAY_LEN(*slot); i++) {
        (*slot)[i] = (zfw_s_batch_vertex){
            .vert_coord = vert_coords[i],
            .pos = write_info->pos,
            .size = write_info->size,
            .rot = write_info->rot,
            .tex_coord = tex_coords[i],
            .blend = write_info->blend
        };
    }
}

static void CleanGLIDs(zfw_s_gl_ids* const gl_ids) {
    assert(gl_ids && ZFW_IsGLIDsValid(gl_ids));

    glDeleteVertexArrays(1, &gl_ids->vert_array_gl_id);
    glDeleteBuffers(1, &gl_ids->vert_buf_gl_id);
    glDeleteBuffers(1, &gl_ids->elem_buf_gl_id);
}

static unsigned short* PushBatchElems(zfw_s_mem_arena* const mem_arena) {
    assert(mem_arena && ZFW_IsMemArenaValid(mem_arena));

    unsigned short* const elems = ZFW_MEM_ARENA_PUSH_TYPE_MANY(mem_arena, unsigned short, ZFW_BATCH_SLOT_ELEM_CNT * ZFW_BATCH_SLOT_CNT);

    if (elems) {
        for (int i = 0; i < ZFW_BATCH_SLOT_CNT; i++) {
            elems[(i * 6) + 0] = (i * 4) + 0;
            elems[(i * 6) + 1] = (i * 4) + 1;
            elems[(i * 6) + 2] = (i * 4) + 2;
            elems[(i * 6) + 3] = (i * 4) + 2;
            elems[(i * 6) + 4] = (i * 4) + 3;
            elems[(i * 6) + 5] = (i * 4) + 0;
        }
    }

    return elems;
}

bool ZFW_InitRenderingBasis(zfw_s_rendering_basis* const basis, zfw_s_mem_arena* const temp_mem_arena) {
    assert(basis && ZFW_IS_ZERO(*basis));
    assert(temp_mem_arena && ZFW_IsMemArenaValid(temp_mem_arena));

    {
        const unsigned short* const batch_elems = PushBatchElems(temp_mem_arena);

        if (!batch_elems) {
            return false;
        }

        basis->batch_gl_ids = GenBatch(NULL, sizeof(zfw_s_batch_vertex) * ZFW_BATCH_SLOT_VERT_CNT * ZFW_BATCH_SLOT_CNT, batch_elems, sizeof(unsigned short) * ZFW_BATCH_SLOT_ELEM_CNT * ZFW_BATCH_SLOT_CNT, zfw_g_batch_vertex_attrib_lens, ZFW_STATIC_ARRAY_LEN(zfw_g_batch_vertex_attrib_lens));
    }

    basis->batch_shader_prog = LoadBatchShaderProg();

    return true;
}

void ZFW_CleanRenderingBasis(zfw_s_rendering_basis* const basis) {
    assert(basis && ZFW_IsRenderingBasisValid(basis));

    CleanGLIDs(&basis->batch_gl_ids);
    glDeleteProgram(basis->batch_shader_prog.gl_id);
    ZFW_ZERO_OUT(*basis);
}

void ZFW_InitRenderingState(zfw_s_rendering_state* const state) {
    assert(state && ZFW_IS_ZERO(*state));
    ZFW_InitIdenMatrix4x4(&state->view_mat);
}

void ZFW_RenderClear(const zfw_s_vec_4d col) {
    assert(ZFW_IsColorValid(col));

    glClearColor(col.x, col.y, col.z, col.w);
    glClear(GL_COLOR_BUFFER_BIT);
}

void ZFW_Render(const zfw_s_rendering_context* const context, const zfw_s_batch_slot_write_info* const write_info) {
    assert(context && ZFW_IsRenderingContextValid(context));
    assert(write_info && ZFW_IsBatchSlotWriteInfoValid(write_info));

    zfw_s_batch_state* const batch_state = &context->state->batch;

    if (batch_state->num_slots_used == 0) {
        // This is the first render to the batch, so set the texture associated with the batch to the one we're trying to render.
        batch_state->tex_gl_id = write_info->tex_gl_id;
    } else if (batch_state->num_slots_used == ZFW_BATCH_SLOT_CNT || write_info->tex_gl_id != batch_state->tex_gl_id) {
        // Submit the batch and then try this same render operation again but on a fresh batch.
        ZFW_SubmitBatch(context);
        ZFW_Render(context, write_info);
        return;
    }

    // Write the vertex data to the topmost slot, then update used slot count.
    const int slot_index = batch_state->num_slots_used;
    ZFW_ZERO_OUT(batch_state->slots[slot_index]);
    WriteBatchSlot(&batch_state->slots[slot_index], write_info);
    batch_state->num_slots_used++;
}

void ZFW_SubmitBatch(const zfw_s_rendering_context* const context) {
    assert(context && ZFW_IsRenderingContextValid(context));

    if (context->state->batch.num_slots_used == 0) {
        // There is nothing to flush.
        return;
    }

    glBindVertexArray(context->basis->batch_gl_ids.vert_array_gl_id);
    glBindBuffer(GL_ARRAY_BUFFER, context->basis->batch_gl_ids.vert_buf_gl_id);

    const size_t write_size = sizeof(zfw_t_batch_slot) * context->state->batch.num_slots_used;
    glBufferSubData(GL_ARRAY_BUFFER, 0, write_size, context->state->batch.slots);

    const zfw_s_batch_shader_prog* const prog = &context->basis->batch_shader_prog;

    glUseProgram(prog->gl_id);

    glUniformMatrix4fv(prog->view_uniform_loc, 1, GL_FALSE, &context->state->view_mat[0][0]);

    zfw_t_matrix_4x4 proj_mat = {0};
    ZFW_InitOrthoMatrix4x4(&proj_mat, 0.0f, context->window_size.x, context->window_size.y, 0.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(prog->proj_uniform_loc, 1, GL_FALSE, &proj_mat[0][0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, context->state->batch.tex_gl_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, context->basis->batch_gl_ids.elem_buf_gl_id);

    glDrawElements(GL_TRIANGLES, ZFW_BATCH_SLOT_ELEM_CNT * context->state->batch.num_slots_used, GL_UNSIGNED_SHORT, NULL);

    context->state->batch.num_slots_used = 0;
    context->state->batch.tex_gl_id = 0;
}
