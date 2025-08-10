#include "zfwc_graphics.h"

static const char g_batch_vert_shader_src[] = "#version 430 core\n"
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

bool InitGLResourceArena(s_gl_resource_arena* const res_arena, s_mem_arena* const mem_arena, const t_s32 res_limit) {
    assert(IS_ZERO(*res_arena));
    assert(res_limit > 0);

    res_arena->ids = PushGLIDArrayToMemArena(mem_arena, res_limit);

    if (IS_ZERO(res_arena->ids)) {
        LOG_ERROR("Failed to reserve memory for OpenGL resource IDs!");
        return false;
    }

    res_arena->res_types = PushGLResourceTypeArrayToMemArena(mem_arena, res_limit);

    if (IS_ZERO(res_arena->res_types)) {
        LOG_ERROR("Failed to reserve memory for OpenGL resource types!");
        return false;
    }

    res_arena->res_limit = res_limit;

    return true;
}

void CleanGLResourceArena(s_gl_resource_arena* const res_arena) {
    for (t_s32 i = 0; i < res_arena->res_used; i++) {
        const t_gl_id gl_id = *GLIDElem(res_arena->ids, i);

        if (!gl_id) {
            continue;
        }

        const e_gl_resource_type res_type = *GLResourceTypeElem(res_arena->res_types, i);

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

s_gl_id_array PushToGLResourceArena(s_gl_resource_arena* const res_arena, const t_s32 cnt, const e_gl_resource_type res_type) {
    if (res_arena->res_used + cnt > res_arena->res_limit) {
        LOG_ERROR("OpenGL resource arena is full!");
        return (s_gl_id_array){0};
    }

    const t_s32 res_used_prev = res_arena->res_used;
    res_arena->res_used += cnt;
    return (s_gl_id_array){(res_arena->ids.buf_raw + res_used_prev), cnt};
}

static size_t CalcStride(const s_s32_array_view vert_attr_lens) {
    t_s32 stride = 0;

    for (t_s32 i = 0; i < vert_attr_lens.len; i++) {
        stride += sizeof(t_r32) * *S32ElemView(vert_attr_lens, i);
    }

    return stride;
}

static s_renderable GenRenderable(s_gl_resource_arena* const gl_res_arena, const s_r32_array_view verts, const s_u16_array_view elems, const s_s32_array_view vert_attr_lens) {
    t_gl_id* const va_gl_id = PushToGLResourceArena(gl_res_arena, 1, ek_gl_resource_type_vert_array).buf_raw;

    if (!va_gl_id) {
        LOG_ERROR("Failed to reserve OpenGL vertex array ID for renderable!");
        return (s_renderable){0};
    }

    t_gl_id* const vb_gl_id = PushToGLResourceArena(gl_res_arena, 1, ek_gl_resource_type_vert_buf).buf_raw;

    if (!vb_gl_id) {
        LOG_ERROR("Failed to reserve OpenGL vertex buffer ID for renderable!");
        return (s_renderable){0};
    }

    t_gl_id* const eb_gl_id = PushToGLResourceArena(gl_res_arena, 1, ek_gl_resource_type_elem_buf).buf_raw;

    if (!eb_gl_id) {
        LOG_ERROR("Failed to reserve OpenGL element buffer ID for renderable!");
        return (s_renderable){0};
    }

    glGenVertexArrays(1, va_gl_id);
    glBindVertexArray(*va_gl_id);

    glGenBuffers(1, vb_gl_id);
    glBindBuffer(GL_ARRAY_BUFFER, *vb_gl_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(*verts.buf_raw) * verts.len, verts.buf_raw, GL_DYNAMIC_DRAW);

    glGenBuffers(1, eb_gl_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *eb_gl_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*elems.buf_raw) * elems.len, elems.buf_raw, GL_STATIC_DRAW);

    const size_t stride = CalcStride(vert_attr_lens);
    t_s32 offs = 0;

    for (t_s32 i = 0; i < vert_attr_lens.len; i++) {
        const t_s32 attr_len = *S32ElemView(vert_attr_lens, i);

        glVertexAttribPointer(i, attr_len, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(t_r32) * offs));
        glEnableVertexAttribArray(i);

        offs += attr_len;
    }

    glBindVertexArray(0);

    return (s_renderable){
        .vert_array_gl_id = va_gl_id,
        .vert_buf_gl_id = vb_gl_id,
        .elem_buf_gl_id = eb_gl_id
    };
}

