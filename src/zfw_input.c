#include "zfw_input.h"

#include <GLFW/glfw3.h>

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

void ZFW_RefreshInputState(zfw_s_input_state* const state, GLFWwindow* const glfw_window, const zfw_e_mouse_scroll_state mouse_scroll_state) {
    assert(state);
    assert(glfw_window);

    ZERO_OUT(*state);

    for (int i = 0; i < zfw_eks_key_code_cnt; i++) {
        if (glfwGetKey(glfw_window, g_glfw_keys[i])) {
            state->keys_down |= (zfw_t_keys_down_bits)1 << i;
        }
    }

    for (int i = 0; i < zfw_eks_mouse_button_code_cnt; i++) {
        if (glfwGetMouseButton(glfw_window, g_glfw_mouse_buttons[i])) {
            state->mouse_buttons_down |= (zfw_t_mouse_buttons_down_bits)1 << i;
        }
    }

    double mouse_x_dbl, mouse_y_dbl;
    glfwGetCursorPos(glfw_window, &mouse_x_dbl, &mouse_y_dbl);
    state->mouse_pos = (zfw_s_vec_2d){mouse_x_dbl, mouse_y_dbl};

    state->mouse_scroll_state = mouse_scroll_state;
}
