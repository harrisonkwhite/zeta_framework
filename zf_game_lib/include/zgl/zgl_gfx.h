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
        ek_gfx_resource_type_uniform,
        ek_gfx_resource_type_texture
    };

    struct s_gfx_resource;

    struct s_gfx_resource_arena {
        s_ptr<s_mem_arena> mem_arena = nullptr;
        s_ptr<s_gfx_resource> head = nullptr;
        s_ptr<s_gfx_resource> tail = nullptr;
    };

    inline s_gfx_resource_arena CreateGFXResourceArena(s_mem_arena &mem_arena) {
        return {.mem_arena = &mem_arena};
    }

    void DestroyGFXResources(s_gfx_resource_arena &arena);

    [[nodiscard]] t_b8 CreateMesh(const t_len verts_len, s_gfx_resource_arena &arena, s_ptr<s_gfx_resource> &o_resource);

    [[nodiscard]] t_b8 CreateShaderProg(const s_array_rdonly<t_u8> vert_shader_bin, const s_array_rdonly<t_u8> frag_shader_bin, s_gfx_resource_arena &arena, s_ptr<s_gfx_resource> &o_resource);

    [[nodiscard]] t_b8 CreateUniform(const s_str_rdonly name, s_gfx_resource_arena &arena, s_mem_arena &temp_mem_arena, s_ptr<s_gfx_resource> &o_resource);

    [[nodiscard]] t_b8 CreateTexture(const s_texture_data_rdonly texture_data, s_gfx_resource_arena &arena, s_ptr<s_gfx_resource> &o_resource);
    s_v2_i TextureSize(const s_gfx_resource &texture_resource);

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

    struct s_render_instr;

    struct s_render_instr_seq {
    public:
        s_render_instr_seq() = default;
        s_render_instr_seq(s_mem_arena &mem_arena) : blocks_mem_arena(&mem_arena) {}

        void SubmitClear(const s_color_rgb24f col);
        void SubmitMeshUpdate(const s_gfx_resource &mesh, const s_array_rdonly<t_f32> verts);
        void SubmitTextureSet(const s_gfx_resource &tex, const s_gfx_resource &sampler_uniform);
        void SubmitMeshDraw(const s_gfx_resource &mesh, const s_gfx_resource &prog);

        void Exec(s_mem_arena &temp_mem_arena);

    private:
        void Submit(const s_render_instr instr);

        struct s_render_instr_block;
        s_ptr<s_mem_arena> blocks_mem_arena;
        s_ptr<s_render_instr_block> blocks_head;
        s_ptr<s_render_instr_block> blocks_tail;
    };
}
