#include <zgl/zgl_input.h>

namespace zgl {
    struct t_gamepad {
        zcl::t_static_bitset<ekm_gamepad_button_code_cnt> buttons_down;
        zcl::t_static_array<zcl::t_f32, ekm_gamepad_axis_code_cnt> axes;
    };

    struct t_gamepad_events {
        zcl::t_static_bitset<ekm_gamepad_button_code_cnt> buttons_pressed;
        zcl::t_static_bitset<ekm_gamepad_button_code_cnt> buttons_released;
    };

    struct t_input_state {
        zcl::t_static_bitset<ekm_key_code_cnt> keys_down;

        zcl::t_static_bitset<ekm_mouse_button_code_cnt> mouse_buttons_down;

        zcl::t_v2 cursor_pos;

        zcl::t_static_bitset<k_gamepad_limit> gamepads_connected;
        zcl::t_static_array<t_gamepad, k_gamepad_limit> gamepads;
        zcl::t_static_array<zcl::t_f32, ekm_gamepad_axis_code_cnt> gamepad_axis_deadzones;

        struct {
            zcl::t_static_bitset<ekm_key_code_cnt> keys_pressed;
            zcl::t_static_bitset<ekm_key_code_cnt> keys_released;

            zcl::t_static_bitset<ekm_mouse_button_code_cnt> mouse_buttons_pressed;
            zcl::t_static_bitset<ekm_mouse_button_code_cnt> mouse_buttons_released;

            zcl::t_v2 scroll_offs;

            zcl::t_static_array<t_gamepad_events, k_gamepad_limit> gamepads;

            zcl::t_static_list<zcl::t_code_point, 32> code_pts;
        } events;
    };

    t_input_state *detail::InputCreateState(zcl::t_arena *const arena) {
        return zcl::ArenaPushItem<t_input_state>(arena);
    }

    void detail::InputClearEvents(t_input_state *const state) {
        zcl::ZeroClearItem(&state->events);
    }

    zcl::t_b8 KeyCheckDown(const t_input_state *const input_state, const t_key_code code) {
        return zcl::BitsetCheckSet(input_state->keys_down, code);
    }

    zcl::t_b8 KeyCheckPressed(const t_input_state *const input_state, const t_key_code code) {
        return zcl::BitsetCheckSet(input_state->events.keys_pressed, code);
    }

    zcl::t_b8 KeyCheckReleased(const t_input_state *const input_state, const t_key_code code) {
        return zcl::BitsetCheckSet(input_state->events.keys_released, code);
    }

    void detail::KeyUpdateState(t_input_state *const input_state, const t_key_code code, const zcl::t_b8 is_down) {
        if (is_down) {
            if (!zcl::BitsetCheckSet(input_state->keys_down, code)) {
                zcl::BitsetSet(input_state->keys_down, code);
                zcl::BitsetSet(input_state->events.keys_pressed, code);
            }
        } else {
            if (zcl::BitsetCheckSet(input_state->keys_down, code)) {
                zcl::BitsetUnset(input_state->keys_down, code);
                zcl::BitsetSet(input_state->events.keys_released, code);
            }
        }
    }

    zcl::t_b8 MouseButtonCheckDown(const t_input_state *const input_state, const t_mouse_button_code btn_code) {
        return zcl::BitsetCheckSet(input_state->mouse_buttons_down, static_cast<zcl::t_i32>(btn_code));
    }

    zcl::t_b8 MouseButtonCheckPressed(const t_input_state *const input_state, const t_mouse_button_code btn_code) {
        return zcl::BitsetCheckSet(input_state->events.mouse_buttons_pressed, static_cast<zcl::t_i32>(btn_code));
    }

    zcl::t_b8 MouseButtonCheckReleased(const t_input_state *const input_state, const t_mouse_button_code btn_code) {
        return zcl::BitsetCheckSet(input_state->events.mouse_buttons_released, static_cast<zcl::t_i32>(btn_code));
    }

    void detail::MouseButtonUpdateState(t_input_state *const input_state, const t_mouse_button_code btn_code, const zcl::t_b8 is_down) {
        if (is_down) {
            if (!zcl::BitsetCheckSet(input_state->mouse_buttons_down, static_cast<zcl::t_i32>(btn_code))) {
                zcl::BitsetSet(input_state->mouse_buttons_down, static_cast<zcl::t_i32>(btn_code));
                zcl::BitsetSet(input_state->events.mouse_buttons_pressed, static_cast<zcl::t_i32>(btn_code));
            }
        } else {
            if (zcl::BitsetCheckSet(input_state->mouse_buttons_down, static_cast<zcl::t_i32>(btn_code))) {
                zcl::BitsetUnset(input_state->mouse_buttons_down, static_cast<zcl::t_i32>(btn_code));
                zcl::BitsetSet(input_state->events.mouse_buttons_released, static_cast<zcl::t_i32>(btn_code));
            }
        }
    }

    zcl::t_v2 CursorGetPos(const t_input_state *const input_state) {
        return input_state->cursor_pos;
    }

    void detail::CursorUpdateState(t_input_state *const input_state, const zcl::t_v2 pos) {
        input_state->cursor_pos = pos;
    }

