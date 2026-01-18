#include <zgl/zgl_platform_private.h>

#include <GLFW/glfw3.h>

#if defined(ZCL_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include <GLFW/glfw3native.h>
#elif defined(ZCL_PLATFORM_MACOS)
    #define GLFW_EXPOSE_NATIVE_COCOA
    #include <GLFW/glfw3native.h>
#elif defined(ZCL_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
    #include <GLFW/glfw3native.h>
#endif

#include <zgl/zgl_input.h>

namespace zgl {
#ifdef ZCL_DEBUG
    static zcl::t_u8 g_platform_identity; // Memory address of this is used as the key. The actual value of this variable is redundant.
#endif

    static struct {
        zcl::t_b8 active;

        GLFWwindow *glfw_window;

        zcl::t_v2_i framebuffer_size_cache;

        zcl::t_b8 fullscreen_active;
        zcl::t_v2_i prefullscreen_pos;
        zcl::t_v2_i prefullscreen_size;
    } g_state;

    static zcl::t_b8 KeyCheckValid(const t_platform *const key) {
        return key == reinterpret_cast<t_platform *>(&g_platform_identity);
    }

    t_platform *internal::PlatformStartup(const zcl::t_v2_i init_window_size, t_input_state *const input_state, zcl::t_arena *const arena) {
        ZCL_ASSERT(!g_state.active);
        ZCL_ASSERT(init_window_size.x > 0 && init_window_size.y > 0);

        g_state.active = true;

        if (!glfwInit()) {
            ZCL_FATAL();
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_VISIBLE, false);

        g_state.glfw_window = glfwCreateWindow(init_window_size.x, init_window_size.y, "", nullptr, nullptr);

        if (!g_state.glfw_window) {
            ZCL_FATAL();
        }

        glfwGetFramebufferSize(g_state.glfw_window, &g_state.framebuffer_size_cache.x, &g_state.framebuffer_size_cache.y);

        {
            const auto fb_size_callback =
                [](GLFWwindow *const window, const zcl::t_i32 width, const zcl::t_i32 height) {
                    if (width > 0 && height > 0) {
                        g_state.framebuffer_size_cache = {width, height};
                    }
                };

            glfwSetFramebufferSizeCallback(g_state.glfw_window, fb_size_callback);
        }

        {
            const auto scroll_callback =
                [](GLFWwindow *const window, const zcl::t_f64 offs_x, const zcl::t_f64 offs_y) {
                    const auto input_state = static_cast<t_input_state *>(glfwGetWindowUserPointer(window));
                    internal::ScrollUpdateState(input_state, {static_cast<zcl::t_f32>(offs_x), static_cast<zcl::t_f32>(offs_y)});
                };

            glfwSetScrollCallback(g_state.glfw_window, scroll_callback);
        }

        {
            const auto chr_callback =
                [](GLFWwindow *const window, const zcl::t_u32 code_pt) {
                    const auto input_state = static_cast<t_input_state *>(glfwGetWindowUserPointer(window));

                    if (!internal::TextSubmitCodePoints(input_state, code_pt)) {
                        zcl::LogWarning(ZCL_STR_LITERAL("Tried to submit input text code point, but there is insufficient space!"));
                    }
                };

            glfwSetCharCallback(g_state.glfw_window, chr_callback);
        }

        return reinterpret_cast<t_platform *>(&g_platform_identity);
    }

    void internal::PlatformShutdown(t_platform *const platform) {
        ZCL_ASSERT(g_state.active);
        ZCL_ASSERT(KeyCheckValid(platform));

        glfwDestroyWindow(g_state.glfw_window);
        glfwTerminate();

        g_state.active = false;
    }

    zcl::t_f64 GetTime(const t_platform *const platform) {
        ZCL_ASSERT(g_state.active);
        ZCL_ASSERT(KeyCheckValid(platform));

        const zcl::t_f64 result = glfwGetTime();
        ZCL_ASSERT(result != 0.0);

        return result;
    }

