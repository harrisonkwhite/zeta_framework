#include "zfwc_graphics.h"

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

bool InitGLResourceArena(s_gl_resource_arena& res_arena, s_mem_arena& mem_arena, const t_s32 res_limit) {
    assert(IS_ZERO(res_arena));
    assert(res_limit > 0);

    res_arena.ids = mem_arena.Push<t_gl_id>(res_limit);

    if (res_arena.ids.IsEmpty()) {
        LOG_ERROR("Failed to reserve memory for OpenGL resource IDs!");
        return false;
    }

    res_arena.res_types = PushGLResourceTypeArrayToMemArena(&mem_arena, res_limit);

    if (IS_ZERO(res_arena.res_types)) {
        LOG_ERROR("Failed to reserve memory for OpenGL resource types!");
        return false;
    }

    res_arena.res_limit = res_limit;

    return true;
}

void CleanGLResourceArena(s_gl_resource_arena& res_arena) {
    for (t_s32 i = 0; i < res_arena.res_used; i++) {
        const t_gl_id gl_id = *GLIDElem(res_arena.ids, i);

        if (!gl_id) {
            continue;
        }

        const e_gl_resource_type res_type = *GLResourceTypeElem(res_arena.res_types, i);

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

s_gl_id_array PushToGLResourceArena(s_gl_resource_arena& res_arena, const t_s32 cnt, const e_gl_resource_type res_type) {
    if (res_arena.res_used + cnt > res_arena.res_limit) {
        LOG_ERROR("OpenGL resource arena is full!");
        return {};
    }

    const t_s32 res_used_prev = res_arena.res_used;
    res_arena.res_used += cnt;
    return s_gl_id_array{res_arena.ids.buf_raw + res_used_prev, cnt};
}

static size_t CalcStride(const s_s32_array_view vert_attr_lens) {
    t_s32 stride = 0;

    for (t_s32 i = 0; i < vert_attr_lens.elem_cnt; i++) {
        stride += sizeof(t_r32) * *S32ElemView(vert_attr_lens, i);
    }

    return stride;
}

static s_renderable GenRenderable(s_gl_resource_arena& gl_res_arena, const s_r32_array_view verts, const s_u16_array_view elems, const s_s32_array_view vert_attr_lens) {
    t_gl_id* const va_gl_id = PushToGLResourceArena(gl_res_arena, 1, ek_gl_resource_type_vert_array).buf_raw;

    if (!va_gl_id) {
        LOG_ERROR("Failed to reserve OpenGL vertex array ID for renderable!");
        return {};
    }

    t_gl_id* const vb_gl_id = PushToGLResourceArena(gl_res_arena, 1, ek_gl_resource_type_vert_buf).buf_raw;

    if (!vb_gl_id) {
        LOG_ERROR("Failed to reserve OpenGL vertex buffer ID for renderable!");
        return {};
    }

    t_gl_id* const eb_gl_id = PushToGLResourceArena(gl_res_arena, 1, ek_gl_resource_type_elem_buf).buf_raw;

    if (!eb_gl_id) {
        LOG_ERROR("Failed to reserve OpenGL element buffer ID for renderable!");
        return {};
    }

    glGenVertexArrays(1, va_gl_id);
    glBindVertexArray(*va_gl_id);

    glGenBuffers(1, vb_gl_id);
    glBindBuffer(GL_ARRAY_BUFFER, *vb_gl_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(*verts.buf_raw) * verts.elem_cnt, verts.buf_raw, GL_DYNAMIC_DRAW);

    glGenBuffers(1, eb_gl_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *eb_gl_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*elems.buf_raw) * elems.elem_cnt, elems.buf_raw, GL_STATIC_DRAW);

    const size_t stride = CalcStride(vert_attr_lens);
    t_s32 offs = 0;

    for (t_s32 i = 0; i < vert_attr_lens.elem_cnt; i++) {
        const t_s32 attr_len = *S32ElemView(vert_attr_lens, i);

        glVertexAttribPointer(i, attr_len, GL_FLOAT, false, stride, reinterpret_cast<void*>(sizeof(t_r32) * offs));
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
