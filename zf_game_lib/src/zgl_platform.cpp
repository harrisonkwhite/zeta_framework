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

namespace zf {
    // ============================================================
    // @section: Types and Declarations
    // ============================================================
    struct {
        t_b8 initted = false;

        s_ptr<GLFWwindow> glfw_window = nullptr;

        s_v2_i framebuffer_size_cache = {};

        t_b8 fullscreen_active = false;
        s_v2_i prefullscreen_pos = {};
        s_v2_i prefullscreen_size = {};

        struct {
            s_static_bit_vec<eks_key_code_cnt> keys_down = {};
            s_static_bit_vec<eks_mouse_button_code_cnt> mouse_buttons_down = {};

            s_v2 cursor_pos = {};

            struct {
                s_static_bit_vec<eks_key_code_cnt> keys_pressed;
                s_static_bit_vec<eks_key_code_cnt> keys_released;

                s_static_bit_vec<eks_mouse_button_code_cnt> mouse_buttons_pressed;
                s_static_bit_vec<eks_mouse_button_code_cnt> mouse_buttons_released;

                s_v2 scroll;
            } events = {};
        } input;
    } g_state;

    static s_ptr<GLFWmonitor> FindGLFWMonitorOfWindow(const s_ptr<GLFWwindow> window);

    static void GLFWKeyCallback(GLFWwindow *window, int key, int scancode, int act, int mods);
    static void GLFWMouseButtonCallback(GLFWwindow *window, int btn, int act, int mods);
    static void GLFWCursorPosCallback(GLFWwindow *window, double x, double y);
    static void GLFWScrollCallback(GLFWwindow *window, double offs_x, double offs_y);

    // ============================================================
    // @section: General
    // ============================================================
    void internal::InitPlatform(const s_v2_i init_window_size) {
        ZF_ASSERT(!g_state.initted);
        ZF_ASSERT(init_window_size.x > 0 && init_window_size.y > 0);

        if (!glfwInit()) {
            ZF_FATAL();
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_VISIBLE, false);

        g_state.glfw_window = glfwCreateWindow(init_window_size.x, init_window_size.y, "", nullptr, nullptr);

        if (!g_state.glfw_window) {
            ZF_FATAL();
        }

        glfwMakeContextCurrent(g_state.glfw_window);

        glfwGetFramebufferSize(g_state.glfw_window, &g_state.framebuffer_size_cache.x, &g_state.framebuffer_size_cache.y);

        {
            const auto fb_size_callback =
                [](GLFWwindow *glfw_window, int width, int height) {
                    if (width > 0 && height > 0) {
                        g_state.framebuffer_size_cache = {width, height};
                    }
                };

            glfwSetFramebufferSizeCallback(g_state.glfw_window, fb_size_callback);
        }

        glfwSetKeyCallback(g_state.glfw_window, GLFWKeyCallback);
        glfwSetMouseButtonCallback(g_state.glfw_window, GLFWMouseButtonCallback);
        glfwSetCursorPosCallback(g_state.glfw_window, GLFWCursorPosCallback);
        glfwSetScrollCallback(g_state.glfw_window, GLFWScrollCallback);

        g_state.initted = true;
    }

    void internal::ShutdownPlatform() {
        ZF_ASSERT(g_state.initted);

        glfwDestroyWindow(g_state.glfw_window);
        glfwTerminate();
        g_state = {};
    }

    t_f64 Time() {
        ZF_ASSERT(g_state.initted);
        return glfwGetTime();
    }

    void internal::PollOSEvents() {
        ZF_ASSERT(g_state.initted);
        glfwPollEvents();
    }

    // ============================================================
    // @section: Display
    // ============================================================
    void *internal::NativeWindowHandle() {
        ZF_ASSERT(g_state.initted);

#if defined(ZF_PLATFORM_WINDOWS)
        return reinterpret_cast<void *>(glfwGetWin32Window(g_state.glfw_window));
#elif defined(ZF_PLATFORM_MACOS)
        return glfwGetCocoaWindow(g_state.glfw_window);
#elif defined(ZF_PLATFORM_LINUX)
        return reinterpret_cast<void *>(static_cast<uintptr_t>(glfwGetX11Window(g_state.glfw_window)));
#endif
    }

    void *internal::NativeDisplayHandle() {
        ZF_ASSERT(g_state.initted);

#if defined(ZF_PLATFORM_WINDOWS)
        return nullptr;
#elif defined(ZF_PLATFORM_MACOS)
        return nullptr;
#elif defined(ZF_PLATFORM_LINUX)
        return glfwGetX11Display();
#endif
    }

