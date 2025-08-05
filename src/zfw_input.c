#include "zfw_input.h"

static const int g_glfw_keys[] = {
    [zfw_ek_key_code_space] = GLFW_KEY_SPACE,
    [zfw_ek_key_code_0] = GLFW_KEY_0,
    [zfw_ek_key_code_1] = GLFW_KEY_1,
    [zfw_ek_key_code_2] = GLFW_KEY_2,
    [zfw_ek_key_code_3] = GLFW_KEY_3,
    [zfw_ek_key_code_4] = GLFW_KEY_4,
    [zfw_ek_key_code_5] = GLFW_KEY_5,
    [zfw_ek_key_code_6] = GLFW_KEY_6,
    [zfw_ek_key_code_7] = GLFW_KEY_7,
    [zfw_ek_key_code_8] = GLFW_KEY_8,
    [zfw_ek_key_code_9] = GLFW_KEY_9,
    [zfw_ek_key_code_a] = GLFW_KEY_A,
    [zfw_ek_key_code_b] = GLFW_KEY_B,
    [zfw_ek_key_code_c] = GLFW_KEY_C,
    [zfw_ek_key_code_d] = GLFW_KEY_D,
    [zfw_ek_key_code_e] = GLFW_KEY_E,
    [zfw_ek_key_code_f] = GLFW_KEY_F,
    [zfw_ek_key_code_g] = GLFW_KEY_G,
    [zfw_ek_key_code_h] = GLFW_KEY_H,
    [zfw_ek_key_code_i] = GLFW_KEY_I,
    [zfw_ek_key_code_j] = GLFW_KEY_J,
    [zfw_ek_key_code_k] = GLFW_KEY_K,
    [zfw_ek_key_code_l] = GLFW_KEY_L,
    [zfw_ek_key_code_m] = GLFW_KEY_M,
    [zfw_ek_key_code_n] = GLFW_KEY_N,
    [zfw_ek_key_code_o] = GLFW_KEY_O,
    [zfw_ek_key_code_p] = GLFW_KEY_P,
    [zfw_ek_key_code_q] = GLFW_KEY_Q,
    [zfw_ek_key_code_r] = GLFW_KEY_R,
    [zfw_ek_key_code_s] = GLFW_KEY_S,
    [zfw_ek_key_code_t] = GLFW_KEY_T,
    [zfw_ek_key_code_u] = GLFW_KEY_U,
    [zfw_ek_key_code_v] = GLFW_KEY_V,
    [zfw_ek_key_code_w] = GLFW_KEY_W,
    [zfw_ek_key_code_x] = GLFW_KEY_X,
    [zfw_ek_key_code_y] = GLFW_KEY_Y,
    [zfw_ek_key_code_z] = GLFW_KEY_Z,
    [zfw_ek_key_code_escape] = GLFW_KEY_ESCAPE,
    [zfw_ek_key_code_enter] = GLFW_KEY_ENTER,
    [zfw_ek_key_code_backspace] = GLFW_KEY_BACKSPACE,
    [zfw_ek_key_code_tab] = GLFW_KEY_TAB,
    [zfw_ek_key_code_right] = GLFW_KEY_RIGHT,
    [zfw_ek_key_code_left] = GLFW_KEY_LEFT,
    [zfw_ek_key_code_down] = GLFW_KEY_DOWN,
    [zfw_ek_key_code_up] = GLFW_KEY_UP,
    [zfw_ek_key_code_f1] = GLFW_KEY_F1,
    [zfw_ek_key_code_f2] = GLFW_KEY_F2,
    [zfw_ek_key_code_f3] = GLFW_KEY_F3,
    [zfw_ek_key_code_f4] = GLFW_KEY_F4,
    [zfw_ek_key_code_f5] = GLFW_KEY_F5,
    [zfw_ek_key_code_f6] = GLFW_KEY_F6,
    [zfw_ek_key_code_f7] = GLFW_KEY_F7,
    [zfw_ek_key_code_f8] = GLFW_KEY_F8,
    [zfw_ek_key_code_f9] = GLFW_KEY_F9,
    [zfw_ek_key_code_f10] = GLFW_KEY_F10,
    [zfw_ek_key_code_f11] = GLFW_KEY_F11,
    [zfw_ek_key_code_f12] = GLFW_KEY_F12,
    [zfw_ek_key_code_left_shift] = GLFW_KEY_LEFT_SHIFT,
    [zfw_ek_key_code_left_control] = GLFW_KEY_LEFT_CONTROL,
    [zfw_ek_key_code_left_alt] = GLFW_KEY_LEFT_ALT,
    [zfw_ek_key_code_right_shift] = GLFW_KEY_RIGHT_SHIFT,
    [zfw_ek_key_code_right_control] = GLFW_KEY_RIGHT_CONTROL,
    [zfw_ek_key_code_right_alt] = GLFW_KEY_RIGHT_ALT
};

STATIC_ARRAY_LEN_CHECK(g_glfw_keys, zfw_eks_key_code_cnt);