    void internal::PollOSEvents(t_platform *const platform, t_input_state *const input_state) {
        ZCL_ASSERT(g_state.active);
        ZCL_ASSERT(KeyCheckValid(platform));

        glfwSetWindowUserPointer(g_state.glfw_window, input_state);

        glfwPollEvents();

        for (zcl::t_i32 i = 0; i < ekm_key_code_cnt; i++) {
            const zcl::t_b8 is_down = glfwGetKey(g_state.glfw_window, ToGLFWKey(static_cast<t_key_code>(i))) == GLFW_PRESS;
            internal::KeyUpdateState(input_state, static_cast<t_key_code>(i), is_down);
        }

        for (zcl::t_i32 i = 0; i < ekm_mouse_button_code_cnt; i++) {
            const zcl::t_b8 is_down = glfwGetMouseButton(g_state.glfw_window, ToGLFWMouseButton(static_cast<t_mouse_button_code>(i))) == GLFW_PRESS;
            internal::MouseButtonUpdateState(input_state, static_cast<t_mouse_button_code>(i), is_down);
        }

        {
            zcl::t_f64 cp_x_f64, cp_y_f64;
            glfwGetCursorPos(g_state.glfw_window, &cp_x_f64, &cp_y_f64);
            internal::CursorUpdateState(input_state, {static_cast<zcl::t_f32>(cp_x_f64), static_cast<zcl::t_f32>(cp_y_f64)});
        }

        for (zcl::t_i32 i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; i++) {
            zcl::t_b8 connected = false;
            zcl::t_static_bitset<ekm_gamepad_button_code_cnt> btns_down = {};
            zcl::t_static_array<zcl::t_f32, ekm_gamepad_axis_code_cnt> axes = {};

            GLFWgamepadstate gamepad_state;

            if (glfwJoystickPresent(i) && glfwJoystickIsGamepad(i) && glfwGetGamepadState(i, &gamepad_state)) {
                connected = true;

                for (zcl::t_i32 j = 0; j <= GLFW_GAMEPAD_BUTTON_LAST; j++) {
                    if (gamepad_state.buttons[j]) {
                        zcl::BitsetSet(btns_down, j);
                    } else {
                        zcl::BitsetUnset(btns_down, j);
                    }
                }

                for (zcl::t_i32 j = 0; j <= GLFW_GAMEPAD_AXIS_LAST; j++) {
                    axes[j] = gamepad_state.axes[j];
                }
            }

            static_assert(GLFW_JOYSTICK_1 == 0);
            internal::GamepadUpdateState(input_state, i, connected, btns_down, axes);
        }
    }

    void *internal::DisplayGetNativeHandle(const t_platform *const platform) {
        ZCL_ASSERT(g_state.active);
        ZCL_ASSERT(KeyCheckValid(platform));

#if defined(ZCL_PLATFORM_WINDOWS)
        return nullptr;
#elif defined(ZCL_PLATFORM_MACOS)
        return nullptr;
#elif defined(ZCL_PLATFORM_LINUX)
        return glfwGetX11Display();
#endif
    }

    void *internal::WindowGetNativeHandle(const t_platform *const platform) {
        ZCL_ASSERT(g_state.active);
        ZCL_ASSERT(KeyCheckValid(platform));

#if defined(ZCL_PLATFORM_WINDOWS)
        return reinterpret_cast<void *>(glfwGetWin32Window(g_state.glfw_window));
#elif defined(ZCL_PLATFORM_MACOS)
        return glfwGetCocoaWindow(g_state.glfw_window);
#elif defined(ZCL_PLATFORM_LINUX)
        return reinterpret_cast<void *>(static_cast<uintptr_t>(glfwGetX11Window(g_state.glfw_window)));
#endif
    }

    void internal::WindowShow(t_platform *const platform) {
        ZCL_ASSERT(g_state.active);
        ZCL_ASSERT(KeyCheckValid(platform));

        glfwShowWindow(g_state.glfw_window);
    }

