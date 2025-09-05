#pragma once

#include <cassert>

#include <GLFW/glfw3.h>
#include <cu.h>

enum e_key_code {
    eks_key_code_none = -1,

    ek_key_code_space,
    ek_key_code_0,
    ek_key_code_1,
    ek_key_code_2,
    ek_key_code_3,
    ek_key_code_4,
    ek_key_code_5,
    ek_key_code_6,
    ek_key_code_7,
    ek_key_code_8,
    ek_key_code_9,
    ek_key_code_a,
    ek_key_code_b,
    ek_key_code_c,
    ek_key_code_d,
    ek_key_code_e,
    ek_key_code_f,
    ek_key_code_g,
    ek_key_code_h,
    ek_key_code_i,
    ek_key_code_j,
    ek_key_code_k,
    ek_key_code_l,
    ek_key_code_m,
    ek_key_code_n,
    ek_key_code_o,
    ek_key_code_p,
    ek_key_code_q,
    ek_key_code_r,
    ek_key_code_s,
    ek_key_code_t,
    ek_key_code_u,
    ek_key_code_v,
    ek_key_code_w,
    ek_key_code_x,
    ek_key_code_y,
    ek_key_code_z,
    ek_key_code_escape,
    ek_key_code_enter,
    ek_key_code_backspace,
    ek_key_code_tab,
    ek_key_code_right,
    ek_key_code_left,
    ek_key_code_down,
    ek_key_code_up,
    ek_key_code_f1,
    ek_key_code_f2,
    ek_key_code_f3,
    ek_key_code_f4,
    ek_key_code_f5,
    ek_key_code_f6,
    ek_key_code_f7,
    ek_key_code_f8,
    ek_key_code_f9,
    ek_key_code_f10,
    ek_key_code_f11,
    ek_key_code_f12,
    ek_key_code_left_shift,
    ek_key_code_left_control,
    ek_key_code_left_alt,
    ek_key_code_right_shift,
    ek_key_code_right_control,
    ek_key_code_right_alt,

    eks_key_code_cnt
};

using t_key_bits = t_u64;

static_assert(eks_key_code_cnt < CU_SIZE_IN_BITS(t_key_bits), "Too many key codes!");

enum e_mouse_button_code {
    eks_mouse_button_code_none = -1,

    ek_mouse_button_code_left,
    ek_mouse_button_code_right,
    ek_mouse_button_code_middle,

    eks_mouse_button_code_cnt
};

using t_mouse_button_bits = t_u8;

static_assert(eks_mouse_button_code_cnt < CU_SIZE_IN_BITS(t_mouse_button_bits), "Too many mouse button codes!");

enum e_mouse_scroll_state {
    ek_mouse_scroll_state_none,
    ek_mouse_scroll_state_down,
    ek_mouse_scroll_state_up
};

using t_unicode_buf = char[32];

struct s_input_state {
    t_key_bits keys_down;
    t_mouse_button_bits mouse_buttons_down;

    s_v2 mouse_pos;
};

struct s_input_events {
    t_key_bits keys_pressed;
    t_key_bits keys_released;

    t_mouse_button_bits mouse_buttons_pressed;
    t_mouse_button_bits mouse_buttons_released;

    e_mouse_scroll_state mouse_scroll_state;

    t_unicode_buf unicode_buf;
};

struct s_input_context {
    const s_input_state& state;
    const s_input_events& events;
};

s_input_state InputState(GLFWwindow* const glfw_window);

void GLFWKeyCallback(GLFWwindow* const window, const int key, const int scancode, const int action, const int mods);
void GLFWMouseButtonCallback(GLFWwindow* const window, const int button, const int action, const int mods);
void GLFWScrollCallback(GLFWwindow* const window, const double offs_x, const double offs_y);
void GLFWCharCallback(GLFWwindow* const window, const unsigned int codepoint);

static inline bool IsKeyDown(const s_input_context& input_context, const e_key_code kc) {
    const t_key_bits key_mask = static_cast<t_key_bits>(1) << kc;
    return (input_context.state.keys_down & key_mask) != 0;
}

static inline bool IsKeyPressed(const s_input_context& input_context, const e_key_code kc) {
    const t_key_bits key_mask = static_cast<t_key_bits>(1) << kc;
    return (input_context.events.keys_pressed & key_mask) != 0;
}

static inline bool IsKeyReleased(const s_input_context& input_context, const e_key_code kc) {
    const t_key_bits key_mask = static_cast<t_key_bits>(1) << kc;
    return (input_context.events.keys_released & key_mask) != 0;
}

static inline bool IsMouseButtonDown(const s_input_context& input_context, const e_mouse_button_code mbc) {
    const t_mouse_button_bits mb_mask = static_cast<t_mouse_button_bits>(1) << mbc;
    return (input_context.state.mouse_buttons_down & mb_mask) != 0;
}

static inline bool IsMouseButtonPressed(const s_input_context& input_context, const e_mouse_button_code mbc) {
    const t_mouse_button_bits mb_mask = static_cast<t_mouse_button_bits>(1) << mbc;
    return (input_context.events.mouse_buttons_pressed & mb_mask) != 0;
}

static inline bool IsMouseButtonReleased(const s_input_context& input_context, const e_mouse_button_code mbc) {
    const t_mouse_button_bits mb_mask = static_cast<t_mouse_button_bits>(1) << mbc;
    return (input_context.events.mouse_buttons_released & mb_mask) != 0;
}
