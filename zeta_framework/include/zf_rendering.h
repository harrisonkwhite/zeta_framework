#pragma once

#include <zc.h>

#include <GLFW/glfw3.h>

namespace zf {
    class c_renderer {
    public:
        static bool Init(GLFWwindow* glfw_window);
        static void Clean();

        static void Render(const s_v2_s32 window_size);

    private:
    };
}