    void WindowRequestClose(t_platform *const platform) {
        ZCL_ASSERT(g_state.active);
        ZCL_ASSERT(KeyCheckValid(platform));

        zcl::Log(ZCL_STR_LITERAL("Window close explicitly requested..."));
        return glfwSetWindowShouldClose(g_state.glfw_window, true);
    }

    zcl::t_b8 WindowCheckCloseRequested(const t_platform *const platform) {
        ZCL_ASSERT(g_state.active);
        ZCL_ASSERT(KeyCheckValid(platform));

        return glfwWindowShouldClose(g_state.glfw_window);
    }

    zcl::t_b8 WindowCheckFocused(const t_platform *const platform) {
        ZCL_ASSERT(g_state.active);
        ZCL_ASSERT(KeyCheckValid(platform));

        return glfwGetWindowAttrib(g_state.glfw_window, GLFW_FOCUSED);
    }

    void WindowSetTitle(t_platform *const platform, const zcl::t_str_rdonly title, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(g_state.active);
        ZCL_ASSERT(KeyCheckValid(platform));

        const zcl::t_str_rdonly title_terminated = zcl::StrCloneButAddTerminator(title, temp_arena);
        glfwSetWindowTitle(g_state.glfw_window, zcl::StrToCStr(title_terminated));
    }

    void WindowSetSize(t_platform *const platform, const zcl::t_v2_i size) {
        ZCL_ASSERT(g_state.active);
        ZCL_ASSERT(KeyCheckValid(platform));
        ZCL_ASSERT(size.x > 0 && size.y > 0);

        glfwSetWindowSize(g_state.glfw_window, size.x, size.y);
    }

    void WindowSetSizeLimits(t_platform *const platform, const zcl::t_i32 min_width, const zcl::t_i32 min_height, const zcl::t_i32 max_width, const zcl::t_i32 max_height) {
        ZCL_ASSERT(g_state.active);
        ZCL_ASSERT(KeyCheckValid(platform));
        ZCL_ASSERT(min_width >= -1 && min_height >= -1);
        ZCL_ASSERT(max_width >= min_width || max_width == -1);
        ZCL_ASSERT(max_height >= min_height || max_height == -1);

        static_assert(GLFW_DONT_CARE == -1);
        glfwSetWindowSizeLimits(g_state.glfw_window, min_width, min_height, max_width, max_height);
    }

    void WindowSetResizable(t_platform *const platform, const zcl::t_b8 resizable) {
        ZCL_ASSERT(g_state.active);
        ZCL_ASSERT(KeyCheckValid(platform));

        glfwSetWindowAttrib(g_state.glfw_window, GLFW_RESIZABLE, resizable);
    }

    zcl::t_v2_i WindowGetFramebufferSizeCache(const t_platform *const platform) {
        ZCL_ASSERT(g_state.active);
        ZCL_ASSERT(KeyCheckValid(platform));

        return g_state.framebuffer_size_cache;
    }

    zcl::t_b8 WindowCheckFullscreen(const t_platform *const platform) {
        ZCL_ASSERT(g_state.active);
        ZCL_ASSERT(KeyCheckValid(platform));

        return g_state.fullscreen_active;
    }

