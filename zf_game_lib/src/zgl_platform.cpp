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

        t_v2_i framebuffer_size_cache;

        t_b8 fullscreen_active;
        t_v2_i prefullscreen_pos;
        t_v2_i prefullscreen_size;
    } g_state;

    void f_startup(const t_v2_i init_window_size) {
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
                    const auto input_state = static_cast<input::t_state *>(glfwGetWindowUserPointer(window));
                    input::f_update_scroll_offs(input_state, {static_cast<t_f32>(offs_x), static_cast<t_f32>(offs_y)});
                };

            glfwSetScrollCallback(g_state.glfw_window, scroll_callback);
        }
    }

    void f_shutdown() {
        ZF_REQUIRE(g_state.active);

        glfwDestroyWindow(g_state.glfw_window);
        glfwTerminate();
        g_state = {};
    }

    t_f64 f_get_time() {
        ZF_ASSERT(g_state.active);
        return glfwGetTime();
    }

    static t_i32 f_to_glfw_key(const input::t_key_code key_code) {
        switch (key_code) {
        case input::ec_key_code_space: return GLFW_KEY_SPACE;

        case input::ec_key_code_0: return GLFW_KEY_0;
        case input::ec_key_code_1: return GLFW_KEY_1;
        case input::ec_key_code_2: return GLFW_KEY_2;
        case input::ec_key_code_3: return GLFW_KEY_3;
        case input::ec_key_code_4: return GLFW_KEY_4;
        case input::ec_key_code_5: return GLFW_KEY_5;
        case input::ec_key_code_6: return GLFW_KEY_6;
        case input::ec_key_code_7: return GLFW_KEY_7;
        case input::ec_key_code_8: return GLFW_KEY_8;
        case input::ec_key_code_9: return GLFW_KEY_9;

        case input::ec_key_code_a: return GLFW_KEY_A;
        case input::ec_key_code_b: return GLFW_KEY_B;
        case input::ec_key_code_c: return GLFW_KEY_C;
        case input::ec_key_code_d: return GLFW_KEY_D;
        case input::ec_key_code_e: return GLFW_KEY_E;
        case input::ec_key_code_f: return GLFW_KEY_F;
        case input::ec_key_code_g: return GLFW_KEY_G;
        case input::ec_key_code_h: return GLFW_KEY_H;
        case input::ec_key_code_i: return GLFW_KEY_I;
        case input::ec_key_code_j: return GLFW_KEY_J;
        case input::ec_key_code_k: return GLFW_KEY_K;
        case input::ec_key_code_l: return GLFW_KEY_L;
        case input::ec_key_code_m: return GLFW_KEY_M;
        case input::ec_key_code_n: return GLFW_KEY_N;
        case input::ec_key_code_o: return GLFW_KEY_O;
        case input::ec_key_code_p: return GLFW_KEY_P;
        case input::ec_key_code_q: return GLFW_KEY_Q;
        case input::ec_key_code_r: return GLFW_KEY_R;
        case input::ec_key_code_s: return GLFW_KEY_S;
        case input::ec_key_code_t: return GLFW_KEY_T;
        case input::ec_key_code_u: return GLFW_KEY_U;
        case input::ec_key_code_v: return GLFW_KEY_V;
        case input::ec_key_code_w: return GLFW_KEY_W;
        case input::ec_key_code_x: return GLFW_KEY_X;
        case input::ec_key_code_y: return GLFW_KEY_Y;
        case input::ec_key_code_z: return GLFW_KEY_Z;

        case input::ec_key_code_escape: return GLFW_KEY_ESCAPE;
        case input::ec_key_code_enter: return GLFW_KEY_ENTER;
        case input::ec_key_code_backspace: return GLFW_KEY_BACKSPACE;
        case input::ec_key_code_tab: return GLFW_KEY_TAB;

        case input::ec_key_code_right: return GLFW_KEY_RIGHT;
        case input::ec_key_code_left: return GLFW_KEY_LEFT;
        case input::ec_key_code_down: return GLFW_KEY_DOWN;
        case input::ec_key_code_up: return GLFW_KEY_UP;

        case input::ec_key_code_f1: return GLFW_KEY_F1;
        case input::ec_key_code_f2: return GLFW_KEY_F2;
        case input::ec_key_code_f3: return GLFW_KEY_F3;
        case input::ec_key_code_f4: return GLFW_KEY_F4;
        case input::ec_key_code_f5: return GLFW_KEY_F5;
        case input::ec_key_code_f6: return GLFW_KEY_F6;
        case input::ec_key_code_f7: return GLFW_KEY_F7;
        case input::ec_key_code_f8: return GLFW_KEY_F8;
        case input::ec_key_code_f9: return GLFW_KEY_F9;
        case input::ec_key_code_f10: return GLFW_KEY_F10;
        case input::ec_key_code_f11: return GLFW_KEY_F11;
        case input::ec_key_code_f12: return GLFW_KEY_F12;

        case input::ec_key_code_left_shift: return GLFW_KEY_LEFT_SHIFT;
        case input::ec_key_code_left_control: return GLFW_KEY_LEFT_CONTROL;
        case input::ec_key_code_left_alt: return GLFW_KEY_LEFT_ALT;

        case input::ec_key_code_right_shift: return GLFW_KEY_RIGHT_SHIFT;
        case input::ec_key_code_right_control: return GLFW_KEY_RIGHT_CONTROL;
        case input::ec_key_code_right_alt: return GLFW_KEY_RIGHT_ALT;

        case input::ecm_key_code_cnt: break;
        }

        ZF_UNREACHABLE();
    }

    static t_i32 f_to_glfw_mouse_button(const input::t_mouse_button_code btn_code) {
        switch (btn_code) {
        case input::ec_mouse_button_code_left: return GLFW_MOUSE_BUTTON_LEFT;
        case input::ec_mouse_button_code_right: return GLFW_MOUSE_BUTTON_RIGHT;
        case input::ec_mouse_button_code_middle: return GLFW_MOUSE_BUTTON_MIDDLE;

        case input::ecm_mouse_button_code_cnt: break;
        }

        ZF_UNREACHABLE();
    };

    void f_poll_os_events(input::t_state *const input_state) {
        ZF_ASSERT(g_state.active);

        glfwSetWindowUserPointer(g_state.glfw_window, input_state); // Scroll callback needs access to this input state.

        glfwPollEvents();

        for (t_i32 i = 0; i < input::ecm_key_code_cnt; i++) {
            const t_b8 is_down = glfwGetKey(g_state.glfw_window, f_to_glfw_key(static_cast<input::t_key_code>(i))) == GLFW_PRESS;
            input::f_update_key_state(input_state, static_cast<input::t_key_code>(i), is_down);
        }

        for (t_i32 i = 0; i < input::ecm_mouse_button_code_cnt; i++) {
            const t_b8 is_down = glfwGetMouseButton(g_state.glfw_window, f_to_glfw_mouse_button(static_cast<input::t_mouse_button_code>(i))) == GLFW_PRESS;
            input::f_update_mouse_button_state(input_state, static_cast<input::t_mouse_button_code>(i), is_down);
        }

        {
            t_f64 cp_x_f64, cp_y_f64;
            glfwGetCursorPos(g_state.glfw_window, &cp_x_f64, &cp_y_f64);
            input::f_update_cursor_pos(input_state, {static_cast<t_f32>(cp_x_f64), static_cast<t_f32>(cp_y_f64)});
        }

        for (t_i32 i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; i++) {
            t_b8 connected = false;
            t_static_bitset<input::ecm_gamepad_button_code_cnt> btns_down = {};
            t_static_array<t_f32, input::ecm_gamepad_axis_code_cnt> axes = {};

            GLFWgamepadstate gamepad_state;

            if (glfwJoystickPresent(i) && glfwJoystickIsGamepad(i) && glfwGetGamepadState(i, &gamepad_state)) {
                connected = true;

                for (t_i32 j = 0; j <= GLFW_GAMEPAD_BUTTON_LAST; j++) {
                    if (gamepad_state.buttons[j]) {
                        f_mem_set_bit(btns_down, j);
                    } else {
                        f_mem_unset_bit(btns_down, j);
                    }
                }

                for (t_i32 j = 0; j <= GLFW_GAMEPAD_AXIS_LAST; j++) {
                    axes[j] = gamepad_state.axes[j];
                }
            }

            static_assert(GLFW_JOYSTICK_1 == 0);
            input::f_update_gamepad_state(input_state, i, connected, btns_down, axes);
        }
    }

    void *f_get_native_display_handle() {
        ZF_ASSERT(g_state.active);

#if defined(ZF_PLATFORM_WINDOWS)
        return nullptr;
#elif defined(ZF_PLATFORM_MACOS)
        return nullptr;
#elif defined(ZF_PLATFORM_LINUX)
        return glfwGetX11Display();
#endif
    }

    void *f_get_native_window_handle() {
        ZF_ASSERT(g_state.active);

#if defined(ZF_PLATFORM_WINDOWS)
        return reinterpret_cast<void *>(glfwGetWin32Window(g_state.glfw_window));
#elif defined(ZF_PLATFORM_MACOS)
        return glfwGetCocoaWindow(g_state.glfw_window);
#elif defined(ZF_PLATFORM_LINUX)
        return reinterpret_cast<void *>(static_cast<uintptr_t>(glfwGetX11Window(g_state.glfw_window)));
#endif
    }

    void f_show_window() {
        ZF_ASSERT(g_state.active);
        glfwShowWindow(g_state.glfw_window);
    }

    t_b8 f_should_window_close() {
        ZF_ASSERT(g_state.active);
        return glfwWindowShouldClose(g_state.glfw_window);
    }

    void f_set_window_title(const t_str_rdonly title, t_arena *const temp_arena) {
        ZF_ASSERT(g_state.active);

        const t_str_rdonly title_terminated = f_strs_clone_but_add_terminator(title, temp_arena);
        glfwSetWindowTitle(g_state.glfw_window, f_strs_get_as_cstr(title_terminated));
    }

    void f_set_window_size(const t_v2_i size) {
        ZF_ASSERT(g_state.active);
        ZF_ASSERT(size.x > 0 && size.y > 0);

        glfwSetWindowSize(g_state.glfw_window, size.x, size.y);
    }

    void f_set_window_size_limits(const t_i32 min_width, const t_i32 min_height, const t_i32 max_width, const t_i32 max_height) {
        ZF_ASSERT(g_state.active);
        ZF_ASSERT(min_width >= -1 && min_height >= -1);
        ZF_ASSERT(max_width >= min_width || max_width == -1);
        ZF_ASSERT(max_height >= min_height || max_height == -1);

        static_assert(GLFW_DONT_CARE == -1);
        glfwSetWindowSizeLimits(g_state.glfw_window, min_width, min_height, max_width, max_height);
    }

    void f_set_window_resizable(const t_b8 resizable) {
        ZF_ASSERT(g_state.active);
        glfwSetWindowAttrib(g_state.glfw_window, GLFW_RESIZABLE, resizable);
    }

    t_v2_i f_get_window_framebuffer_size_cache() {
        ZF_ASSERT(g_state.active);
        return g_state.framebuffer_size_cache;
    }

    t_b8 f_get_fullscreen() {
        ZF_ASSERT(g_state.active);
        return g_state.fullscreen_active;
    }

    static GLFWmonitor *f_find_glfw_monitor_of_window(GLFWwindow *const window) {
        t_v2_i window_pos;
        glfwGetWindowPos(window, &window_pos.x, &window_pos.y);

        t_v2_i window_size;
        glfwGetWindowSize(window, &window_size.x, &window_size.y);

        const auto window_rect = f_math_create_rect_i(window_pos, window_size);

        // Get the monitor containing the most amount of the window.
        t_f32 max_occupancy_perc = 0.0f;
        t_i32 max_occupancy_monitor_index = -1;

        t_i32 monitor_cnt;
        const auto monitors = glfwGetMonitors(&monitor_cnt);

        for (t_i32 i = 0; i < monitor_cnt; i++) {
            t_v2_i monitor_pos;
            glfwGetMonitorPos(monitors[i], &monitor_pos.x, &monitor_pos.y);

            t_v2 monitor_scale;
            glfwGetMonitorContentScale(monitors[i], &monitor_scale.x, &monitor_scale.y);

            const GLFWvidmode *const mode = glfwGetVideoMode(monitors[i]);

            const t_rect_i monitor_rect = {
                monitor_pos.x,
                monitor_pos.y,
                static_cast<t_i32>(static_cast<t_f32>(mode->width) / monitor_scale.x),
                static_cast<t_i32>(static_cast<t_f32>(mode->height) / monitor_scale.y),
            };

            const t_f32 occupancy_perc = f_math_calc_perc_of_occupance(window_rect, monitor_rect);

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

    void f_set_fullscreen(const t_b8 active) {
        ZF_ASSERT(g_state.active);

        if (active == g_state.fullscreen_active) {
            return;
        }

        if (active) {
            glfwGetWindowPos(g_state.glfw_window, &g_state.prefullscreen_pos.x, &g_state.prefullscreen_pos.y);
            glfwGetWindowSize(g_state.glfw_window, &g_state.prefullscreen_size.x, &g_state.prefullscreen_size.y);

            const auto monitor = f_find_glfw_monitor_of_window(g_state.glfw_window);

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

    t_v2_i f_get_monitor_size_pixels() {
        ZF_ASSERT(g_state.active);

        const auto monitor = f_find_glfw_monitor_of_window(g_state.glfw_window);

        if (!monitor) {
            return {};
        }

        const GLFWvidmode *const mode = glfwGetVideoMode(monitor);
        return {mode->width, mode->height};
    }

    t_v2_i f_get_monitor_size_logical() {
        ZF_ASSERT(g_state.active);

        const auto monitor = f_find_glfw_monitor_of_window(g_state.glfw_window);

        if (!monitor) {
            return {};
        }

        const GLFWvidmode *const mode = glfwGetVideoMode(monitor);

        t_v2 monitor_scale;
        glfwGetMonitorContentScale(monitor, &monitor_scale.x, &monitor_scale.y);

        return {
            static_cast<t_i32>(static_cast<t_f32>(mode->width) / monitor_scale.x),
            static_cast<t_i32>(static_cast<t_f32>(mode->height) / monitor_scale.y),
        };
    }

    void f_set_cursor_visibility(const t_b8 visible) {
        ZF_ASSERT(g_state.active);
        glfwSetInputMode(g_state.glfw_window, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
    }
}
