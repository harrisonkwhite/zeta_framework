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

namespace zgl::platform {
    static struct {
        zcl::t_b8 active;

        GLFWwindow *glfw_window;

        zcl::math::t_v2_i framebuffer_size_cache;

        zcl::t_b8 fullscreen_active;
        zcl::math::t_v2_i prefullscreen_pos;
        zcl::math::t_v2_i prefullscreen_size;
    } g_module_state;

    void module_startup(const zcl::math::t_v2_i init_window_size) {
        ZF_REQUIRE(!g_module_state.active);
        ZF_REQUIRE(init_window_size.x > 0 && init_window_size.y > 0);

        g_module_state = {.active = true};

        if (!glfwInit()) {
            ZF_FATAL();
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_VISIBLE, false);

        g_module_state.glfw_window = glfwCreateWindow(init_window_size.x, init_window_size.y, "", nullptr, nullptr);

        if (!g_module_state.glfw_window) {
            ZF_FATAL();
        }

        glfwGetFramebufferSize(g_module_state.glfw_window, &g_module_state.framebuffer_size_cache.x, &g_module_state.framebuffer_size_cache.y);

        {
            const auto fb_size_callback =
                [](GLFWwindow *const window, const zcl::t_i32 width, const zcl::t_i32 height) {
                    if (width > 0 && height > 0) {
                        g_module_state.framebuffer_size_cache = {width, height};
                    }
                };

            glfwSetFramebufferSizeCallback(g_module_state.glfw_window, fb_size_callback);
        }

        {
            const auto scroll_callback =
                [](GLFWwindow *const window, const zcl::t_f64 offs_x, const zcl::t_f64 offs_y) {
                    const auto input_state = static_cast<input::t_state *>(glfwGetWindowUserPointer(window));
                    input::scroll_update_state(input_state, {static_cast<zcl::t_f32>(offs_x), static_cast<zcl::t_f32>(offs_y)});
                };

            glfwSetScrollCallback(g_module_state.glfw_window, scroll_callback);
        }

        {
            const auto chr_callback =
                [](GLFWwindow *const window, const zcl::t_u32 code_pt) {
                    const auto input_state = static_cast<input::t_state *>(glfwGetWindowUserPointer(window));

                    if (!input::text_submit_code_point(input_state, code_pt)) {
                        zcl::io::log_warning(ZF_STR_LITERAL("Tried to submit input code point, but there is insufficient space!"));
                    }
                };

            glfwSetCharCallback(g_module_state.glfw_window, chr_callback);
        }
    }

    void module_shutdown() {
        ZF_REQUIRE(g_module_state.active);

        glfwDestroyWindow(g_module_state.glfw_window);
        glfwTerminate();
        g_module_state = {};
    }

    zcl::t_f64 get_time() {
        ZF_ASSERT(g_module_state.active);

        const zcl::t_f64 result = glfwGetTime();
        ZF_REQUIRE(result != 0.0);

        return result;
    }

