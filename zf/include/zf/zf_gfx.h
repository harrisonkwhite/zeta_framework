#pragma once

#include <glad/glad.h>
#include <zc.h>

namespace zf {
    using t_gl_id = GLuint;

    enum class ec_gfx_resource_type {
        invalid,
        mesh,
        shader_prog,
        texture
    };

    struct s_gl_mesh {
        t_gl_id vert_arr_gl_id;
        t_gl_id vert_buf_gl_id;
        t_gl_id elem_buf_gl_id;
    };

    struct s_gl_shader_prog {
        t_gl_id gl_id;
    };

    struct s_gl_texture {
        t_gl_id gl_id;
    };

    struct s_gfx_resource_handle {
        s_gfx_resource_handle() = default;
        s_gfx_resource_handle(const s_gl_mesh mesh) : type(ec_gfx_resource_type::mesh), raw({.mesh = mesh}) {}
        s_gfx_resource_handle(const s_gl_shader_prog shader_prog) : type(ec_gfx_resource_type::shader_prog), raw({.shader_prog = shader_prog}) {}
        s_gfx_resource_handle(const s_gl_texture tex) : type(ec_gfx_resource_type::texture), raw({.tex = tex}) {}

        ec_gfx_resource_type Type() const {
            return type;
        }

        t_b8 IsValid() const {
            return type != ec_gfx_resource_type::invalid;
        }

        const s_gl_mesh& Mesh() const {
            ZF_ASSERT(type == ec_gfx_resource_type::mesh);
            return raw.mesh;
        }

        const s_gl_shader_prog& ShaderProg() const {
            ZF_ASSERT(type == ec_gfx_resource_type::shader_prog);
            return raw.shader_prog;
        }

        const s_gl_texture& Texture() const {
            ZF_ASSERT(type == ec_gfx_resource_type::texture);
            return raw.tex;
        }

    private:
        ec_gfx_resource_type type = ec_gfx_resource_type::invalid;

        union {
            s_gl_mesh mesh;
            s_gl_shader_prog shader_prog;
            s_gl_texture tex;
        } raw;
    };

    struct s_gfx_resource_arena {
        s_list<s_gfx_resource_handle> hdls;
    };

    t_b8 AreGFXResourcesEqual(const s_gfx_resource_handle& a, const s_gfx_resource_handle& b);
    void ReleaseGFXResource(const s_gfx_resource_handle hdl);

    [[nodiscard]] t_b8 MakeGFXResourceArena(s_mem_arena& mem_arena, const t_size cap, s_gfx_resource_arena& o_res_arena);
    void ReleaseGFXResourceArena(s_gfx_resource_arena& res_arena);

    s_gfx_resource_handle MakeMesh(s_gfx_resource_arena& gfx_res_arena, const t_f32* const verts_raw, const t_size verts_len, const s_array<const t_u16> elems, const s_array<const t_s32> vert_attr_lens); // You might not want to provide vertices to start with, and only the count - passing nullptr in for verts_raw allows this.
    s_gfx_resource_handle MakeShaderProg(s_gfx_resource_arena& gfx_res_arena, const s_str_rdonly vert_src, const s_str_rdonly frag_src, s_mem_arena& temp_mem_arena);
    s_gfx_resource_handle MakeTexture(s_gfx_resource_arena& gfx_res_arena, const s_rgba_texture_data& tex_data);

    struct s_texture_asset {
        s_gfx_resource_handle hdl;
        s_v2<t_s32> size_cache;
    };

    [[nodiscard]] inline t_b8 LoadTextureAsset(const s_rgba_texture_data& tex_data, s_gfx_resource_arena& gfx_res_arena, s_texture_asset& o_asset) {
        const auto hdl = MakeTexture(gfx_res_arena, tex_data);

        if (!hdl.IsValid()) {
            return false;
        }

        o_asset.hdl = hdl;
        o_asset.size_cache = tex_data.size_in_pxs;

        return true;
    }

    [[nodiscard]] inline t_b8 LoadTextureAssetFromRaw(const s_str_rdonly file_path, s_gfx_resource_arena& gfx_res_arena, s_mem_arena& temp_mem_arena, s_texture_asset& o_asset) {
        s_rgba_texture_data tex_data;

        if (!LoadRGBATextureDataFromRaw(file_path, temp_mem_arena, tex_data)) {
            return false;
        }

        return LoadTextureAsset(tex_data, gfx_res_arena, o_asset);
    }

    [[nodiscard]] inline t_b8 LoadTextureAssetFromPacked(const s_str_rdonly file_path, s_gfx_resource_arena& gfx_res_arena, s_mem_arena& temp_mem_arena, s_texture_asset& o_asset) {
        s_rgba_texture_data tex_data;

        if (!UnpackTexture(file_path, temp_mem_arena, tex_data)) {
            return false;
        }

        return LoadTextureAsset(tex_data, gfx_res_arena, o_asset);
    }
}