static s_u16_array_view GenBatchRenderableElems(s_mem_arena* const mem_arena) {
    const s_u16_array elems = PushU16ArrayToMemArena(mem_arena, BATCH_SLOT_ELEM_CNT * BATCH_SLOT_CNT);

    if (IS_ZERO(elems)) {
        LOG_ERROR("Failed to reserve memory for batch renderable elements!");
        return (s_u16_array_view){0};
    }

    for (t_s32 i = 0; i < BATCH_SLOT_CNT; i++) {
        *U16Elem(elems, (i * 6) + 0) = (i * 4) + 0;
        *U16Elem(elems, (i * 6) + 1) = (i * 4) + 1;
        *U16Elem(elems, (i * 6) + 2) = (i * 4) + 2;
        *U16Elem(elems, (i * 6) + 3) = (i * 4) + 2;
        *U16Elem(elems, (i * 6) + 4) = (i * 4) + 3;
        *U16Elem(elems, (i * 6) + 5) = (i * 4) + 0;
    }

    return U16ArrayView(elems);
}

static s_renderable GenRenderableOfType(s_gl_resource_arena* const gl_res_arena, const e_renderable type, s_mem_arena* const temp_mem_arena) {
    switch (type) {
        case ek_renderable_batch:
            {
                const s_r32_array_view verts = {
                    .buf_raw = NULL,
                    .len = sizeof(s_batch_vert) * BATCH_SLOT_VERT_CNT * BATCH_SLOT_CNT
                };

                const s_u16_array_view elems = GenBatchRenderableElems(temp_mem_arena);

                if (IS_ZERO(elems)) {
                    return (s_renderable){0};
                }

                const t_s32 vert_attr_lens[] = {2, 2, 2, 1, 2, 4};

                return GenRenderable(gl_res_arena, verts, elems, ARRAY_FROM_STATIC(s_s32_array_view, vert_attr_lens));
            }

        case ek_renderable_surface:
            {
                const t_r32 verts[] = {
                    -1.0f, -1.0f, 0.0f, 0.0f,
                     1.0f, -1.0f, 1.0f, 0.0f,
                     1.0f,  1.0f, 1.0f, 1.0f,
                    -1.0f,  1.0f, 0.0f, 1.0f
                };

                const t_u16 elems[] = {
                    0, 1, 2,
                    2, 3, 0
                };

                const t_s32 vert_attr_lens[] = {2, 2};

                return GenRenderable(gl_res_arena, ARRAY_FROM_STATIC(s_r32_array_view, verts), ARRAY_FROM_STATIC(s_u16_array_view, elems), ARRAY_FROM_STATIC(s_s32_array_view, vert_attr_lens));
            }

        default:
            assert(false && "Unhandled renderable case!");
            return (s_renderable){0};
    }
}

static s_rgba_texture BuiltinTextureRGBAGenerator(const t_s32 tex_index, s_mem_arena* const mem_arena) {
    switch ((e_builtin_texture)tex_index) {
        case ek_builtin_texture_pixel:
            {
                const s_u8_array px_data = PushU8ArrayToMemArena(mem_arena, 4);

                if (IS_ZERO(px_data)) {
                    return (s_rgba_texture){0};
                }

                *U8Elem(px_data, 0) = 255;
                *U8Elem(px_data, 1) = 255;
                *U8Elem(px_data, 2) = 255;
                *U8Elem(px_data, 3) = 255;

                return (s_rgba_texture){
                    .tex_size = {1, 1},
                    .px_data = px_data
                };
            }

        default:
            assert(false && "Unhandled built-in texture case!");
            return (s_rgba_texture){0};
    }
}

bool InitRenderingBasis(s_rendering_basis* const basis, s_gl_resource_arena* const gl_res_arena, s_mem_arena* const mem_arena, s_mem_arena* const temp_mem_arena) {
    assert(IS_ZERO(*basis));

    basis->builtin_textures = GenTextureGroup(eks_builtin_texture_cnt, BuiltinTextureRGBAGenerator, mem_arena, gl_res_arena, temp_mem_arena);

    if (IS_ZERO(basis->builtin_textures)) {
        LOG_ERROR("Failed to generate built-in textures for rendering basis!");
        return false;
    }

    {
        const s_shader_prog_gen_info gen_infos[] = {
            [ek_builtin_shader_prog_batch] = {
                .holds_srcs = true,
                .vert_src = g_batch_vert_shader_src,
                .frag_src = g_batch_frag_shader_src
            }
        };

        basis->builtin_shader_progs = GenShaderProgGroup(ARRAY_FROM_STATIC(s_shader_prog_gen_info_array_view, gen_infos), gl_res_arena, temp_mem_arena);

        if (IS_ZERO(basis->builtin_shader_progs)) {
            LOG_ERROR("Failed to generate built-in shader programs for rendering basis!");
            return false;
        }
    }

    for (t_s32 i = 0; i < eks_renderable_cnt; i++) {
        basis->renderables[i] = GenRenderableOfType(gl_res_arena, i, temp_mem_arena);

        if (IS_ZERO(basis->renderables)) {
            return false;
        }
    }

    return true;
}

