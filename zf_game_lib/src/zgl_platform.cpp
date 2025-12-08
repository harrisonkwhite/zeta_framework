#include <zgl/zgl_platform.h>

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <zgl/zgl_config.h>
#include <zgl/zgl_input.h>

namespace zf {
    constexpr e_key_code ConvertGLFWKeyCode(const t_s32 glfw_key) {
        switch (glfw_key) {
        case GLFW_KEY_SPACE:
            return ek_key_code_space;

        case GLFW_KEY_0:
            return ek_key_code_0;

        case GLFW_KEY_1:
            return ek_key_code_1;

        case GLFW_KEY_2:
            return ek_key_code_2;

        case GLFW_KEY_3:
            return ek_key_code_3;

        case GLFW_KEY_4:
            return ek_key_code_4;

        case GLFW_KEY_5:
            return ek_key_code_5;

        case GLFW_KEY_6:
            return ek_key_code_6;

        case GLFW_KEY_7:
            return ek_key_code_7;

        case GLFW_KEY_8:
            return ek_key_code_8;

        case GLFW_KEY_9:
            return ek_key_code_9;

        case GLFW_KEY_A:
            return ek_key_code_a;

        case GLFW_KEY_B:
            return ek_key_code_b;

        case GLFW_KEY_C:
            return ek_key_code_c;

        case GLFW_KEY_D:
            return ek_key_code_d;

        case GLFW_KEY_E:
            return ek_key_code_e;

        case GLFW_KEY_F:
            return ek_key_code_f;

        case GLFW_KEY_G:
            return ek_key_code_g;

        case GLFW_KEY_H:
            return ek_key_code_h;

        case GLFW_KEY_I:
            return ek_key_code_i;

        case GLFW_KEY_J:
            return ek_key_code_j;

        case GLFW_KEY_K:
            return ek_key_code_k;

        case GLFW_KEY_L:
            return ek_key_code_l;

        case GLFW_KEY_M:
            return ek_key_code_m;

        case GLFW_KEY_N:
            return ek_key_code_n;

        case GLFW_KEY_O:
            return ek_key_code_o;

        case GLFW_KEY_P:
            return ek_key_code_p;

        case GLFW_KEY_Q:
            return ek_key_code_q;

        case GLFW_KEY_R:
            return ek_key_code_r;

        case GLFW_KEY_S:
            return ek_key_code_s;

        case GLFW_KEY_T:
            return ek_key_code_t;

        case GLFW_KEY_U:
            return ek_key_code_u;

        case GLFW_KEY_V:
            return ek_key_code_v;

        case GLFW_KEY_W:
            return ek_key_code_w;

        case GLFW_KEY_X:
            return ek_key_code_x;

        case GLFW_KEY_Y:
            return ek_key_code_y;

        case GLFW_KEY_Z:
            return ek_key_code_z;

        case GLFW_KEY_ESCAPE:
            return ek_key_code_escape;

        case GLFW_KEY_ENTER:
            return ek_key_code_enter;

        case GLFW_KEY_BACKSPACE:
            return ek_key_code_backspace;

        case GLFW_KEY_TAB:
            return ek_key_code_tab;

        case GLFW_KEY_RIGHT:
            return ek_key_code_right;

        case GLFW_KEY_LEFT:
            return ek_key_code_left;

        case GLFW_KEY_DOWN:
            return ek_key_code_down;

        case GLFW_KEY_UP:
            return ek_key_code_up;

        case GLFW_KEY_F1:
            return ek_key_code_f1;

        case GLFW_KEY_F2:
            return ek_key_code_f2;

        case GLFW_KEY_F3:
            return ek_key_code_f3;

        case GLFW_KEY_F4:
            return ek_key_code_f4;

        case GLFW_KEY_F5:
            return ek_key_code_f5;

        case GLFW_KEY_F6:
            return ek_key_code_f6;

        case GLFW_KEY_F7:
            return ek_key_code_f7;

        case GLFW_KEY_F8:
            return ek_key_code_f8;

        case GLFW_KEY_F9:
            return ek_key_code_f9;

        case GLFW_KEY_F10:
            return ek_key_code_f10;

        case GLFW_KEY_F11:
            return ek_key_code_f11;

        case GLFW_KEY_F12:
            return ek_key_code_f12;

        case GLFW_KEY_LEFT_SHIFT:
            return ek_key_code_left_shift;

        case GLFW_KEY_LEFT_CONTROL:
            return ek_key_code_left_control;

        case GLFW_KEY_LEFT_ALT:
            return ek_key_code_left_alt;

        case GLFW_KEY_RIGHT_SHIFT:
            return ek_key_code_right_shift;

        case GLFW_KEY_RIGHT_CONTROL:
            return ek_key_code_right_control;

        case GLFW_KEY_RIGHT_ALT:
            return ek_key_code_right_alt;

        default:
            return eks_key_code_none;
        }
    }

