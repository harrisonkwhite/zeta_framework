#pragma once

#include <zgl/zgl_renderer.h>

namespace zf {
    struct s_batch_renderer_resources {
        renderer::s_resource_arena arena;

        s_ptr<renderer::s_resource> mesh;
        s_ptr<renderer::s_resource> shader_prog;
        s_ptr<renderer::s_resource> texture_sampler_uniform;
        s_ptr<renderer::s_resource> px_texture;
    };

    struct s_batch_renderer {
    public:
        void SubmitTriangle(const s_static_array<s_v2, 3> &pts, const s_static_array<s_color_rgba32f, 3> &pt_colors);
        void SubmitRect();
        void SubmitTexture();
        void SubmitStr();

    private:
        struct s_vert {
            s_v2 pos;
            s_color_rgba32f blend;
            s_v2 uv;

            s_vert() = default;
            s_vert(const s_v2 pos, const s_color_rgba32f blend, const s_v2 uv) : pos(pos), blend(blend), uv(uv) {}
        };

        void Flush();

        const s_batch_renderer_resources &basis;

        s_static_array<s_vert, 8192> verts;
        t_len verts_used_cnt = 0;

        s_ptr<const renderer::s_resource> texture_resource;
    };

    [[nodiscard]] t_b8 CreateBatchRenderer(s_batch_renderer_resources &o_resources);
    void DestroyBatchRenderer(s_batch_renderer_resources &o_resources);

#if 0
    struct s_rendering_basis {
        s_gfx_resource_arena gfx_res_arena;
        s_ptr<s_gfx_resource> batch_mesh_resource;
        s_ptr<s_gfx_resource> batch_shader_prog_resource;
        s_ptr<s_gfx_resource> batch_sampler_uniform_resource;
        s_ptr<s_gfx_resource> px_texture;
    };

    s_rendering_basis CreateRenderingBasis(s_mem_arena &mem_arena, s_mem_arena &temp_mem_arena);
    void ReleaseRenderingBasis(s_rendering_basis &basis);

    struct s_frame_state;

    void DrawTriangle(s_frame_state &rs, const s_static_array<s_v2, 3> &pts, const s_static_array<s_color_rgba32f, 3> &pt_colors);

    inline void DrawTriangle(s_frame_state &rs, const s_static_array<s_v2, 3> &pts, const s_color_rgba32f color) {
        DrawTriangle(rs, pts, {color, color, color});
    }

    void DrawRect(s_frame_state &rs, const s_rect_f rect, const s_color_rgba32f color_topleft, const s_color_rgba32f color_topright, const s_color_rgba32f color_bottomright, const s_color_rgba32f color_bottomleft);

    inline void DrawRect(s_frame_state &rs, const s_rect_f rect, const s_color_rgba32f color) {
        DrawRect(rs, rect, color, color, color, color);
    }

    void DrawTexture(s_frame_state &rs, const s_v2 pos, const s_gfx_resource &texture_resource);

    namespace internal {
        s_frame_state &BeginFrame(const s_rendering_basis &basis, s_mem_arena &mem_arena);
        void EndFrame(s_frame_state &rs, s_mem_arena &temp_mem_arena);
    }
#endif
}
