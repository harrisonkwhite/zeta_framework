#include <zgl/zgl_input.h>

namespace zgl::input {
    struct t_gamepad {
        zcl::mem::t_static_bitset<ekm_gamepad_button_code_cnt> buttons_down;
        zcl::t_static_array<zcl::t_f32, ekm_gamepad_axis_code_cnt> axes;
    };

    struct t_gamepad_events {
        zcl::mem::t_static_bitset<ekm_gamepad_button_code_cnt> buttons_pressed;
        zcl::mem::t_static_bitset<ekm_gamepad_button_code_cnt> buttons_released;
    };

    struct t_state {
        zcl::mem::t_static_bitset<ekm_key_code_cnt> keys_down;

        zcl::mem::t_static_bitset<ekm_mouse_button_code_cnt> mouse_buttons_down;

        zcl::math::t_v2 cursor_pos;

        zcl::mem::t_static_bitset<k_gamepad_limit> gamepads_connected;
        zcl::t_static_array<t_gamepad, k_gamepad_limit> gamepads;
        zcl::t_static_array<zcl::t_f32, ekm_gamepad_axis_code_cnt> gamepad_axis_deadzones;

        struct {
            zcl::mem::t_static_bitset<ekm_key_code_cnt> keys_pressed;
            zcl::mem::t_static_bitset<ekm_key_code_cnt> keys_released;

            zcl::mem::t_static_bitset<ekm_mouse_button_code_cnt> mouse_buttons_pressed;
            zcl::mem::t_static_bitset<ekm_mouse_button_code_cnt> mouse_buttons_released;

            zcl::math::t_v2 scroll_offs;

            zcl::t_static_array<t_gamepad_events, k_gamepad_limit> gamepads;

            zcl::ds::t_static_list<zcl::strs::t_code_pt, 32> code_pts;
        } events;
    };

    t_state *create_state(zcl::mem::t_arena *const arena) {
        return zcl::mem::arena_push_item<t_state>(arena);
    }

    void clear_events(t_state *const state) {
        zcl::mem::zero_clear_item(&state->events);
    }

    zcl::t_b8 key_check_down(const t_state *const state, const t_key_code code) {
        return zcl::mem::bitset_check_set(state->keys_down, code);
    }

    zcl::t_b8 key_check_pressed(const t_state *const state, const t_key_code code) {
        return zcl::mem::bitset_check_set(state->events.keys_pressed, code);
    }

    zcl::t_b8 key_check_released(const t_state *const state, const t_key_code code) {
        return zcl::mem::bitset_check_set(state->events.keys_released, code);
    }

    void key_update_state(t_state *const state, const t_key_code code, const zcl::t_b8 is_down) {
        if (is_down) {
            if (!zcl::mem::bitset_check_set(state->keys_down, code)) {
                zcl::mem::bitset_set(state->keys_down, code);
                zcl::mem::bitset_set(state->events.keys_pressed, code);
            }
        } else {
            if (zcl::mem::bitset_check_set(state->keys_down, code)) {
                zcl::mem::bitset_unset(state->keys_down, code);
                zcl::mem::bitset_set(state->events.keys_released, code);
            }
        }
    }

    zcl::t_b8 mouse_button_check_down(const t_state *const state, const t_mouse_button_code btn_code) {
        return zcl::mem::bitset_check_set(state->mouse_buttons_down, static_cast<zcl::t_i32>(btn_code));
    }

    zcl::t_b8 mouse_button_check_pressed(const t_state *const state, const t_mouse_button_code btn_code) {
        return zcl::mem::bitset_check_set(state->events.mouse_buttons_pressed, static_cast<zcl::t_i32>(btn_code));
    }

    zcl::t_b8 mouse_button_check_released(const t_state *const state, const t_mouse_button_code btn_code) {
        return zcl::mem::bitset_check_set(state->events.mouse_buttons_released, static_cast<zcl::t_i32>(btn_code));
    }

    void mouse_button_update_state(t_state *const state, const t_mouse_button_code btn_code, const zcl::t_b8 is_down) {
        if (is_down) {
            if (!zcl::mem::bitset_check_set(state->mouse_buttons_down, static_cast<zcl::t_i32>(btn_code))) {
                zcl::mem::bitset_set(state->mouse_buttons_down, static_cast<zcl::t_i32>(btn_code));
                zcl::mem::bitset_set(state->events.mouse_buttons_pressed, static_cast<zcl::t_i32>(btn_code));
            }
        } else {
            if (zcl::mem::bitset_check_set(state->mouse_buttons_down, static_cast<zcl::t_i32>(btn_code))) {
                zcl::mem::bitset_unset(state->mouse_buttons_down, static_cast<zcl::t_i32>(btn_code));
                zcl::mem::bitset_set(state->events.mouse_buttons_released, static_cast<zcl::t_i32>(btn_code));
            }
        }
    }

    zcl::math::t_v2 cursor_get_pos(const t_state *const state) {
        return state->cursor_pos;
    }

