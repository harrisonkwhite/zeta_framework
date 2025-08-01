#include "zfw_rendering.h"

#include <stb_image.h>

static const char* const g_batch_vert_shader_src = "#version 430 core\n"
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

static const char* const g_batch_frag_shader_src = "#version 430 core\n"
    "in vec2 v_tex_coord;\n"
    "in vec4 v_blend;\n"
    "out vec4 o_frag_color;\n"
    "uniform sampler2D u_tex;\n"
    "void main() {\n"
    "    vec4 tex_color = texture(u_tex, v_tex_coord);\n"
    "    o_frag_color = tex_color * v_blend;\n"
    "}";

static zfw_s_texture_group GenBuiltinTextures(zfw_s_gl_resource_arena* const gl_res_arena, s_mem_arena* const mem_arena) {
    assert(gl_res_arena);
    assert(mem_arena && IsMemArenaValid(mem_arena));

    zfw_t_gl_id* const gl_ids = ZFW_ReserveGLIDs(gl_res_arena, zfw_eks_builtin_texture_cnt, zfw_ek_gl_resource_type_texture);

    if (!gl_ids) {
        LOG_ERROR("Failed to reserve OpenGL texture IDs for built-in textures!");
        return (zfw_s_texture_group){0};
    }

    zfw_s_vec_2d_s32* const sizes = MEM_ARENA_PUSH_TYPE_CNT(mem_arena, zfw_s_vec_2d_s32, zfw_eks_builtin_texture_cnt);

    if (!sizes) {
        LOG_ERROR("Failed to reserve memory for built-in texture sizes!");
        return (zfw_s_texture_group){0};
    }

    for (int i = 0; i < zfw_eks_builtin_texture_cnt; i++) {
        switch ((zfw_e_builtin_texture)i) {
            case zfw_ek_builtin_texture_pixel:
                {
                    const t_u8 rgba_px_data[4] = {255, 255, 255, 255};
                    sizes[i] = (zfw_s_vec_2d_s32){1, 1};
                    gl_ids[i] = ZFW_GenGLTextureFromRGBAPixelData(rgba_px_data, sizes[i]);
                }

                break;
        }
    }

    return (zfw_s_texture_group){
        .gl_ids = gl_ids,
        .sizes = sizes,
        .cnt = zfw_eks_builtin_texture_cnt
    };
}

static zfw_s_shader_prog_group GenBuiltinShaderProgs(zfw_s_gl_resource_arena* const gl_res_arena, s_mem_arena* const temp_mem_arena) {
    assert(gl_res_arena);
    assert(temp_mem_arena && IsMemArenaValid(temp_mem_arena));

    zfw_t_gl_id* const gl_ids = ZFW_ReserveGLIDs(gl_res_arena, zfw_eks_builtin_shader_prog_cnt, zfw_ek_gl_resource_type_shader_prog);

    if (!gl_ids) {
        LOG_ERROR("Failed to reserve OpenGL shader program IDs for built-in shader programs!");
        return (zfw_s_shader_prog_group){0};
    }

    for (int i = 0; i < zfw_eks_builtin_shader_prog_cnt; i++) {
        switch ((zfw_e_builtin_shader_prog)i) {
            case zfw_ek_builtin_shader_prog_batch:
                gl_ids[i] = ZFW_GenShaderProg(g_batch_vert_shader_src, g_batch_frag_shader_src, temp_mem_arena);

                if (!gl_ids[i]) {
                    LOG_ERROR("Failed to generate built-in batch shader program!");
                    return (zfw_s_shader_prog_group){0};
                }

                break;
        }
    }

    return (zfw_s_shader_prog_group){
        .gl_ids = gl_ids,
        .cnt = zfw_eks_builtin_shader_prog_cnt
    };
}

static unsigned short* PushBatchRenderableElems(s_mem_arena* const mem_arena) {
    assert(mem_arena && IsMemArenaValid(mem_arena));

    unsigned short* const elems = MEM_ARENA_PUSH_TYPE_CNT(mem_arena, unsigned short, ZFW_BATCH_SLOT_ELEM_CNT * ZFW_BATCH_SLOT_CNT);

    if (!elems) {
        LOG_ERROR("Failed to reserve memory for batch renderable elements!");
        return NULL;
    }

    for (int i = 0; i < ZFW_BATCH_SLOT_CNT; i++) {
        elems[(i * 6) + 0] = (i * 4) + 0;
        elems[(i * 6) + 1] = (i * 4) + 1;
        elems[(i * 6) + 2] = (i * 4) + 2;
        elems[(i * 6) + 3] = (i * 4) + 2;
        elems[(i * 6) + 4] = (i * 4) + 3;
        elems[(i * 6) + 5] = (i * 4) + 0;
    }

    return elems;
}

