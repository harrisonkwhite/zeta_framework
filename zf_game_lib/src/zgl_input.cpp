#include <zgl/zgl_input.h>

namespace zf {
    void UpdateKeyState(s_input_state *const input_state, const e_key_code code, const t_b8 is_down) {
        if (is_down) {
            if (!IsBitSet(input_state->keys_down, code)) {
                SetBit(input_state->keys_down, code);
                SetBit(input_state->events.keys_pressed, code);
            }
        } else {
            if (IsBitSet(input_state->keys_down, code)) {
                UnsetBit(input_state->keys_down, code);
                SetBit(input_state->events.keys_released, code);
            }
        }
    }

    void UpdateMouseButtonState(s_input_state *const input_state, const e_mouse_button_code code, const t_b8 is_down) {
        if (is_down) {
            if (!IsBitSet(input_state->mouse_buttons_down, code)) {
                SetBit(input_state->mouse_buttons_down, code);
                SetBit(input_state->events.mouse_buttons_pressed, code);
            }
        } else {
            if (IsBitSet(input_state->mouse_buttons_down, code)) {
                UnsetBit(input_state->mouse_buttons_down, code);
                SetBit(input_state->events.mouse_buttons_released, code);
            }
        }
    }

    void UpdateGamepadState(s_input_state *const input_state, const t_i32 gamepad_index, const t_b8 connected, const s_static_bit_vec<eks_gamepad_button_code_cnt> &btns_down, const s_static_array<t_f32, eks_gamepad_axis_code_cnt> &axes) {
        if (!connected) {
            ZF_ASSERT(AreAllBitsUnset(btns_down) && DoAllEqual(axes.ToNonstatic(), 0.0f));
            return;
        }

        if (!IsBitSet(input_state->gamepads_connected, gamepad_index)) {
            SetBit(input_state->gamepads_connected, gamepad_index);
            input_state->gamepads[gamepad_index] = {};
            input_state->events.gamepads[gamepad_index] = {};
        }

        for (t_i32 i = 0; i < eks_gamepad_button_code_cnt; i++) {
            if (IsBitSet(btns_down, i)) {
                if (!IsBitSet(input_state->gamepads[i].buttons_down, i)) {
                    SetBit(input_state->gamepads[i].buttons_down, i);
                    SetBit(input_state->events.gamepads[i].buttons_pressed, i);
                }
            } else {
                if (IsBitSet(input_state->gamepads[i].buttons_down, i)) {
                    UnsetBit(input_state->gamepads[i].buttons_down, i);
                    SetBit(input_state->events.gamepads[i].buttons_released, i);
                }
            }
        }

        Copy(input_state->gamepads[gamepad_index].axes.ToNonstatic(), axes.ToNonstatic());
    }
}