    void cursor_update_state(t_state *const state, const zcl::math::t_v2 pos) {
        state->cursor_pos = pos;
    }

    zcl::math::t_v2 scroll_get_offset(const t_state *const state) {
        return state->events.scroll_offs;
    }

    void scroll_update_state(t_state *const state, const zcl::math::t_v2 offs_to_apply) {
        state->events.scroll_offs += offs_to_apply;
    }

    zcl::t_b8 gamepad_check_connected(const t_state *const state, const zcl::t_i32 index) {
        ZF_ASSERT(index >= 0 && index < k_gamepad_limit);
        return zcl::mem::bitset_check_set(state->gamepads_connected, index);
    }

    zcl::t_b8 gamepad_check_button_down(const t_state *const state, const zcl::t_i32 gamepad_index, const t_gamepad_button_code btn_code) {
        ZF_ASSERT(gamepad_check_connected(state, gamepad_index));
        return zcl::mem::bitset_check_set(state->gamepads[gamepad_index].buttons_down, btn_code);
    }

    zcl::t_b8 gamepad_check_button_pressed(const t_state *const state, const zcl::t_i32 gamepad_index, const t_gamepad_button_code btn_code) {
        ZF_ASSERT(gamepad_check_connected(state, gamepad_index));
        return zcl::mem::bitset_check_set(state->events.gamepads[gamepad_index].buttons_pressed, btn_code);
    }

    zcl::t_b8 gamepad_check_button_released(const t_state *const state, const zcl::t_i32 gamepad_index, const t_gamepad_button_code btn_code) {
        ZF_ASSERT(gamepad_check_connected(state, gamepad_index));
        return zcl::mem::bitset_check_set(state->events.gamepads[gamepad_index].buttons_released, btn_code);
    }

    zcl::t_f32 gamepad_get_axis_value_raw(const t_state *const state, const zcl::t_i32 gamepad_index, const t_gamepad_axis_code axis_code) {
        ZF_ASSERT(gamepad_check_connected(state, gamepad_index));
        return state->gamepads[gamepad_index].axes[axis_code];
    }

    zcl::t_f32 gamepad_get_axis_value_with_deadzone(const t_state *const state, const zcl::t_i32 gamepad_index, const t_gamepad_axis_code axis_code) {
        ZF_ASSERT(gamepad_check_connected(state, gamepad_index));

        const zcl::t_f32 raw = state->gamepads[gamepad_index].axes[axis_code];
        const zcl::t_f32 raw_abs = abs(raw);

        const zcl::t_f32 dz = state->gamepad_axis_deadzones[axis_code];

        if (raw_abs <= dz) {
            return 0.0f;
        }

        return static_cast<zcl::t_f32>(zcl::calc_sign(raw)) * ((raw_abs - dz) / (1.0f - dz));
    }

    void gamepad_update_state(t_state *const state, const zcl::t_i32 gamepad_index, const zcl::t_b8 connected, const zcl::mem::t_static_bitset<ekm_gamepad_button_code_cnt> &btns_down, const zcl::t_static_array<zcl::t_f32, ekm_gamepad_axis_code_cnt> &axes) {
        if (!connected) {
            ZF_ASSERT(zcl::mem::bitset_check_all_unset(btns_down) && zcl::array_check_all_equal(zcl::array_to_nonstatic(&axes), 0.0f));
            return;
        }

        if (!zcl::mem::bitset_check_set(state->gamepads_connected, gamepad_index)) {
            zcl::mem::bitset_set(state->gamepads_connected, gamepad_index);
            state->gamepads[gamepad_index] = {};
            state->events.gamepads[gamepad_index] = {};
        }

        for (zcl::t_i32 i = 0; i < ekm_gamepad_button_code_cnt; i++) {
            if (zcl::mem::bitset_check_set(btns_down, i)) {
                if (!zcl::mem::bitset_check_set(state->gamepads[i].buttons_down, i)) {
                    zcl::mem::bitset_set(state->gamepads[i].buttons_down, i);
                    zcl::mem::bitset_set(state->events.gamepads[i].buttons_pressed, i);
                }
            } else {
                if (zcl::mem::bitset_check_set(state->gamepads[i].buttons_down, i)) {
                    zcl::mem::bitset_unset(state->gamepads[i].buttons_down, i);
                    zcl::mem::bitset_set(state->events.gamepads[i].buttons_released, i);
                }
            }
        }

        zcl::array_copy(zcl::array_to_nonstatic(&axes), zcl::array_to_nonstatic(&state->gamepads[gamepad_index].axes));
    }

    zcl::t_array_rdonly<zcl::strs::t_code_pt> text_get_code_pts(const t_state *const state) {
        return zcl::ds::list_to_array(&state->events.code_pts);
    }

    zcl::t_b8 text_submit_code_point(t_state *const state, const zcl::strs::t_code_pt cp) {
        if (state->events.code_pts.len < zcl::ds::list_get_cap(&state->events.code_pts)) {
            zcl::ds::list_append(&state->events.code_pts, cp);
            return true;
        }

        return false;
    }
}