static zfw_s_renderables GenRenderables(zfw_s_gl_resource_arena* const gl_res_arena, s_mem_arena* const temp_mem_arena) {
    assert(gl_res_arena);
    assert(temp_mem_arena && IsMemArenaValid(temp_mem_arena));

    zfw_t_gl_id* const va_gl_ids = ZFW_ReserveGLIDs(gl_res_arena, zfw_eks_renderable_cnt, zfw_ek_gl_resource_type_vert_array);

    if (!va_gl_ids) {
        LOG_ERROR("Failed to reserve OpenGL vertex array IDs for renderables!");
        return (zfw_s_renderables){0};
    }

    zfw_t_gl_id* const vb_gl_ids = ZFW_ReserveGLIDs(gl_res_arena, zfw_eks_renderable_cnt, zfw_ek_gl_resource_type_vert_buf);

    if (!va_gl_ids) {
        LOG_ERROR("Failed to reserve OpenGL vertex buffer IDs for renderables!");
        return (zfw_s_renderables){0};
    }

    zfw_t_gl_id* const eb_gl_ids = ZFW_ReserveGLIDs(gl_res_arena, zfw_eks_renderable_cnt, zfw_ek_gl_resource_type_elem_buf);

    if (!eb_gl_ids) {
        LOG_ERROR("Failed to reserve OpenGL element buffer IDs for renderables!");
        return (zfw_s_renderables){0};
    }

    for (int i = 0; i < zfw_eks_renderable_cnt; i++) {
        switch ((zfw_e_renderable)i) {
            case zfw_ek_renderable_batch:
                {
                    const unsigned short* const batch_elems = PushBatchRenderableElems(temp_mem_arena);

                    if (!batch_elems) {
                        return (zfw_s_renderables){0};
                    }

                    ZFW_GenRenderable(&va_gl_ids[i], &vb_gl_ids[i], &eb_gl_ids[i], NULL, sizeof(zfw_s_batch_vertex) * ZFW_BATCH_SLOT_VERT_CNT * ZFW_BATCH_SLOT_CNT, batch_elems, sizeof(unsigned short) * ZFW_BATCH_SLOT_ELEM_CNT * ZFW_BATCH_SLOT_CNT, zfw_g_batch_vertex_attrib_lens, STATIC_ARRAY_LEN(zfw_g_batch_vertex_attrib_lens));
                }

                break;
        }
    }

    return (zfw_s_renderables){
        .vert_array_gl_ids = va_gl_ids,
        .vert_buf_gl_ids = vb_gl_ids,
        .elem_buf_gl_ids = eb_gl_ids,
        .cnt = zfw_eks_renderable_cnt
    };
}

bool ZFW_InitRenderingBasis(zfw_s_rendering_basis* const basis, zfw_s_gl_resource_arena* const gl_res_arena, s_mem_arena* const mem_arena, s_mem_arena* const temp_mem_arena) {
    assert(basis && IS_ZERO(*basis));
    assert(gl_res_arena);
    assert(mem_arena && IsMemArenaValid(mem_arena));
    assert(temp_mem_arena && IsMemArenaValid(temp_mem_arena));

    basis->builtin_textures = GenBuiltinTextures(gl_res_arena, mem_arena);

    if (IS_ZERO(basis->builtin_textures)) {
        LOG_ERROR("Failed to generate built-in textures for rendering basis!");
        return false;
    }

    basis->builtin_shader_progs = GenBuiltinShaderProgs(gl_res_arena, temp_mem_arena);

    if (IS_ZERO(basis->builtin_shader_progs)) {
        LOG_ERROR("Failed to generate built-in shader programs for rendering basis!");
        return false;
    }

    basis->renderables = GenRenderables(gl_res_arena, temp_mem_arena);

    if (IS_ZERO(basis->renderables)) {
        LOG_ERROR("Failed to generate renderables for rendering basis!");
        return false;
    }

    return true;
}

void ZFW_InitRenderingState(zfw_s_rendering_state* const state) {
    assert(state && IS_ZERO(*state));
    ZFW_InitIdenMatrix4x4(&state->view_mat);
}

