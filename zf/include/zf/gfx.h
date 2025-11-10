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

    struct us_gl_mesh {
        t_gl_id vert_arr_gl_id;
        t_gl_id vert_buf_gl_id;
        t_gl_id elem_buf_gl_id;
    };

    struct us_gl_shader_prog {
        t_gl_id gl_id;
    };

    struct us_gl_texture {
        t_gl_id gl_id;
    };

    class c_gfx_resource_handle {
    public:
        c_gfx_resource_handle() = default;
        c_gfx_resource_handle(const us_gl_mesh mesh) : m_type(ec_gfx_resource_type::mesh), m_raw({.mesh = mesh}) {}
        c_gfx_resource_handle(const us_gl_shader_prog shader_prog) : m_type(ec_gfx_resource_type::shader_prog), m_raw({.shader_prog = shader_prog}) {}
        c_gfx_resource_handle(const us_gl_texture tex) : m_type(ec_gfx_resource_type::texture), m_raw({.tex = tex}) {}

        ec_gfx_resource_type Type() const {
            return m_type;
        }

        t_b8 IsValid() const {
            return m_type != ec_gfx_resource_type::invalid;
        }

        const us_gl_mesh& Mesh() const {
            ZF_ASSERT(m_type == ec_gfx_resource_type::mesh);
            return m_raw.mesh;
        }

        const us_gl_shader_prog& ShaderProg() const {
            ZF_ASSERT(m_type == ec_gfx_resource_type::shader_prog);
            return m_raw.shader_prog;
        }

        const us_gl_texture& Texture() const {
            ZF_ASSERT(m_type == ec_gfx_resource_type::texture);
            return m_raw.tex;
        }

        t_b8 Equals(const c_gfx_resource_handle& other) const {
            if (m_type != other.m_type) {
                return false;
            }

            switch (m_type) {
            case ec_gfx_resource_type::mesh:
                return m_raw.mesh.vert_arr_gl_id == other.m_raw.mesh.vert_arr_gl_id
                    && m_raw.mesh.vert_buf_gl_id == other.m_raw.mesh.vert_buf_gl_id
                    && m_raw.mesh.elem_buf_gl_id == other.m_raw.mesh.elem_buf_gl_id;

            case ec_gfx_resource_type::shader_prog:
                return m_raw.shader_prog.gl_id == other.m_raw.shader_prog.gl_id;

            case ec_gfx_resource_type::texture:
                return m_raw.tex.gl_id == other.m_raw.tex.gl_id;

            case ec_gfx_resource_type::invalid:
                return true;
            }
        }

    private:
        ec_gfx_resource_type m_type = ec_gfx_resource_type::invalid;

        union {
            us_gl_mesh mesh;
            us_gl_shader_prog shader_prog;
            us_gl_texture tex;
        } m_raw = {};
    };

    class c_gfx_resource_arena {
    public:
        [[nodiscard]] t_b8 Init(c_mem_arena& mem_arena, const t_size cap);
        void Release();

        c_gfx_resource_handle AddMesh(const t_f32* const verts_raw, const t_size verts_len, const c_array<const t_u16> elems, const c_array<const t_s32> vert_attr_lens); // You might not want to provide vertices to start with, and only the count - passing nullptr in for verts_raw allows this.
        c_gfx_resource_handle AddShaderProg(const s_str_view vert_src, const s_str_view frag_src, c_mem_arena& temp_mem_arena);
        c_gfx_resource_handle AddTexture(const c_rgba_texture& rgba_tex);

    private:
        c_array<c_gfx_resource_handle> m_hdls; // @todo: Consider making this a dynamic array. Any capacity on this is kind of arbitrary...
        t_size m_hdls_taken = 0;
    };

    struct s_texture {
        c_gfx_resource_handle hdl;
        s_v2<t_s32> size;

        [[nodiscard]] t_b8 LoadFromRGBA(const c_rgba_texture& rgba_tex, c_gfx_resource_arena& gfx_res_arena);
        [[nodiscard]] t_b8 LoadFromRaw(const s_str_view file_path, c_gfx_resource_arena& gfx_res_arena, c_mem_arena& temp_mem_arena);
        [[nodiscard]] t_b8 LoadFromPacked(const s_str_view file_path, c_gfx_resource_arena& gfx_res_arena);
    };
}
