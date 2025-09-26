#include "zf_gl.h"

namespace zf {
    static inline t_gl_id BoundGLFramebuffer() {
        t_s32 fb;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fb);
        return static_cast<t_gl_id>(fb);
    }

    static inline s_rect_s32 GLViewport() {
        s_rect_s32 vp;
        glGetIntegerv(GL_VIEWPORT, reinterpret_cast<t_s32*>(&vp));
        return vp;
    }

    static s_renderable GenRenderable(s_gl_resource_arena& gl_res_arena, const c_array<const float> verts, const c_array<const t_u16> elems, const c_array<const t_s32> vert_attr_lens) {
        t_gl_id va_gl_id = PushToGLResourceArena(gl_res_arena, ek_gl_resource_type_vert_array);

        if (!va_gl_id) {
            ZF_LOG_ERROR("Failed to reserve OpenGL vertex array ID for renderable!");
            return {};
        }

        t_gl_id vb_gl_id = PushToGLResourceArena(gl_res_arena, ek_gl_resource_type_vert_buf);

        if (!vb_gl_id) {
            ZF_LOG_ERROR("Failed to reserve OpenGL vertex buffer ID for renderable!");
            return {};
        }

        t_gl_id eb_gl_id = PushToGLResourceArena(gl_res_arena, ek_gl_resource_type_elem_buf);

        if (!eb_gl_id) {
            ZF_LOG_ERROR("Failed to reserve OpenGL element buffer ID for renderable!");
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

    bool s_gl_resource_arena::Init(c_mem_arena& mem_arena, const t_s32 res_limit) {
        assert(res_limit > 0);

        res_arena.ids = PushArrayToMemArena<t_gl_id>(mem_arena, res_limit);

        if (res_arena.ids.IsEmpty()) {
            ZF_LOG_ERROR("Failed to reserve memory for OpenGL resource IDs!");
            return false;
        }

        res_arena.res_types = PushArrayToMemArena<e_gl_resource_type>(mem_arena, res_limit);

        if (res_arena.res_types.IsEmpty()) {
            ZF_LOG_ERROR("Failed to reserve memory for OpenGL resource types!");
            return false;
        }

        res_arena.res_limit = res_limit;

        return true;
    }

    void s_gl_resource_arena::Clean() {
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
            ZF_LOG_ERROR("OpenGL resource arena is full!");
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
}