    zcl::t_v2 ScrollGetOffset(const t_input_state *const input_state) {
        return input_state->events.scroll_offs;
    }

    void detail::ScrollUpdateState(t_input_state *const input_state, const zcl::t_v2 offs_to_apply) {
        input_state->events.scroll_offs += offs_to_apply;
    }

    zcl::t_b8 GamepadCheckConnected(const t_input_state *const input_state, const zcl::t_i32 index) {
        ZCL_ASSERT(index >= 0 && index < k_gamepad_limit);
        return zcl::BitsetCheckSet(input_state->gamepads_connected, index);
    }

    zcl::t_b8 GamepadCheckButtonDown(const t_input_state *const input_state, const zcl::t_i32 gamepad_index, const t_gamepad_button_code btn_code) {
        ZCL_ASSERT(GamepadCheckConnected(input_state, gamepad_index));
        return zcl::BitsetCheckSet(input_state->gamepads[gamepad_index].buttons_down, btn_code);
    }

    zcl::t_b8 GamepadCheckButtonPressed(const t_input_state *const input_state, const zcl::t_i32 gamepad_index, const t_gamepad_button_code btn_code) {
        ZCL_ASSERT(GamepadCheckConnected(input_state, gamepad_index));
        return zcl::BitsetCheckSet(input_state->events.gamepads[gamepad_index].buttons_pressed, btn_code);
    }

    zcl::t_b8 GamepadCheckButtonReleased(const t_input_state *const input_state, const zcl::t_i32 gamepad_index, const t_gamepad_button_code btn_code) {
        ZCL_ASSERT(GamepadCheckConnected(input_state, gamepad_index));
        return zcl::BitsetCheckSet(input_state->events.gamepads[gamepad_index].buttons_released, btn_code);
    }

    zcl::t_f32 GamepadGetAxisValueRaw(const t_input_state *const input_state, const zcl::t_i32 gamepad_index, const t_gamepad_axis_code axis_code) {
        ZCL_ASSERT(GamepadCheckConnected(input_state, gamepad_index));
        return input_state->gamepads[gamepad_index].axes[axis_code];
    }

    zcl::t_f32 GamepadGetAxisValueWithDeadzone(const t_input_state *const input_state, const zcl::t_i32 gamepad_index, const t_gamepad_axis_code axis_code) {
        ZCL_ASSERT(GamepadCheckConnected(input_state, gamepad_index));

        const zcl::t_f32 raw = input_state->gamepads[gamepad_index].axes[axis_code];
        const zcl::t_f32 raw_abs = zcl::CalcAbs(raw);

        const zcl::t_f32 dz = input_state->gamepad_axis_deadzones[axis_code];

        if (raw_abs <= dz) {
            return 0.0f;
        }

        return static_cast<zcl::t_f32>(zcl::CalcSign(raw)) * ((raw_abs - dz) / (1.0f - dz));
    }

    void detail::GamepadUpdateState(t_input_state *const input_state, const zcl::t_i32 gamepad_index, const zcl::t_b8 connected, const zcl::t_static_bitset<ekm_gamepad_button_code_cnt> &btns_down, const zcl::t_static_array<zcl::t_f32, ekm_gamepad_axis_code_cnt> &axes) {
        if (!connected) {
            ZCL_ASSERT(zcl::BitsetCheckAllUnset(btns_down) && zcl::CheckAllEqual(zcl::ArrayToNonstatic(&axes), 0.0f));
            return;
        }

        if (!zcl::BitsetCheckSet(input_state->gamepads_connected, gamepad_index)) {
            zcl::BitsetSet(input_state->gamepads_connected, gamepad_index);
            input_state->gamepads[gamepad_index] = {};
            input_state->events.gamepads[gamepad_index] = {};
        }

        for (zcl::t_i32 i = 0; i < ekm_gamepad_button_code_cnt; i++) {
            if (zcl::BitsetCheckSet(btns_down, i)) {
                if (!zcl::BitsetCheckSet(input_state->gamepads[i].buttons_down, i)) {
                    zcl::BitsetSet(input_state->gamepads[i].buttons_down, i);
                    zcl::BitsetSet(input_state->events.gamepads[i].buttons_pressed, i);
                }
            } else {
                if (zcl::BitsetCheckSet(input_state->gamepads[i].buttons_down, i)) {
                    zcl::BitsetUnset(input_state->gamepads[i].buttons_down, i);
                    zcl::BitsetSet(input_state->events.gamepads[i].buttons_released, i);
                }
            }
        }

        zcl::ArrayCopy(zcl::ArrayToNonstatic(&axes), zcl::ArrayToNonstatic(&input_state->gamepads[gamepad_index].axes));
    }

    zcl::t_array_rdonly<zcl::t_code_point> TextGetCodePoints(const t_input_state *const input_state) {
        return zcl::ListToArray(&input_state->events.code_pts);
    }

    zcl::t_b8 detail::TextSubmitCodePoints(t_input_state *const input_state, const zcl::t_code_point cp) {
        if (input_state->events.code_pts.len < zcl::ListGetCap(&input_state->events.code_pts)) {
            zcl::ListAppend(&input_state->events.code_pts, cp);
            return true;
        }

        return false;
    }
}
