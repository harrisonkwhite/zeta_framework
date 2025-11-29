#pragma once

#include <glad/glad.h>
#include <zc.h>

namespace zf::gfx {
    using t_gl_id = GLuint;

    enum e_resource_type : t_s32 {
        ek_resource_type_invalid,
        ek_resource_type_mesh,
        ek_resource_type_shader_prog,
        ek_resource_type_texture,
        ek_resource_type_surface
    };

    struct s_gl_mesh {
        t_gl_id vert_arr_gl_id;
        t_gl_id vert_buf_gl_id;
        t_gl_id elem_buf_gl_id;
    };

    struct s_resource_handle {
        e_resource_type type;

        union {
            s_gl_mesh mesh;

            struct {
                t_gl_id gl_id;
            } shader_prog;

            struct {
                t_gl_id gl_id;
            } tex;

            struct {
                t_gl_id fb_gl_id;
                t_gl_id tex_gl_id;
                s_v2<t_size> size;
            } surf;
        } raw;
    };

    enum e_shader_prog_uniform_val_type {
        ek_shader_prog_uniform_val_type_s32,
        ek_shader_prog_uniform_val_type_u32,
        ek_shader_prog_uniform_val_type_f32,
        ek_shader_prog_uniform_val_type_v2,
        ek_shader_prog_uniform_val_type_v3,
        ek_shader_prog_uniform_val_type_v4,
        ek_shader_prog_uniform_val_type_mat4x4
    };

    struct s_shader_prog_uniform_val {
        e_shader_prog_uniform_val_type type;

        union {
            t_s32 s32;
            t_u32 u32;
            t_f32 f32;
            s_v2<t_f32> v2;
            s_v3<t_f32> v3;
            s_v4<t_f32> v4;
            s_matrix_4x4 mat4x4;
        } type_data;

        s_shader_prog_uniform_val() = default;
        s_shader_prog_uniform_val(const t_s32 v) : type(ek_shader_prog_uniform_val_type_s32), type_data({.s32 = v}) {};
        s_shader_prog_uniform_val(const t_u32 v) : type(ek_shader_prog_uniform_val_type_u32), type_data({.u32 = v}) {};
        s_shader_prog_uniform_val(const t_f32 v) : type(ek_shader_prog_uniform_val_type_f32), type_data({.f32 = v}) {};
        s_shader_prog_uniform_val(const s_v2<t_f32> v) : type(ek_shader_prog_uniform_val_type_v2), type_data({.v2 = v}) {};
        s_shader_prog_uniform_val(const s_v3<t_f32> v) : type(ek_shader_prog_uniform_val_type_v3), type_data({.v3 = v}) {};
        s_shader_prog_uniform_val(const s_v4<t_f32> v) : type(ek_shader_prog_uniform_val_type_v4), type_data({.v4 = v}) {};
        s_shader_prog_uniform_val(const s_matrix_4x4& v) : type(ek_shader_prog_uniform_val_type_mat4x4), type_data({.mat4x4 = v}) {};
    };

    struct s_resource_arena {
        s_list<s_resource_handle> hdls;
    };

    inline t_b8 IsResourceHandleValid(const s_resource_handle& hdl) {
        return hdl.type != ek_resource_type_invalid;
    }

    inline s_resource_handle MakeMeshHandle(const s_gl_mesh gl_mesh) {
        ZF_ASSERT(gl_mesh.vert_arr_gl_id);
        ZF_ASSERT(gl_mesh.vert_buf_gl_id);
        ZF_ASSERT(gl_mesh.elem_buf_gl_id);

        return {
            .type = ek_resource_type_mesh,
            .raw = {.mesh = gl_mesh}
        };
    }

    inline s_resource_handle MakeShaderProgHandle(const t_gl_id shader_prog_gl_id) {
        ZF_ASSERT(shader_prog_gl_id);

        return {
            .type = ek_resource_type_shader_prog,
            .raw = {.shader_prog = {shader_prog_gl_id}}
        };
    }