    void internal::ShowWindow() {
        ZF_ASSERT(g_state.initted);
        glfwShowWindow(g_state.glfw_window);
    }

    t_b8 internal::ShouldWindowClose() {
        ZF_ASSERT(g_state.initted);
        return glfwWindowShouldClose(g_state.glfw_window);
    }

    void SetWindowTitle(const s_str_rdonly title, s_mem_arena &temp_mem_arena) {
        ZF_ASSERT(g_state.initted);

        const s_str_rdonly title_terminated = AllocStrCloneButAddTerminator(title, temp_mem_arena);
        glfwSetWindowTitle(g_state.glfw_window, title_terminated.Cstr());
    }

    void SetWindowSize(const s_v2_i size) {
        ZF_ASSERT(g_state.initted);
        ZF_ASSERT(size.x > 0 && size.y > 0);

        glfwSetWindowSize(g_state.glfw_window, size.x, size.y);
    }

    void SetWindowSizeLimits(const t_i32 min_width, const t_i32 min_height, const t_i32 max_width, const t_i32 max_height) {
        ZF_ASSERT(g_state.initted);
        ZF_ASSERT(min_width >= -1 && min_height >= -1);
        ZF_ASSERT(max_width >= min_width || max_width == -1);
        ZF_ASSERT(max_height >= min_height || max_height == -1);

        static_assert(GLFW_DONT_CARE == -1);
        glfwSetWindowSizeLimits(g_state.glfw_window, min_width, min_height, max_width, max_height);
    }

    void SetWindowResizability(const t_b8 resizable) {
        ZF_ASSERT(g_state.initted);
        glfwSetWindowAttrib(g_state.glfw_window, GLFW_RESIZABLE, resizable);
    }

    s_v2_i WindowFramebufferSizeCache() {
        ZF_ASSERT(g_state.initted);
        return g_state.framebuffer_size_cache;
    }

    t_b8 IsFullscreen() {
        ZF_ASSERT(g_state.initted);
        return g_state.fullscreen_active;
    }

