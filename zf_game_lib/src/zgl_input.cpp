#include <zgl/zgl_input.h>

namespace zf {
    struct s_gamepad {
        s_static_bit_vec<eks_gamepad_button_code_cnt> buttons_down;
        s_static_array<t_f32, eks_gamepad_axis_code_cnt> axes;
    };

    struct s_gamepad_events {
        s_static_bit_vec<eks_gamepad_button_code_cnt> buttons_pressed;
        s_static_bit_vec<eks_gamepad_button_code_cnt> buttons_released;
    };

    struct s_input_state {
        s_static_bit_vec<eks_key_code_cnt> keys_down;

        s_static_bit_vec<eks_mouse_button_code_cnt> mouse_buttons_down;

        s_v2 cursor_pos;

        s_static_bit_vec<g_gamepad_limit> gamepads_connected;
        s_static_array<s_gamepad, g_gamepad_limit> gamepads;
        s_static_array<t_f32, eks_gamepad_axis_code_cnt> gamepad_axis_deadzones;

        struct {
            s_static_bit_vec<eks_key_code_cnt> keys_pressed;
            s_static_bit_vec<eks_key_code_cnt> keys_released;

            s_static_bit_vec<eks_mouse_button_code_cnt> mouse_buttons_pressed;
            s_static_bit_vec<eks_mouse_button_code_cnt> mouse_buttons_released;

            s_v2 scroll_offs;

            s_static_array<s_gamepad_events, g_gamepad_limit> gamepads;
        } events;
    };

    s_input_state *detail::CreateInputState(s_arena *const arena) {
        return PushItemZeroed<s_input_state>(arena);
    }

    void detail::ClearInputEvents(s_input_state *const input_state) {
        ClearItem(&input_state->events, 0);
    }

    t_b8 KeyIsDown(const s_input_state *const input_state, const e_key_code code) {
        return IsBitSet(input_state->keys_down, code);
    }

    t_b8 KeyIsPressed(const s_input_state *const input_state, const e_key_code code) {
        return IsBitSet(input_state->events.keys_pressed, code);
    }

    t_b8 KeyIsReleased(const s_input_state *const input_state, const e_key_code code) {
        return IsBitSet(input_state->events.keys_released, code);
    }

    void detail::KeyUpdateState(s_input_state *const input_state, const e_key_code code, const t_b8 is_down) {
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

    t_b8 MouseButtonIsDown(const s_input_state *const input_state, const e_mouse_button_code btn_code) {
        return IsBitSet(input_state->mouse_buttons_down, btn_code);
    }

    t_b8 MouseButtonIsPressed(const s_input_state *const input_state, const e_mouse_button_code btn_code) {
        return IsBitSet(input_state->events.mouse_buttons_pressed, btn_code);
    }

    t_b8 MouseButtonIsReleased(const s_input_state *const input_state, const e_mouse_button_code btn_code) {
        return IsBitSet(input_state->events.mouse_buttons_released, btn_code);
    }

    void detail::MouseButtonUpdateState(s_input_state *const input_state, const e_mouse_button_code code, const t_b8 is_down) {
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

    s_v2 CursorPos(const s_input_state *const input_state) {
        return input_state->cursor_pos;
    }

    void detail::CursorPosUpdate(s_input_state *const input_state, const s_v2 val) {
        input_state->cursor_pos = val;
    }

    s_v2 ScrollOffset(const s_input_state *const input_state) {
        return input_state->events.scroll_offs;
    }

    void detail::ScrollOffsetUpdate(s_input_state *const input_state, const s_v2 offs_to_apply) {
        input_state->events.scroll_offs += offs_to_apply;
    }

    t_b8 GamepadIsConnected(const s_input_state *const input_state, const t_i32 index) {
        ZF_ASSERT(index >= 0 && index < g_gamepad_limit);
        return IsBitSet(input_state->gamepads_connected, index);
    }

    t_b8 GamepadIsButtonDown(const s_input_state *const input_state, const t_i32 gamepad_index, const e_gamepad_button_code btn_code) {
        ZF_ASSERT(GamepadIsConnected(input_state, gamepad_index));
        return IsBitSet(input_state->gamepads[gamepad_index].buttons_down, btn_code);
    }

    t_b8 GamepadIsButtonPressed(const s_input_state *const input_state, const t_i32 gamepad_index, const e_gamepad_button_code btn_code) {
        ZF_ASSERT(GamepadIsConnected(input_state, gamepad_index));
        return IsBitSet(input_state->events.gamepads[gamepad_index].buttons_pressed, btn_code);
    }

    t_b8 GamepadIsButtonReleased(const s_input_state *const input_state, const t_i32 gamepad_index, const e_gamepad_button_code btn_code) {
        ZF_ASSERT(GamepadIsConnected(input_state, gamepad_index));
        return IsBitSet(input_state->events.gamepads[gamepad_index].buttons_released, btn_code);
    }

    inline t_f32 GamepadAxisValueRaw(const s_input_state *const input_state, const t_i32 gamepad_index, const e_gamepad_axis_code axis_code) {
        ZF_ASSERT(GamepadIsConnected(input_state, gamepad_index));
        return input_state->gamepads[gamepad_index].axes[axis_code];
    }

    t_f32 GamepadAxisValueWithDeadzone(const s_input_state *const input_state, const t_i32 gamepad_index, const e_gamepad_axis_code axis_code) {
        ZF_ASSERT(GamepadIsConnected(input_state, gamepad_index));

        const t_f32 raw = input_state->gamepads[gamepad_index].axes[axis_code];
        const t_f32 raw_abs = Abs(raw);

        const t_f32 dz = input_state->gamepad_axis_deadzones[axis_code];

        if (raw_abs <= dz) {
            return 0.0f;
        }

        return static_cast<t_f32>(Sign(raw)) * ((raw_abs - dz) / (1.0f - dz));
    }

    void detail::GamepadUpdateState(s_input_state *const input_state, const t_i32 gamepad_index, const t_b8 connected, const s_static_bit_vec<eks_gamepad_button_code_cnt> &btns_down, const s_static_array<t_f32, eks_gamepad_axis_code_cnt> &axes) {
        if (!connected) {
            ZF_ASSERT(AreAllBitsUnset(btns_down) && DoAllEqual(AsNonstatic(axes), 0.0f));
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

        CopyAll(AsNonstatic(axes), AsNonstatic(input_state->gamepads[gamepad_index].axes));
    }
}