void ZFW_RenderClear(const zfw_u_vec_4d col) {
    assert(ZFW_IsColorValid(col));

    glClearColor(col.r, col.g, col.b, col.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

static void WriteBatchSlot(zfw_t_batch_slot* const slot, const zfw_s_batch_slot_write_info* const write_info) {
    assert(slot && IS_ZERO(*slot));
    assert(write_info);

    const zfw_s_vec_2d vert_coords[] = {
        {0.0f - write_info->origin.x, 0.0f - write_info->origin.y},
        {1.0f - write_info->origin.x, 0.0f - write_info->origin.y},
        {1.0f - write_info->origin.x, 1.0f - write_info->origin.y},
        {0.0f - write_info->origin.x, 1.0f - write_info->origin.y}
    };

    STATIC_ARRAY_LEN_CHECK(vert_coords, STATIC_ARRAY_LEN(*slot));

    const zfw_s_vec_2d tex_coords[] = {
        {write_info->tex_coords.left, write_info->tex_coords.top},
        {write_info->tex_coords.right, write_info->tex_coords.top},
        {write_info->tex_coords.right, write_info->tex_coords.bottom},
        {write_info->tex_coords.left, write_info->tex_coords.bottom}
    };

    STATIC_ARRAY_LEN_CHECK(tex_coords, STATIC_ARRAY_LEN(*slot));

    for (int i = 0; i < STATIC_ARRAY_LEN(*slot); i++) {
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

void ZFW_Render(const zfw_s_rendering_context* const context, const zfw_s_batch_slot_write_info* const write_info) {
    assert(context);
    assert(write_info);

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
    ZERO_OUT(batch_state->slots[slot_index]);
    WriteBatchSlot(&batch_state->slots[slot_index], write_info);
    batch_state->num_slots_used++;
}

void ZFW_RenderTexture(const zfw_s_rendering_context* const context, const int tex_index, const zfw_s_texture_group* const textures, const zfw_s_rect_s32 src_rect, const zfw_s_vec_2d pos, const zfw_s_vec_2d origin, const zfw_s_vec_2d scale, const float rot, const zfw_u_vec_4d blend) {
    assert(context);
    assert(textures && tex_index >= 0 && tex_index < textures->cnt);
    assert(IS_ZERO(src_rect) || ZFW_IsSrcRectValid(src_rect, textures->sizes[tex_index]));
    assert(ZFW_IsOriginValid(origin));
    assert(ZFW_IsColorValid(blend));

    const zfw_s_rect_s32 src_rect_to_use = IS_ZERO(src_rect) ? (zfw_s_rect_s32){0, 0, textures->sizes[tex_index].x, textures->sizes[tex_index].y} : src_rect;

    const zfw_s_batch_slot_write_info write_info = {
        .tex_gl_id = textures->gl_ids[tex_index],
        .tex_coords = ZFW_TextureCoords(src_rect_to_use, textures->sizes[tex_index]),
        .pos = pos,
        .size = {src_rect_to_use.width * scale.x, src_rect_to_use.height * scale.y},
        .origin = origin,
        .rot = rot,
        .blend = blend
    };

    ZFW_Render(context, &write_info);
}

void ZFW_SubmitBatch(const zfw_s_rendering_context* const context) {
    assert(context);

    if (context->state->batch.num_slots_used == 0) {
        // There is nothing to flush.
        return;
    }

    const zfw_t_gl_id va_gl_id = context->basis->renderables.vert_array_gl_ids[zfw_ek_renderable_batch];
    const zfw_t_gl_id vb_gl_id = context->basis->renderables.vert_buf_gl_ids[zfw_ek_renderable_batch];
    const zfw_t_gl_id eb_gl_id = context->basis->renderables.elem_buf_gl_ids[zfw_ek_renderable_batch];
    const zfw_t_gl_id prog_gl_id = context->basis->builtin_shader_progs.gl_ids[zfw_ek_builtin_shader_prog_batch];

    glBindVertexArray(va_gl_id);
    glBindBuffer(GL_ARRAY_BUFFER, vb_gl_id);

    const size_t write_size = sizeof(zfw_t_batch_slot) * context->state->batch.num_slots_used;
    glBufferSubData(GL_ARRAY_BUFFER, 0, write_size, context->state->batch.slots);

    glUseProgram(prog_gl_id);

    const int view_uniform_loc = glGetUniformLocation(prog_gl_id, "u_view");
    glUniformMatrix4fv(view_uniform_loc, 1, GL_FALSE, &context->state->view_mat[0][0]);

    zfw_t_matrix_4x4 proj_mat = {0};
    ZFW_InitOrthoMatrix4x4(&proj_mat, 0.0f, context->window_size.x, context->window_size.y, 0.0f, -1.0f, 1.0f);

    const int proj_uniform_loc = glGetUniformLocation(prog_gl_id, "u_proj");
    glUniformMatrix4fv(proj_uniform_loc, 1, GL_FALSE, &proj_mat[0][0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, context->state->batch.tex_gl_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eb_gl_id);

    glDrawElements(GL_TRIANGLES, ZFW_BATCH_SLOT_ELEM_CNT * context->state->batch.num_slots_used, GL_UNSIGNED_SHORT, NULL);

    context->state->batch.num_slots_used = 0;
    context->state->batch.tex_gl_id = 0;
}
