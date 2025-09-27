#pragma once

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <zc.h>

namespace zf {
    struct s_renderable {
        bgfx::VertexBufferHandle vbh = {};
        bgfx::IndexBufferHandle ibh = {};
    };

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