s_rendering_state* GenRenderingState(s_mem_arena* const mem_arena) {
    s_rendering_state* const state = PushToMemArena(mem_arena, sizeof(s_rendering_state), ALIGN_OF(s_rendering_state));

    if (state) {
        state->view_mat = IdentityMatrix4x4();
    }

    return state;
}

void Clear(const s_rendering_context* const rendering_context, const u_v4 col) {
    glClearColor(col.r, col.g, col.b, col.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void SetViewMatrix(const s_rendering_context* const rendering_context, const s_matrix_4x4* const mat) {
    SubmitBatch(rendering_context);
    rendering_context->state->view_mat = *mat;
}

static void WriteBatchSlot(t_batch_slot* const slot, const s_batch_slot_write_info* const write_info) {
    assert(slot && IS_ZERO(*slot));

    const s_v2 vert_coords[] = {
        {0.0f - write_info->origin.x, 0.0f - write_info->origin.y},
        {1.0f - write_info->origin.x, 0.0f - write_info->origin.y},
        {1.0f - write_info->origin.x, 1.0f - write_info->origin.y},
        {0.0f - write_info->origin.x, 1.0f - write_info->origin.y}
    };

    const s_v2 tex_coords[] = {
        {write_info->tex_coords.left, write_info->tex_coords.top},
        {write_info->tex_coords.right, write_info->tex_coords.top},
        {write_info->tex_coords.right, write_info->tex_coords.bottom},
        {write_info->tex_coords.left, write_info->tex_coords.bottom}
    };

    for (t_s32 i = 0; i < STATIC_ARRAY_LEN(*slot); i++) {
        (*slot)[i] = (s_batch_vert){
            .vert_coord = *STATIC_ARRAY_ELEM(vert_coords, i),
            .pos = write_info->pos,
            .size = write_info->size,
            .rot = write_info->rot,
            .tex_coord = *STATIC_ARRAY_ELEM(tex_coords, i),
            .blend = write_info->blend
        };
    }
}

void Render(const s_rendering_context* const rendering_context, const s_batch_slot_write_info* const write_info) {
    s_batch_state* const batch_state = &rendering_context->state->batch;

    if (batch_state->num_slots_used == 0) {
        // This is the first render to the batch, so set the texture associated with the batch to the one we're trying to render.
        batch_state->tex_gl_id = write_info->tex_gl_id;
    } else if (batch_state->num_slots_used == BATCH_SLOT_CNT || write_info->tex_gl_id != batch_state->tex_gl_id) {
        // Submit the batch and then try this same render operation again but on a fresh batch.
        SubmitBatch(rendering_context);
        Render(rendering_context, write_info);
        return;
    }

    // Write the vertex data to the topmost slot, then update used slot count.
    const t_s32 slot_index = batch_state->num_slots_used;
    t_batch_slot* const slot = STATIC_ARRAY_ELEM(batch_state->slots, slot_index);
    ZERO_OUT(*slot);
    WriteBatchSlot(slot, write_info);
    batch_state->num_slots_used++;
}

static inline s_rect InnerRect(const s_rect rect, const float outline_thickness) {
    return (s_rect){
        rect.x + outline_thickness,
        rect.y + outline_thickness,
        rect.width - (outline_thickness * 2.0f),
        rect.height - (outline_thickness * 2.0f)
    };
}

void RenderRectWithOutline(const s_rendering_context* const rendering_context, const s_rect rect, const u_v4 fill_color, const u_v4 outline_color, const float outline_thickness) {
    // Top Outline
    RenderRect(rendering_context, (s_rect){rect.x, rect.y, rect.width - outline_thickness, outline_thickness}, outline_color);

    // Right Outline
    RenderRect(rendering_context, (s_rect){rect.x + rect.width - outline_thickness, rect.y, outline_thickness, rect.height - outline_thickness}, outline_color);

    // Bottom Outline
    RenderRect(rendering_context, (s_rect){rect.x + outline_thickness, rect.y + rect.height - outline_thickness, rect.width - outline_thickness, outline_thickness}, outline_color);

    // Left Outline
    RenderRect(rendering_context, (s_rect){rect.x, rect.y + outline_thickness, outline_thickness, rect.height - outline_thickness}, outline_color);

    // Inside
    RenderRect(rendering_context, InnerRect(rect, outline_thickness), fill_color);
}

void RenderRectWithOutlineAndOpaqueFill(const s_rendering_context* const rendering_context, const s_rect rect, const u_v3 fill_color, const u_v4 outline_color, const float outline_thickness) {
    // Outline
    RenderRect(rendering_context, rect, outline_color);

    // Inside
    RenderRect(rendering_context, InnerRect(rect, outline_thickness), (u_v4){fill_color.r, fill_color.g, fill_color.b, 1.0f});
}

void RenderBarHor(const s_rendering_context* const rendering_context, const s_rect rect, const float perc, const u_v4 front_color, const u_v4 bg_color) {
    assert(perc >= 0.0f && perc <= 1.0f);

    const float front_rect_width = rect.width * perc;

    if (front_rect_width > 0.0f) {
        RenderRect(rendering_context, (s_rect){rect.x, rect.y, front_rect_width, rect.height}, front_color);
    }

    const float bg_rect_x = rect.x + front_rect_width;
    const float bg_rect_width = rect.width - front_rect_width;

    if (bg_rect_width > 0.0f) {
        RenderRect(rendering_context, (s_rect){bg_rect_x, rect.y, bg_rect_width, rect.height}, bg_color);
    }
}

void RenderBarVer(const s_rendering_context* const rendering_context, const s_rect rect, const float perc, const u_v4 front_color, const u_v4 bg_color) {
    assert(perc >= 0.0f && perc <= 1.0f);

    const float front_rect_height = rect.height * perc;

    if (front_rect_height > 0.0f) {
        RenderRect(rendering_context, (s_rect){rect.x, rect.y, rect.width, front_rect_height}, front_color);
    }

    const float bg_rect_y = rect.x + front_rect_height;
    const float bg_rect_height = rect.width - front_rect_height;

    if (bg_rect_height > 0.0f) {
        RenderRect(rendering_context, (s_rect){rect.x, bg_rect_y, rect.width, bg_rect_height}, bg_color);
    }
}

void SubmitBatch(const s_rendering_context* const rendering_context) {
    if (rendering_context->state->batch.num_slots_used == 0) {
        // Nothing to flush!
        return;
    }

    const s_renderable* const renderable = &rendering_context->basis->renderables[ek_renderable_batch];

    //
    // Submitting Vertex Data to GPU
    //
    glBindVertexArray(*renderable->vert_array_gl_id);
    glBindBuffer(GL_ARRAY_BUFFER, *renderable->vert_buf_gl_id);

    {
        const size_t write_size = sizeof(t_batch_slot) * rendering_context->state->batch.num_slots_used;
        glBufferSubData(GL_ARRAY_BUFFER, 0, write_size, rendering_context->state->batch.slots);
    }

    //
    // Rendering the Batch
    //
    const t_gl_id prog_gl_id = *GLIDElemView(rendering_context->basis->builtin_shader_progs.gl_ids, ek_builtin_shader_prog_batch);

    glUseProgram(prog_gl_id);

    const t_s32 view_uniform_loc = glGetUniformLocation(prog_gl_id, "u_view");
    glUniformMatrix4fv(view_uniform_loc, 1, false, (const t_r32*)rendering_context->state->view_mat.elems);

    const s_matrix_4x4 proj_mat = OrthographicMatrix(0.0f, rendering_context->window_size.x, rendering_context->window_size.y, 0.0f, -1.0f, 1.0f);

    const t_s32 proj_uniform_loc = glGetUniformLocation(prog_gl_id, "u_proj");
    glUniformMatrix4fv(proj_uniform_loc, 1, false, (const t_r32*)proj_mat.elems);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rendering_context->state->batch.tex_gl_id);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *renderable->elem_buf_gl_id);
    glDrawElements(GL_TRIANGLES, BATCH_SLOT_ELEM_CNT * rendering_context->state->batch.num_slots_used, GL_UNSIGNED_SHORT, NULL);

    glUseProgram(0);

    rendering_context->state->batch.num_slots_used = 0;
    rendering_context->state->batch.tex_gl_id = 0;
}
