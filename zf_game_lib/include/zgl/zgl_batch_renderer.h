#pragma once

#include <zgl/zgl_renderer.h>

namespace zf {
    struct s_batch_renderer_resources {
        renderer::s_resource_arena arena;

        renderer::s_resource &mesh;
        renderer::s_resource &shader_prog;
        renderer::s_resource &texture_sampler_uniform;
        renderer::s_resource &px_texture;
    };

    struct s_batch_renderer {
    public:
        s_batch_renderer(const s_batch_renderer_resources &resources) : resources(resources) {}

        s_batch_renderer(const s_batch_renderer &) = delete;
        s_batch_renderer &operator=(const s_batch_renderer &) = delete;

        void SubmitTriangle(const s_static_array<s_v2, 3> &pts, const s_static_array<s_color_rgba32f, 3> &pt_colors);

        inline void SubmitTriangle(const s_static_array<s_v2, 3> &pts, const s_color_rgba32f color) {
            SubmitTriangle(pts, {color, color, color});
        }

        void SubmitRect(const s_rect_f rect, const s_color_rgba32f color_topleft, const s_color_rgba32f color_topright, const s_color_rgba32f color_bottomright, const s_color_rgba32f color_bottomleft);

        inline void SubmitRect(const s_rect_f rect, const s_color_rgba32f color) {
            SubmitRect(rect, color, color, color, color);
        }

        void SubmitTexture(const s_v2 pos, const renderer::s_resource &texture);

    private:
        struct s_vert {
            s_v2 pos;
            s_color_rgba32f blend;
            s_v2 uv;

            s_vert() = default;
            s_vert(const s_v2 pos, const s_color_rgba32f blend, const s_v2 uv) : pos(pos), blend(blend), uv(uv) {}
        };

        void Flush();

        const s_batch_renderer_resources &resources;

        s_static_array<s_vert, 8192> verts;
        t_len verts_used_cnt = 0;

        s_ptr<const renderer::s_resource> texture_resource;
    };

    s_batch_renderer_resources CreateBatchRenderer(s_mem_arena &mem_arena, s_mem_arena &temp_mem_arena);
    void DestroyBatchRenderer(s_batch_renderer_resources &resources);
}
