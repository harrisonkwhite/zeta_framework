#include <stb_image.h>
#include "zfw_rendering.h"
#include "zfw_utils.h"

static zfw_t_gl_id CreateShaderFromSrc(const char* const src, const bool frag) {
    const GLenum shader_type = frag ? GL_FRAGMENT_SHADER : GL_VERTEX_SHADER;
    const zfw_t_gl_id shader_gl_id = glCreateShader(shader_type);
    glShaderSource(shader_gl_id, 1, &src, NULL);
    glCompileShader(shader_gl_id);

    GLint success;
    glGetShaderiv(shader_gl_id, GL_COMPILE_STATUS, &success);

    if (!success) {
        glDeleteShader(shader_gl_id);
        return 0;
    }

    return shader_gl_id;
}

static zfw_t_gl_id CreateShaderProgFromSrcs(const char* const vert_src, const char* const frag_src) {
    const zfw_t_gl_id vert_shader_gl_id = CreateShaderFromSrc(vert_src, false);

    if (!vert_shader_gl_id) {
        return 0;
    }

    const zfw_t_gl_id frag_shader_gl_id = CreateShaderFromSrc(frag_src, true);

    if (!frag_shader_gl_id) {
        glDeleteShader(vert_shader_gl_id);
        return 0;
    }

    const zfw_t_gl_id prog_gl_id = glCreateProgram();
    glAttachShader(prog_gl_id, vert_shader_gl_id);
    glAttachShader(prog_gl_id, frag_shader_gl_id);
    glLinkProgram(prog_gl_id);

    glDeleteShader(vert_shader_gl_id);
    glDeleteShader(frag_shader_gl_id);

    return prog_gl_id;
}

static zfw_s_batch_shader_prog LoadBatchShaderProg() {
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
    prog.gl_id = CreateShaderProgFromSrcs(vert_shader_src, frag_shader_src);
    assert(prog.gl_id != 0);

    prog.proj_uniform_loc = glGetUniformLocation(prog.gl_id, "u_proj");
    prog.view_uniform_loc = glGetUniformLocation(prog.gl_id, "u_view");
    prog.textures_uniform_loc = glGetUniformLocation(prog.gl_id, "u_textures");

    return prog;
}

static zfw_s_rect_edges TextureCoords(const zfw_s_rect_i src_rect, const zfw_s_vec_2d_i tex_size) {
    assert(ZFW_IsSrcRectValid(src_rect, tex_size));
    assert(tex_size.x > 0 && tex_size.y > 0);

    return (zfw_s_rect_edges){
        .left = (float)src_rect.x / tex_size.x,
        .top = (float)src_rect.y / tex_size.y,
        .right = (float)ZFWRectIRight(src_rect) / tex_size.x,
        .bottom = (float)ZFWRectIBottom(src_rect) / tex_size.y
    };
}

static void WriteBatchSlot(zfw_t_batch_slot* const slot, const zfw_s_batch_slot_write_info* const write_info) {
    assert(slot && ZFW_IS_ZERO(*slot));
    assert(write_info && ZFW_IsBatchSlotWriteInfoValid(write_info));

    const zfw_s_vec_2d mysteries[] = {
        {0.0f - write_info->origin.x, 0.0f - write_info->origin.y},
        {1.0f - write_info->origin.x, 0.0f - write_info->origin.y},
        {1.0f - write_info->origin.x, 1.0f - write_info->origin.y},
        {0.0f - write_info->origin.x, 1.0f - write_info->origin.y}
    };

    ZFW_CHECK_STATIC_ARRAY_LEN(mysteries, ZFW_STATIC_ARRAY_LEN(*slot));

    const zfw_s_vec_2d whats[] = {
        {write_info->tex_coords.left, write_info->tex_coords.top},
        {write_info->tex_coords.left, write_info->tex_coords.top},
        {write_info->tex_coords.left, write_info->tex_coords.top},
        {write_info->tex_coords.left, write_info->tex_coords.top}
    };

    ZFW_CHECK_STATIC_ARRAY_LEN(whats, ZFW_STATIC_ARRAY_LEN(*slot));

    for (int i = 0; i < ZFW_STATIC_ARRAY_LEN(*slot); i++) {
        (*slot)[i] = (zfw_s_batch_vertex){
            .mystery = mysteries[i],
            .pos = write_info->pos,
            .size = write_info->size,
            .rot = write_info->rot,
            .what = whats[i],
            .blend = write_info->blend
        };
    }
}

static int Stride(const int* const vert_attr_lens, const int vert_attr_cnt) {
    assert(vert_attr_lens);
    assert(vert_attr_cnt > 0);

    int stride = 0;

    for (int i = 0; i < vert_attr_cnt; i++) {
        assert(vert_attr_lens[i] > 0);
        stride += vert_attr_lens[i];
    }

    return stride;
}

