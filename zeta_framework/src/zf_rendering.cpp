#include "zf_rendering.h"

#include "zf_window.h"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

namespace zf {
    bool c_renderer::Init() {
        bgfx::Init init;
        init.type = bgfx::RendererType::Count;

        init.resolution.reset = BGFX_RESET_VSYNC;
        const s_v2_s32 fb_size = c_window::GetFramebufferSize();
        init.resolution.width = static_cast<uint32_t>(fb_size.x);
        init.resolution.height = static_cast<uint32_t>(fb_size.y);

        init.platformData.nwh = c_window::GetNativeWindowHandle();
        init.platformData.ndt = c_window::GetNativeDisplayHandle();
        init.platformData.type = bgfx::NativeWindowHandleType::Default;

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

    void c_renderer::Render() {
        const s_v2_s32 window_size = c_window::GetSize();
        bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(window_size.x), static_cast<uint16_t>(window_size.y));

        bgfx::touch(0);

        // @todo: Render operations.

        bgfx::frame();
    }
}