    static zcl::t_i32 f_to_glfw_key(const input::t_key_code key_code) {
        switch (key_code) {
        case input::ek_key_code_space: return GLFW_KEY_SPACE;

        case input::ek_key_code_0: return GLFW_KEY_0;
        case input::ek_key_code_1: return GLFW_KEY_1;
        case input::ek_key_code_2: return GLFW_KEY_2;
        case input::ek_key_code_3: return GLFW_KEY_3;
        case input::ek_key_code_4: return GLFW_KEY_4;
        case input::ek_key_code_5: return GLFW_KEY_5;
        case input::ek_key_code_6: return GLFW_KEY_6;
        case input::ek_key_code_7: return GLFW_KEY_7;
        case input::ek_key_code_8: return GLFW_KEY_8;
        case input::ek_key_code_9: return GLFW_KEY_9;

        case input::ek_key_code_a: return GLFW_KEY_A;
        case input::ek_key_code_b: return GLFW_KEY_B;
        case input::ek_key_code_c: return GLFW_KEY_C;
        case input::ek_key_code_d: return GLFW_KEY_D;
        case input::ek_key_code_e: return GLFW_KEY_E;
        case input::ek_key_code_f: return GLFW_KEY_F;
        case input::ek_key_code_g: return GLFW_KEY_G;
        case input::ek_key_code_h: return GLFW_KEY_H;
        case input::ek_key_code_i: return GLFW_KEY_I;
        case input::ek_key_code_j: return GLFW_KEY_J;
        case input::ek_key_code_k: return GLFW_KEY_K;
        case input::ek_key_code_l: return GLFW_KEY_L;
        case input::ek_key_code_m: return GLFW_KEY_M;
        case input::ek_key_code_n: return GLFW_KEY_N;
        case input::ek_key_code_o: return GLFW_KEY_O;
        case input::ek_key_code_p: return GLFW_KEY_P;
        case input::ek_key_code_q: return GLFW_KEY_Q;
        case input::ek_key_code_r: return GLFW_KEY_R;
        case input::ek_key_code_s: return GLFW_KEY_S;
        case input::ek_key_code_t: return GLFW_KEY_T;
        case input::ek_key_code_u: return GLFW_KEY_U;
        case input::ek_key_code_v: return GLFW_KEY_V;
        case input::ek_key_code_w: return GLFW_KEY_W;
        case input::ek_key_code_x: return GLFW_KEY_X;
        case input::ek_key_code_y: return GLFW_KEY_Y;
        case input::ek_key_code_z: return GLFW_KEY_Z;

        case input::ek_key_code_escape: return GLFW_KEY_ESCAPE;
        case input::ek_key_code_enter: return GLFW_KEY_ENTER;
        case input::ek_key_code_backspace: return GLFW_KEY_BACKSPACE;
        case input::ek_key_code_tab: return GLFW_KEY_TAB;

        case input::ek_key_code_right: return GLFW_KEY_RIGHT;
        case input::ek_key_code_left: return GLFW_KEY_LEFT;
        case input::ek_key_code_down: return GLFW_KEY_DOWN;
        case input::ek_key_code_up: return GLFW_KEY_UP;

        case input::ek_key_code_f1: return GLFW_KEY_F1;
        case input::ek_key_code_f2: return GLFW_KEY_F2;
        case input::ek_key_code_f3: return GLFW_KEY_F3;
        case input::ek_key_code_f4: return GLFW_KEY_F4;
        case input::ek_key_code_f5: return GLFW_KEY_F5;
        case input::ek_key_code_f6: return GLFW_KEY_F6;
        case input::ek_key_code_f7: return GLFW_KEY_F7;
        case input::ek_key_code_f8: return GLFW_KEY_F8;
        case input::ek_key_code_f9: return GLFW_KEY_F9;
        case input::ek_key_code_f10: return GLFW_KEY_F10;
        case input::ek_key_code_f11: return GLFW_KEY_F11;
        case input::ek_key_code_f12: return GLFW_KEY_F12;

        case input::ek_key_code_left_shift: return GLFW_KEY_LEFT_SHIFT;
        case input::ek_key_code_left_control: return GLFW_KEY_LEFT_CONTROL;
        case input::ek_key_code_left_alt: return GLFW_KEY_LEFT_ALT;

        case input::ek_key_code_right_shift: return GLFW_KEY_RIGHT_SHIFT;
        case input::ek_key_code_right_control: return GLFW_KEY_RIGHT_CONTROL;
        case input::ek_key_code_right_alt: return GLFW_KEY_RIGHT_ALT;

        case input::ekm_key_code_cnt: break;
        }

        ZF_UNREACHABLE();
    }

    static zcl::t_i32 f_to_glfw_mouse_button(const input::t_mouse_button_code btn_code) {
        switch (btn_code) {
        case input::ek_mouse_button_code_left: return GLFW_MOUSE_BUTTON_LEFT;
        case input::ek_mouse_button_code_right: return GLFW_MOUSE_BUTTON_RIGHT;
        case input::ek_mouse_button_code_middle: return GLFW_MOUSE_BUTTON_MIDDLE;

        case input::ekm_mouse_button_code_cnt: break;
        }

        ZF_UNREACHABLE();
    };

