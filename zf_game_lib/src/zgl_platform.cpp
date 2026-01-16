#include <zgl/zgl_platform.h>

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
    struct t_platform {
        GLFWwindow *glfw_window;

        zcl::t_v2_i framebuffer_size_cache;

        zcl::t_b8 fullscreen_active;
        zcl::t_v2_i prefullscreen_pos;
        zcl::t_v2_i prefullscreen_size;

        t_input_state *input_state;
    };

    static zcl::t_b8 g_module_active;

    t_platform *detail::PlatformStartup(const zcl::t_v2_i init_window_size, t_input_state *const input_state, zcl::t_arena *const arena) {
        ZCL_ASSERT(!g_module_active);
        ZCL_ASSERT(init_window_size.x > 0 && init_window_size.y > 0);

        g_module_active = true;

        const auto platform = zcl::ArenaPushItem<t_platform>(arena);

        if (!glfwInit()) {
            ZCL_FATAL();
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_VISIBLE, false);

        platform->glfw_window = glfwCreateWindow(init_window_size.x, init_window_size.y, "", nullptr, nullptr);

        if (!platform->glfw_window) {
            ZCL_FATAL();
        }

        glfwGetFramebufferSize(platform->glfw_window, &platform->framebuffer_size_cache.x, &platform->framebuffer_size_cache.y);

        platform->input_state = input_state;

        glfwSetWindowUserPointer(platform->glfw_window, platform);

        {
            const auto fb_size_callback =
                [](GLFWwindow *const window, const zcl::t_i32 width, const zcl::t_i32 height) {
                    const auto platform = static_cast<t_platform *>(glfwGetWindowUserPointer(window));

                    if (width > 0 && height > 0) {
                        platform->framebuffer_size_cache = {width, height};
                    }
                };

            glfwSetFramebufferSizeCallback(platform->glfw_window, fb_size_callback);
        }

        {
            const auto scroll_callback =
                [](GLFWwindow *const window, const zcl::t_f64 offs_x, const zcl::t_f64 offs_y) {
                    const auto platform = static_cast<t_platform *>(glfwGetWindowUserPointer(window));
                    detail::ScrollUpdateState(platform->input_state, {static_cast<zcl::t_f32>(offs_x), static_cast<zcl::t_f32>(offs_y)});
                };

            glfwSetScrollCallback(platform->glfw_window, scroll_callback);
        }

        {
            const auto chr_callback =
                [](GLFWwindow *const window, const zcl::t_u32 code_pt) {
                    const auto platform = static_cast<t_platform *>(glfwGetWindowUserPointer(window));

                    if (!detail::TextSubmitCodePoints(platform->input_state, code_pt)) {
                        zcl::LogWarning(ZCL_STR_LITERAL("Tried to submit input text code point, but there is insufficient space!"));
                    }
                };

            glfwSetCharCallback(platform->glfw_window, chr_callback);
        }

        // @todo: Window Focus Callback

        return platform;
    }

    void detail::PlatformShutdown(t_platform *const platform) {
        ZCL_ASSERT(g_module_active);

        glfwDestroyWindow(platform->glfw_window);
        glfwTerminate();

        g_module_active = false;
    }

    zcl::t_f64 GetTime(const t_platform *const platform) {
        ZCL_ASSERT(g_module_active);

        const zcl::t_f64 result = glfwGetTime();
        ZCL_ASSERT(result != 0.0);

        return result;
    }

    static zcl::t_i32 ToGLFWKey(const t_key_code key_code) {
        switch (key_code) {
        case ek_key_code_space: return GLFW_KEY_SPACE;
        case ek_key_code_0: return GLFW_KEY_0;
        case ek_key_code_1: return GLFW_KEY_1;
        case ek_key_code_2: return GLFW_KEY_2;
        case ek_key_code_3: return GLFW_KEY_3;
        case ek_key_code_4: return GLFW_KEY_4;
        case ek_key_code_5: return GLFW_KEY_5;
        case ek_key_code_6: return GLFW_KEY_6;
        case ek_key_code_7: return GLFW_KEY_7;
        case ek_key_code_8: return GLFW_KEY_8;
        case ek_key_code_9: return GLFW_KEY_9;
        case ek_key_code_a: return GLFW_KEY_A;
        case ek_key_code_b: return GLFW_KEY_B;
        case ek_key_code_c: return GLFW_KEY_C;
        case ek_key_code_d: return GLFW_KEY_D;
        case ek_key_code_e: return GLFW_KEY_E;
        case ek_key_code_f: return GLFW_KEY_F;
        case ek_key_code_g: return GLFW_KEY_G;
        case ek_key_code_h: return GLFW_KEY_H;
        case ek_key_code_i: return GLFW_KEY_I;
        case ek_key_code_j: return GLFW_KEY_J;
        case ek_key_code_k: return GLFW_KEY_K;
        case ek_key_code_l: return GLFW_KEY_L;
        case ek_key_code_m: return GLFW_KEY_M;
        case ek_key_code_n: return GLFW_KEY_N;
        case ek_key_code_o: return GLFW_KEY_O;
        case ek_key_code_p: return GLFW_KEY_P;
        case ek_key_code_q: return GLFW_KEY_Q;
        case ek_key_code_r: return GLFW_KEY_R;
        case ek_key_code_s: return GLFW_KEY_S;
        case ek_key_code_t: return GLFW_KEY_T;
        case ek_key_code_u: return GLFW_KEY_U;
        case ek_key_code_v: return GLFW_KEY_V;
        case ek_key_code_w: return GLFW_KEY_W;
        case ek_key_code_x: return GLFW_KEY_X;
        case ek_key_code_y: return GLFW_KEY_Y;
        case ek_key_code_z: return GLFW_KEY_Z;
        case ek_key_code_escape: return GLFW_KEY_ESCAPE;
        case ek_key_code_enter: return GLFW_KEY_ENTER;
        case ek_key_code_backspace: return GLFW_KEY_BACKSPACE;
        case ek_key_code_tab: return GLFW_KEY_TAB;
        case ek_key_code_right: return GLFW_KEY_RIGHT;
        case ek_key_code_left: return GLFW_KEY_LEFT;
        case ek_key_code_down: return GLFW_KEY_DOWN;
        case ek_key_code_up: return GLFW_KEY_UP;
        case ek_key_code_f1: return GLFW_KEY_F1;
        case ek_key_code_f2: return GLFW_KEY_F2;
        case ek_key_code_f3: return GLFW_KEY_F3;
        case ek_key_code_f4: return GLFW_KEY_F4;
        case ek_key_code_f5: return GLFW_KEY_F5;
        case ek_key_code_f6: return GLFW_KEY_F6;
        case ek_key_code_f7: return GLFW_KEY_F7;
        case ek_key_code_f8: return GLFW_KEY_F8;
        case ek_key_code_f9: return GLFW_KEY_F9;
        case ek_key_code_f10: return GLFW_KEY_F10;
        case ek_key_code_f11: return GLFW_KEY_F11;
        case ek_key_code_f12: return GLFW_KEY_F12;
        case ek_key_code_left_shift: return GLFW_KEY_LEFT_SHIFT;
        case ek_key_code_left_control: return GLFW_KEY_LEFT_CONTROL;
        case ek_key_code_left_alt: return GLFW_KEY_LEFT_ALT;
        case ek_key_code_right_shift: return GLFW_KEY_RIGHT_SHIFT;
        case ek_key_code_right_control: return GLFW_KEY_RIGHT_CONTROL;
        case ek_key_code_right_alt: return GLFW_KEY_RIGHT_ALT;
        case ek_key_code_grave: return GLFW_KEY_GRAVE_ACCENT;
        case ek_key_code_minus: return GLFW_KEY_MINUS;
        case ek_key_code_equals: return GLFW_KEY_EQUAL;
        case ek_key_code_left_bracket: return GLFW_KEY_LEFT_BRACKET;
        case ek_key_code_right_bracket: return GLFW_KEY_RIGHT_BRACKET;
        case ek_key_code_backslash: return GLFW_KEY_BACKSLASH;
        case ek_key_code_semicolon: return GLFW_KEY_SEMICOLON;
        case ek_key_code_apostrophe: return GLFW_KEY_APOSTROPHE;
        case ek_key_code_comma: return GLFW_KEY_COMMA;
        case ek_key_code_period: return GLFW_KEY_PERIOD;
        case ek_key_code_slash: return GLFW_KEY_SLASH;
        case ek_key_code_insert: return GLFW_KEY_INSERT;
        case ek_key_code_delete: return GLFW_KEY_DELETE;
        case ek_key_code_home: return GLFW_KEY_HOME;
        case ek_key_code_end: return GLFW_KEY_END;
        case ek_key_code_page_up: return GLFW_KEY_PAGE_UP;
        case ek_key_code_page_down: return GLFW_KEY_PAGE_DOWN;
        case ek_key_code_caps_lock: return GLFW_KEY_CAPS_LOCK;
        case ek_key_code_num_lock: return GLFW_KEY_NUM_LOCK;
        case ek_key_code_scroll_lock: return GLFW_KEY_SCROLL_LOCK;
        case ek_key_code_print_screen: return GLFW_KEY_PRINT_SCREEN;
        case ek_key_code_pause: return GLFW_KEY_PAUSE;
        case ek_key_code_left_super: return GLFW_KEY_LEFT_SUPER;
        case ek_key_code_right_super: return GLFW_KEY_RIGHT_SUPER;
        case ek_key_code_menu: return GLFW_KEY_MENU;
        case ek_key_code_numpad_0: return GLFW_KEY_KP_0;
        case ek_key_code_numpad_1: return GLFW_KEY_KP_1;
        case ek_key_code_numpad_2: return GLFW_KEY_KP_2;
        case ek_key_code_numpad_3: return GLFW_KEY_KP_3;
        case ek_key_code_numpad_4: return GLFW_KEY_KP_4;
        case ek_key_code_numpad_5: return GLFW_KEY_KP_5;
        case ek_key_code_numpad_6: return GLFW_KEY_KP_6;
        case ek_key_code_numpad_7: return GLFW_KEY_KP_7;
        case ek_key_code_numpad_8: return GLFW_KEY_KP_8;
        case ek_key_code_numpad_9: return GLFW_KEY_KP_9;
        case ek_key_code_numpad_decimal: return GLFW_KEY_KP_DECIMAL;
        case ek_key_code_numpad_divide: return GLFW_KEY_KP_DIVIDE;
        case ek_key_code_numpad_multiply: return GLFW_KEY_KP_MULTIPLY;
        case ek_key_code_numpad_subtract: return GLFW_KEY_KP_SUBTRACT;
        case ek_key_code_numpad_add: return GLFW_KEY_KP_ADD;
        case ek_key_code_numpad_enter: return GLFW_KEY_KP_ENTER;
        case ek_key_code_numpad_equals: return GLFW_KEY_KP_EQUAL;

        case ekm_key_code_cnt: break;
        }

        ZCL_UNREACHABLE();
    }


    static zcl::t_i32 ToGLFWMouseButton(const t_mouse_button_code btn_code) {
        switch (btn_code) {
        case ek_mouse_button_code_left: return GLFW_MOUSE_BUTTON_LEFT;
        case ek_mouse_button_code_right: return GLFW_MOUSE_BUTTON_RIGHT;
        case ek_mouse_button_code_middle: return GLFW_MOUSE_BUTTON_MIDDLE;

        case ekm_mouse_button_code_cnt: break;
        }

        ZCL_UNREACHABLE();
    };

    void detail::PollOSEvents(t_platform *const platform) {
        ZCL_ASSERT(g_module_active);

        glfwPollEvents();

        for (zcl::t_i32 i = 0; i < ekm_key_code_cnt; i++) {
            const zcl::t_b8 is_down = glfwGetKey(platform->glfw_window, ToGLFWKey(static_cast<t_key_code>(i))) == GLFW_PRESS;
            detail::KeyUpdateState(platform->input_state, static_cast<t_key_code>(i), is_down);
        }

        for (zcl::t_i32 i = 0; i < ekm_mouse_button_code_cnt; i++) {
            const zcl::t_b8 is_down = glfwGetMouseButton(platform->glfw_window, ToGLFWMouseButton(static_cast<t_mouse_button_code>(i))) == GLFW_PRESS;
            detail::MouseButtonUpdateState(platform->input_state, static_cast<t_mouse_button_code>(i), is_down);
        }

        {
            zcl::t_f64 cp_x_f64, cp_y_f64;
            glfwGetCursorPos(platform->glfw_window, &cp_x_f64, &cp_y_f64);
            detail::CursorUpdateState(platform->input_state, {static_cast<zcl::t_f32>(cp_x_f64), static_cast<zcl::t_f32>(cp_y_f64)});
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
            detail::GamepadUpdateState(platform->input_state, i, connected, btns_down, axes);
        }
    }

    void *detail::DisplayGetNativeHandle(const t_platform *const platform) {
        ZCL_ASSERT(g_module_active);

#if defined(ZCL_PLATFORM_WINDOWS)
        return nullptr;
#elif defined(ZCL_PLATFORM_MACOS)
        return nullptr;
#elif defined(ZCL_PLATFORM_LINUX)
        return glfwGetX11Display();
#endif
    }

    void *detail::WindowGetNativeHandle(const t_platform *const platform) {
        ZCL_ASSERT(g_module_active);

#if defined(ZCL_PLATFORM_WINDOWS)
        return reinterpret_cast<void *>(glfwGetWin32Window(platform->glfw_window));
#elif defined(ZCL_PLATFORM_MACOS)
        return glfwGetCocoaWindow(g_state.glfw_window);
#elif defined(ZCL_PLATFORM_LINUX)
        return reinterpret_cast<void *>(static_cast<uintptr_t>(glfwGetX11Window(g_state.glfw_window)));
#endif
    }

    void detail::WindowShow(t_platform *const platform) {
        ZCL_ASSERT(g_module_active);
        glfwShowWindow(platform->glfw_window);
    }

    void WindowRequestClose(t_platform *const platform) {
        ZCL_ASSERT(g_module_active);
        zcl::Log(ZCL_STR_LITERAL("Window close explicitly requested..."));
        return glfwSetWindowShouldClose(platform->glfw_window, true);
    }

    zcl::t_b8 WindowCheckCloseRequested(const t_platform *const platform) {
        ZCL_ASSERT(g_module_active);
        return glfwWindowShouldClose(platform->glfw_window);
    }

    zcl::t_b8 WindowCheckFocused(const t_platform *const platform) {
        ZCL_ASSERT(g_module_active);
        return glfwGetWindowAttrib(platform->glfw_window, GLFW_FOCUSED);
    }

    void WindowSetTitle(t_platform *const platform, const zcl::t_str_rdonly title, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(g_module_active);
        const zcl::t_str_rdonly title_terminated = zcl::StrCloneButAddTerminator(title, temp_arena);
        glfwSetWindowTitle(platform->glfw_window, zcl::StrToCStr(title_terminated));
    }

    void WindowSetSize(t_platform *const platform, const zcl::t_v2_i size) {
        ZCL_ASSERT(g_module_active);
        ZCL_ASSERT(size.x > 0 && size.y > 0);

        glfwSetWindowSize(platform->glfw_window, size.x, size.y);
    }

    void WindowSetSizeLimits(t_platform *const platform, const zcl::t_i32 min_width, const zcl::t_i32 min_height, const zcl::t_i32 max_width, const zcl::t_i32 max_height) {
        ZCL_ASSERT(g_module_active);
        ZCL_ASSERT(min_width >= -1 && min_height >= -1);
        ZCL_ASSERT(max_width >= min_width || max_width == -1);
        ZCL_ASSERT(max_height >= min_height || max_height == -1);

        static_assert(GLFW_DONT_CARE == -1);
        glfwSetWindowSizeLimits(platform->glfw_window, min_width, min_height, max_width, max_height);
    }

    void WindowSetResizable(t_platform *const platform, const zcl::t_b8 resizable) {
        ZCL_ASSERT(g_module_active);
        glfwSetWindowAttrib(platform->glfw_window, GLFW_RESIZABLE, resizable);
    }

    zcl::t_v2_i WindowGetFramebufferSizeCache(const t_platform *const platform) {
        ZCL_ASSERT(g_module_active);
        return platform->framebuffer_size_cache;
    }

    zcl::t_b8 WindowCheckFullscreen(const t_platform *const platform) {
        ZCL_ASSERT(g_module_active);
        return platform->fullscreen_active;
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
        ZCL_ASSERT(g_module_active);

        if (active == platform->fullscreen_active) {
            return;
        }

        if (active) {
            glfwGetWindowPos(platform->glfw_window, &platform->prefullscreen_pos.x, &platform->prefullscreen_pos.y);
            glfwGetWindowSize(platform->glfw_window, &platform->prefullscreen_size.x, &platform->prefullscreen_size.y);

            const auto monitor = FindGLFWMonitorOfWindow(platform->glfw_window);

            if (!monitor) {
                return;
            }

            const GLFWvidmode *const mode = glfwGetVideoMode(monitor);
            glfwSetWindowMonitor(platform->glfw_window, monitor, 0, 0, mode->width, mode->height, 0);
        } else {
            glfwSetWindowMonitor(platform->glfw_window, nullptr, platform->prefullscreen_pos.x, platform->prefullscreen_pos.y, platform->prefullscreen_size.x, platform->prefullscreen_size.y, 0);
        }

        platform->fullscreen_active = active;
    }

    zcl::t_v2_i MonitorCalcSizePixels(const t_platform *const platform) {
        ZCL_ASSERT(g_module_active);

        const auto monitor = FindGLFWMonitorOfWindow(platform->glfw_window);

        if (!monitor) {
            return {};
        }

        const GLFWvidmode *const mode = glfwGetVideoMode(monitor);
        return {mode->width, mode->height};
    }

    zcl::t_v2_i MonitorCalcSizeLogical(const t_platform *const platform) {
        ZCL_ASSERT(g_module_active);

        const auto monitor = FindGLFWMonitorOfWindow(platform->glfw_window);

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
        ZCL_ASSERT(g_module_active);
        glfwSetInputMode(platform->glfw_window, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
    }
}