    static GLFWmonitor *FindGLFWMonitorOfWindow(GLFWwindow *const window) {
        zcl::t_v2_i window_pos;
        glfwGetWindowPos(window, &window_pos.x, &window_pos.y);

        zcl::t_v2_i window_size;
        glfwGetWindowSize(window, &window_size.x, &window_size.y);

        const auto window_rect = zcl::RectCreateI(window_pos, window_size);

        // Get the monitor containing the most amount of the window.
        zcl::t_f32 max_occupancy_perc = 0.0f;
        zcl::t_i32 max_occupancy_monitor_index = -1;

        zcl::t_i32 monitor_cnt;
        const auto monitors = glfwGetMonitors(&monitor_cnt);

        for (zcl::t_i32 i = 0; i < monitor_cnt; i++) {
            zcl::t_v2_i monitor_pos;
            glfwGetMonitorPos(monitors[i], &monitor_pos.x, &monitor_pos.y);

            zcl::t_v2 monitor_scale;
            glfwGetMonitorContentScale(monitors[i], &monitor_scale.x, &monitor_scale.y);

            const GLFWvidmode *const mode = glfwGetVideoMode(monitors[i]);

            const zcl::t_rect_i monitor_rect = {
                monitor_pos.x,
                monitor_pos.y,
                static_cast<zcl::t_i32>(static_cast<zcl::t_f32>(mode->width) / monitor_scale.x),
                static_cast<zcl::t_i32>(static_cast<zcl::t_f32>(mode->height) / monitor_scale.y),
            };

            const zcl::t_f32 occupancy_perc = zcl::CalcPercOfOccupance(window_rect, monitor_rect);

            if (occupancy_perc > max_occupancy_perc) {
                max_occupancy_perc = occupancy_perc;
                max_occupancy_monitor_index = i;
            }
        }

        if (max_occupancy_monitor_index == -1) {
            return glfwGetPrimaryMonitor();
        }

        return monitors[max_occupancy_monitor_index];
    }

    void WindowSetFullscreen(t_platform *const platform, const zcl::t_b8 active) {
        ZCL_ASSERT(g_state.active);
        ZCL_ASSERT(KeyCheckValid(platform));

        if (active == g_state.fullscreen_active) {
            return;
        }

        if (active) {
            glfwGetWindowPos(g_state.glfw_window, &g_state.prefullscreen_pos.x, &g_state.prefullscreen_pos.y);
            glfwGetWindowSize(g_state.glfw_window, &g_state.prefullscreen_size.x, &g_state.prefullscreen_size.y);

            const auto monitor = FindGLFWMonitorOfWindow(g_state.glfw_window);

            if (!monitor) {
                return;
            }

            const GLFWvidmode *const mode = glfwGetVideoMode(monitor);
            glfwSetWindowMonitor(g_state.glfw_window, monitor, 0, 0, mode->width, mode->height, 0);
        } else {
            glfwSetWindowMonitor(g_state.glfw_window, nullptr, g_state.prefullscreen_pos.x, g_state.prefullscreen_pos.y, g_state.prefullscreen_size.x, g_state.prefullscreen_size.y, 0);
        }

        g_state.fullscreen_active = active;
    }

    zcl::t_v2_i MonitorCalcSizePixels(const t_platform *const platform) {
        ZCL_ASSERT(g_state.active);
        ZCL_ASSERT(KeyCheckValid(platform));

        const auto monitor = FindGLFWMonitorOfWindow(g_state.glfw_window);

        if (!monitor) {
            return {};
        }

        const GLFWvidmode *const mode = glfwGetVideoMode(monitor);
        return {mode->width, mode->height};
    }

    zcl::t_v2_i MonitorCalcSizeLogical(const t_platform *const platform) {
        ZCL_ASSERT(g_state.active);
        ZCL_ASSERT(KeyCheckValid(platform));

        const auto monitor = FindGLFWMonitorOfWindow(g_state.glfw_window);

        if (!monitor) {
            return {};
        }

        const GLFWvidmode *const mode = glfwGetVideoMode(monitor);

        zcl::t_v2 monitor_scale;
        glfwGetMonitorContentScale(monitor, &monitor_scale.x, &monitor_scale.y);

        return {
            static_cast<zcl::t_i32>(static_cast<zcl::t_f32>(mode->width) / monitor_scale.x),
            static_cast<zcl::t_i32>(static_cast<zcl::t_f32>(mode->height) / monitor_scale.y),
        };
    }

    void CursorSetVisible(t_platform *const platform, const zcl::t_b8 visible) {
        ZCL_ASSERT(g_state.active);
        ZCL_ASSERT(KeyCheckValid(platform));

        glfwSetInputMode(g_state.glfw_window, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
    }
}
