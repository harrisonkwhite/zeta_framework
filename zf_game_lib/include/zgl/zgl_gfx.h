#pragma once

#include <zcl.h>

namespace zf {
    // Initialises the GFX module. This depends on the platform module being initialised beforehand.
    void InitGFX();

    void ShutdownGFX();

    // ============================================================
    // @section: Resources
    // ============================================================
    enum e_gfx_resource_type {
        ek_gfx_resource_type_invalid,
        ek_gfx_resource_type_mesh,
        ek_gfx_resource_type_shader_prog,
        ek_gfx_resource_type_texture
    };

    struct s_gfx_resource;

    struct s_gfx_resource_arena {
        s_ptr<s_mem_arena> mem_arena = nullptr;
        s_ptr<s_gfx_resource> head = nullptr;
        s_ptr<s_gfx_resource> tail = nullptr;
    };

    [[nodiscard]] t_b8 CreateMesh(const s_array_rdonly<t_f32> verts, const s_array_rdonly<t_u16> elems, const s_array_rdonly<t_i32> vert_attr_lens, s_gfx_resource_arena &arena, s_ptr<s_gfx_resource> &o_mesh);
    [[nodiscard]] t_b8 CreateShaderProg(const s_str_rdonly vert_src, const s_str_rdonly frag_src, s_gfx_resource_arena &res_arena, s_mem_arena &temp_mem_arena, s_ptr<s_gfx_resource> &o_prog);
    [[nodiscard]] t_b8 CreateTexture(const s_texture_data_rdonly tex_data, s_gfx_resource_arena &arena, s_ptr<s_gfx_resource> &o_tex);

    // ============================================================
    // @section: Rendering
    // ============================================================
    enum e_shader_prog_uniform_val_type : t_i32 {
        ek_shader_prog_uniform_val_type_i32,
        ek_shader_prog_uniform_val_type_u32,
        ek_shader_prog_uniform_val_type_f32,
        ek_shader_prog_uniform_val_type_v2,
        ek_shader_prog_uniform_val_type_v3,
        ek_shader_prog_uniform_val_type_v4,
        ek_shader_prog_uniform_val_type_mat4x4
    };

    struct s_shader_prog_uniform_val {
    public:
        constexpr s_shader_prog_uniform_val(const t_i32 v) : type(ek_shader_prog_uniform_val_type_i32), type_data({.i32 = v}) {};
        constexpr s_shader_prog_uniform_val(const t_u32 v) : type(ek_shader_prog_uniform_val_type_u32), type_data({.u32 = v}) {};
        constexpr s_shader_prog_uniform_val(const t_f32 v) : type(ek_shader_prog_uniform_val_type_f32), type_data({.f32 = v}) {};
        constexpr s_shader_prog_uniform_val(const s_v2 v) : type(ek_shader_prog_uniform_val_type_v2), type_data({.v2 = v}) {};
        constexpr s_shader_prog_uniform_val(const s_v3 v) : type(ek_shader_prog_uniform_val_type_v3), type_data({.v3 = v}) {};
        constexpr s_shader_prog_uniform_val(const s_v4 v) : type(ek_shader_prog_uniform_val_type_v4), type_data({.v4 = v}) {};
        constexpr s_shader_prog_uniform_val(const s_mat4x4 &v) : type(ek_shader_prog_uniform_val_type_mat4x4), type_data({.mat4x4 = v}) {};

        constexpr e_shader_prog_uniform_val_type Type() const {
            return type;
        }

        constexpr t_i32 &I32() {
            ZF_ASSERT(type == ek_shader_prog_uniform_val_type_i32);
            return type_data.i32;
        }

        constexpr const t_i32 &I32() const {
            ZF_ASSERT(type == ek_shader_prog_uniform_val_type_i32);
            return type_data.i32;
        }

        constexpr t_u32 &U32() {
            ZF_ASSERT(type == ek_shader_prog_uniform_val_type_u32);
            return type_data.u32;
        }

        constexpr const t_u32 &U32() const {
            ZF_ASSERT(type == ek_shader_prog_uniform_val_type_u32);
            return type_data.u32;
        }

        constexpr t_f32 &F32() {
            ZF_ASSERT(type == ek_shader_prog_uniform_val_type_f32);
            return type_data.f32;
        }

        constexpr const t_f32 &F32() const {
            ZF_ASSERT(type == ek_shader_prog_uniform_val_type_f32);
            return type_data.f32;
        }

        constexpr s_v2 &V2() {
            ZF_ASSERT(type == ek_shader_prog_uniform_val_type_v2);
            return type_data.v2;
        }

        constexpr const s_v2 &V2() const {
            ZF_ASSERT(type == ek_shader_prog_uniform_val_type_v2);
            return type_data.v2;
        }

        constexpr s_v3 &V3() {
            ZF_ASSERT(type == ek_shader_prog_uniform_val_type_v3);
            return type_data.v3;
        }

        constexpr const s_v3 &V3() const {
            ZF_ASSERT(type == ek_shader_prog_uniform_val_type_v3);
            return type_data.v3;
        }

        constexpr s_v4 &V4() {
            ZF_ASSERT(type == ek_shader_prog_uniform_val_type_v4);
            return type_data.v4;
        }

