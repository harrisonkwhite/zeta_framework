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

        bool IsValid() const {
            return type != ec_gfx_resource_type::invalid;
        }
    };

    class c_gfx_resource_arena {
    public:
        bool Init(c_mem_arena& mem_arena, const int cap);
        void Release();

        s_gfx_resource_handle AddTexture(const s_rgba_texture& rgba_tex);

    private:
        c_array<s_gfx_resource_handle> m_hdls; // @todo: Consider making this a dynamic array. Any capacity on this is kind of arbitrary...
        int m_hdls_taken = 0;
    };

    class c_texture_group {
    public:
        bool Load(c_mem_arena& mem_arena, c_gfx_resource_arena& lifetime, const int cnt, bool (* const rgba_tex_loader_func)(s_rgba_texture& rgba_tex, const int index));

        s_gfx_resource_handle GetHandle(const int index) {
            return m_hdls[index];
        }

        s_v2<int> GetSize(const int index) {
            return m_sizes[index];
        }

    private:
        c_array<s_gfx_resource_handle> m_hdls;
        c_array<s_v2<int>> m_sizes;
    };
}
