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

zfw_s_renderable ZFW_GenRenderable(const float* const vert_buf, const size_t vert_buf_size, const unsigned short* const elem_buf, const size_t elem_buf_size, const int* const vert_attr_lens, const int vert_attr_cnt) {
    assert(vert_buf_size > 0);
    assert(elem_buf && elem_buf_size > 0);
    assert(vert_attr_lens && vert_attr_cnt > 0);

    zfw_s_renderable renderable = {0};

    glGenVertexArrays(1, &renderable.vert_array_gl_id);
    glBindVertexArray(renderable.vert_array_gl_id);

    glGenBuffers(1, &renderable.vert_buf_gl_id);
    glBindBuffer(GL_ARRAY_BUFFER, renderable.vert_buf_gl_id);
    glBufferData(GL_ARRAY_BUFFER, vert_buf_size, vert_buf, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &renderable.elem_buf_gl_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderable.elem_buf_gl_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, elem_buf_size, elem_buf, GL_STATIC_DRAW);

    const GLsizei stride = Stride(vert_attr_lens, vert_attr_cnt);
    int offs = 0;

    for (int i = 0; i < vert_attr_cnt; i++) {
        assert(vert_attr_lens[i] > 0);

        glVertexAttribPointer(i, vert_attr_lens[i], GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * offs));
        glEnableVertexAttribArray(i);

        offs += vert_attr_lens[i];
    }

    glBindVertexArray(0);

    return renderable;
}

void ZFW_CleanRenderable(zfw_s_renderable* const gl_ids) {
    assert(gl_ids && ZFW_IsRenderableValid(gl_ids));

    glDeleteVertexArrays(1, &gl_ids->vert_array_gl_id);
    glDeleteBuffers(1, &gl_ids->vert_buf_gl_id);
    glDeleteBuffers(1, &gl_ids->elem_buf_gl_id);
}

bool ZFW_InitRenderingBasis(zfw_s_rendering_basis* const basis, zfw_s_mem_arena* const mem_arena, const int surf_cnt, const zfw_s_vec_2d_i window_size, zfw_s_mem_arena* const temp_mem_arena) {
    assert(basis && ZFW_IS_ZERO(*basis));
    assert(mem_arena && ZFW_IsMemArenaValid(mem_arena));
    assert(surf_cnt >= 0 && surf_cnt <= ZFW_SURFACE_LIMIT);
    assert(window_size.x > 0 && window_size.y > 0);
    assert(temp_mem_arena && ZFW_IsMemArenaValid(temp_mem_arena));

    // NOTE: Doing things in an odd order here to minimise cleanup work.

    const unsigned short* const batch_elems = PushBatchElems(temp_mem_arena);

    if (!batch_elems) {
        return false;
    }

    if (surf_cnt > 0) {
        if (!ZFW_InitSurfaces(&basis->surfs, mem_arena, surf_cnt, window_size)) {
            return false;
        }
    }

    basis->batch_renderable = ZFW_GenRenderable(NULL, sizeof(zfw_s_batch_vertex) * ZFW_BATCH_SLOT_VERT_CNT * ZFW_BATCH_SLOT_CNT, batch_elems, sizeof(unsigned short) * ZFW_BATCH_SLOT_ELEM_CNT * ZFW_BATCH_SLOT_CNT, zfw_g_batch_vertex_attrib_lens, ZFW_STATIC_ARRAY_LEN(zfw_g_batch_vertex_attrib_lens));

    basis->batch_shader_prog = LoadBatchShaderProg();

    const zfw_t_byte px_tex_rgba_data[ZFW_RGBA_CHANNEL_CNT] = {255, 255, 255, 255};
    basis->px_tex_gl_id = ZFW_GenTexture((zfw_s_vec_2d_i){1, 1}, px_tex_rgba_data);

    basis->surf_renderable = ZFW_GenSurfaceRenderable();

    return true;
}