        constexpr const s_v4 &V4() const {
            ZF_ASSERT(type == ek_shader_prog_uniform_val_type_v4);
            return type_data.v4;
        }

        constexpr s_mat4x4 &Mat4x4() {
            ZF_ASSERT(type == ek_shader_prog_uniform_val_type_mat4x4);
            return type_data.mat4x4;
        }

        constexpr const s_mat4x4 &Mat4x4() const {
            ZF_ASSERT(type == ek_shader_prog_uniform_val_type_mat4x4);
            return type_data.mat4x4;
        }

    private:
        e_shader_prog_uniform_val_type type = {};

        union {
            t_i32 i32;
            t_u32 u32;
            t_f32 f32;
            s_v2 v2;
            s_v3 v3;
            s_v4 v4;
            s_mat4x4 mat4x4;
        } type_data = {};
    };

    enum e_render_instr_type {
        ek_render_instr_type_invalid,
        ek_render_instr_type_clear,
        ek_render_instr_type_shader_prog_set,
        ek_render_instr_type_shader_prog_uniform_set,
        ek_render_instr_type_mesh_update,
        ek_render_instr_type_mesh_draw
    };

    // @todo: If the list of these is wrapped in a struct, can bypass needing to use pointer and can also encapsulate this implementation within the module.
    struct s_render_instr {
    public:
        e_render_instr_type type = ek_render_instr_type_invalid;

        auto &Clear() {
            ZF_ASSERT(type == ek_render_instr_type_clear);
            return type_data.clear;
        }

        auto &Clear() const {
            ZF_ASSERT(type == ek_render_instr_type_clear);
            return type_data.clear;
        }

        auto &ShaderProgSet() {
            ZF_ASSERT(type == ek_render_instr_type_shader_prog_set);
            return type_data.shader_prog_set;
        }

        auto &ShaderProgSet() const {
            ZF_ASSERT(type == ek_render_instr_type_shader_prog_set);
            return type_data.shader_prog_set;
        }

        auto &ShaderProgUniformSet() {
            ZF_ASSERT(type == ek_render_instr_type_shader_prog_uniform_set);
            return type_data.shader_prog_uniform_set;
        }

        auto &ShaderProgUniformSet() const {
            ZF_ASSERT(type == ek_render_instr_type_shader_prog_uniform_set);
            return type_data.shader_prog_uniform_set;
        }

        auto &MeshUpdate() {
            ZF_ASSERT(type == ek_render_instr_type_mesh_update);
            return type_data.mesh_update;
        }

        auto &MeshUpdate() const {
            ZF_ASSERT(type == ek_render_instr_type_mesh_update);
            return type_data.mesh_update;
        }

        auto &MeshDraw() {
            ZF_ASSERT(type == ek_render_instr_type_mesh_draw);
            return type_data.mesh_draw;
        }

        auto &MeshDraw() const {
            ZF_ASSERT(type == ek_render_instr_type_mesh_draw);
            return type_data.mesh_draw;
        }

    private:
        union {
            struct {
                s_color_rgb24f col;
            } clear;

            struct {
                s_ptr<const s_gfx_resource> prog;
            } shader_prog_set;

            struct {
                s_str_rdonly name;
                s_shader_prog_uniform_val val;
            } shader_prog_uniform_set;

            struct {
                s_ptr<const s_gfx_resource> mesh;
                s_array_rdonly<t_f32> verts;
                s_array_rdonly<t_u16> elems;
            } mesh_update;

            struct {
                s_ptr<const s_gfx_resource> mesh;
                s_ptr<const s_gfx_resource> tex;
            } mesh_draw;
        } type_data = {};
    };

    struct s_render_instr_seq {
    public:
        s_render_instr_seq(s_mem_arena &mem_arena) : blocks_mem_arena(mem_arena) {}

        [[nodiscard]] t_b8 SubmitClear(const s_color_rgb24f col);

        [[nodiscard]] t_b8 SubmitShaderProgSet(const s_ptr<const s_gfx_resource> prog);

        [[nodiscard]] t_b8 SubmitShaderProgUniformSet(const e_shader_prog_uniform_val_type val_type, const s_shader_prog_uniform_val &val);

        // Leave verts as empty if you want to leave the vertices as they are.
        // Leave elems as empty if you want to leave the elements as they are.
        [[nodiscard]] t_b8 SubmitMeshUpdate(const s_ptr<const s_gfx_resource> mesh, const s_array_rdonly<t_f32> verts, const s_array_rdonly<t_u16> elems);

        [[nodiscard]] t_b8 SubmitMeshDraw(const s_ptr<s_gfx_resource> mesh, const s_ptr<const s_gfx_resource> tex);

        [[nodiscard]] t_b8 Exec(s_mem_arena &temp_mem_arena);

    private:
        struct s_render_instr_block {
            s_static_list<s_render_instr, 32> instrs = {};
            s_ptr<s_render_instr_block> next = nullptr;
        };

        s_mem_arena &blocks_mem_arena;
        s_ptr<s_render_instr_block> blocks_head = nullptr;
        s_ptr<s_render_instr_block> blocks_tail = nullptr;
    };
}
