#include <zgl/zgl_input.h>

namespace zf {
    struct zf_input_gamepad {
        s_static_bit_vec<ecm_gamepad_button_code_cnt> buttons_down;
        s_static_array<F32, ecm_gamepad_axis_code_cnt> axes;
    };

    struct zf_input_gamepad_events {
        s_static_bit_vec<ecm_gamepad_button_code_cnt> buttons_pressed;
        s_static_bit_vec<ecm_gamepad_button_code_cnt> buttons_released;
    };

    struct t_input_state {
        s_static_bit_vec<ecm_key_code_cnt> keys_down;

        s_static_bit_vec<ecm_mouse_button_code_cnt> mouse_buttons_down;

        s_v2 cursor_pos;

        s_static_bit_vec<g_gamepad_limit> gamepads_connected;
        s_static_array<zf_input_gamepad, g_gamepad_limit> gamepads;
        s_static_array<F32, ecm_gamepad_axis_code_cnt> gamepad_axis_deadzones;

        struct {
            s_static_bit_vec<ecm_key_code_cnt> keys_pressed;
            s_static_bit_vec<ecm_key_code_cnt> keys_released;

            s_static_bit_vec<ecm_mouse_button_code_cnt> mouse_buttons_pressed;
            s_static_bit_vec<ecm_mouse_button_code_cnt> mouse_buttons_released;

            s_v2 scroll_offs;

            s_static_array<zf_input_gamepad_events, g_gamepad_limit> gamepads;
        } events;
    };

    t_input_state *input_create_state(s_arena *const arena) {
        return mem_push_item_zeroed<t_input_state>(arena);
    }

    void input_clear_events(t_input_state *const state) {
        ClearItem(&state->events, 0);
    }

    B8 input_get_key_is_down(const t_input_state *const state, const t_key_code code) {
        return IsBitSet(state->keys_down, code);
    }

    B8 input_get_key_is_pressed(const t_input_state *const state, const t_key_code code) {
        return IsBitSet(state->events.keys_pressed, code);
    }

    B8 input_get_key_is_released(const t_input_state *const state, const t_key_code code) {
        return IsBitSet(state->events.keys_released, code);
    }

    void input_update_key_state(t_input_state *const state, const t_key_code code, const B8 is_down) {
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

    B8 input_get_mouse_button_is_down(const t_input_state *const state, const t_mouse_button_code btn_code) {
        return IsBitSet(state->mouse_buttons_down, static_cast<I32>(btn_code));
    }

    B8 input_get_mouse_button_is_pressed(const t_input_state *const state, const t_mouse_button_code btn_code) {
        return IsBitSet(state->events.mouse_buttons_pressed, static_cast<I32>(btn_code));
    }

    B8 input_get_mouse_button_is_released(const t_input_state *const state, const t_mouse_button_code btn_code) {
        return IsBitSet(state->events.mouse_buttons_released, static_cast<I32>(btn_code));
    }

    void input_update_mouse_button_state(t_input_state *const state, const t_mouse_button_code btn_code, const B8 is_down) {
        if (is_down) {
            if (!IsBitSet(state->mouse_buttons_down, static_cast<I32>(btn_code))) {
                SetBit(state->mouse_buttons_down, static_cast<I32>(btn_code));
                SetBit(state->events.mouse_buttons_pressed, static_cast<I32>(btn_code));
            }
        } else {
            if (IsBitSet(state->mouse_buttons_down, static_cast<I32>(btn_code))) {
                UnsetBit(state->mouse_buttons_down, static_cast<I32>(btn_code));
                SetBit(state->events.mouse_buttons_released, static_cast<I32>(btn_code));
            }
        }
    }

    s_v2 input_get_cursor_pos(const t_input_state *const state) {
        return state->cursor_pos;
    }

    void input_update_cursor_pos(t_input_state *const state, const s_v2 val) {
        state->cursor_pos = val;
    }

    s_v2 input_get_scroll_offs(const t_input_state *const state) {
        return state->events.scroll_offs;
    }

    void input_update_scroll_offs(t_input_state *const state, const s_v2 offs_to_apply) {
        state->events.scroll_offs += offs_to_apply;
    }

    B8 input_get_gamepad_is_connected(const t_input_state *const state, const I32 index) {
        ZF_ASSERT(index >= 0 && index < g_gamepad_limit);
        return IsBitSet(state->gamepads_connected, index);
    }

    B8 input_get_gamepad_button_is_down(const t_input_state *const state, const I32 gamepad_index, const t_gamepad_button_code btn_code) {
        ZF_ASSERT(input_get_gamepad_is_connected(state, gamepad_index));
        return IsBitSet(state->gamepads[gamepad_index].buttons_down, btn_code);
    }

    B8 input_get_gamepad_button_is_pressed(const t_input_state *const state, const I32 gamepad_index, const t_gamepad_button_code btn_code) {
        ZF_ASSERT(input_get_gamepad_is_connected(state, gamepad_index));
        return IsBitSet(state->events.gamepads[gamepad_index].buttons_pressed, btn_code);
    }

    B8 input_get_gamepad_button_is_released(const t_input_state *const state, const I32 gamepad_index, const t_gamepad_button_code btn_code) {
        ZF_ASSERT(input_get_gamepad_is_connected(state, gamepad_index));
        return IsBitSet(state->events.gamepads[gamepad_index].buttons_released, btn_code);
    }

    inline F32 input_get_gamepad_axis_value_raw(const t_input_state *const state, const I32 gamepad_index, const t_gamepad_axis_code axis_code) {
        ZF_ASSERT(input_get_gamepad_is_connected(state, gamepad_index));
        return state->gamepads[gamepad_index].axes[axis_code];
    }

    F32 input_calc_gamepad_axis_value_with_deadzone(const t_input_state *const state, const I32 gamepad_index, const t_gamepad_axis_code axis_code) {
        ZF_ASSERT(input_get_gamepad_is_connected(state, gamepad_index));

        const F32 raw = state->gamepads[gamepad_index].axes[axis_code];
        const F32 raw_abs = get_abs(raw);

        const F32 dz = state->gamepad_axis_deadzones[axis_code];

        if (raw_abs <= dz) {
            return 0.0f;
        }

        return static_cast<F32>(get_sign(raw)) * ((raw_abs - dz) / (1.0f - dz));
    }

    void input_update_gamepad_state(t_input_state *const state, const I32 gamepad_index, const B8 connected, const s_static_bit_vec<ecm_gamepad_button_code_cnt> &btns_down, const s_static_array<F32, ecm_gamepad_axis_code_cnt> &axes) {
        if (!connected) {
            ZF_ASSERT(AreAllBitsUnset(btns_down) && DoAllEqual(AsNonstatic(axes), 0.0f));
            return;
        }

        if (!IsBitSet(state->gamepads_connected, gamepad_index)) {
            SetBit(state->gamepads_connected, gamepad_index);
            state->gamepads[gamepad_index] = {};
            state->events.gamepads[gamepad_index] = {};
        }

        for (I32 i = 0; i < ecm_gamepad_button_code_cnt; i++) {
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
