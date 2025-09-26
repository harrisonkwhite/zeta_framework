#pragma once

#include <glad/glad.h>

namespace zf {
    constexpr t_s32 g_gl_version_major = 4;
    constexpr t_s32 g_gl_version_minor = 3;

    using t_gl_id = t_u32;

    enum e_gl_resource_type {
        ek_gl_resource_type_texture,
        ek_gl_resource_type_shader_prog,
        ek_gl_resource_type_vert_array,
        ek_gl_resource_type_vert_buf,
        ek_gl_resource_type_elem_buf,
        ek_gl_resource_type_framebuffer,

        eks_gl_resource_type_cnt
    };

    class s_gl_resource_arena {
    public:
        [[nodiscard]] bool Init(c_mem_arena& mem_arena, t_s32 res_limit);
        void Clean();
        t_gl_id Push(const e_gl_resource_type res_type);
        c_array<t_gl_id> PushArray(const t_s32 cnt, const e_gl_resource_type res_type);

    private:
        c_array<t_gl_id> ids;
        c_array<e_gl_resource_type> res_types;

        t_s32 res_used;
        t_s32 res_limit;
    };

    struct s_renderable {
        t_gl_id vert_array_gl_id;
        t_gl_id vert_buf_gl_id;
        t_gl_id elem_buf_gl_id;
    };

    t_gl_id GenGLTextureFromRGBA(const s_rgba_texture& rgba_tex);
}
