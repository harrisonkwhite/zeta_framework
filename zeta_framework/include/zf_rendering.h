#pragma once

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <zc.h>

namespace zf {
    // @todo: We need a module for the generic graphics abstraction, another for the renderer.

    struct s_renderable {
        bgfx::VertexBufferHandle vbh;
        bgfx::IndexBufferHandle ibh;
    };

#if 0
    enum e_renderable {
        ek_renderable_batch,
        eks_renderable_cnt
    };

    // PROPER IMPLEMENTATION STUFF, GET THE RECTANGLE DONE FIRST!
    struct s_batch_vert {
        s_v2 vert_coord;
        s_v2 pos;
        s_v2 size;
        float rot;
        s_v2 tex_coord;
        s_v4 blend;
    }; // @todo: Figure out how to map attributes using the BGFX enums!

    static void BuildBatchVertexLayout(bgfx::VertexLayout& layout) {
        layout.begin()
            .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
            .end();
    };

#endif

    class c_renderer {
    public:
        c_renderer() = delete;
        c_renderer(const c_renderer&) = delete;
        c_renderer& operator=(const c_renderer&) = delete;

        static bool Init(c_mem_arena& temp_mem_arena);
        static void Clean();

        static void Render();

    private:
        static inline s_v2 sm_size;
        static inline bgfx::ProgramHandle sm_prog = {};
        static inline s_renderable sm_quad_renderable;
    };
}
