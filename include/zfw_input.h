#ifndef ZFW_INPUT_H
#define ZFW_INPUT_H

#include <cu.h>
#include <GLFW/glfw3.h>
#include "zfw_math.h"

typedef enum {
    zfw_ek_key_code_space,
    zfw_ek_key_code_0,
    zfw_ek_key_code_1,
    zfw_ek_key_code_2,
    zfw_ek_key_code_3,
    zfw_ek_key_code_4,
    zfw_ek_key_code_5,
    zfw_ek_key_code_6,
    zfw_ek_key_code_7,
    zfw_ek_key_code_8,
    zfw_ek_key_code_9,
    zfw_ek_key_code_a,
    zfw_ek_key_code_b,
    zfw_ek_key_code_c,
    zfw_ek_key_code_d,
    zfw_ek_key_code_e,
    zfw_ek_key_code_f,
    zfw_ek_key_code_g,
    zfw_ek_key_code_h,
    zfw_ek_key_code_i,
    zfw_ek_key_code_j,
    zfw_ek_key_code_k,
    zfw_ek_key_code_l,
    zfw_ek_key_code_m,
    zfw_ek_key_code_n,
    zfw_ek_key_code_o,
    zfw_ek_key_code_p,
    zfw_ek_key_code_q,
    zfw_ek_key_code_r,
    zfw_ek_key_code_s,
    zfw_ek_key_code_t,
    zfw_ek_key_code_u,
    zfw_ek_key_code_v,
    zfw_ek_key_code_w,
    zfw_ek_key_code_x,
    zfw_ek_key_code_y,
    zfw_ek_key_code_z,
    zfw_ek_key_code_escape,
    zfw_ek_key_code_enter,
    zfw_ek_key_code_backspace,
    zfw_ek_key_code_tab,
    zfw_ek_key_code_right,
    zfw_ek_key_code_left,
    zfw_ek_key_code_down,
    zfw_ek_key_code_up,
    zfw_ek_key_code_f1,
    zfw_ek_key_code_f2,
    zfw_ek_key_code_f3,
    zfw_ek_key_code_f4,
    zfw_ek_key_code_f5,
    zfw_ek_key_code_f6,
    zfw_ek_key_code_f7,
    zfw_ek_key_code_f8,
    zfw_ek_key_code_f9,
    zfw_ek_key_code_f10,
    zfw_ek_key_code_f11,
    zfw_ek_key_code_f12,
    zfw_ek_key_code_left_shift,
    zfw_ek_key_code_left_control,
    zfw_ek_key_code_left_alt,
    zfw_ek_key_code_right_shift,
    zfw_ek_key_code_right_control,
    zfw_ek_key_code_right_alt,

    zfw_eks_key_code_cnt
} zfw_e_key_code;

typedef t_u64 zfw_t_keys_down_bits;

static_assert(zfw_eks_key_code_cnt < SIZE_IN_BITS(zfw_t_keys_down_bits), "Too many key codes!");

typedef enum {
    zfw_ek_mouse_button_code_left,
    zfw_ek_mouse_button_code_right,
    zfw_ek_mouse_button_code_middle,

    zfw_eks_mouse_button_code_cnt
} zfw_e_mouse_button_code;

typedef t_u8 zfw_t_mouse_buttons_down_bits;

static_assert(zfw_eks_mouse_button_code_cnt < SIZE_IN_BITS(zfw_t_mouse_buttons_down_bits), "Too many mouse button codes!");

typedef enum {
    zfw_ek_mouse_scroll_state_none,
    zfw_ek_mouse_scroll_state_down,
    zfw_ek_mouse_scroll_state_up
} zfw_e_mouse_scroll_state;

typedef struct {
    zfw_t_keys_down_bits keys_down;
    zfw_t_mouse_buttons_down_bits mouse_buttons_down;
    zfw_s_vec_2d mouse_pos;
    zfw_e_mouse_scroll_state mouse_scroll_state;
} zfw_s_input_state;

typedef char zfw_t_unicode_buf[32];

void ZFW_RefreshInputState(zfw_s_input_state* const state, GLFWwindow* const glfw_window, const zfw_e_mouse_scroll_state mouse_scroll_state);

static inline bool ZFW_IsKeyDown(const zfw_e_key_code kc, const zfw_s_input_state* const input_state) {
    assert(kc < zfw_eks_key_code_cnt);
    assert(input_state);

    return (input_state->keys_down & ((zfw_t_keys_down_bits)1 << kc)) != 0;
}

static inline bool ZFW_IsKeyPressed(const zfw_e_key_code kc, const zfw_s_input_state* const input_state, const zfw_s_input_state* const input_state_last) {
    assert(kc < zfw_eks_key_code_cnt);
    assert(input_state);
    assert(input_state_last);

    return ZFW_IsKeyDown(kc, input_state) && !ZFW_IsKeyDown(kc, input_state_last);
}

static inline bool ZFW_IsKeyReleased(const zfw_e_key_code kc, const zfw_s_input_state* const input_state, const zfw_s_input_state* const input_state_last) {
    assert(kc < zfw_eks_key_code_cnt);
    assert(input_state);
    assert(input_state_last);

    return !ZFW_IsKeyDown(kc, input_state) && ZFW_IsKeyDown(kc, input_state_last);
}

static inline bool ZFW_IsMouseButtonDown(const zfw_e_mouse_button_code mbc, const zfw_s_input_state* const input_state) {
    assert(mbc < zfw_eks_mouse_button_code_cnt);
    assert(input_state);

    return (input_state->mouse_buttons_down & ((zfw_t_mouse_buttons_down_bits)1 << mbc)) != 0;
}

static inline bool ZFW_IsMouseButtonPressed(const zfw_e_mouse_button_code mbc, const zfw_s_input_state* const input_state, const zfw_s_input_state* const input_state_last) {
    assert(mbc < zfw_eks_mouse_button_code_cnt);
    assert(input_state);
    assert(input_state_last);

    return ZFW_IsMouseButtonDown(mbc, input_state) && !ZFW_IsMouseButtonDown(mbc, input_state_last);
}

static inline bool ZFW_IsMouseButtonReleased(const zfw_e_mouse_button_code mbc, const zfw_s_input_state* const input_state, const zfw_s_input_state* const input_state_last) {
    assert(mbc < zfw_eks_mouse_button_code_cnt);
    assert(input_state);
    assert(input_state_last);

    return !ZFW_IsMouseButtonDown(mbc, input_state) && ZFW_IsMouseButtonDown(mbc, input_state_last);
}

#endif