    void poll_events(input::t_state *const input_state) {
        ZF_ASSERT(g_module_state.active);

        glfwSetWindowUserPointer(g_module_state.glfw_window, input_state); // Scroll callback needs access to this input state.

        glfwPollEvents();

        for (zcl::t_i32 i = 0; i < input::ekm_key_code_cnt; i++) {
            const zcl::t_b8 is_down = glfwGetKey(g_module_state.glfw_window, f_to_glfw_key(static_cast<input::t_key_code>(i))) == GLFW_PRESS;
            input::key_update_state(input_state, static_cast<input::t_key_code>(i), is_down);
        }

        for (zcl::t_i32 i = 0; i < input::ekm_mouse_button_code_cnt; i++) {
            const zcl::t_b8 is_down = glfwGetMouseButton(g_module_state.glfw_window, f_to_glfw_mouse_button(static_cast<input::t_mouse_button_code>(i))) == GLFW_PRESS;
            input::mouse_button_update_state(input_state, static_cast<input::t_mouse_button_code>(i), is_down);
        }

        {
            zcl::t_f64 cp_x_f64, cp_y_f64;
            glfwGetCursorPos(g_module_state.glfw_window, &cp_x_f64, &cp_y_f64);
            input::cursor_update_state(input_state, {static_cast<zcl::t_f32>(cp_x_f64), static_cast<zcl::t_f32>(cp_y_f64)});
        }

        for (zcl::t_i32 i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; i++) {
            zcl::t_b8 connected = false;
            zcl::mem::t_static_bitset<input::ekm_gamepad_button_code_cnt> btns_down = {};
            zcl::t_static_array<zcl::t_f32, input::ekm_gamepad_axis_code_cnt> axes = {};

            GLFWgamepadstate gamepad_state;

            if (glfwJoystickPresent(i) && glfwJoystickIsGamepad(i) && glfwGetGamepadState(i, &gamepad_state)) {
                connected = true;

                for (zcl::t_i32 j = 0; j <= GLFW_GAMEPAD_BUTTON_LAST; j++) {
                    if (gamepad_state.buttons[j]) {
                        zcl::mem::set(btns_down, j);
                    } else {
                        zcl::mem::unset(btns_down, j);
                    }
                }

                for (zcl::t_i32 j = 0; j <= GLFW_GAMEPAD_AXIS_LAST; j++) {
                    axes[j] = gamepad_state.axes[j];
                }
            }

            static_assert(GLFW_JOYSTICK_1 == 0);
            input::gamepad_update_state(input_state, i, connected, btns_down, axes);
        }
    }

    void *display_get_native_handle() {
        ZF_ASSERT(g_module_state.active);

#if defined(ZF_PLATFORM_WINDOWS)
        return nullptr;
#elif defined(ZF_PLATFORM_MACOS)
        return nullptr;
#elif defined(ZF_PLATFORM_LINUX)
        return glfwGetX11Display();
#endif
    }

    void *window_get_native_handle() {
        ZF_ASSERT(g_module_state.active);

#if defined(ZF_PLATFORM_WINDOWS)
        return reinterpret_cast<void *>(glfwGetWin32Window(g_module_state.glfw_window));
#elif defined(ZF_PLATFORM_MACOS)
        return glfwGetCocoaWindow(g_state.glfw_window);
#elif defined(ZF_PLATFORM_LINUX)
        return reinterpret_cast<void *>(static_cast<uintptr_t>(glfwGetX11Window(g_state.glfw_window)));
#endif
    }

    void window_show() {
        ZF_ASSERT(g_module_state.active);
        glfwShowWindow(g_module_state.glfw_window);
    }

    void window_request_close() {
        ZF_ASSERT(g_module_state.active);
        zcl::io::log(ZF_STR_LITERAL("Window close explicitly requested..."));
        return glfwSetWindowShouldClose(g_module_state.glfw_window, true);
    }

    zcl::t_b8 window_check_close_requested() {
        ZF_ASSERT(g_module_state.active);
        return glfwWindowShouldClose(g_module_state.glfw_window);
    }

    void window_set_title(const zcl::strs::t_str_rdonly title, zcl::t_arena *const temp_arena) {
        ZF_ASSERT(g_module_state.active);

        const zcl::strs::t_str_rdonly title_terminated = zcl::strs::clone_but_add_terminator(title, temp_arena);
        glfwSetWindowTitle(g_module_state.glfw_window, zcl::strs::to_cstr(title_terminated));
    }

    void window_set_size(const zcl::math::t_v2_i size) {
        ZF_ASSERT(g_module_state.active);
        ZF_ASSERT(size.x > 0 && size.y > 0);

        glfwSetWindowSize(g_module_state.glfw_window, size.x, size.y);
    }

    void window_set_size_limits(const zcl::t_i32 min_width, const zcl::t_i32 min_height, const zcl::t_i32 max_width, const zcl::t_i32 max_height) {
        ZF_ASSERT(g_module_state.active);
        ZF_ASSERT(min_width >= -1 && min_height >= -1);
        ZF_ASSERT(max_width >= min_width || max_width == -1);
        ZF_ASSERT(max_height >= min_height || max_height == -1);

        static_assert(GLFW_DONT_CARE == -1);
        glfwSetWindowSizeLimits(g_module_state.glfw_window, min_width, min_height, max_width, max_height);
    }

    void window_set_resizable(const zcl::t_b8 resizable) {
        ZF_ASSERT(g_module_state.active);
        glfwSetWindowAttrib(g_module_state.glfw_window, GLFW_RESIZABLE, resizable);
    }

