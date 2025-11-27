#include <zf/zf_window.h>

#include <GLFW/glfw3.h>

#if defined(ZF_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(ZF_PLATFORM_MACOS)
    #define GLFW_EXPOSE_NATIVE_COCOA
#elif defined(ZF_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
#endif

#include <GLFW/glfw3native.h>
#include <glad/glad.h>

namespace zf {
    struct s_input_events {
        s_static_bit_vector<eks_key_code_cnt> keys_pressed;
        s_static_bit_vector<eks_key_code_cnt> keys_released;

        s_static_bit_vector<eks_mouse_button_code_cnt> mouse_buttons_pressed;
        s_static_bit_vector<eks_mouse_button_code_cnt> mouse_buttons_released;

        e_mouse_scroll_state mouse_scroll_state;

        s_static_array<char, 32> unicode_buf;
    };

    static GLFWwindow* g_glfw_window;
    static s_input_events g_input_events; // Events such as key presses and mouse wheel scrolls are recordeded here in callbacks.

    constexpr e_key_code ConvertGLFWKeyCode(const t_s32 glfw_key) {
        switch (glfw_key) {
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

    constexpr e_mouse_button_code ConvertGLFWMouseButtonCode(const t_s32 glfw_button) {
        switch (glfw_button) {
            case GLFW_MOUSE_BUTTON_LEFT: return ek_mouse_button_code_left;
            case GLFW_MOUSE_BUTTON_RIGHT: return ek_mouse_button_code_right;
            case GLFW_MOUSE_BUTTON_MIDDLE: return ek_mouse_button_code_middle;

            default: return eks_mouse_button_code_none;
        }
    }

    constexpr t_s32 ToGLFWKey(const e_key_code kc) {
        switch (kc) {
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

            default: return GLFW_KEY_UNKNOWN;
        }
    }

    constexpr t_s32 ToGLFWMouseButton(const e_mouse_button_code mbc) {
        switch (mbc) {
            case ek_mouse_button_code_left: return GLFW_MOUSE_BUTTON_LEFT;
            case ek_mouse_button_code_right: return GLFW_MOUSE_BUTTON_RIGHT;
            case ek_mouse_button_code_middle: return GLFW_MOUSE_BUTTON_MIDDLE;

            default: return -1;
        }
    }

    t_b8 InitWindow(const s_v2<t_s32> size, const s_str_rdonly title, const e_window_flags flags, s_mem_arena& temp_mem_arena) {
        ZF_ASSERT(!g_glfw_window);
        ZF_ASSERT(size.x > 0 && size.y > 0);

        if (!glfwInit()) {
            ZF_REPORT_FAILURE();
            return false;
        }

        // Set up the GLFW window.
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_VISIBLE, false);

        s_str title_terminated;

        if (!CloneStrButAddTerminator(title, temp_mem_arena, title_terminated)) {
            ZF_REPORT_FAILURE();
            return false;
        }

        g_glfw_window = glfwCreateWindow(size.x, size.y, StrRaw(title_terminated), nullptr, nullptr);

        if (!g_glfw_window) {
            ZF_REPORT_FAILURE();
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(g_glfw_window);

        glfwSetWindowAttrib(g_glfw_window, GLFW_RESIZABLE, (flags & ek_window_flags_resizable) ? true : false);

        glfwSetInputMode(g_glfw_window, GLFW_CURSOR, (flags & ek_window_flags_hide_cursor) ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);

        // Set up GLFW callbacks.
        glfwSetFramebufferSizeCallback(g_glfw_window,
            [](GLFWwindow* const window, const t_s32 width, const t_s32 height) {
                glViewport(0, 0, width, height);
            }
        );

        glfwSetKeyCallback(g_glfw_window,
            [](GLFWwindow* const window, const t_s32 key, const t_s32, const t_s32 action, const t_s32 mods) {
                const e_key_code key_code = ConvertGLFWKeyCode(key);

                if (key_code == eks_key_code_none) {
                    return;
                }

                if (action == GLFW_PRESS) {
                    SetBit(g_input_events.keys_pressed, key_code);
                } else if (action == GLFW_RELEASE) {
                    SetBit(g_input_events.keys_released, key_code);
                }
            }
        );

        glfwSetMouseButtonCallback(g_glfw_window,
            [](GLFWwindow* const window, const t_s32 button, const t_s32 action, const t_s32 mods) {
                const e_mouse_button_code mb_code = ConvertGLFWMouseButtonCode(button);

                if (mb_code == eks_mouse_button_code_none) {
                    return;
                }

                if (action == GLFW_PRESS) {
                    SetBit(g_input_events.mouse_buttons_pressed, mb_code);
                } else if (action == GLFW_RELEASE) {
                    SetBit(g_input_events.mouse_buttons_released, mb_code);
                }
            }
        );

        glfwSetScrollCallback(g_glfw_window,
            [](GLFWwindow* const window, const t_f64, const t_f64 offs_y) {
                if (offs_y > 0.0) {
                    g_input_events.mouse_scroll_state = ek_mouse_scroll_state_up;
                } else if (offs_y < 0.0) {
                    g_input_events.mouse_scroll_state = ek_mouse_scroll_state_down;
                } else {
                    g_input_events.mouse_scroll_state = ek_mouse_scroll_state_none;
                }
            }
        );

        glfwSetCharCallback(g_glfw_window,
            [](GLFWwindow* window, const t_u32 code_pt) {
                for (t_s32 i = 0; i < g_input_events.unicode_buf.g_len; i++) {
                    if (!g_input_events.unicode_buf[i]) {
                        g_input_events.unicode_buf[i] = static_cast<char>(code_pt);
                        return;
                    }
                }

                LogWarning("Unicode buffer is full!");
            }
        );

        // Initialise OpenGL function pointers.
        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
            ZF_REPORT_FAILURE();
            return false;
        }

        // Initialise OpenGL blending.
        // @todo: I hate having this GFX stuff here!
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        return true;
    }

    void ReleaseWindow() {
        ZF_ASSERT(g_glfw_window);

        glfwDestroyWindow(g_glfw_window);
        glfwTerminate();
    }

    void* GetNativeWindowHandle() {
#if defined(ZF_PLATFORM_WINDOWS)
        return glfwGetWin32Window(g_glfw_window);
#elif defined(ZF_PLATFORM_MACOS)
        return glfwGetCocoaWindow(sm_glfw_window);
#elif defined(ZF_PLATFORM_LINUX)
        return glfwGetX11Window(sm_glfw_window);
#endif
    }

    void* GetNativeDisplayHandle() {
#if defined(ZF_PLATFORM_WINDOWS)
        return nullptr;
#elif defined(ZF_PLATFORM_MACOS)
        return nullptr;
#elif defined(ZF_PLATFORM_LINUX)
        return glfwGetX11Display();
#endif
    }

    void PollEvents() {
        glfwPollEvents();
    }

    void SwapBuffers() {
        glfwSwapBuffers(g_glfw_window);
    }

    void ShowWindow() {
        glfwShowWindow(g_glfw_window);
    }

    t_b8 ShouldWindowClose() {
        return glfwWindowShouldClose(g_glfw_window);
    }

    void SetWindowShouldClose(const t_b8 close) {
        glfwSetWindowShouldClose(g_glfw_window, close);
    }

    s_v2<t_s32> GetWindowSize() {
        t_s32 w, h;
        glfwGetWindowSize(g_glfw_window, &w, &h);
        return {w, h};
    }

    s_v2<t_s32> GetFramebufferSize() {
        t_s32 w, h;
        glfwGetFramebufferSize(g_glfw_window, &w, &h);
        return {w, h};
    }

    t_f64 GetTime() {
        return glfwGetTime();
    }

    void ClearInputEvents() {
        g_input_events = {};
    }

    t_b8 IsKeyDown(const e_key_code kc) {
        ZF_ASSERT(kc != eks_key_code_none);
        return glfwGetKey(g_glfw_window, ToGLFWKey(kc)) != GLFW_RELEASE;
    }

    t_b8 IsKeyPressed(const e_key_code kc) {
        ZF_ASSERT(kc != eks_key_code_none);
        return IsBitSet(g_input_events.keys_pressed, kc);
    }

    t_b8 IsKeyReleased(const e_key_code kc) {
        ZF_ASSERT(kc != eks_key_code_none);
        return IsBitSet(g_input_events.keys_released, kc);
    }

    t_b8 IsMouseButtonDown(const e_mouse_button_code mbc) {
        ZF_ASSERT(mbc != eks_mouse_button_code_none);
        return glfwGetMouseButton(g_glfw_window, ToGLFWMouseButton(mbc)) != GLFW_RELEASE;
    }

    t_b8 IsMouseButtonPressed(const e_mouse_button_code mbc) {
        ZF_ASSERT(mbc != eks_mouse_button_code_none);
        return IsBitSet(g_input_events.mouse_buttons_pressed, mbc);
    }

    t_b8 IsMouseButtonReleased(const e_mouse_button_code mbc) {
        ZF_ASSERT(mbc != eks_mouse_button_code_none);
        return IsBitSet(g_input_events.mouse_buttons_released, mbc);
    }

    s_v2<t_f64> GetMousePos() {
        t_f64 mx, my;
        glfwGetCursorPos(g_glfw_window, &mx, &my);
        return {mx, my};
    }

    e_mouse_scroll_state GetMouseScrollState() {
        return g_input_events.mouse_scroll_state;
    }
}