    constexpr e_mouse_button_code ConvertGLFWMouseButtonCode(const t_s32 glfw_button) {
        switch (glfw_button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            return ek_mouse_button_code_left;

        case GLFW_MOUSE_BUTTON_RIGHT:
            return ek_mouse_button_code_right;

        case GLFW_MOUSE_BUTTON_MIDDLE:
            return ek_mouse_button_code_middle;

        default:
            return eks_mouse_button_code_none;
        }
    }

    constexpr i_e_key_action ConvertGLFWKeyAction(const t_s32 glfw_act) {
        switch (glfw_act) {
        case GLFW_PRESS:
            return i_e_key_action::press;

        case GLFW_RELEASE:
            return i_e_key_action::release;
        }

        ZF_ASSERT(false);
        return i_e_key_action::invalid;
    }

    constexpr i_e_mouse_button_action ConvertGLFWMouseButtonAction(const t_s32 glfw_act) {
        switch (glfw_act) {
        case GLFW_PRESS:
            return i_e_mouse_button_action::press;

        case GLFW_RELEASE:
            return i_e_mouse_button_action::release;
        }

        ZF_ASSERT(false);
        return i_e_mouse_button_action::invalid;
    }

    static t_b8 g_initted;

    struct s_platform_layer_info {
        GLFWwindow *glfw_window;

        s_v2<t_s32> framebuffer_size_cache;

        s_input_state *input_state;

        t_b8 fullscreen_active;
        s_v2<t_s32> prefullscreen_pos;
        s_v2<t_s32> prefullscreen_size;
    };