    inline s_resource_handle MakeTextureHandle(const t_gl_id tex_gl_id) {
        ZF_ASSERT(tex_gl_id);

        return {
            .type = ek_resource_type_texture,
            .raw = {.tex = {tex_gl_id}}
        };
    }

    inline s_resource_handle MakeSurfaceHandle(const t_gl_id fb_gl_id, const t_gl_id tex_gl_id, const s_v2<t_size> size) {
        ZF_ASSERT(fb_gl_id);
        ZF_ASSERT(tex_gl_id);

        return {
            .type = ek_resource_type_surface,
            .raw = {.surf = {.fb_gl_id = fb_gl_id, .tex_gl_id = tex_gl_id, .size = size}}
        };
    }

    t_b8 AreResourcesEqual(const s_resource_handle& a, const s_resource_handle& b);
    void ReleaseResource(const s_resource_handle& hdl);

    [[nodiscard]] t_b8 MakeResourceArena(s_mem_arena& mem_arena, const t_size cap, s_resource_arena& o_res_arena);
    void ReleaseResources(s_resource_arena& res_arena);

    s_resource_handle MakeMesh(s_resource_arena& res_arena, const t_f32* const verts_raw, const t_size verts_len, const s_array_rdonly<t_u16> elems, const s_array_rdonly<t_s32> vert_attr_lens); // You might not want to provide vertices to start with, and only the count - passing nullptr in for verts_raw allows this.
    s_resource_handle MakeShaderProg(s_resource_arena& res_arena, const s_str_rdonly vert_src, const s_str_rdonly frag_src, s_mem_arena& temp_mem_arena);
    s_resource_handle MakeTexture(s_resource_arena& res_arena, const s_rgba_texture_data_rdonly& tex_data);
    s_resource_handle MakeSurface(s_resource_arena& res_arena, const s_v2<t_size> size);

    struct s_texture_asset {
        s_resource_handle hdl;
        s_v2<t_s32> size_cache;
    };

    [[nodiscard]] inline t_b8 LoadTextureAsset(const s_rgba_texture_data_rdonly& tex_data, s_resource_arena& res_arena, s_texture_asset& o_asset) {
        o_asset = {
            .hdl = MakeTexture(res_arena, tex_data),
            .size_cache = tex_data.size_in_pxs
        };

        return IsResourceHandleValid(o_asset.hdl);
    }

    [[nodiscard]] inline t_b8 LoadTextureAssetFromRaw(const s_str_rdonly file_path, s_resource_arena& res_arena, s_mem_arena& temp_mem_arena, s_texture_asset& o_asset) {
        s_rgba_texture_data tex_data;

        if (!LoadRGBATextureDataFromRaw(file_path, temp_mem_arena, temp_mem_arena, tex_data)) {
            return false;
        }

        return LoadTextureAsset(tex_data, res_arena, o_asset);
    }

    [[nodiscard]] inline t_b8 LoadTextureAssetFromPacked(const s_str_rdonly file_path, s_resource_arena& res_arena, s_mem_arena& temp_mem_arena, s_texture_asset& o_asset) {
        s_rgba_texture_data tex_data;

        if (!UnpackTexture(file_path, temp_mem_arena, temp_mem_arena, tex_data)) {
            return false;
        }

        return LoadTextureAsset(tex_data, res_arena, o_asset);
    }

    struct s_font_asset {
        s_font_arrangement arrangement;
        s_array<s_resource_handle> atlas_tex_hdls;
    };

    [[nodiscard]] t_b8 LoadFontAssetFromRaw(const s_str_rdonly file_path, const t_s32 height, const t_unicode_code_pt_bit_vec& code_pts, s_mem_arena& mem_arena, s_resource_arena& res_arena, s_mem_arena& temp_mem_arena, s_font_asset& o_asset, e_font_load_from_raw_result* const o_load_from_raw_res, t_unicode_code_pt_bit_vec* const o_unsupported_code_pts);
    [[nodiscard]] t_b8 LoadFontAssetFromPacked(const s_str_rdonly file_path, s_mem_arena& mem_arena, s_resource_arena& res_arena, s_mem_arena& temp_mem_arena, s_font_asset& o_asset);
}