    static s_ptr<GLFWmonitor> FindGLFWMonitorOfWindow(const s_ptr<GLFWwindow> window) {
        ZF_ASSERT(g_state.initted);

        s_v2_i window_pos;
        glfwGetWindowPos(window, &window_pos.x, &window_pos.y);

        s_v2_i window_size;
        glfwGetWindowSize(window, &window_size.x, &window_size.y);

        const s_rect_i window_rect = {window_pos, window_size};

        // Get the monitor containing the most amount of the window.
        t_f32 max_occupancy_perc = 0.0f;
        t_len max_occupancy_monitor_index = -1;

        t_i32 monitor_cnt = 0;
        const auto monitors = glfwGetMonitors(&monitor_cnt);

        for (t_len i = 0; i < monitor_cnt; i++) {
            s_v2_i monitor_pos;
            glfwGetMonitorPos(monitors[i], &monitor_pos.x, &monitor_pos.y);

            s_v2 monitor_scale;
            glfwGetMonitorContentScale(monitors[i], &monitor_scale.x, &monitor_scale.y);

            const s_ptr<const GLFWvidmode> mode = glfwGetVideoMode(monitors[i]);

            const s_rect_i monitor_rect = {
                monitor_pos.x,
                monitor_pos.y,
                static_cast<t_i32>(static_cast<t_f32>(mode->width) / monitor_scale.x),
                static_cast<t_i32>(static_cast<t_f32>(mode->height) / monitor_scale.y),
            };

            const t_f32 occupancy_perc = window_rect.CalcOccupancyPerc(monitor_rect);

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

    s_v2_i CalcMonitorPixelSize() {
        ZF_ASSERT(g_state.initted);

        const auto monitor = FindGLFWMonitorOfWindow(g_state.glfw_window);

        if (!monitor) {
            return {};
        }

        const s_ptr<const GLFWvidmode> mode = glfwGetVideoMode(monitor);
        return {mode->width, mode->height};
    }

    s_v2_i CalcMonitorLogicalSize() {
        ZF_ASSERT(g_state.initted);

        const auto monitor = FindGLFWMonitorOfWindow(g_state.glfw_window);

        if (!monitor) {
            return {};
        }

        const s_ptr<const GLFWvidmode> mode = glfwGetVideoMode(monitor);

        s_v2 monitor_scale = {};
        glfwGetMonitorContentScale(monitor, &monitor_scale.x, &monitor_scale.y);

        return {
            static_cast<t_i32>(static_cast<t_f32>(mode->width) / monitor_scale.x),
            static_cast<t_i32>(static_cast<t_f32>(mode->height) / monitor_scale.y),
        };
    }

    void SetFullscreen(const t_b8 fs) {
        ZF_ASSERT(g_state.initted);

        if (fs == g_state.fullscreen_active) {
            return;
        }

        if (fs) {
            glfwGetWindowPos(g_state.glfw_window, &g_state.prefullscreen_pos.x, &g_state.prefullscreen_pos.y);
            glfwGetWindowSize(g_state.glfw_window, &g_state.prefullscreen_size.x, &g_state.prefullscreen_size.y);

            const auto monitor = FindGLFWMonitorOfWindow(g_state.glfw_window);

            if (!monitor) {
                return;
            }

            const s_ptr<const GLFWvidmode> mode = glfwGetVideoMode(monitor);
            glfwSetWindowMonitor(g_state.glfw_window, monitor, 0, 0, mode->width, mode->height, 0);
        } else {
            glfwSetWindowMonitor(g_state.glfw_window, nullptr, g_state.prefullscreen_pos.x, g_state.prefullscreen_pos.y, g_state.prefullscreen_size.x, g_state.prefullscreen_size.y, 0);
        }

        g_state.fullscreen_active = fs;
    }

    // ============================================================
    // @section: Input
    // ============================================================
    constexpr e_key_code ConvertGLFWKeyCode(const t_i32 key) {
        switch (key) {
        case GLFW_KEY_SPACE: return ek_key_code_space;

        case GLFW_KEY_0: return ek_key_code_0;
        case GLFW_KEY_1: return ek_key_code_1;
        case GLFW_KEY_2: return ek_key_code_2;
        case GLFW_KEY_3: return ek_key_code_3;
        case GLFW_KEY_4: return ek_key_code_4;
        case GLFW_KEY_5: return ek_key_code_5;
        case GLFW_KEY_6: return ek_key_code_6;
        case GLFW_KEY_7: return ek_key_code_7;
        case GLFW_KEY_8: return ek_key_code_8;
        case GLFW_KEY_9: return ek_key_code_9;

        case GLFW_KEY_A: return ek_key_code_a;
        case GLFW_KEY_B: return ek_key_code_b;
        case GLFW_KEY_C: return ek_key_code_c;
        case GLFW_KEY_D: return ek_key_code_d;
        case GLFW_KEY_E: return ek_key_code_e;
        case GLFW_KEY_F: return ek_key_code_f;
        case GLFW_KEY_G: return ek_key_code_g;
        case GLFW_KEY_H: return ek_key_code_h;
        case GLFW_KEY_I: return ek_key_code_i;
        case GLFW_KEY_J: return ek_key_code_j;
        case GLFW_KEY_K: return ek_key_code_k;
        case GLFW_KEY_L: return ek_key_code_l;
        case GLFW_KEY_M: return ek_key_code_m;
        case GLFW_KEY_N: return ek_key_code_n;
        case GLFW_KEY_O: return ek_key_code_o;
        case GLFW_KEY_P: return ek_key_code_p;
        case GLFW_KEY_Q: return ek_key_code_q;
        case GLFW_KEY_R: return ek_key_code_r;
        case GLFW_KEY_S: return ek_key_code_s;
        case GLFW_KEY_T: return ek_key_code_t;
        case GLFW_KEY_U: return ek_key_code_u;
        case GLFW_KEY_V: return ek_key_code_v;
        case GLFW_KEY_W: return ek_key_code_w;
        case GLFW_KEY_X: return ek_key_code_x;
        case GLFW_KEY_Y: return ek_key_code_y;
        case GLFW_KEY_Z: return ek_key_code_z;

        case GLFW_KEY_ESCAPE: return ek_key_code_escape;
        case GLFW_KEY_ENTER: return ek_key_code_enter;
        case GLFW_KEY_BACKSPACE: return ek_key_code_backspace;
        case GLFW_KEY_TAB: return ek_key_code_tab;

        case GLFW_KEY_RIGHT: return ek_key_code_right;
        case GLFW_KEY_LEFT: return ek_key_code_left;
        case GLFW_KEY_DOWN: return ek_key_code_down;
        case GLFW_KEY_UP: return ek_key_code_up;

        case GLFW_KEY_F1: return ek_key_code_f1;
        case GLFW_KEY_F2: return ek_key_code_f2;
        case GLFW_KEY_F3: return ek_key_code_f3;
        case GLFW_KEY_F4: return ek_key_code_f4;
        case GLFW_KEY_F5: return ek_key_code_f5;
        case GLFW_KEY_F6: return ek_key_code_f6;
        case GLFW_KEY_F7: return ek_key_code_f7;
        case GLFW_KEY_F8: return ek_key_code_f8;
        case GLFW_KEY_F9: return ek_key_code_f9;
        case GLFW_KEY_F10: return ek_key_code_f10;
        case GLFW_KEY_F11: return ek_key_code_f11;
        case GLFW_KEY_F12: return ek_key_code_f12;

        case GLFW_KEY_LEFT_SHIFT: return ek_key_code_left_shift;
        case GLFW_KEY_LEFT_CONTROL: return ek_key_code_left_control;
        case GLFW_KEY_LEFT_ALT: return ek_key_code_left_alt;

        case GLFW_KEY_RIGHT_SHIFT: return ek_key_code_right_shift;
        case GLFW_KEY_RIGHT_CONTROL: return ek_key_code_right_control;
        case GLFW_KEY_RIGHT_ALT: return ek_key_code_right_alt;

        default: return eks_key_code_none;
        }
    }

    static void GLFWKeyCallback(GLFWwindow *window, int key, int scancode, int act, int mods) {
        const auto kc = ConvertGLFWKeyCode(key);

        switch (act) {
        case GLFW_PRESS:
            SetBit(g_state.input.keys_down, kc);
            SetBit(g_state.input.events.keys_pressed, kc);
            break;

        case GLFW_RELEASE:
            UnsetBit(g_state.input.keys_down, kc);
            SetBit(g_state.input.events.keys_released, kc);
            break;

        case GLFW_REPEAT:
            break;
        }
    }

    constexpr e_mouse_button_code ConvertGLFWMouseButton(const t_i32 btn) {
        switch (btn) {
        case GLFW_MOUSE_BUTTON_LEFT: return ek_mouse_button_code_left;
        case GLFW_MOUSE_BUTTON_RIGHT: return ek_mouse_button_code_right;
        case GLFW_MOUSE_BUTTON_MIDDLE: return ek_mouse_button_code_middle;

        default: return eks_mouse_button_code_none;
        }
    }

    static void GLFWMouseButtonCallback(GLFWwindow *window, int btn, int act, int mods) {
        const auto mbc = ConvertGLFWMouseButton(btn);

        switch (act) {
        case GLFW_PRESS:
            SetBit(g_state.input.mouse_buttons_down, mbc);
            SetBit(g_state.input.events.mouse_buttons_pressed, mbc);
            break;

        case GLFW_RELEASE:
            UnsetBit(g_state.input.mouse_buttons_down, mbc);
            SetBit(g_state.input.events.mouse_buttons_pressed, mbc);
            break;
        }
    }

    static void GLFWCursorPosCallback(GLFWwindow *window, double x, double y) {
        g_state.input.cursor_pos = {
            static_cast<t_f32>(x),
            static_cast<t_f32>(y),
        };
    }

    static void GLFWScrollCallback(GLFWwindow *window, double offs_x, double offs_y) {
        g_state.input.events.scroll += s_v2(static_cast<t_f32>(offs_x), static_cast<t_f32>(offs_y));
    }

    t_b8 IsKeyDown(const e_key_code kc) {
        ZF_ASSERT(g_state.initted);
        return IsBitSet(g_state.input.keys_down, kc);
    }

    t_b8 IsKeyPressed(const e_key_code kc) {
        ZF_ASSERT(g_state.initted);
        return IsBitSet(g_state.input.events.keys_pressed, kc);
    }

    t_b8 IsKeyReleased(const e_key_code kc) {
        ZF_ASSERT(g_state.initted);
        return IsBitSet(g_state.input.events.keys_released, kc);
    }

    t_b8 IsMouseButtonDown(const e_mouse_button_code mbc) {
        ZF_ASSERT(g_state.initted);
        return IsBitSet(g_state.input.mouse_buttons_down, mbc);
    }

    t_b8 IsMouseButtonPressed(const e_mouse_button_code mbc) {
        ZF_ASSERT(g_state.initted);
        return IsBitSet(g_state.input.events.mouse_buttons_pressed, mbc);
    }

    t_b8 IsMouseButtonReleased(const e_mouse_button_code mbc) {
        ZF_ASSERT(g_state.initted);
        return IsBitSet(g_state.input.events.mouse_buttons_released, mbc);
    }

    s_v2 CursorPos() {
        ZF_ASSERT(g_state.initted);
        return g_state.input.cursor_pos;
    }

    s_v2 ScrollOffset() {
        ZF_ASSERT(g_state.initted);
        return g_state.input.events.scroll;
    }

    void SetCursorVisibility(const t_b8 visible) {
        ZF_ASSERT(g_state.initted);
        glfwSetInputMode(g_state.glfw_window, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
    }

    void internal::ClearInputEvents() {
        ZF_ASSERT(g_state.initted);
        g_state.input.events = {};
    }
}
