#pragma once

#include <zcl.h>

namespace zf {
    // ============================================================
    // @section: General
    // ============================================================
    struct s_rendering_basis;

    // Initialises the GFX module. This depends on the platform module being initialised beforehand.
    s_rendering_basis &InitGFX(s_mem_arena &mem_arena);

    void ShutdownGFX(s_rendering_basis &rendering_basis);

    // ============================================================
    // @section: Resources
    // ============================================================
    enum e_gfx_resource_type {
        ek_gfx_resource_type_invalid,
        ek_gfx_resource_type_texture
    };

    struct s_gfx_resource;

    s_v2_i TextureSize(const s_gfx_resource &texture);

    struct s_gfx_resource_arena {
    public:
        s_gfx_resource_arena() = default;
        s_gfx_resource_arena(s_mem_arena &mem_arena) : m_mem_arena(&mem_arena) {}

        void Release();

        t_b8 IsInitted() const { return m_mem_arena; }

        s_mem_arena &MemArena() const {
            ZF_ASSERT(IsInitted());
            return *m_mem_arena;
        }

        [[nodiscard]] t_b8 AddTexture(const s_texture_data_rdonly texture_data, s_ptr<s_gfx_resource> &o_resource);

        [[nodiscard]] t_b8 AddTextureFromRaw(const s_str_rdonly file_path, s_mem_arena &temp_mem_arena, s_ptr<s_gfx_resource> &o_resource) {
            ZF_ASSERT(IsInitted());

            s_texture_data texture_data;

            if (!LoadTextureFromRaw(file_path, temp_mem_arena, temp_mem_arena, texture_data)) {
                return false;
            }

            return AddTexture(texture_data, o_resource);
        }

        [[nodiscard]] t_b8 AddTextureFromPacked(const s_str_rdonly file_path, s_mem_arena &temp_mem_arena, s_ptr<s_gfx_resource> &o_resource) {
            ZF_ASSERT(IsInitted());

            s_texture_data texture_data;

            if (!UnpackTexture(file_path, temp_mem_arena, temp_mem_arena, texture_data)) {
                return false;
            }

            return AddTexture(texture_data, o_resource);
        }

    private:
        s_gfx_resource &Add(const e_gfx_resource_type type);

        s_ptr<s_mem_arena> m_mem_arena = nullptr;
        s_ptr<s_gfx_resource> m_head = nullptr;
        s_ptr<s_gfx_resource> m_tail = nullptr;
    };

    s_gfx_resource_arena &PermGFXResourceArena();

    // ============================================================
    // @section: Rendering
    // ============================================================
    struct s_rendering_context;

    struct s_batch_vert {
        s_v2 pos = {};
        s_color_rgba32f blend = {};
        s_v2 uv = {};
    };

    struct s_batch_triangle {
        s_static_array<s_batch_vert, 3> verts = {};
    };

    void SubmitTriangles(s_rendering_context &rc, const s_array_rdonly<s_batch_triangle> triangles, const s_ptr<const s_gfx_resource> texture);

    inline void SubmitTriangle(s_rendering_context &rc, const s_batch_triangle tri, const s_ptr<const s_gfx_resource> texture) {
        return SubmitTriangles(rc, {&tri, 1}, texture);
    }

    namespace internal {
        s_rendering_context &BeginFrame(const s_rendering_basis &rendering_basis, const s_color_rgb24f clear_col, s_mem_arena &mem_arena);
        void EndFrame(s_rendering_context &rc);
    }
}
