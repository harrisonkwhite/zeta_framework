#pragma once

#include <zgl.h>

namespace zf {
    struct s_input_state {
        s_static_bit_vec<eks_key_code_cnt> keys_down;
        s_static_bit_vec<eks_mouse_button_code_cnt> mouse_buttons_down;

        s_v2<t_f32> mouse_pos;

        struct {
            s_static_bit_vec<eks_key_code_cnt> keys_pressed;
            s_static_bit_vec<eks_key_code_cnt> keys_released;

            s_static_bit_vec<eks_mouse_button_code_cnt> mouse_buttons_pressed;
            s_static_bit_vec<eks_mouse_button_code_cnt> mouse_buttons_released;
        } events;
    };

    enum class e_key_action {
        invalid,
        press,
        release
    };

    enum class e_mouse_button_action {
        invalid,
        press,
        release
    };

    void ProcKeyAction(s_input_state& is, const e_key_code code, const e_key_action act);
    void ProcMouseButtonAction(s_input_state& is, const e_mouse_button_code code, const e_mouse_button_action act);
}