    s_platform_layer_info *internal::InitPlatformLayer(s_mem_arena *const mem_arena,
                                                       s_input_state *const input_state) {
        ZF_ASSERT(!g_initted);

        t_b8 clean_up = false;

        const auto info = PushToMemArena<s_platform_layer_info>(mem_arena);

        if (!info) {
            ZF_REPORT_ERROR();
            clean_up = true;
            return nullptr;
        }

        ZeroOut(info);

        info->input_state = input_state;

        if (!glfwInit()) {
            ZF_REPORT_ERROR();
            clean_up = true;
            return nullptr;
        }

        ZF_DEFER({
            if (clean_up) {
                glfwTerminate();
            }
        });

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, i_g_gl_version_major);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, i_g_gl_version_minor);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_VISIBLE, false);

        info->glfw_window = glfwCreateWindow(1280, 720, "", nullptr, nullptr);

        if (!info->glfw_window) {
            ZF_REPORT_ERROR();
            clean_up = true;
            return nullptr;
        }

        ZF_DEFER({
            if (clean_up) {
                glfwDestroyWindow(info->glfw_window);
            }
        });

        glfwMakeContextCurrent(info->glfw_window);

        glfwGetFramebufferSize(info->glfw_window, &info->framebuffer_size_cache.x,
                               &info->framebuffer_size_cache.y);

        // Initialise OpenGL function pointers.
        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
            ZF_REPORT_ERROR();
            clean_up = true;
            return nullptr;
        }

        // Set up callbacks.
        glfwSetWindowUserPointer(info->glfw_window, info);

        {
            const auto fb_size_callback = [](GLFWwindow *const glfw_window, const t_s32 width,
                                             const t_s32 height) {
                if (width > 0 && height > 0) {
                    const auto pl = static_cast<s_platform_layer_info *>(
                        glfwGetWindowUserPointer(glfw_window));
                    pl->framebuffer_size_cache = {width, height};
                }
            };

            glfwSetFramebufferSizeCallback(info->glfw_window, fb_size_callback);
        }

        {
            const auto key_callback = [](GLFWwindow *const glfw_window, const t_s32 key,
                                         const t_s32 scancode, const t_s32 act,
                                         const t_s32 mods) {
                if (act == GLFW_REPEAT) {
                    return;
                }

                const auto pl = static_cast<s_platform_layer_info *>(
                    glfwGetWindowUserPointer(glfw_window));
                I_ProcKeyAction(pl->input_state, ConvertGLFWKeyCode(key),
                                ConvertGLFWKeyAction(act));
            };

            glfwSetKeyCallback(info->glfw_window, key_callback);
        }

        {
            const auto mb_callback = [](GLFWwindow *const glfw_window, const t_s32 btn,
                                        const t_s32 act, const t_s32 mods) {
                const auto pl = static_cast<s_platform_layer_info *>(
                    glfwGetWindowUserPointer(glfw_window));
                I_ProcMouseButtonAction(pl->input_state, ConvertGLFWMouseButtonCode(btn),
                                        ConvertGLFWMouseButtonAction(act));
            };

            glfwSetMouseButtonCallback(info->glfw_window, mb_callback);
        }

        {
            const auto cursor_pos_callback = [](GLFWwindow *const glfw_window, const t_f64 x,
                                                const t_f64 y) {
                const auto pl = static_cast<s_platform_layer_info *>(
                    glfwGetWindowUserPointer(glfw_window));
                I_ProcCursorMove(pl->input_state,
                                 {static_cast<t_f32>(x), static_cast<t_f32>(y)});
            };

            glfwSetCursorPosCallback(info->glfw_window, cursor_pos_callback);
        }

        {
            const auto scroll_callback = [](GLFWwindow *const glfw_window, const t_f64 offs_x,
                                            const t_f64 offs_y) {
                const auto pl = static_cast<s_platform_layer_info *>(
                    glfwGetWindowUserPointer(glfw_window));
                I_ProcScroll(pl->input_state,
                             {static_cast<t_f32>(offs_x), static_cast<t_f32>(offs_y)});
            };

            glfwSetScrollCallback(info->glfw_window, scroll_callback);
        }

        g_initted = true;

        return info;
    }

    void internal::ShutdownPlatformLayer(const s_platform_layer_info *const pli) {
        ZF_ASSERT(g_initted);

        glfwDestroyWindow(pli->glfw_window);
        glfwTerminate();
        g_initted = false;
    }

    t_f64 Time() {
        return glfwGetTime();
    }

    void internal::PollOSEvents() {
        glfwPollEvents();
    }

    void internal::ShowWindow(const s_platform_layer_info *const pli) {
        glfwShowWindow(pli->glfw_window);
    }

    t_b8 internal::ShouldWindowClose(const s_platform_layer_info *const pli) {
        return glfwWindowShouldClose(pli->glfw_window);
    }

    void internal::SwapWindowBuffers(const s_platform_layer_info *const pli) {
        glfwSwapBuffers(pli->glfw_window);
    }

    void SetWindowTitle(const s_platform_layer_info *const pli, const s_str_rdonly title) {
        ZF_ASSERT(IsValidUTF8Str(title));

        s_static_array<t_u8, 256> title_terminated_bytes = {};
        CopyOrTruncate(Slice(ToNonstaticArray(title_terminated_bytes), 0,
                             title_terminated_bytes.g_len - 1),
                       title.bytes);
        glfwSetWindowTitle(pli->glfw_window,
                           reinterpret_cast<const char *>(title_terminated_bytes.buf_raw));
    }

    void SetWindowSize(const s_platform_layer_info *const pli, const s_v2<t_s32> size) {
        ZF_ASSERT(size.x > 0 && size.y > 0);
        glfwSetWindowSize(pli->glfw_window, size.x, size.y);
    }

    void SetWindowSizeLimits(const s_platform_layer_info *const pli, const t_s32 min_width,
                             const t_s32 min_height, const t_s32 max_width,
                             const t_s32 max_height) {
        ZF_ASSERT(min_width >= -1 && min_height >= -1);
        ZF_ASSERT(max_width >= min_width || max_width == -1);
        ZF_ASSERT(max_height >= min_height || max_height == -1);

        static_assert(GLFW_DONT_CARE == -1);
        glfwSetWindowSizeLimits(pli->glfw_window, min_width, min_height, max_width,
                                max_height);
    }

    void SetWindowResizability(const s_platform_layer_info *const pli, const t_b8 resizable) {
        glfwSetWindowAttrib(pli->glfw_window, GLFW_RESIZABLE, resizable);
    }

    s_v2<t_s32> WindowFramebufferSizeCache(const s_platform_layer_info *const pli) {
        return pli->framebuffer_size_cache;
    }

    t_b8 IsFullscreen(const s_platform_layer_info *const pli) {
        return pli->fullscreen_active;
    }

    static GLFWmonitor *MonitorOfWindow(GLFWwindow *const glfw_window) {
        s_rect<t_s32> window_rect;
        glfwGetWindowPos(glfw_window, &window_rect.x, &window_rect.y);
        glfwGetWindowSize(glfw_window, &window_rect.width, &window_rect.height);

        // Get the monitor containing the most amount of the window.
        t_f32 max_occupancy_perc = 0.0f;
        t_size max_occupancy_monitor_index = -1;

        t_s32 monitor_cnt;
        const auto monitors = glfwGetMonitors(&monitor_cnt);

        for (t_size i = 0; i < monitor_cnt; i++) {
            s_v2<t_s32> monitor_pos;
            glfwGetMonitorPos(monitors[i], &monitor_pos.x, &monitor_pos.y);

            s_v2<t_f32> monitor_scale;
            glfwGetMonitorContentScale(monitors[i], &monitor_scale.x, &monitor_scale.y);

            const auto mode = glfwGetVideoMode(monitors[i]);

            const s_rect<t_s32> monitor_rect = {
                monitor_pos.x, monitor_pos.y,
                static_cast<t_s32>(static_cast<t_f32>(mode->width) / monitor_scale.x),
                static_cast<t_s32>(static_cast<t_f32>(mode->height) / monitor_scale.y)};

            const t_f32 occupancy_perc = CalcRectOccupancyPerc(window_rect, monitor_rect);

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

    s_v2<t_s32> CalcMonitorPixelSize(const s_platform_layer_info *const pli) {
        const auto monitor = MonitorOfWindow(pli->glfw_window);

        if (!monitor) {
            return {};
        }

        const auto mode = glfwGetVideoMode(monitor);
        return {mode->width, mode->height};
    }

    s_v2<t_s32> CalcMonitorLogicalSize(const s_platform_layer_info *const pli) {
        const auto monitor = MonitorOfWindow(pli->glfw_window);

        if (!monitor) {
            return {};
        }

        const auto mode = glfwGetVideoMode(monitor);

        s_v2<t_f32> monitor_scale;
        glfwGetMonitorContentScale(monitor, &monitor_scale.x, &monitor_scale.y);

        return {static_cast<t_s32>(static_cast<t_f32>(mode->width) / monitor_scale.x),
                static_cast<t_s32>(static_cast<t_f32>(mode->height) / monitor_scale.y)};
    }

    void SetFullscreen(s_platform_layer_info *const pli, const t_b8 fs) {
        if (fs == pli->fullscreen_active) {
            return;
        }

        if (fs) {
            glfwGetWindowPos(pli->glfw_window, &pli->prefullscreen_pos.x,
                             &pli->prefullscreen_pos.y);
            glfwGetWindowSize(pli->glfw_window, &pli->prefullscreen_size.x,
                              &pli->prefullscreen_size.y);

            const auto monitor = MonitorOfWindow(pli->glfw_window);

            if (!monitor) {
                return;
            }

            const auto mode = glfwGetVideoMode(monitor);
            glfwSetWindowMonitor(pli->glfw_window, monitor, 0, 0, mode->width, mode->height,
                                 0);
        } else {
            glfwSetWindowMonitor(pli->glfw_window, nullptr, pli->prefullscreen_pos.x,
                                 pli->prefullscreen_pos.y, pli->prefullscreen_size.x,
                                 pli->prefullscreen_size.y, 0);
        }

        pli->fullscreen_active = fs;
    }

    void SetCursorVisibility(const s_platform_layer_info *const pli, const t_b8 visible) {
        glfwSetInputMode(pli->glfw_window, GLFW_CURSOR,
                         visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
    }
}
