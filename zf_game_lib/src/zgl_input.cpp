#include <zgl/zgl_input.h>

namespace zf {
    struct zf_input_gamepad {
        s_static_bit_vec<eks_gamepad_button_code_cnt> buttons_down;
        s_static_array<t_f32, eks_gamepad_axis_code_cnt> axes;
    };

    struct zf_input_gamepad_events {
        s_static_bit_vec<eks_gamepad_button_code_cnt> buttons_pressed;
        s_static_bit_vec<eks_gamepad_button_code_cnt> buttons_released;
    };

    struct zf_input_state {
        s_static_bit_vec<zf_input_key_code_cnt> keys_down;

        s_static_bit_vec<eks_mouse_button_code_cnt> mouse_buttons_down;

        s_v2 cursor_pos;

        s_static_bit_vec<g_gamepad_limit> gamepads_connected;
        s_static_array<zf_input_gamepad, g_gamepad_limit> gamepads;
        s_static_array<t_f32, eks_gamepad_axis_code_cnt> gamepad_axis_deadzones;

        struct {
            s_static_bit_vec<zf_input_key_code_cnt> keys_pressed;
            s_static_bit_vec<zf_input_key_code_cnt> keys_released;

            s_static_bit_vec<eks_mouse_button_code_cnt> mouse_buttons_pressed;
            s_static_bit_vec<eks_mouse_button_code_cnt> mouse_buttons_released;

            s_v2 scroll_offs;

            s_static_array<zf_input_gamepad_events, g_gamepad_limit> gamepads;
        } events;
    };

    zf_input_state *zf_input_create_state(s_arena *const arena) {
        return mem_push_item_zeroed<zf_input_state>(arena);
    }

    void zf_input_clear_events(zf_input_state *const state) {
        ClearItem(&state->events, 0);
    }

    B8 zf_input_get_key_is_down(const zf_input_state *const state, const zf_input_key_code code) {
        return IsBitSet(state->keys_down, code);
    }

    B8 zf_input_get_key_is_pressed(const zf_input_state *const state, const zf_input_key_code code) {
        return IsBitSet(state->events.keys_pressed, code);
    }

    B8 zf_input_get_key_is_released(const zf_input_state *const state, const zf_input_key_code code) {
        return IsBitSet(state->events.keys_released, code);
    }

    void zf_input_update_key_state(zf_input_state *const state, const zf_input_key_code code, const B8 is_down) {
        if (is_down) {
            if (!IsBitSet(state->keys_down, code)) {
                SetBit(state->keys_down, code);
                SetBit(state->events.keys_pressed, code);
            }
        } else {
            if (IsBitSet(state->keys_down, code)) {
                UnsetBit(state->keys_down, code);
                SetBit(state->events.keys_released, code);
            }
        }
    }

    B8 IsMouseButtonDown(const zf_input_state *const state, const e_mouse_button_code btn_code) {
        return IsBitSet(state->mouse_buttons_down, btn_code);
    }

    B8 IsMouseButtonPressed(const zf_input_state *const state, const e_mouse_button_code btn_code) {
        return IsBitSet(state->events.mouse_buttons_pressed, btn_code);
    }

    B8 IsMouseButtonReleased(const zf_input_state *const state, const e_mouse_button_code btn_code) {
        return IsBitSet(state->events.mouse_buttons_released, btn_code);
    }

    void UpdateMouseButtonState(zf_input_state *const state, const e_mouse_button_code code, const B8 is_down) {
        if (is_down) {
            if (!IsBitSet(state->mouse_buttons_down, code)) {
                SetBit(state->mouse_buttons_down, code);
                SetBit(state->events.mouse_buttons_pressed, code);
            }
        } else {
            if (IsBitSet(state->mouse_buttons_down, code)) {
                UnsetBit(state->mouse_buttons_down, code);
                SetBit(state->events.mouse_buttons_released, code);
            }
        }
    }

    s_v2 CursorPos(const zf_input_state *const state) {
        return state->cursor_pos;
    }

