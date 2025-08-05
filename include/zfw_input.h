#ifndef ZFW_INPUT_H
#define ZFW_INPUT_H

#include <cu.h>
#include <GLFW/glfw3.h>
#include "zfw_math.h"

typedef enum {
    zfw_eks_key_code_none = -1,

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

typedef uint64_t zfw_t_key_bits;

static_assert(zfw_eks_key_code_cnt < SIZE_IN_BITS(zfw_t_key_bits), "Too many key codes!");

static inline bool ZFW_IsKeyBitsValid(const zfw_t_key_bits key_bits) {
    // Check if any bit is set outside the valid range.
    const zfw_t_key_bits mask = ((zfw_t_key_bits)1 << zfw_eks_key_code_cnt) - 1;
    return (key_bits & ~mask) == 0;
}

typedef enum {
    zfw_eks_mouse_button_code_none = -1,

    zfw_ek_mouse_button_code_left,
    zfw_ek_mouse_button_code_right,
    zfw_ek_mouse_button_code_middle,

    zfw_eks_mouse_button_code_cnt
} zfw_e_mouse_button_code;

typedef uint8_t zfw_t_mouse_button_bits;

static_assert(zfw_eks_mouse_button_code_cnt < SIZE_IN_BITS(zfw_t_mouse_button_bits), "Too many mouse button codes!");

static inline bool ZFW_IsMouseButtonBitsValid(const zfw_t_mouse_button_bits mb_bits) {
    // Check if any bit is set outside the valid range.
    const zfw_t_mouse_button_bits mask = ((zfw_t_mouse_button_bits)1 << zfw_eks_mouse_button_code_cnt) - 1;
    return (mb_bits & ~mask) == 0;
}

typedef enum {
    zfw_ek_mouse_scroll_state_none,
    zfw_ek_mouse_scroll_state_down,
    zfw_ek_mouse_scroll_state_up
} zfw_e_mouse_scroll_state;

typedef char zfw_t_unicode_buf[32];

typedef struct {
    zfw_t_key_bits keys_down;
    zfw_t_mouse_button_bits mouse_buttons_down;

    s_v2 mouse_pos;
} zfw_s_input_state;

static inline void ZFW_AssertInputStateValidity(const zfw_s_input_state* const state) {
    assert(state);

    assert(ZFW_IsKeyBitsValid(state->keys_down));
    assert(ZFW_IsMouseButtonBitsValid(state->mouse_buttons_down));
}

typedef struct {
    zfw_t_key_bits keys_pressed;
    zfw_t_key_bits keys_released;

    zfw_t_mouse_button_bits mouse_buttons_pressed;
    zfw_t_mouse_button_bits mouse_buttons_released;

    zfw_e_mouse_scroll_state mouse_scroll_state;

    zfw_t_unicode_buf unicode_buf;
} zfw_s_input_events;

static inline void ZFW_AssertInputEventsValidity(const zfw_s_input_events* const events) {
    assert(events);

    // Check key bits.
    const zfw_t_key_bits keys_mask = ((zfw_t_key_bits)1 << zfw_eks_key_code_cnt) - 1;
    assert(ZFW_IsKeyBitsValid(events->keys_pressed));
    assert(ZFW_IsKeyBitsValid(events->keys_released));

    // Check mouse button bits.
    const zfw_t_mouse_button_bits mouse_buttons_mask = ((zfw_t_mouse_button_bits)1 << zfw_eks_mouse_button_code_cnt) - 1;
    assert(ZFW_IsMouseButtonBitsValid(events->mouse_buttons_pressed));
    assert(ZFW_IsMouseButtonBitsValid(events->mouse_buttons_released));

    // Check the unicode buffer.
    for (size_t i = 0; i < sizeof(zfw_t_unicode_buf); i++) {
        if (!events->unicode_buf[i]) {
            for (size_t j = i + 1; j < sizeof(zfw_t_unicode_buf); j++) {
                assert(!events->unicode_buf[j] && "Everything after the first '\0' in the unicode buffer should also be '\0'!");
            }

            break;
        }
    }
}

typedef struct {
    const zfw_s_input_state* state;
    const zfw_s_input_events* events;
} zfw_s_input_context;

static inline void ZFW_AssertInputContextValidity(const zfw_s_input_context* const input_context) {
    ZFW_AssertInputStateValidity(input_context->state);
    ZFW_AssertInputEventsValidity(input_context->events);
}

zfw_s_input_state ZFW_InputState(GLFWwindow* const glfw_window);

void ZFW_GLFWKeyCallback(GLFWwindow* const window, const int key, const int scancode, const int action, const int mods);
void ZFW_GLFWMouseButtonCallback(GLFWwindow* const window, const int button, const int action, const int mods);
void ZFW_GLFWScrollCallback(GLFWwindow* const window, const double offs_x, const double offs_y);
void ZFW_GLFWCharCallback(GLFWwindow* const window, const unsigned int codepoint);

static inline bool ZFW_IsKeyDown(const zfw_s_input_context* const input_context, const zfw_e_key_code kc) {
    ZFW_AssertInputContextValidity(input_context);
    assert(kc >= 0 && kc < zfw_eks_key_code_cnt);

    const zfw_t_key_bits key_mask = (zfw_t_key_bits)1 << kc;
    return (input_context->state->keys_down & key_mask) != 0;
}

static inline bool ZFW_IsKeyPressed(const zfw_s_input_context* const input_context, const zfw_e_key_code kc) {
    ZFW_AssertInputContextValidity(input_context);
    assert(kc >= 0 && kc < zfw_eks_key_code_cnt);

    const zfw_t_key_bits key_mask = (zfw_t_key_bits)1 << kc;
    return (input_context->events->keys_pressed & key_mask) != 0;
}

static inline bool ZFW_IsKeyReleased(const zfw_s_input_context* const input_context, const zfw_e_key_code kc) {
    ZFW_AssertInputContextValidity(input_context);
    assert(kc >= 0 && kc < zfw_eks_key_code_cnt);

    const zfw_t_key_bits key_mask = (zfw_t_key_bits)1 << kc;
    return (input_context->events->keys_released & key_mask) != 0;
}

static inline bool ZFW_IsMouseButtonDown(const zfw_s_input_context* const input_context, const zfw_e_mouse_button_code mbc) {
    ZFW_AssertInputContextValidity(input_context);
    assert(mbc >= 0 && mbc < zfw_eks_mouse_button_code_cnt);

    const zfw_t_mouse_button_bits mb_mask = (zfw_t_mouse_button_bits)1 << mbc;
    return (input_context->state->mouse_buttons_down & mb_mask) != 0;
}

static inline bool ZFW_IsMouseButtonPressed(const zfw_s_input_context* const input_context, const zfw_e_mouse_button_code mbc) {
    ZFW_AssertInputContextValidity(input_context);
    assert(mbc >= 0 && mbc < zfw_eks_mouse_button_code_cnt);

    const zfw_t_mouse_button_bits mb_mask = (zfw_t_mouse_button_bits)1 << mbc;
    return (input_context->events->mouse_buttons_pressed & mb_mask) != 0;
}

static inline bool ZFW_IsMouseButtonReleased(const zfw_s_input_context* const input_context, const zfw_e_mouse_button_code mbc) {
    ZFW_AssertInputContextValidity(input_context);
    assert(mbc >= 0 && mbc < zfw_eks_mouse_button_code_cnt);

    const zfw_t_mouse_button_bits mb_mask = (zfw_t_mouse_button_bits)1 << mbc;
    return (input_context->events->mouse_buttons_released & mb_mask) != 0;
}

#endif
