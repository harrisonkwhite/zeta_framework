#include "zf_rendering.h"

#if defined(_WIN32)
    #define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__linux__)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(__APPLE__)
    #define GLFW_EXPOSE_NATIVE_COCOA
#endif

#include <GLFW/glfw3native.h>

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

namespace zf {
    bool c_renderer::Init(GLFWwindow* glfw_window) {
        bgfx::PlatformData pd = {};

#if defined(_WIN32)
        pd.nwh = glfwGetWin32Window(glfw_window);
#elif defined(__linux__)
        pd.nwh = (void*)(uintptr_t)glfwGetX11Window(glfw_window);
        pd.ndt = glfwGetX11Display();
#elif defined(__APPLE__)
        pd.nwh = glfwGetCocoaWindow(glfw_window);
#endif

        bgfx::Init init;
        init.type = bgfx::RendererType::Count;
        init.resolution.reset = BGFX_RESET_VSYNC;
        init.platformData = pd;

        {
            int fbWidth = 0, fbHeight = 0;

            glfwGetFramebufferSize(glfw_window, &fbWidth, &fbHeight);

            if (fbWidth == 0 || fbHeight == 0) {
                fbWidth = 1;
                fbHeight = 1;
            }

            init.resolution.width = static_cast<uint32_t>(fbWidth);
            init.resolution.height = static_cast<uint32_t>(fbHeight);
        }

        if (!bgfx::init(init)) {
            ZF_LOG_ERROR("Failed to initialise bgfx!");
            return false;
        }

        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);

        return true;
    }

    void c_renderer::Clean() {
        bgfx::shutdown();
    }

    void c_renderer::Render(const s_v2_s32 window_size) {
        bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(window_size.x), static_cast<uint16_t>(window_size.y));

        bgfx::touch(0);

        // @todo: Render operations.

        bgfx::frame();
    }
}
