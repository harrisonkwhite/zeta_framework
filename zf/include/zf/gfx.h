#pragma once

#include <glad/glad.h>
#include <zc/mem/arrays.h>
#include <zc/math.h>
#include <zc/gfx.h>

namespace zf {
    using t_gl_id = GLuint;

    enum class ec_gfx_resource_type {
        invalid,
        mesh,
        shader_prog,
        texture
    };

    struct s_gl_mesh {
        t_gl_id vert_arr_gl_id = 0;
        t_gl_id vert_buf_gl_id = 0;
        t_gl_id elem_buf_gl_id = 0;
    };

    struct s_gfx_resource_handle {
        ec_gfx_resource_type type = ec_gfx_resource_type::invalid;

        // @todo: These should really be in a union, but unions are a pain in C++...
        // Alternative could just be to point into some GL ID table, where the number of GL IDs corresponds to the resource type.
        s_gl_mesh mesh;
        struct { t_gl_id gl_id = 0; } shader_prog;
        struct { t_gl_id gl_id = 0; } tex;

        t_b8 IsValid() const {
            return type != ec_gfx_resource_type::invalid;
        }

        t_b8 Equals(const s_gfx_resource_handle& other) const {
            if (type != other.type) {
                return false;
            }

            switch (type) {
            case ec_gfx_resource_type::mesh:
                return mesh.vert_arr_gl_id == other.mesh.vert_arr_gl_id
                    && mesh.vert_buf_gl_id == other.mesh.vert_buf_gl_id
                    && mesh.elem_buf_gl_id == other.mesh.elem_buf_gl_id;

            case ec_gfx_resource_type::shader_prog:
                return shader_prog.gl_id == other.shader_prog.gl_id;

            case ec_gfx_resource_type::texture:
                return tex.gl_id == other.tex.gl_id;
            }

            return true;
        }
    };

    class c_gfx_resource_arena {
    public:
        [[nodiscard]] t_b8 Init(c_mem_arena& mem_arena, const t_s32 cap);
        void Release();

        s_gfx_resource_handle AddMesh(const t_f32* const verts_raw, const t_s32 verts_len, const c_array<const unsigned short> elems, const c_array<const t_s32> vert_attr_lens); // You might not want to provide vertices to start with, and only the count - passing nullptr in for verts_raw allows this.
        s_gfx_resource_handle AddShaderProg(const s_str_view vert_src, const s_str_view frag_src, c_mem_arena& temp_mem_arena);
        s_gfx_resource_handle AddTexture(const c_rgba_texture& rgba_tex);

    private:
        c_array<s_gfx_resource_handle> m_hdls; // @todo: Consider making this a dynamic array. Any capacity on this is kind of arbitrary...
        t_s32 m_hdls_taken = 0;
    };

    struct s_texture {
        s_gfx_resource_handle hdl;
        s_v2<t_s32> size;

        [[nodiscard]] t_b8 LoadFromRGBA(const c_rgba_texture& rgba_tex, c_gfx_resource_arena& gfx_res_arena);
        [[nodiscard]] t_b8 LoadFromRaw(const s_str_view file_path, c_gfx_resource_arena& gfx_res_arena, c_mem_arena& temp_mem_arena);
        [[nodiscard]] t_b8 LoadFromPacked(const s_str_view file_path, c_gfx_resource_arena& gfx_res_arena);
    };
}