    void UpdateCursorPos(zf_input_state *const state, const s_v2 val) {
        state->cursor_pos = val;
    }

    s_v2 zf_input_get_scroll_offset(const zf_input_state *const state) {
        return state->events.scroll_offs;
    }

    void zf_input_update_scroll_offset(zf_input_state *const state, const s_v2 offs_to_apply) {
        state->events.scroll_offs += offs_to_apply;
    }

    B8 IsGamepadConnected(const zf_input_state *const state, const t_i32 index) {
        ZF_ASSERT(index >= 0 && index < g_gamepad_limit);
        return IsBitSet(state->gamepads_connected, index);
    }

    B8 IsGamepadButtonDown(const zf_input_state *const state, const t_i32 gamepad_index, const e_gamepad_button_code btn_code) {
        ZF_ASSERT(IsGamepadConnected(state, gamepad_index));
        return IsBitSet(state->gamepads[gamepad_index].buttons_down, btn_code);
    }

    B8 IsGamepadButtonPressed(const zf_input_state *const state, const t_i32 gamepad_index, const e_gamepad_button_code btn_code) {
        ZF_ASSERT(IsGamepadConnected(state, gamepad_index));
        return IsBitSet(state->events.gamepads[gamepad_index].buttons_pressed, btn_code);
    }

    B8 IsGamepadButtonReleased(const zf_input_state *const state, const t_i32 gamepad_index, const e_gamepad_button_code btn_code) {
        ZF_ASSERT(IsGamepadConnected(state, gamepad_index));
        return IsBitSet(state->events.gamepads[gamepad_index].buttons_released, btn_code);
    }

    inline t_f32 zf_input_get_gamepad_axis_value_raw(const zf_input_state *const state, const t_i32 gamepad_index, const e_gamepad_axis_code axis_code) {
        ZF_ASSERT(IsGamepadConnected(state, gamepad_index));
        return state->gamepads[gamepad_index].axes[axis_code];
    }

    t_f32 zf_input_calc_gamepad_axis_value_with_deadzone(const zf_input_state *const state, const t_i32 gamepad_index, const e_gamepad_axis_code axis_code) {
        ZF_ASSERT(IsGamepadConnected(state, gamepad_index));

        const t_f32 raw = state->gamepads[gamepad_index].axes[axis_code];
        const t_f32 raw_abs = Abs(raw);

        const t_f32 dz = state->gamepad_axis_deadzones[axis_code];

        if (raw_abs <= dz) {
            return 0.0f;
        }

        return static_cast<t_f32>(Sign(raw)) * ((raw_abs - dz) / (1.0f - dz));
    }

    void UpdateGamepadState(zf_input_state *const state, const t_i32 gamepad_index, const B8 connected, const s_static_bit_vec<eks_gamepad_button_code_cnt> &btns_down, const s_static_array<t_f32, eks_gamepad_axis_code_cnt> &axes) {
        if (!connected) {
            ZF_ASSERT(AreAllBitsUnset(btns_down) && DoAllEqual(AsNonstatic(axes), 0.0f));
            return;
        }

        if (!IsBitSet(state->gamepads_connected, gamepad_index)) {
            SetBit(state->gamepads_connected, gamepad_index);
            state->gamepads[gamepad_index] = {};
            state->events.gamepads[gamepad_index] = {};
        }

        for (t_i32 i = 0; i < eks_gamepad_button_code_cnt; i++) {
            if (IsBitSet(btns_down, i)) {
                if (!IsBitSet(state->gamepads[i].buttons_down, i)) {
                    SetBit(state->gamepads[i].buttons_down, i);
                    SetBit(state->events.gamepads[i].buttons_pressed, i);
                }
            } else {
                if (IsBitSet(state->gamepads[i].buttons_down, i)) {
                    UnsetBit(state->gamepads[i].buttons_down, i);
                    SetBit(state->events.gamepads[i].buttons_released, i);
                }
            }
        }

        CopyAll(AsNonstatic(axes), AsNonstatic(state->gamepads[gamepad_index].axes));
    }
}
