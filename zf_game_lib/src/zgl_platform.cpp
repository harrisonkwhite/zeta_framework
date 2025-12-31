#include <zgl/zgl_platform.h>

#include <GLFW/glfw3.h>

#if defined(ZF_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include <GLFW/glfw3native.h>
#elif defined(ZF_PLATFORM_MACOS)
    #define GLFW_EXPOSE_NATIVE_COCOA
    #include <GLFW/glfw3native.h>
#elif defined(ZF_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
    #include <GLFW/glfw3native.h>
#endif

#include <zgl/zgl_input.h>

namespace zf::platform {
    struct {
        t_b8 active;

        GLFWwindow *glfw_window;

        s_v2_i framebuffer_size_cache;

        t_b8 fullscreen_active;
        s_v2_i prefullscreen_pos;
        s_v2_i prefullscreen_size;
    } g_state;

    void StartupModule(const s_v2_i init_window_size) {
        ZF_REQUIRE(!g_state.active);
        ZF_REQUIRE(init_window_size.x > 0 && init_window_size.y > 0);

        g_state.active = true;

        if (!glfwInit()) {
            ZF_FATAL();
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_VISIBLE, false);

        g_state.glfw_window = glfwCreateWindow(init_window_size.x, init_window_size.y, "", nullptr, nullptr);

        if (!g_state.glfw_window) {
            ZF_FATAL();
        }

        glfwGetFramebufferSize(g_state.glfw_window, &g_state.framebuffer_size_cache.x, &g_state.framebuffer_size_cache.y);

        {
            const auto fb_size_callback =
                [](GLFWwindow *const window, const t_i32 width, const t_i32 height) {
                    if (width > 0 && height > 0) {
                        g_state.framebuffer_size_cache = {width, height};
                    }
                };

            glfwSetFramebufferSizeCallback(g_state.glfw_window, fb_size_callback);
        }

        {
            const auto scroll_callback =
                [](GLFWwindow *const window, const t_f64 offs_x, const t_f64 offs_y) {
                    const auto input_state = static_cast<s_input_state *>(glfwGetWindowUserPointer(window));
                    input_state->events.scroll += s_v2(static_cast<t_f32>(offs_x), static_cast<t_f32>(offs_y));
                };

            glfwSetScrollCallback(g_state.glfw_window, scroll_callback);
        }
    }

    void ShutdownModule() {
        ZF_REQUIRE(g_state.active);

        glfwDestroyWindow(g_state.glfw_window);
        glfwTerminate();
        g_state = {};
    }

    t_f64 Time() {
        ZF_ASSERT(g_state.active);
        return glfwGetTime();
    }

    static t_i32 ToGLFWKey(const e_key_code key_code) {
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

        case eks_key_code_cnt: break;
        }

