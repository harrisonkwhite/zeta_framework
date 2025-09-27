#pragma once

#include <zc.h>

namespace zf {
    class c_renderer {
    public:
        c_renderer() = delete;
        c_renderer(const c_renderer&) = delete;
        c_renderer& operator=(const c_renderer&) = delete;

        static bool Init();
        static void Clean();

        static void Render();

    private:
    };
}