static const int g_glfw_mouse_buttons[] = {
    [zfw_ek_mouse_button_code_left] = GLFW_MOUSE_BUTTON_LEFT,
    [zfw_ek_mouse_button_code_right] = GLFW_MOUSE_BUTTON_RIGHT,
    [zfw_ek_mouse_button_code_middle] = GLFW_MOUSE_BUTTON_MIDDLE
};

STATIC_ARRAY_LEN_CHECK(g_glfw_mouse_buttons, zfw_eks_mouse_button_code_cnt);

static zfw_t_key_bits KeysDownBits(GLFWwindow* const glfw_window) {
    assert(glfw_window);

    zfw_t_key_bits keys_down = 0;

    for (int i = 0; i < zfw_eks_key_code_cnt; i++) {
        if (glfwGetKey(glfw_window, g_glfw_keys[i])) {
            keys_down |= (zfw_t_key_bits)1 << i;
        }
    }

    return keys_down;
}

static zfw_t_mouse_button_bits MouseButtonsDownBits(GLFWwindow* const glfw_window) {
    assert(glfw_window);

    zfw_t_mouse_button_bits mouse_buttons_down = 0;

    for (int i = 0; i < zfw_eks_mouse_button_code_cnt; i++) {
        if (glfwGetMouseButton(glfw_window, g_glfw_mouse_buttons[i])) {
            mouse_buttons_down |= (zfw_t_mouse_button_bits)1 << i;
        }
    }

    return mouse_buttons_down;
}

static s_v2 MousePos(GLFWwindow* const glfw_window) {
    assert(glfw_window);

    double mouse_x_dbl, mouse_y_dbl;
    glfwGetCursorPos(glfw_window, &mouse_x_dbl, &mouse_y_dbl);
    return (s_v2){mouse_x_dbl, mouse_y_dbl};
}

zfw_s_input_state ZFW_InputState(GLFWwindow* const glfw_window) {
    assert(glfw_window);

    return (zfw_s_input_state){
        .keys_down = KeysDownBits(glfw_window),
        .mouse_buttons_down = MouseButtonsDownBits(glfw_window),
        .mouse_pos = MousePos(glfw_window)
    };
}

static zfw_e_key_code GLFWToZFWKeyCode(const int glfw_key) {
    switch (glfw_key) {
        case GLFW_KEY_SPACE: return zfw_ek_key_code_space;
        case GLFW_KEY_0: return zfw_ek_key_code_0;
        case GLFW_KEY_1: return zfw_ek_key_code_1;
        case GLFW_KEY_2: return zfw_ek_key_code_2;
        case GLFW_KEY_3: return zfw_ek_key_code_3;
        case GLFW_KEY_4: return zfw_ek_key_code_4;
        case GLFW_KEY_5: return zfw_ek_key_code_5;
        case GLFW_KEY_6: return zfw_ek_key_code_6;
        case GLFW_KEY_7: return zfw_ek_key_code_7;
        case GLFW_KEY_8: return zfw_ek_key_code_8;
        case GLFW_KEY_9: return zfw_ek_key_code_9;
        case GLFW_KEY_A: return zfw_ek_key_code_a;
        case GLFW_KEY_B: return zfw_ek_key_code_b;
        case GLFW_KEY_C: return zfw_ek_key_code_c;
        case GLFW_KEY_D: return zfw_ek_key_code_d;
        case GLFW_KEY_E: return zfw_ek_key_code_e;
        case GLFW_KEY_F: return zfw_ek_key_code_f;
        case GLFW_KEY_G: return zfw_ek_key_code_g;
        case GLFW_KEY_H: return zfw_ek_key_code_h;
        case GLFW_KEY_I: return zfw_ek_key_code_i;
        case GLFW_KEY_J: return zfw_ek_key_code_j;
        case GLFW_KEY_K: return zfw_ek_key_code_k;
        case GLFW_KEY_L: return zfw_ek_key_code_l;
        case GLFW_KEY_M: return zfw_ek_key_code_m;
        case GLFW_KEY_N: return zfw_ek_key_code_n;
        case GLFW_KEY_O: return zfw_ek_key_code_o;
        case GLFW_KEY_P: return zfw_ek_key_code_p;
        case GLFW_KEY_Q: return zfw_ek_key_code_q;
        case GLFW_KEY_R: return zfw_ek_key_code_r;
        case GLFW_KEY_S: return zfw_ek_key_code_s;
        case GLFW_KEY_T: return zfw_ek_key_code_t;
        case GLFW_KEY_U: return zfw_ek_key_code_u;
        case GLFW_KEY_V: return zfw_ek_key_code_v;
        case GLFW_KEY_W: return zfw_ek_key_code_w;
        case GLFW_KEY_X: return zfw_ek_key_code_x;
        case GLFW_KEY_Y: return zfw_ek_key_code_y;
        case GLFW_KEY_Z: return zfw_ek_key_code_z;
        case GLFW_KEY_ESCAPE: return zfw_ek_key_code_escape;
        case GLFW_KEY_ENTER: return zfw_ek_key_code_enter;
        case GLFW_KEY_BACKSPACE: return zfw_ek_key_code_backspace;
        case GLFW_KEY_TAB: return zfw_ek_key_code_tab;
        case GLFW_KEY_RIGHT: return zfw_ek_key_code_right;
        case GLFW_KEY_LEFT: return zfw_ek_key_code_left;
        case GLFW_KEY_DOWN: return zfw_ek_key_code_down;
        case GLFW_KEY_UP: return zfw_ek_key_code_up;
        case GLFW_KEY_F1: return zfw_ek_key_code_f1;
        case GLFW_KEY_F2: return zfw_ek_key_code_f2;
        case GLFW_KEY_F3: return zfw_ek_key_code_f3;
        case GLFW_KEY_F4: return zfw_ek_key_code_f4;
        case GLFW_KEY_F5: return zfw_ek_key_code_f5;
        case GLFW_KEY_F6: return zfw_ek_key_code_f6;
        case GLFW_KEY_F7: return zfw_ek_key_code_f7;
        case GLFW_KEY_F8: return zfw_ek_key_code_f8;
        case GLFW_KEY_F9: return zfw_ek_key_code_f9;
        case GLFW_KEY_F10: return zfw_ek_key_code_f10;
        case GLFW_KEY_F11: return zfw_ek_key_code_f11;
        case GLFW_KEY_F12: return zfw_ek_key_code_f12;
        case GLFW_KEY_LEFT_SHIFT: return zfw_ek_key_code_left_shift;
        case GLFW_KEY_LEFT_CONTROL: return zfw_ek_key_code_left_control;
        case GLFW_KEY_LEFT_ALT: return zfw_ek_key_code_left_alt;
        case GLFW_KEY_RIGHT_SHIFT: return zfw_ek_key_code_right_shift;
        case GLFW_KEY_RIGHT_CONTROL: return zfw_ek_key_code_right_control;
        case GLFW_KEY_RIGHT_ALT: return zfw_ek_key_code_right_alt;

        default: return zfw_eks_key_code_none;
    }
}