static zfw_s_gl_ids Car(const zfw_t_vert* const verts, const int vert_cnt, const zfw_t_elem* const elems, const int elem_cnt, const int* const vert_attr_lens, const int vert_attr_cnt) {
    assert((!verts && vert_cnt == 0) || (verts && vert_cnt > 0));
    assert(elems && elem_cnt > 0);

    zfw_s_gl_ids gl_ids = {0};

    glGenVertexArrays(1, &gl_ids.vert_array_gl_id);
    glBindVertexArray(gl_ids.vert_array_gl_id);

    glGenBuffers(1, &gl_ids.vert_buf_gl_id);
    glBindBuffer(GL_ARRAY_BUFFER, gl_ids.vert_buf_gl_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(*verts) * vert_cnt, verts, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &gl_ids.elem_buf_gl_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_ids.elem_buf_gl_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*elems) * elem_cnt, elems, GL_STATIC_DRAW);

    const GLsizei stride = Stride(vert_attr_lens, vert_attr_cnt);
    int offs = 0;

    for (int i = 0; i < vert_attr_cnt; i++) {
        glVertexAttribPointer(i, vert_attr_lens[i], GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(zfw_t_vert) * offs));
        glEnableVertexAttribArray(i);

        offs += vert_attr_lens[i];
    }

    return gl_ids;
}

static bool IsGLIDsValid(const zfw_s_gl_ids* const gl_ids) {
    return glIsVertexArray(gl_ids->vert_array_gl_id) && glIsBuffer(gl_ids->vert_buf_gl_id) && glIsBuffer(gl_ids->elem_buf_gl_id);
}

static void CleanGLIDs(zfw_s_gl_ids* const gl_ids) {
    assert(gl_ids && IsGLIDsValid(gl_ids));

    glDeleteVertexArrays(1, &gl_ids->vert_array_gl_id);
    glDeleteBuffers(1, &gl_ids->vert_buf_gl_id);
    glDeleteBuffers(1, &gl_ids->elem_buf_gl_id);
}

static zfw_t_elem* PushBatchElems(zfw_s_mem_arena* const mem_arena) {
    zfw_t_elem* const elems = ZFW_MEM_ARENA_PUSH_TYPE_MANY(mem_arena, zfw_t_elem, ZFW_BATCH_SLOT_ELEM_CNT * ZFW_BATCH_SLOT_CNT);

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
    assert(ZFW_IS_ZERO(*basis));
    assert(temp_mem_arena && ZFWIsMemArenaValid(temp_mem_arena));

    {
        zfw_t_elem* const batch_elems = PushBatchElems(temp_mem_arena);

        if (!batch_elems) {
            return false;
        }

        basis->batch_gl_ids = Car(NULL, ZFW_BATCH_SLOT_VERT_CNT * ZFW_BATCH_SLOT_CNT, batch_elems, ZFW_BATCH_SLOT_ELEM_CNT * ZFW_BATCH_SLOT_CNT, zfw_g_batch_vertex_attrib_lens, ZFW_STATIC_ARRAY_LEN(zfw_g_batch_vertex_attrib_lens));
    }

    basis->batch_shader_prog = LoadBatchShaderProg();

    return true;
}

void ZFW_CleanRenderingBasis(zfw_s_rendering_basis* const basis) {
    CleanGLIDs(&basis->batch_gl_ids);
    glDeleteProgram(basis->batch_shader_prog.gl_id);
    ZFW_ZERO_OUT(*basis);
}

void ZFW_InitRenderingState(zfw_s_rendering_state* const state) {
    assert(ZFW_IS_ZERO(*state));
    ZFWInitIdenMatrix4x4(&state->view_mat);
}

void ZFW_RenderClear(const zfw_s_vec_4d col) {
    assert(ZFW_IsColorValid(col));

    glClearColor(col.x, col.y, col.z, col.w);
    glClear(GL_COLOR_BUFFER_BIT);
}

void ZFW_Render(const zfw_s_rendering_context* const context, const zfw_s_batch_slot_write_info* const write_info) {
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
    WriteBatchSlot(&batch_state->slots[slot_index], write_info);
    batch_state->num_slots_used++;
}

void ZFW_RenderTexture(const zfw_s_rendering_context* const context, const int tex_index, const zfw_s_textures* const textures, const zfw_s_rect_i src_rect, const zfw_s_vec_2d pos, const zfw_s_vec_2d origin, const zfw_s_vec_2d scale, const float rot, const zfw_s_vec_4d blend) {
    const zfw_s_batch_slot_write_info write_info = {
        .tex_gl_id = textures->gl_ids[tex_index],
        .tex_coords = TextureCoords(src_rect, textures->sizes[tex_index]),
        .pos = pos,
        .size = {src_rect.width * scale.x, src_rect.height * scale.y},
        .origin = origin,
        .rot = rot,
        .blend = blend
    };

    ZFW_Render(context, &write_info);
}

void ZFW_SubmitBatch(const zfw_s_rendering_context* const context) {
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

    zfw_t_matrix_4x4 proj_mat = {0};
    ZFWInitOrthoMatrix4x4(&proj_mat, 0.0f, context->window_size.x, context->window_size.y, 0.0f, -1.0f, 1.0f);

    glUniformMatrix4fv(prog->proj_uniform_loc, 1, GL_FALSE, &proj_mat[0][0]);
    glUniformMatrix4fv(prog->view_uniform_loc, 1, GL_FALSE, &context->state->view_mat[0][0]);

    glBindTexture(GL_TEXTURE_2D, context->state->batch.tex_gl_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, context->basis->batch_gl_ids.elem_buf_gl_id);

    glDrawElements(GL_TRIANGLES, ZFW_BATCH_SLOT_ELEM_CNT * context->state->batch.num_slots_used, GL_UNSIGNED_SHORT, NULL);

    context->state->batch.num_slots_used = 0;
}
