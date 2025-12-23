#pragma once

#include <zcl.h>

namespace zf {
    enum e_key_code : t_i32 {
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

    enum e_mouse_button_code : t_i32 {
        eks_mouse_button_code_none = -1,

        ek_mouse_button_code_left,
        ek_mouse_button_code_right,
        ek_mouse_button_code_middle,

        eks_mouse_button_code_cnt
    };

    // +Y: Scroll up / away from you
    // -Y: Scroll down / towards you
    // +X: Scroll right
    // +X: Scroll left
    s_v2 ScrollOffset();

    enum e_gamepad_button_code : t_i32 {
        eks_gamepad_button_code_none = -1,

        ek_gamepad_button_code_a,
        ek_gamepad_button_code_b,
        ek_gamepad_button_code_x,
        ek_gamepad_button_code_y,
        ek_gamepad_button_code_left_bumper,
        ek_gamepad_button_code_right_bumper,
        ek_gamepad_button_code_back,
        ek_gamepad_button_code_start,
        ek_gamepad_button_code_guide,
        ek_gamepad_button_code_left_thumb,
        ek_gamepad_button_code_right_thumb,
        ek_gamepad_button_code_dpad_up,
        ek_gamepad_button_code_dpad_right,
        ek_gamepad_button_code_dpad_down,
        ek_gamepad_button_code_dpad_left,

        eks_gamepad_button_code_cnt
    };

    enum e_gamepad_axis_code : t_i32 {
        eks_gamepad_axis_code_none = -1,

        ek_gamepad_axis_code_left_x,
        ek_gamepad_axis_code_left_y,
        ek_gamepad_axis_code_right_x,
        ek_gamepad_axis_code_right_y,
        ek_gamepad_axis_code_left_trigger,
        ek_gamepad_axis_code_right_trigger,

        eks_gamepad_axis_code_cnt
    };

    struct s_input_state {
        s_static_bit_vec<eks_key_code_cnt> keys_down;
        s_static_bit_vec<eks_mouse_button_code_cnt> mouse_buttons_down;

        s_v2 cursor_pos;

        struct {
            s_static_bit_vec<eks_key_code_cnt> keys_pressed;
            s_static_bit_vec<eks_key_code_cnt> keys_released;

            s_static_bit_vec<eks_mouse_button_code_cnt> mouse_buttons_pressed;
            s_static_bit_vec<eks_mouse_button_code_cnt> mouse_buttons_released;

            s_v2 scroll;
        } events;
    };

    enum e_key_event_type {
        ek_key_event_type_press,
        ek_key_event_type_release,
        ek_key_event_type_repeat
    };

    void RegKeyEvent(s_input_state &is, const e_key_code kc, const e_key_event_type type);

    enum e_mouse_button_event_type {
        ek_mouse_button_event_type_press,
        ek_mouse_button_event_type_release
    };

    void RegMouseButtonEvent(s_input_state &is, const e_mouse_button_code mbc, const e_mouse_button_event_type type);

    void RegScrollEvent(s_input_state &is, const s_v2 offs);

    inline t_b8 IsKeyDown(const s_input_state &is, const e_key_code kc) {
        return IsBitSet(is.keys_down, kc);
    }

    inline t_b8 IsKeyPressed(const s_input_state &is, const e_key_code kc) {
        return IsBitSet(is.events.keys_pressed, kc);
    }

    inline t_b8 IsKeyReleased(const s_input_state &is, const e_key_code kc) {
        return IsBitSet(is.events.keys_released, kc);
    }

    inline t_b8 IsMouseButtonDown(const s_input_state &is, const e_mouse_button_code mbc) {
        return IsBitSet(is.mouse_buttons_down, mbc);
    }

    inline t_b8 IsMouseButtonPressed(const s_input_state &is, const e_mouse_button_code mbc) {
        return IsBitSet(is.events.mouse_buttons_pressed, mbc);
    }

    inline t_b8 IsMouseButtonReleased(const s_input_state &is, const e_mouse_button_code mbc) {
        return IsBitSet(is.events.mouse_buttons_released, mbc);
    }

    inline s_v2 ScrollOffset(const s_input_state &is) {
        return is.events.scroll;
    }
}
