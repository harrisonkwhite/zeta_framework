#include <zf/zf_window.h>

#include <glad/glad.h>

namespace zf {
    static e_key_code GLFWToZFKeyCode(const t_s32 glfw_key) {
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

    static e_mouse_button_code GLFWToZFMouseButtonCode(const t_s32 glfw_button) {
        switch (glfw_button) {
            case GLFW_MOUSE_BUTTON_LEFT: return ek_mouse_button_code_left;
            case GLFW_MOUSE_BUTTON_RIGHT: return ek_mouse_button_code_right;
            case GLFW_MOUSE_BUTTON_MIDDLE: return ek_mouse_button_code_middle;

            default: return eks_mouse_button_code_none;
        }
    }

    t_b8 c_window::Init(const s_v2<t_s32> size, const s_str_ro title, const e_window_flags flags) {
        ZF_ASSERT(!sm_glfw_window);
        ZF_ASSERT(size.x > 0 && size.y > 0);
        ZF_ASSERT(IsStrTerminated(title));

        if (!glfwInit()) {
            ZF_REPORT_FAILURE();
            return false;
        }

        // Set up the GLFW window.
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_VISIBLE, false);

        sm_glfw_window = glfwCreateWindow(size.x, size.y, title.Raw(), nullptr, nullptr);

        if (!sm_glfw_window) {
            ZF_REPORT_FAILURE();
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(sm_glfw_window);

        glfwSetWindowAttrib(sm_glfw_window, GLFW_RESIZABLE, (flags & ek_window_flags_resizable) ? true : false);

        glfwSetInputMode(sm_glfw_window, GLFW_CURSOR, (flags & ek_window_flags_hide_cursor) ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);

        // Set up GLFW callbacks.
        glfwSetFramebufferSizeCallback(sm_glfw_window,
            [](GLFWwindow* const window, const t_s32 width, const t_s32 height) {
                //c_renderer::RefreshSize();
            }
        );

        glfwSetKeyCallback(sm_glfw_window,
            [](GLFWwindow* const window, const t_s32 key, const t_s32, const t_s32 action, const t_s32 mods) {
                const e_key_code key_code = GLFWToZFKeyCode(key);

                if (key_code == eks_key_code_none) {
                    return;
                }

                if (action == GLFW_PRESS) {
                    SetBit(sm_input_events.keys_pressed, key_code);
                } else if (action == GLFW_RELEASE) {
                    SetBit(sm_input_events.keys_released, key_code);
                }
            }
        );

        glfwSetMouseButtonCallback(sm_glfw_window,
            [](GLFWwindow* const window, const t_s32 button, const t_s32 action, const t_s32 mods) {
                const e_mouse_button_code mb_code = GLFWToZFMouseButtonCode(button);

                if (mb_code == eks_mouse_button_code_none) {
                    return;
                }

                if (action == GLFW_PRESS) {
                    SetBit(sm_input_events.mouse_buttons_pressed, mb_code);
                } else if (action == GLFW_RELEASE) {
                    SetBit(sm_input_events.mouse_buttons_released, mb_code);
                }
            }
        );

        glfwSetScrollCallback(sm_glfw_window,
            [](GLFWwindow* const window, const t_f64, const t_f64 offs_y) {
                if (offs_y > 0.0) {
                    sm_input_events.mouse_scroll_state = ec_mouse_scroll_state::up;
                } else if (offs_y < 0.0) {
                    sm_input_events.mouse_scroll_state = ec_mouse_scroll_state::down;
                } else {
                    sm_input_events.mouse_scroll_state = ec_mouse_scroll_state::none;
                }
            }
        );

        glfwSetCharCallback(sm_glfw_window,
            [](GLFWwindow* window, const t_u32 codepoint) {
                for (t_s32 i = 0; i < sm_input_events.unicode_buf.Len(); i++) {
                    if (!sm_input_events.unicode_buf[i]) {
                        sm_input_events.unicode_buf[i] = static_cast<char>(codepoint);
                        return;
                    }
                }

                ZF_LOG_WARNING("Unicode buffer is full!");
            }
        );

        // Initialise OpenGL function pointers.
        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
            ZF_REPORT_FAILURE();
            return false;
        }

        return true;
    }

    void c_window::Clean() {
        ZF_ASSERT(sm_glfw_window);

        glfwDestroyWindow(sm_glfw_window);
        glfwTerminate();
    }
}