    zcl::math::t_v2_i window_get_framebuffer_size_cache() {
        ZF_ASSERT(g_module_state.active);
        return g_module_state.framebuffer_size_cache;
    }

    zcl::t_b8 window_check_fullscreen() {
        ZF_ASSERT(g_module_state.active);
        return g_module_state.fullscreen_active;
    }

    static GLFWmonitor *find_glfw_monitor_of_window(GLFWwindow *const window) {
        zcl::math::t_v2_i window_pos;
        glfwGetWindowPos(window, &window_pos.x, &window_pos.y);

        zcl::math::t_v2_i window_size;
        glfwGetWindowSize(window, &window_size.x, &window_size.y);

        const auto window_rect = zcl::math::rect_create_i32(window_pos, window_size);

        // Get the monitor containing the most amount of the window.
        zcl::t_f32 max_occupancy_perc = 0.0f;
        zcl::t_i32 max_occupancy_monitor_index = -1;

        zcl::t_i32 monitor_cnt;
        const auto monitors = glfwGetMonitors(&monitor_cnt);

        for (zcl::t_i32 i = 0; i < monitor_cnt; i++) {
            zcl::math::t_v2_i monitor_pos;
            glfwGetMonitorPos(monitors[i], &monitor_pos.x, &monitor_pos.y);

            zcl::math::t_v2 monitor_scale;
            glfwGetMonitorContentScale(monitors[i], &monitor_scale.x, &monitor_scale.y);

            const GLFWvidmode *const mode = glfwGetVideoMode(monitors[i]);

            const zcl::math::t_rect_i monitor_rect = {
                monitor_pos.x,
                monitor_pos.y,
                static_cast<zcl::t_i32>(static_cast<zcl::t_f32>(mode->width) / monitor_scale.x),
                static_cast<zcl::t_i32>(static_cast<zcl::t_f32>(mode->height) / monitor_scale.y),
            };

            const zcl::t_f32 occupancy_perc = zcl::math::calc_perc_of_occupance(window_rect, monitor_rect);

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

    void window_set_fullscreen(const zcl::t_b8 active) {
        ZF_ASSERT(g_module_state.active);

        if (active == g_module_state.fullscreen_active) {
            return;
        }

        if (active) {
            glfwGetWindowPos(g_module_state.glfw_window, &g_module_state.prefullscreen_pos.x, &g_module_state.prefullscreen_pos.y);
            glfwGetWindowSize(g_module_state.glfw_window, &g_module_state.prefullscreen_size.x, &g_module_state.prefullscreen_size.y);

            const auto monitor = find_glfw_monitor_of_window(g_module_state.glfw_window);

            if (!monitor) {
                return;
            }

            const GLFWvidmode *const mode = glfwGetVideoMode(monitor);
            glfwSetWindowMonitor(g_module_state.glfw_window, monitor, 0, 0, mode->width, mode->height, 0);
        } else {
            glfwSetWindowMonitor(g_module_state.glfw_window, nullptr, g_module_state.prefullscreen_pos.x, g_module_state.prefullscreen_pos.y, g_module_state.prefullscreen_size.x, g_module_state.prefullscreen_size.y, 0);
        }

        g_module_state.fullscreen_active = active;
    }

    zcl::math::t_v2_i monitor_calc_size_pixels() {
        ZF_ASSERT(g_module_state.active);

        const auto monitor = find_glfw_monitor_of_window(g_module_state.glfw_window);

        if (!monitor) {
            return {};
        }

        const GLFWvidmode *const mode = glfwGetVideoMode(monitor);
        return {mode->width, mode->height};
    }

    zcl::math::t_v2_i monitor_calc_size_logical() {
        ZF_ASSERT(g_module_state.active);

        const auto monitor = find_glfw_monitor_of_window(g_module_state.glfw_window);

        if (!monitor) {
            return {};
        }

        const GLFWvidmode *const mode = glfwGetVideoMode(monitor);

        zcl::math::t_v2 monitor_scale;
        glfwGetMonitorContentScale(monitor, &monitor_scale.x, &monitor_scale.y);

        return {
            static_cast<zcl::t_i32>(static_cast<zcl::t_f32>(mode->width) / monitor_scale.x),
            static_cast<zcl::t_i32>(static_cast<zcl::t_f32>(mode->height) / monitor_scale.y),
        };
    }

    void cursor_set_visible(const zcl::t_b8 visible) {
        ZF_ASSERT(g_module_state.active);
        glfwSetInputMode(g_module_state.glfw_window, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
    }
}
