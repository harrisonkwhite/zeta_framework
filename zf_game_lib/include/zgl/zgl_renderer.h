#pragma once

#include <zcl.h>

namespace zf::renderer {
    // Initialises the renderer module. This depends on the platform module being initialised beforehand.
    void Init();

    void Shutdown();

    // ============================================================
    // @section: Resources
    // ============================================================
    enum e_resource_type {
        ek_resource_type_invalid,
        ek_resource_type_mesh,
        ek_resource_type_shader_prog,
        ek_resource_type_uniform,
        ek_resource_type_texture
    };

    struct s_resource;

    struct s_resource_arena {
        s_ptr<s_mem_arena> mem_arena;
        s_ptr<s_resource> head;
        s_ptr<s_resource> tail;
    };

    inline s_resource_arena CreateResourceArena(s_mem_arena &mem_arena) {
        return {.mem_arena = &mem_arena};
    }

    void DestroyResources(s_resource_arena &arena);

    [[nodiscard]] t_b8 CreateMesh(const t_i32 vert_cnt, s_resource_arena &arena, s_ptr<s_resource> &o_resource);
    void UpdateMeshVerts(const s_resource &mesh, const t_len vert_begin, const s_array_rdonly<t_f32> vert_data);

    [[nodiscard]] t_b8 CreateShaderProg(const s_array_rdonly<t_u8> vert_shader_bin, const s_array_rdonly<t_u8> frag_shader_bin, s_resource_arena &arena, s_ptr<s_resource> &o_resource);

    [[nodiscard]] t_b8 CreateUniform(const s_str_rdonly name, s_resource_arena &arena, s_mem_arena &temp_mem_arena, s_ptr<s_resource> &o_resource);

    [[nodiscard]] t_b8 CreateTexture(const s_texture_data_rdonly texture_data, s_resource_arena &arena, s_ptr<s_resource> &o_resource);
    s_v2_i TextureSize(const s_resource &texture);

    // ============================================================
    // @section: Rendering
    // ============================================================
    void BeginFrame(const s_color_rgb24f clear_col);
    void CompleteFrame();

    void DrawMesh(const s_resource &mesh, const t_len vert_begin, const t_len vert_cnt, const s_resource &shader_prog, const s_resource &texture, const s_resource &texture_sampler_uniform);
}
