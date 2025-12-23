#include <zgl/zgl_input.h>

namespace zf {
    void UpdateKeyState(s_input_state &input_state, const e_key_code code, const t_b8 is_down) {
        if (is_down) {
            if (!IsBitSet(input_state.keys_down, code)) {
                SetBit(input_state.keys_down, code);
                SetBit(input_state.events.keys_pressed, code);
            }
        } else {
            if (IsBitSet(input_state.keys_down, code)) {
                UnsetBit(input_state.keys_down, code);
                SetBit(input_state.events.keys_released, code);
            }
        }
    }

    void UpdateMouseButtonState(s_input_state &input_state, const e_mouse_button_code code, const t_b8 is_down) {
        if (is_down) {
            if (!IsBitSet(input_state.mouse_buttons_down, code)) {
                SetBit(input_state.mouse_buttons_down, code);
                SetBit(input_state.events.mouse_buttons_pressed, code);
            }
        } else {
            if (IsBitSet(input_state.mouse_buttons_down, code)) {
                UnsetBit(input_state.mouse_buttons_down, code);
                SetBit(input_state.events.mouse_buttons_released, code);
            }
        }
    }
}
