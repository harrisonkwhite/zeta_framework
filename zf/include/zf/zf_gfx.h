#pragma once

#include <glad/glad.h>
#include <zc.h>

namespace zf::gfx {
    using t_gl_id = GLuint;

    enum class ec_resource_type {
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

    struct s_resource_handle {
        ec_resource_type type;

        union {
            s_gl_mesh mesh;

            struct {
                t_gl_id gl_id;
            } shader_prog;

            struct {
                t_gl_id gl_id;
            } tex;
        } raw;
    };

    struct s_resource_arena {
        s_list<s_resource_handle> hdls;
    };

    inline t_b8 IsResourceHandleValid(const s_resource_handle& hdl) {
        return hdl.type != ec_resource_type::invalid;
    }

    inline s_resource_handle MakeMeshHandle(const s_gl_mesh gl_mesh) {
        ZF_ASSERT(gl_mesh.vert_arr_gl_id);
        ZF_ASSERT(gl_mesh.vert_buf_gl_id);
        ZF_ASSERT(gl_mesh.elem_buf_gl_id);

        return {
            .type = ec_resource_type::mesh,
            .raw = {.mesh = gl_mesh}
        };
    }

    inline s_resource_handle MakeShaderProgHandle(const t_gl_id shader_prog_gl_id) {
        ZF_ASSERT(shader_prog_gl_id);

        return {
            .type = ec_resource_type::shader_prog,
            .raw = {.shader_prog = {shader_prog_gl_id}}
        };
    }

    inline s_resource_handle MakeTextureHandle(const t_gl_id tex_gl_id) {
        ZF_ASSERT(tex_gl_id);

        return {
            .type = ec_resource_type::texture,
            .raw = {.tex = {tex_gl_id}}
        };
    }

    t_b8 AreResourcesEqual(const s_resource_handle& a, const s_resource_handle& b);
    void ReleaseResource(const s_resource_handle& hdl);

    [[nodiscard]] t_b8 MakeResourceArena(s_mem_arena& mem_arena, const t_size cap, s_resource_arena& o_res_arena);
    void ReleaseResources(s_resource_arena& res_arena);

    s_resource_handle MakeMesh(s_resource_arena& res_arena, const t_f32* const verts_raw, const t_size verts_len, const s_array_rdonly<t_u16> elems, const s_array_rdonly<t_s32> vert_attr_lens); // You might not want to provide vertices to start with, and only the count - passing nullptr in for verts_raw allows this.
    s_resource_handle MakeShaderProg(s_resource_arena& res_arena, const s_str_rdonly vert_src, const s_str_rdonly frag_src, s_mem_arena& temp_mem_arena);
    s_resource_handle MakeTexture(s_resource_arena& res_arena, const s_rgba_texture_data_rdonly& tex_data);

    struct s_texture_asset {
        s_resource_handle hdl;
        s_v2<t_s32> size_cache;
    };

    [[nodiscard]] inline t_b8 LoadTextureAsset(const s_rgba_texture_data_rdonly& tex_data, s_resource_arena& res_arena, s_texture_asset& o_asset) {
        const auto hdl = MakeTexture(res_arena, tex_data);

        if (!IsResourceHandleValid(hdl)) {
            return false;
        }

        o_asset.hdl = hdl;
        o_asset.size_cache = tex_data.size_in_pxs;

        return true;
    }

    [[nodiscard]] inline t_b8 LoadTextureAssetFromRaw(const s_str_rdonly file_path, s_resource_arena& res_arena, s_mem_arena& temp_mem_arena, s_texture_asset& o_asset) {
        ZF_DEFER_MEM_ARENA_REWIND(temp_mem_arena);

        s_rgba_texture_data tex_data;

        if (!LoadRGBATextureDataFromRaw(file_path, temp_mem_arena, tex_data)) {
            return false;
        }

        return LoadTextureAsset(tex_data, res_arena, o_asset);
    }

    [[nodiscard]] inline t_b8 LoadTextureAssetFromPacked(const s_str_rdonly file_path, s_resource_arena& res_arena, s_mem_arena& temp_mem_arena, s_texture_asset& o_asset) {
        ZF_DEFER_MEM_ARENA_REWIND(temp_mem_arena);

        s_rgba_texture_data tex_data;

        if (!UnpackTexture(file_path, temp_mem_arena, tex_data)) {
            return false;
        }

        return LoadTextureAsset(tex_data, res_arena, o_asset);
    }
}