        ZF_UNREACHABLE();
    }

    static t_i32 ToGLFWMouseButton(const e_mouse_button_code btn_code) {
        switch (btn_code) {
        case ek_mouse_button_code_left: return GLFW_MOUSE_BUTTON_LEFT;
        case ek_mouse_button_code_right: return GLFW_MOUSE_BUTTON_RIGHT;
        case ek_mouse_button_code_middle: return GLFW_MOUSE_BUTTON_MIDDLE;

        case eks_mouse_button_code_cnt: break;
        }

        ZF_UNREACHABLE();
    };

    void PollOSEvents(s_input_state *const input_state) {
        ZF_ASSERT(g_state.active);

        glfwSetWindowUserPointer(g_state.glfw_window, input_state); // Scroll callback needs access to this input state.

        glfwPollEvents();

        for (t_i32 i = 0; i < eks_key_code_cnt; i++) {
            const t_b8 is_down = glfwGetKey(g_state.glfw_window, ToGLFWKey(static_cast<e_key_code>(i))) == GLFW_PRESS;
            UpdateKeyState(input_state, static_cast<e_key_code>(i), is_down);
        }

        for (t_i32 i = 0; i < eks_mouse_button_code_cnt; i++) {
            const t_b8 is_down = glfwGetMouseButton(g_state.glfw_window, ToGLFWMouseButton(static_cast<e_mouse_button_code>(i))) == GLFW_PRESS;
            UpdateMouseButtonState(input_state, static_cast<e_mouse_button_code>(i), is_down);
        }

        {
            t_f64 cp_x_f64, cp_y_f64;
            glfwGetCursorPos(g_state.glfw_window, &cp_x_f64, &cp_y_f64);
            input_state->cursor_pos = {static_cast<t_f32>(cp_x_f64), static_cast<t_f32>(cp_y_f64)};
        }

        for (t_i32 i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; i++) {
            t_b8 connected = false;
            s_bit_vec_static<eks_gamepad_button_code_cnt> btns_down = {};
            s_static_array<t_f32, eks_gamepad_axis_code_cnt> axes = {};

            GLFWgamepadstate gamepad_state;

            if (glfwJoystickPresent(i) && glfwJoystickIsGamepad(i) && glfwGetGamepadState(i, &gamepad_state)) {
                connected = true;

                for (t_i32 j = 0; j <= GLFW_GAMEPAD_BUTTON_LAST; j++) {
                    if (gamepad_state.buttons[j]) {
                        SetBit(btns_down, j);
                    } else {
                        UnsetBit(btns_down, j);
                    }
                }

                for (t_i32 j = 0; j <= GLFW_GAMEPAD_AXIS_LAST; j++) {
                    axes[j] = gamepad_state.axes[j];
                }
            }

            static_assert(GLFW_JOYSTICK_1 == 0);
            UpdateGamepadState(input_state, i, connected, btns_down, axes);
        }
    }

    void *NativeDisplayHandle() {
        ZF_ASSERT(g_state.active);

#if defined(ZF_PLATFORM_WINDOWS)
        return nullptr;
#elif defined(ZF_PLATFORM_MACOS)
        return nullptr;
#elif defined(ZF_PLATFORM_LINUX)
        return glfwGetX11Display();
#endif
    }

    void *NativeWindowHandle() {
        ZF_ASSERT(g_state.active);

#if defined(ZF_PLATFORM_WINDOWS)
        return reinterpret_cast<void *>(glfwGetWin32Window(g_state.glfw_window));
#elif defined(ZF_PLATFORM_MACOS)
        return glfwGetCocoaWindow(g_state.glfw_window);
#elif defined(ZF_PLATFORM_LINUX)
        return reinterpret_cast<void *>(static_cast<uintptr_t>(glfwGetX11Window(g_state.glfw_window)));
#endif
    }

    void ShowWindow() {
        ZF_ASSERT(g_state.active);
        glfwShowWindow(g_state.glfw_window);
    }

    t_b8 ShouldWindowClose() {
        ZF_ASSERT(g_state.active);
        return glfwWindowShouldClose(g_state.glfw_window);
    }

    void SetWindowTitle(const s_str_rdonly title, s_arena *const temp_arena) {
        ZF_ASSERT(g_state.active);

        const s_str_rdonly title_terminated = AllocStrCloneButAddTerminator(title, temp_arena);
        glfwSetWindowTitle(g_state.glfw_window, title_terminated.AsCstr());
    }

    void SetWindowSize(const s_v2_i size) {
        ZF_ASSERT(g_state.active);
        ZF_ASSERT(size.x > 0 && size.y > 0);

        glfwSetWindowSize(g_state.glfw_window, size.x, size.y);
    }

    void SetWindowSizeLimits(const t_i32 min_width, const t_i32 min_height, const t_i32 max_width, const t_i32 max_height) {
        ZF_ASSERT(g_state.active);
        ZF_ASSERT(min_width >= -1 && min_height >= -1);
        ZF_ASSERT(max_width >= min_width || max_width == -1);
        ZF_ASSERT(max_height >= min_height || max_height == -1);

        static_assert(GLFW_DONT_CARE == -1);
        glfwSetWindowSizeLimits(g_state.glfw_window, min_width, min_height, max_width, max_height);
    }

    void SetWindowResizability(const t_b8 resizable) {
        ZF_ASSERT(g_state.active);
        glfwSetWindowAttrib(g_state.glfw_window, GLFW_RESIZABLE, resizable);
    }

    s_v2_i WindowFramebufferSizeCache() {
        ZF_ASSERT(g_state.active);
        return g_state.framebuffer_size_cache;
    }

    t_b8 IsFullscreen() {
        ZF_ASSERT(g_state.active);
        return g_state.fullscreen_active;
    }

    static GLFWmonitor *FindGLFWMonitorOfWindow(GLFWwindow *const window) {
        s_v2_i window_pos;
        glfwGetWindowPos(window, &window_pos.x, &window_pos.y);

        s_v2_i window_size;
        glfwGetWindowSize(window, &window_size.x, &window_size.y);

        const s_rect_i window_rect = {window_pos, window_size};

        // Get the monitor containing the most amount of the window.
        t_f32 max_occupancy_perc = 0.0f;
        t_i32 max_occupancy_monitor_index = -1;

        t_i32 monitor_cnt;
        const auto monitors = glfwGetMonitors(&monitor_cnt);

        for (t_i32 i = 0; i < monitor_cnt; i++) {
            s_v2_i monitor_pos;
            glfwGetMonitorPos(monitors[i], &monitor_pos.x, &monitor_pos.y);

            s_v2 monitor_scale;
            glfwGetMonitorContentScale(monitors[i], &monitor_scale.x, &monitor_scale.y);

            const GLFWvidmode *const mode = glfwGetVideoMode(monitors[i]);

            const s_rect_i monitor_rect = {
                monitor_pos.x,
                monitor_pos.y,
                static_cast<t_i32>(static_cast<t_f32>(mode->width) / monitor_scale.x),
                static_cast<t_i32>(static_cast<t_f32>(mode->height) / monitor_scale.y),
            };

            const t_f32 occupancy_perc = CalcPercOfOccupance(window_rect, monitor_rect);

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

    void SetFullscreen(const t_b8 active) {
        ZF_ASSERT(g_state.active);

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

    s_v2_i CalcMonitorSizeInPixels() {
        ZF_ASSERT(g_state.active);

        const auto monitor = FindGLFWMonitorOfWindow(g_state.glfw_window);

        if (!monitor) {
            return {};
        }

        const GLFWvidmode *const mode = glfwGetVideoMode(monitor);
        return {mode->width, mode->height};
    }

    s_v2_i CalcMonitorSizeLogical() {
        ZF_ASSERT(g_state.active);

        const auto monitor = FindGLFWMonitorOfWindow(g_state.glfw_window);

        if (!monitor) {
            return {};
        }

        const GLFWvidmode *const mode = glfwGetVideoMode(monitor);

        s_v2 monitor_scale;
        glfwGetMonitorContentScale(monitor, &monitor_scale.x, &monitor_scale.y);

        return {
            static_cast<t_i32>(static_cast<t_f32>(mode->width) / monitor_scale.x),
            static_cast<t_i32>(static_cast<t_f32>(mode->height) / monitor_scale.y),
        };
    }

    void SetCursorVisibility(const t_b8 visible) {
        ZF_ASSERT(g_state.active);
        glfwSetInputMode(g_state.glfw_window, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
    }
}
