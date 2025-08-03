#include "zfw_rendering.h"

#include <ctype.h>
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

static zfw_s_texture_info GenBuiltinTextureInfo(const int tex_index, s_mem_arena* const mem_arena) {
    switch ((zfw_e_builtin_texture)tex_index) {
        case zfw_ek_builtin_texture_pixel:
            {
                t_byte* const rgba_px_data = MEM_ARENA_PUSH_TYPE_CNT(mem_arena, t_byte, 4);
                rgba_px_data[0] = 255;
                rgba_px_data[1] = 255;
                rgba_px_data[2] = 255;
                rgba_px_data[3] = 255;

                return (zfw_s_texture_info){
                    .rgba_px_data = rgba_px_data,
                    .tex_size = {1, 1}
                };
            }

            break;

        default:
            assert(false);
            return (zfw_s_texture_info){0};
    }
}

static zfw_s_shader_prog_info GenBuiltinShaderProgInfo(const int prog_index, s_mem_arena* const mem_arena) {
    switch ((zfw_e_builtin_shader_prog)prog_index) {
        case zfw_ek_builtin_shader_prog_batch:
            return (zfw_s_shader_prog_info){
                .vs_src = g_batch_vert_shader_src,
                .fs_src = g_batch_frag_shader_src
            };

        default:
            assert(false && "Unhandled built-in shader program case!");
            return (zfw_s_shader_prog_info){0};
    }
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
    ZFW_AssertGLResourceArenaValidity(gl_res_arena);
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

            case zfw_ek_renderable_surface:
                {
                    const float verts[] = {
                        -1.0, -1.0, 0.0, 0.0,
                        1.0, -1.0, 1.0, 0.0,
                        1.0,  1.0, 1.0, 1.0,
                        -1.0,  1.0, 0.0, 1.0
                    };

                    const unsigned short elems[] = {
                        0, 1, 2,
                        2, 3, 0
                    };

                    const int vert_attr_lens[] = {
                        2,
                        2
                    };

                    ZFW_GenRenderable(&va_gl_ids[i], &vb_gl_ids[i], &eb_gl_ids[i], verts, sizeof(verts), elems, sizeof(elems), vert_attr_lens, STATIC_ARRAY_LEN(vert_attr_lens));
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
    ZFW_AssertGLResourceArenaValidity(gl_res_arena);
    assert(mem_arena && IsMemArenaValid(mem_arena));
    assert(temp_mem_arena && IsMemArenaValid(temp_mem_arena));

    basis->builtin_textures = ZFW_GenTextures(zfw_eks_builtin_texture_cnt, GenBuiltinTextureInfo, gl_res_arena, mem_arena, temp_mem_arena);

    if (IS_ZERO(basis->builtin_textures)) {
        LOG_ERROR("Failed to generate built-in textures for rendering basis!");
        return false;
    }

    basis->builtin_shader_progs = ZFW_GenShaderProgs(zfw_eks_builtin_shader_prog_cnt, GenBuiltinShaderProgInfo, gl_res_arena, temp_mem_arena);

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
    state->view_mat = ZFW_IdentityMatrix4x4();
}

void ZFW_Clear(const zfw_s_rendering_context* const rendering_context, const zfw_u_vec_4d col) {
    ZFW_AssertRenderingContextValidity(rendering_context);
    assert(ZFW_IsColorValid(col));

    glClearColor(col.r, col.g, col.b, col.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void ZFW_SetViewMatrix(const zfw_s_rendering_context* const rendering_context, const zfw_s_matrix_4x4* const mat) {
    ZFW_AssertRenderingContextValidity(rendering_context);
    assert(mat);

    ZFW_SubmitBatch(rendering_context);
    rendering_context->state->view_mat = *mat;
}

static void WriteBatchSlot(zfw_t_batch_slot* const slot, const zfw_s_batch_slot_write_info* const write_info) {
    assert(slot && IS_ZERO(*slot));
    ZFW_AssertBatchSlotWriteInfoValidity(write_info);

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

void ZFW_Render(const zfw_s_rendering_context* const rendering_context, const zfw_s_batch_slot_write_info* const write_info) {
    ZFW_AssertRenderingContextValidity(rendering_context);
    ZFW_AssertBatchSlotWriteInfoValidity(write_info);

    zfw_s_batch_state* const batch_state = &rendering_context->state->batch;

    if (batch_state->num_slots_used == 0) {
        // This is the first render to the batch, so set the texture associated with the batch to the one we're trying to render.
        batch_state->tex_gl_id = write_info->tex_gl_id;
    } else if (batch_state->num_slots_used == ZFW_BATCH_SLOT_CNT || write_info->tex_gl_id != batch_state->tex_gl_id) {
        // Submit the batch and then try this same render operation again but on a fresh batch.
        ZFW_SubmitBatch(rendering_context);
        ZFW_Render(rendering_context, write_info);
        return;
    }

    // Write the vertex data to the topmost slot, then update used slot count.
    const int slot_index = batch_state->num_slots_used;
    ZERO_OUT(batch_state->slots[slot_index]);
    WriteBatchSlot(&batch_state->slots[slot_index], write_info);
    batch_state->num_slots_used++;
}

void ZFW_RenderTexture(const zfw_s_rendering_context* const rendering_context, const zfw_s_texture_group* const textures, const int tex_index, const zfw_s_rect_int src_rect, const zfw_s_vec_2d pos, const zfw_s_vec_2d origin, const zfw_s_vec_2d scale, const float rot, const zfw_u_vec_4d blend) {
    ZFW_AssertRenderingContextValidity(rendering_context);
    ZFW_AssertTextureGroupValidity(textures);
    assert(tex_index >= 0 && tex_index < textures->cnt);
    assert(IS_ZERO(src_rect) || ZFW_IsSrcRectValid(src_rect, textures->sizes[tex_index]));
    assert(ZFW_IsOriginValid(origin));
    assert(scale.x != 0.0f && scale.y != 0.0f);
    assert(ZFW_IsColorValid(blend));

    const zfw_s_rect_int src_rect_to_use = IS_ZERO(src_rect) ? (zfw_s_rect_int){0, 0, textures->sizes[tex_index].x, textures->sizes[tex_index].y} : src_rect;

    const zfw_s_batch_slot_write_info write_info = {
        .tex_gl_id = textures->gl_ids[tex_index],
        .tex_coords = ZFW_TextureCoords(src_rect_to_use, textures->sizes[tex_index]),
        .pos = pos,
        .size = {src_rect_to_use.width * scale.x, src_rect_to_use.height * scale.y},
        .origin = origin,
        .rot = rot,
        .blend = blend
    };

    ZFW_Render(rendering_context, &write_info);
}

static inline zfw_s_rect InnerRect(const zfw_s_rect rect, const float outline_thickness) {
    return (zfw_s_rect){
        rect.x + outline_thickness,
        rect.y + outline_thickness,
        rect.width - (outline_thickness * 2.0f),
        rect.height - (outline_thickness * 2.0f)
    };
}

void ZFW_RenderRectWithOutline(const zfw_s_rendering_context* const rendering_context, const zfw_s_rect rect, const zfw_u_vec_4d fill_color, const zfw_u_vec_4d outline_color, const float outline_thickness) {
    ZFW_AssertRenderingContextValidity(rendering_context);
    assert(rect.width > 0 && rect.height > 0);
    assert(ZFW_IsColorValid(fill_color));
    assert(ZFW_IsColorValid(outline_color));
    assert(outline_thickness != 0.0f && outline_thickness <= MIN(rect.width, rect.height) / 2.0f);

#ifndef NDEBUG
    if (fabsf(fill_color.a - 1.0f) < 0.001f) { // TODO: Create function for float comparisons with precision level.
        LOG_WARNING("ZFW_RenderRectWithOutline() being called despite opaque fill colour. Consider using ZFW_RenderRectWithOutlineAndOpaqueFill() instead.");
    }
#endif

    // Top Outline
    ZFW_RenderRect(rendering_context, (zfw_s_rect){rect.x, rect.y, rect.width - outline_thickness, outline_thickness}, outline_color);

    // Right Outline
    ZFW_RenderRect(rendering_context, (zfw_s_rect){rect.x + rect.width - outline_thickness, rect.y, outline_thickness, rect.height - outline_thickness}, outline_color);

    // Bottom Outline
    ZFW_RenderRect(rendering_context, (zfw_s_rect){rect.x + outline_thickness, rect.y + rect.height - outline_thickness, rect.width - outline_thickness, outline_thickness}, outline_color);

    // Left Outline
    ZFW_RenderRect(rendering_context, (zfw_s_rect){rect.x, rect.y + outline_thickness, outline_thickness, rect.height - outline_thickness}, outline_color);

    // Inside
    ZFW_RenderRect(rendering_context, InnerRect(rect, outline_thickness), fill_color);
}

void ZFW_RenderRectWithOutlineAndOpaqueFill(const zfw_s_rendering_context* const rendering_context, const zfw_s_rect rect, const zfw_u_vec_3d fill_color, const zfw_u_vec_4d outline_color, const float outline_thickness) {
    ZFW_AssertRenderingContextValidity(rendering_context);
    assert(rect.width > 0.0f && rect.height > 0.0f);
    assert(ZFW_IsColorRGBValid(fill_color));
    assert(ZFW_IsColorValid(outline_color));
    assert(outline_thickness != 0.0f && outline_thickness <= MIN(rect.width, rect.height) / 2.0f);

    // Outline
    ZFW_RenderRect(rendering_context, rect, outline_color);

    // Inside
    ZFW_RenderRect(rendering_context, InnerRect(rect, outline_thickness), (zfw_u_vec_4d){fill_color.r, fill_color.g, fill_color.b, 1.0f});
}

void ZFW_RenderBarHor(const zfw_s_rendering_context* const rendering_context, const zfw_s_rect rect, const float perc, const zfw_u_vec_4d front_color, const zfw_u_vec_4d bg_color) {
    ZFW_AssertRenderingContextValidity(rendering_context);
    assert(rect.width > 0.0f && rect.height > 0.0f);
    assert(perc >= 0.0f && perc <= 1.0f);
    assert(ZFW_IsColorValid(front_color));
    assert(ZFW_IsColorValid(bg_color));

    const float front_rect_width = rect.width * perc;

    if (front_rect_width > 0.0f) {
        ZFW_RenderRect(rendering_context, (zfw_s_rect){rect.x, rect.y, front_rect_width, rect.height}, front_color);
    }

    const float bg_rect_x = rect.x + front_rect_width;
    const float bg_rect_width = rect.width - front_rect_width;

    if (bg_rect_width > 0.0f) {
        ZFW_RenderRect(rendering_context, (zfw_s_rect){bg_rect_x, rect.y, bg_rect_width, rect.height}, bg_color);
    }
}

void ZFW_RenderBarVer(const zfw_s_rendering_context* const rendering_context, const zfw_s_rect rect, const float perc, const zfw_u_vec_4d front_color, const zfw_u_vec_4d bg_color) {
    ZFW_AssertRenderingContextValidity(rendering_context);
    assert(rect.width > 0.0f && rect.height > 0.0f);
    assert(perc >= 0.0f && perc <= 1.0f);
    assert(ZFW_IsColorValid(front_color));
    assert(ZFW_IsColorValid(bg_color));

    const float front_rect_height = rect.height * perc;

    if (front_rect_height > 0.0f) {
        ZFW_RenderRect(rendering_context, (zfw_s_rect){rect.x, rect.y, rect.width, front_rect_height}, front_color);
    }

    const float bg_rect_y = rect.x + front_rect_height;
    const float bg_rect_height = rect.width - front_rect_height;

    if (bg_rect_height > 0.0f) {
        ZFW_RenderRect(rendering_context, (zfw_s_rect){rect.x, bg_rect_y, rect.width, bg_rect_height}, bg_color);
    }
}

bool ZFW_RenderStr(const zfw_s_rendering_context* const rendering_context, const char* const str, const zfw_s_font_group* const fonts, const int font_index, const zfw_s_vec_2d pos, const zfw_s_vec_2d alignment, const zfw_u_vec_4d color, s_mem_arena* const temp_mem_arena) {
    ZFW_AssertRenderingContextValidity(rendering_context);
    assert(str && str[0]);
    ZFW_AssertFontGroupValidity(fonts);
    assert(font_index >= 0 && font_index < fonts->cnt);
    assert(ZFW_IsStrAlignmentValid(alignment));
    assert(ZFW_IsColorValid(color));
    assert(temp_mem_arena && IsMemArenaValid(temp_mem_arena));

    const zfw_s_vec_2d* const chr_render_positions = ZFW_PushStrChrRenderPositions(temp_mem_arena, str, fonts, font_index, pos, alignment);

    if (!chr_render_positions) {
        LOG_ERROR("Failed to reserve memory for character render positions!");
        return false;
    }

    for (int i = 0; str[i]; i++) {
        const char chr = str[i];

        if (chr == ' ' || chr == '\n') {
            continue;
        }

        assert(isprint(chr));

        const int chr_ascii_printable_index = chr - ZFW_ASCII_PRINTABLE_MIN;

        const zfw_s_rect_int chr_src_rect = {
            .x = fonts->tex_chr_positions[font_index][chr_ascii_printable_index].x,
            .y = fonts->tex_chr_positions[font_index][chr_ascii_printable_index].y,
            .width = fonts->arrangement_infos[font_index].chr_sizes[chr_ascii_printable_index].x,
            .height = fonts->arrangement_infos[font_index].chr_sizes[chr_ascii_printable_index].y
        };

        const zfw_s_rect_edges chr_tex_coords = ZFW_TextureCoords(chr_src_rect, fonts->tex_sizes[font_index]);

        const zfw_s_batch_slot_write_info write_info = {
            .tex_gl_id = fonts->tex_gl_ids[font_index],
            .tex_coords = chr_tex_coords,
            .pos = chr_render_positions[i],
            .size = {chr_src_rect.width, chr_src_rect.height},
            .blend = color
        };

        ZFW_Render(rendering_context, &write_info);
    }

    return true;
}

void ZFW_SubmitBatch(const zfw_s_rendering_context* const rendering_context) {
    ZFW_AssertRenderingContextValidity(rendering_context);

    if (rendering_context->state->batch.num_slots_used == 0) {
        // Nothing to flush!
        return;
    }

    const zfw_t_gl_id va_gl_id = rendering_context->basis->renderables.vert_array_gl_ids[zfw_ek_renderable_batch];
    const zfw_t_gl_id vb_gl_id = rendering_context->basis->renderables.vert_buf_gl_ids[zfw_ek_renderable_batch];
    const zfw_t_gl_id eb_gl_id = rendering_context->basis->renderables.elem_buf_gl_ids[zfw_ek_renderable_batch];

    //
    // Submitting Vertex Data to GPU
    //
    glBindVertexArray(va_gl_id);
    glBindBuffer(GL_ARRAY_BUFFER, vb_gl_id);

    {
        const size_t write_size = sizeof(zfw_t_batch_slot) * rendering_context->state->batch.num_slots_used;
        glBufferSubData(GL_ARRAY_BUFFER, 0, write_size, rendering_context->state->batch.slots);
    }

    //
    // Rendering the Batch
    //
    const zfw_t_gl_id prog_gl_id = rendering_context->basis->builtin_shader_progs.gl_ids[zfw_ek_builtin_shader_prog_batch];

    glUseProgram(prog_gl_id);

    const int view_uniform_loc = glGetUniformLocation(prog_gl_id, "u_view");
    glUniformMatrix4fv(view_uniform_loc, 1, GL_FALSE, &rendering_context->state->view_mat.elems[0][0]);

    const zfw_s_matrix_4x4 proj_mat = ZFW_OrthographicMatrix(0.0f, rendering_context->window_size.x, rendering_context->window_size.y, 0.0f, -1.0f, 1.0f);

    const int proj_uniform_loc = glGetUniformLocation(prog_gl_id, "u_proj");
    glUniformMatrix4fv(proj_uniform_loc, 1, GL_FALSE, &proj_mat.elems[0][0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rendering_context->state->batch.tex_gl_id);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eb_gl_id);
    glDrawElements(GL_TRIANGLES, ZFW_BATCH_SLOT_ELEM_CNT * rendering_context->state->batch.num_slots_used, GL_UNSIGNED_SHORT, NULL);

    glUseProgram(0);

    rendering_context->state->batch.num_slots_used = 0;
    rendering_context->state->batch.tex_gl_id = 0;
}

static inline zfw_t_gl_id BoundGLFramebuffer() {
    int fb;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fb);
    return fb;
}

void ZFW_SetSurface(const zfw_s_rendering_context* const rendering_context, const zfw_s_surface_group* const surfs, const int surf_index) {
    ZFW_AssertRenderingContextValidity(rendering_context);
    ZFW_AssertSurfaceGroupValidity(surfs);
    assert(surf_index >= 0 && surf_index < surfs->cnt);

    const zfw_t_gl_id fb_gl_id = surfs->fb_gl_ids[surf_index];
    assert(fb_gl_id != BoundGLFramebuffer() && "Trying to set a surface that is already set!");

    ZFW_SubmitBatch(rendering_context);

    glBindFramebuffer(GL_FRAMEBUFFER, fb_gl_id);
}

void ZFW_UnsetSurface(const zfw_s_rendering_context* const rendering_context) {
    ZFW_AssertRenderingContextValidity(rendering_context);

    assert(BoundGLFramebuffer() != 0 && "Trying to unset surface but no OpenGL framebuffer is bound!");

    ZFW_SubmitBatch(rendering_context);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static inline zfw_t_gl_id CurrentGLShaderProgram() {
    int prog;
    glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
    return prog;
}

void ZFW_SetSurfaceShaderProg(const zfw_s_rendering_context* const rendering_context, const zfw_s_shader_prog_group* const progs, const int prog_index) {
    ZFW_AssertRenderingContextValidity(rendering_context);
    ZFW_AssertShaderProgGroupValidity(progs);
    assert(prog_index >= 0 && prog_index < progs->cnt);

    assert(CurrentGLShaderProgram() == 0 && "Potential attempted double-assignment of surface shader program?");

    glUseProgram(progs->gl_ids[prog_index]);
}

void ZFW_SetSurfaceShaderProgUniform(const zfw_s_rendering_context* const rendering_context, const char* const name, const zfw_s_shader_prog_uniform_value val) {
    ZFW_AssertRenderingContextValidity(rendering_context);
    assert(name);

    const zfw_t_gl_id prog_gl_id = CurrentGLShaderProgram();

    assert(prog_gl_id != 0 && "Surface shader program must be set before setting uniforms!");

    const int loc = glGetUniformLocation(prog_gl_id, name);
    assert(loc != -1 && "Failed to get location of shader uniform!");

    switch (val.type) {
        case zfw_ek_shader_prog_uniform_value_type_int:
            glUniform1i(loc, val.as_int);
            break;

        case zfw_ek_shader_prog_uniform_value_type_float:
            glUniform1f(loc, val.as_float);
            break;

        case zfw_ek_shader_prog_uniform_value_type_v2:
            glUniform2f(loc, val.as_v2.x, val.as_v2.y);
            break;

        case zfw_ek_shader_prog_uniform_value_type_v3:
            glUniform3f(loc, val.as_v3.x, val.as_v3.y, val.as_v3.z);
            break;

        case zfw_ek_shader_prog_uniform_value_type_v4:
            glUniform4f(loc, val.as_v4.x, val.as_v4.y, val.as_v4.z, val.as_v4.w);
            break;

        case zfw_ek_shader_prog_uniform_value_type_mat4x4:
            glUniformMatrix4fv(loc, 1, false, &val.as_mat4x4.elems[0][0]);
            break;
    }
}

void ZFW_RenderSurface(const zfw_s_rendering_context* const rendering_context, const zfw_s_surface_group* const surfs, const int surf_index) {
    ZFW_AssertRenderingContextValidity(rendering_context);
    ZFW_AssertSurfaceGroupValidity(surfs);
    assert(surf_index >= 0 && surf_index < surfs->cnt);

    assert(CurrentGLShaderProgram() != 0 && "Surface shader program must be set before rendering a surface!");

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, surfs->fb_tex_gl_ids[surf_index]);

    glBindVertexArray(rendering_context->basis->renderables.vert_array_gl_ids[zfw_ek_renderable_surface]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rendering_context->basis->renderables.elem_buf_gl_ids[zfw_ek_renderable_surface]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);

    glUseProgram(0);
}