static zfw_e_mouse_button_code GLFWToZFWMouseButtonCode(const int glfw_button) {
    switch (glfw_button) {
        case GLFW_MOUSE_BUTTON_LEFT: return zfw_ek_mouse_button_code_left;
        case GLFW_MOUSE_BUTTON_RIGHT: return zfw_ek_mouse_button_code_right;
        case GLFW_MOUSE_BUTTON_MIDDLE: return zfw_ek_mouse_button_code_middle;

        default: return zfw_eks_mouse_button_code_none;
    }
}

void ZFW_GLFWKeyCallback(GLFWwindow* const window, const int key, const int scancode, const int action, const int mods) {
    zfw_s_input_events* const input_events = glfwGetWindowUserPointer(window);

    const zfw_e_key_code zfw_key_code = GLFWToZFWKeyCode(key);

    if (zfw_key_code == zfw_eks_key_code_none) {
        return;
    }

    const zfw_t_key_bits key_mask = (zfw_t_key_bits)1 << zfw_key_code;

    if (action == GLFW_PRESS) {
        input_events->keys_pressed |= key_mask;
    } else if (action == GLFW_RELEASE) {
        input_events->keys_released |= key_mask;
    }
}

void ZFW_GLFWMouseButtonCallback(GLFWwindow* const window, const int button, const int action, const int mods) {
    zfw_s_input_events* const input_events = glfwGetWindowUserPointer(window);

    const zfw_e_mouse_button_code zfw_mb_code = GLFWToZFWMouseButtonCode(button);

    if (zfw_mb_code == zfw_eks_mouse_button_code_none) {
        return;
    }

    const zfw_t_mouse_button_bits mb_mask = (zfw_t_mouse_button_bits)1 << zfw_mb_code;

    if (action == GLFW_PRESS) {
        input_events->mouse_buttons_pressed |= mb_mask;
    } else if (action == GLFW_RELEASE) {
        input_events->mouse_buttons_released |= mb_mask;
    }
}

void ZFW_GLFWScrollCallback(GLFWwindow* const window, const double offs_x, const double offs_y) {
    zfw_s_input_events* const input_events = glfwGetWindowUserPointer(window);

    if (offs_y > 0.0) {
        input_events->mouse_scroll_state = zfw_ek_mouse_scroll_state_up;
    } else if (offs_y < 0.0) {
        input_events->mouse_scroll_state = zfw_ek_mouse_scroll_state_down;
    } else {
        input_events->mouse_scroll_state = zfw_ek_mouse_scroll_state_none;
    }
}

void ZFW_GLFWCharCallback(GLFWwindow* const window, const unsigned int codepoint) {
    zfw_s_input_events* const input_events = glfwGetWindowUserPointer(window);

    for (size_t i = 0; i < sizeof(input_events->unicode_buf); i++) {
        if (!input_events->unicode_buf[i]) {
            input_events->unicode_buf[i] = codepoint;
            return;
        }
    }

    LOG_WARNING("Unicode buffer is full!");
}