void ZFW_CleanRenderingBasis(zfw_s_rendering_basis* const basis) {
    assert(basis && ZFW_IsRenderingBasisValid(basis));

    ZFW_CleanRenderable(&basis->surf_renderable);
    glDeleteTextures(1, &basis->px_tex_gl_id);
    glDeleteProgram(basis->batch_shader_prog.gl_id);
    ZFW_CleanRenderable(&basis->batch_renderable);
    ZFW_CleanSurfaces(&basis->surfs);

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

void ZFW_RenderRect(const zfw_s_rendering_context* const context, const zfw_s_rect rect, const zfw_s_vec_4d blend) {
    assert(context && ZFW_IsRenderingContextValid(context));
    assert(rect.width > 0.0f && rect.height > 0.0f);
    assert(ZFW_IsColorValid(blend));

    const zfw_s_batch_slot_write_info write_info = {
        .tex_gl_id = context->basis->px_tex_gl_id,
        .tex_coords = {0.0f, 0.0f, 1.0f, 1.0f},
        .pos = ZFW_RectPos(rect),
        .size = ZFW_RectSize(rect),
        .blend = blend
    };

    ZFW_Render(context, &write_info);
}

void ZFW_RenderRectOutline(const zfw_s_rendering_context* const context, const zfw_s_rect rect, const zfw_s_vec_4d blend, const float thickness) {
    assert(context && ZFW_IsRenderingContextValid(context));
    assert(rect.width > 0.0f && rect.height > 0.0f);
    assert(ZFW_IsColorValid(blend));
    assert(thickness > 0.0f);

    const zfw_s_rect top = {rect.x - thickness, rect.y - thickness, rect.width + thickness, thickness};
    const zfw_s_rect right = {rect.x + rect.width, rect.y - thickness, thickness, rect.height + thickness};
    const zfw_s_rect bottom = {rect.x, rect.y + rect.height, rect.width + thickness, thickness};
    const zfw_s_rect left = {rect.x - thickness, rect.y, thickness, rect.height + thickness};

    ZFW_RenderRect(context, top, blend);
    ZFW_RenderRect(context, right, blend);
    ZFW_RenderRect(context, bottom, blend);
    ZFW_RenderRect(context, left, blend);
}

void ZFW_RenderLine(const zfw_s_rendering_context* const context, const zfw_s_vec_2d a, const zfw_s_vec_2d b, const zfw_s_vec_4d blend, const float width) {
    assert(context && ZFW_IsRenderingContextValid(context));
    assert(ZFW_IsColorValid(blend));
    assert(width > 0.0f);

    const float dx = b.x - a.x;
    const float dy = b.y - a.y;
    const float len = sqrtf(dx * dx + dy * dy);

    const zfw_s_batch_slot_write_info write_info = {
        .tex_gl_id = context->basis->px_tex_gl_id,
        .tex_coords = {0.0f, 0.0f, 1.0f, 1.0f},
        .pos = a,
        .size = {len, width},
        .origin = {0.0f, 0.5f},
        .rot = atan2f(dy, dx),
        .blend = blend
    };

    ZFW_Render(context, &write_info);
}

void ZFW_RenderPolyOutline(const zfw_s_rendering_context* const context, const zfw_s_poly poly, const zfw_s_vec_4d blend, const float width) {
    assert(context && ZFW_IsRenderingContextValid(context));
    // TODO: Check polygon validity.
    assert(ZFW_IsColorValid(blend));
    assert(width > 0.0f);

    for (int i = 0; i < poly.cnt; i++) {
        const zfw_s_vec_2d a = poly.pts[i];
        const zfw_s_vec_2d b = poly.pts[(i + 1) % poly.cnt];
        ZFW_RenderLine(context, a, b, blend, width);
    }
}

void ZFW_RenderBarHor(const zfw_s_rendering_context* const context, const zfw_s_rect rect, const float perc, const zfw_s_vec_3d col_front, const zfw_s_vec_3d col_back) {
    assert(context && ZFW_IsRenderingContextValid(context));
    assert(rect.width > 0.0f && rect.height > 0.0f);
    assert(perc >= 0.0f && perc <= 1.0f);
    assert(ZFW_IsColorRGBValid(col_front));
    assert(ZFW_IsColorRGBValid(col_back));

    zfw_s_rect left_rect = {rect.x, rect.y, 0.0f, rect.height};

    // Only render the left rectangle if percentage is not 0.
    if (perc > 0.0f) {
        left_rect.width = rect.width * perc;
        const zfw_s_vec_4d col = {col_front.x, col_front.y, col_front.z, 1.0f};
        ZFW_RenderRect(context, left_rect, col);
    }

    // Only render right rectangle if percentage is not 100.
    if (perc < 1.0f) {
        const zfw_s_rect right_rect = {rect.x + left_rect.width, rect.y, rect.width - left_rect.width, rect.height};
        const zfw_s_vec_4d col = {col_back.x, col_back.y, col_back.z, 1.0f};
        ZFW_RenderRect(context, right_rect, col);
    }
}

void ZFW_SubmitBatch(const zfw_s_rendering_context* const context) {
    assert(context && ZFW_IsRenderingContextValid(context));

    if (context->state->batch.num_slots_used == 0) {
        // There is nothing to flush.
        return;
    }

    glBindVertexArray(context->basis->batch_renderable.vert_array_gl_id);
    glBindBuffer(GL_ARRAY_BUFFER, context->basis->batch_renderable.vert_buf_gl_id);

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
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, context->basis->batch_renderable.elem_buf_gl_id);

    glDrawElements(GL_TRIANGLES, ZFW_BATCH_SLOT_ELEM_CNT * context->state->batch.num_slots_used, GL_UNSIGNED_SHORT, NULL);

    context->state->batch.num_slots_used = 0;
    context->state->batch.tex_gl_id = 0;
}
