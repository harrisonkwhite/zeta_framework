#pragma once

#include <glad/glad.h>
#include <zc/mem/arrays.h>
#include <zc/math.h>
#include <zc/gfx.h>

namespace zf {
    using t_gl_id = GLuint;

    enum class ec_gfx_resource_type {
        invalid,
        texture
    };

    struct s_gfx_resource_handle {
        ec_gfx_resource_type type = ec_gfx_resource_type::invalid;
        t_gl_id gl_id = 0;
    };

    class c_gfx_resource_arena {
    public:
        bool Init(c_mem_arena& mem_arena, const int cap);
        void Release();

        s_gfx_resource_handle AddTexture(const s_rgba_texture& rgba_tex);

    private:
        c_array<s_gfx_resource_handle> m_hdls;
        int m_hdls_taken = 0;
    };

#if 0
    class c_texture_group {
    public:
        bool Load(c_mem_arena& mem_arena, c_gfx_resource_arena& lifetime, const int cnt) {
            if (!m_hdls.Init(mem_arena, cnt)) {
                return false;
            }

            if (!m_sizes.Init(mem_arena, cnt)) {
                return false;
            }

            return true;
        }

    private:
        c_array<s_gfx_resource_handle> m_hdls;
        c_array<s_v2<int>> m_sizes;
    };
#endif
}
