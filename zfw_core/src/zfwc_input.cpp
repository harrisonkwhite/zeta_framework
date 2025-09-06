#include "zfwc_input.h"

static constexpr t_s32 ToGLFWKey(const e_key_code kc) {
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

        default: return eks_key_code_none;
    }
}

static constexpr t_s32 ToGLFWMouseButton(const e_mouse_button_code mbc) {
    switch (mbc) {
        case ek_mouse_button_code_left: return GLFW_MOUSE_BUTTON_LEFT;
        case ek_mouse_button_code_right: return GLFW_MOUSE_BUTTON_RIGHT;
        case ek_mouse_button_code_middle: return GLFW_MOUSE_BUTTON_MIDDLE;

        default: return eks_mouse_button_code_none;
    }
}

static t_key_bits KeysDownBits(GLFWwindow* glfw_window) {
    t_key_bits keys_down = 0;

    for (t_s32 i = 0; i < eks_key_code_cnt; i++) {
        if (glfwGetKey(glfw_window, ToGLFWKey(static_cast<e_key_code>(i)))) {
            keys_down |= static_cast<t_key_bits>(1) << i;
        }
    }

    return keys_down;
}

static t_mouse_button_bits MouseButtonsDownBits(GLFWwindow* glfw_window) {
    t_mouse_button_bits mouse_buttons_down = 0;

    for (t_s32 i = 0; i < eks_mouse_button_code_cnt; i++) {
        if (glfwGetMouseButton(glfw_window, ToGLFWMouseButton(static_cast<e_mouse_button_code>(i)))) {
            mouse_buttons_down |= static_cast<t_mouse_button_bits>(1) << i;
        }
    }

    return mouse_buttons_down;
}

static s_v2 MousePos(GLFWwindow* glfw_window) {
    double mouse_x_dbl, mouse_y_dbl;
    glfwGetCursorPos(glfw_window, &mouse_x_dbl, &mouse_y_dbl);
    return {static_cast<float>(mouse_x_dbl), static_cast<float>(mouse_y_dbl)};
}

s_input_state InputState(GLFWwindow* glfw_window) {
    return {
        .keys_down = KeysDownBits(glfw_window),
        .mouse_buttons_down = MouseButtonsDownBits(glfw_window),
        .mouse_pos = MousePos(glfw_window)
    };
}

static e_key_code GLFWToZFWKeyCode(const t_s32 glfw_key) {
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

static e_mouse_button_code GLFWToZFWMouseButtonCode(const t_s32 glfw_button) {
    switch (glfw_button) {
        case GLFW_MOUSE_BUTTON_LEFT: return ek_mouse_button_code_left;
        case GLFW_MOUSE_BUTTON_RIGHT: return ek_mouse_button_code_right;
        case GLFW_MOUSE_BUTTON_MIDDLE: return ek_mouse_button_code_middle;

        default: return eks_mouse_button_code_none;
    }
}

void GLFWKeyCallback(GLFWwindow* window, const int key, const int, const int action, const int) {
    auto* input_events = reinterpret_cast<s_input_events*>(glfwGetWindowUserPointer(window));

    const e_key_code key_code = GLFWToZFWKeyCode(key);

    if (key_code == eks_key_code_none) {
        return;
    }

    const t_key_bits key_mask = static_cast<t_key_bits>(1) << key_code;

    if (action == GLFW_PRESS) {
        input_events->keys_pressed |= key_mask;
    } else if (action == GLFW_RELEASE) {
        input_events->keys_released |= key_mask;
    }
}

void GLFWMouseButtonCallback(GLFWwindow* window, const int button, const int action, const int) {
    auto* input_events = reinterpret_cast<s_input_events*>(glfwGetWindowUserPointer(window));

    const e_mouse_button_code mb_code = GLFWToZFWMouseButtonCode(button);

    if (mb_code == eks_mouse_button_code_none) {
        return;
    }

    const t_mouse_button_bits mb_mask = static_cast<t_mouse_button_bits>(1) << mb_code;

    if (action == GLFW_PRESS) {
        input_events->mouse_buttons_pressed |= mb_mask;
    } else if (action == GLFW_RELEASE) {
        input_events->mouse_buttons_released |= mb_mask;
    }
}

void GLFWScrollCallback(GLFWwindow* window, const double, const double offs_y) {
    auto* input_events = reinterpret_cast<s_input_events*>(glfwGetWindowUserPointer(window));

    if (offs_y > 0.0) {
        input_events->mouse_scroll_state = ek_mouse_scroll_state_up;
    } else if (offs_y < 0.0) {
        input_events->mouse_scroll_state = ek_mouse_scroll_state_down;
    } else {
        input_events->mouse_scroll_state = ek_mouse_scroll_state_none;
    }
}

void GLFWCharCallback(GLFWwindow* window, const unsigned int codepoint) {
    auto* input_events = reinterpret_cast<s_input_events*>(glfwGetWindowUserPointer(window));

    for (size_t i = 0; i < sizeof(input_events->unicode_buf); i++) {
        if (!input_events->unicode_buf[i]) {
            input_events->unicode_buf[i] = static_cast<char>(codepoint);
            return;
        }
    }

    //LOG_WARNING("Unicode buffer is full!");
}
