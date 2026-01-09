#include <zgl/zgl_input.h>

namespace zgl::input {
    struct t_gamepad {
        zf::mem::t_static_bitset<ekm_gamepad_button_code_cnt> buttons_down;
        zf::t_static_array<zf::t_f32, ekm_gamepad_axis_code_cnt> axes;
    };

    struct t_gamepad_events {
        zf::mem::t_static_bitset<ekm_gamepad_button_code_cnt> buttons_pressed;
        zf::mem::t_static_bitset<ekm_gamepad_button_code_cnt> buttons_released;
    };

    struct t_state {
        zf::mem::t_static_bitset<ekm_key_code_cnt> keys_down;

        zf::mem::t_static_bitset<ekm_mouse_button_code_cnt> mouse_buttons_down;

        zf::math::t_v2 cursor_pos;

        zf::mem::t_static_bitset<k_gamepad_limit> gamepads_connected;
        zf::t_static_array<t_gamepad, k_gamepad_limit> gamepads;
        zf::t_static_array<zf::t_f32, ekm_gamepad_axis_code_cnt> gamepad_axis_deadzones;

        struct {
            zf::mem::t_static_bitset<ekm_key_code_cnt> keys_pressed;
            zf::mem::t_static_bitset<ekm_key_code_cnt> keys_released;

            zf::mem::t_static_bitset<ekm_mouse_button_code_cnt> mouse_buttons_pressed;
            zf::mem::t_static_bitset<ekm_mouse_button_code_cnt> mouse_buttons_released;

            zf::math::t_v2 scroll_offs;

            zf::t_static_array<t_gamepad_events, k_gamepad_limit> gamepads;
        } events;
    };

    t_state *create_state(zf::mem::t_arena *const arena) {
        return zf::mem::arena_push_item<t_state>(arena);
    }

    void clear_events(t_state *const state) {
        zf::mem::zero_clear_item(&state->events);
    }

    zf::t_b8 key_check_down(const t_state *const state, const t_key_code code) {
        return zf::mem::bitset_check_set(state->keys_down, code);
    }

    zf::t_b8 key_check_pressed(const t_state *const state, const t_key_code code) {
        return zf::mem::bitset_check_set(state->events.keys_pressed, code);
    }

    zf::t_b8 key_check_released(const t_state *const state, const t_key_code code) {
        return zf::mem::bitset_check_set(state->events.keys_released, code);
    }

    void key_update_state(t_state *const state, const t_key_code code, const zf::t_b8 is_down) {
        if (is_down) {
            if (!zf::mem::bitset_check_set(state->keys_down, code)) {
                zf::mem::bitset_set(state->keys_down, code);
                zf::mem::bitset_set(state->events.keys_pressed, code);
            }
        } else {
            if (zf::mem::bitset_check_set(state->keys_down, code)) {
                zf::mem::bitset_unset(state->keys_down, code);
                zf::mem::bitset_set(state->events.keys_released, code);
            }
        }
    }

    zf::t_b8 mouse_button_check_down(const t_state *const state, const t_mouse_button_code btn_code) {
        return zf::mem::bitset_check_set(state->mouse_buttons_down, static_cast<zf::t_i32>(btn_code));
    }

    zf::t_b8 mouse_button_check_pressed(const t_state *const state, const t_mouse_button_code btn_code) {
        return zf::mem::bitset_check_set(state->events.mouse_buttons_pressed, static_cast<zf::t_i32>(btn_code));
    }

    zf::t_b8 mouse_button_check_released(const t_state *const state, const t_mouse_button_code btn_code) {
        return zf::mem::bitset_check_set(state->events.mouse_buttons_released, static_cast<zf::t_i32>(btn_code));
    }

    void mouse_button_update_state(t_state *const state, const t_mouse_button_code btn_code, const zf::t_b8 is_down) {
        if (is_down) {
            if (!zf::mem::bitset_check_set(state->mouse_buttons_down, static_cast<zf::t_i32>(btn_code))) {
                zf::mem::bitset_set(state->mouse_buttons_down, static_cast<zf::t_i32>(btn_code));
                zf::mem::bitset_set(state->events.mouse_buttons_pressed, static_cast<zf::t_i32>(btn_code));
            }
        } else {
            if (zf::mem::bitset_check_set(state->mouse_buttons_down, static_cast<zf::t_i32>(btn_code))) {
                zf::mem::bitset_unset(state->mouse_buttons_down, static_cast<zf::t_i32>(btn_code));
                zf::mem::bitset_set(state->events.mouse_buttons_released, static_cast<zf::t_i32>(btn_code));
            }
        }
    }

    zf::math::t_v2 cursor_get_pos(const t_state *const state) {
        return state->cursor_pos;
    }

    void cursor_update_state(t_state *const state, const zf::math::t_v2 pos) {
        state->cursor_pos = pos;
    }

    zf::math::t_v2 scroll_get_offset(const t_state *const state) {
        return state->events.scroll_offs;
    }

    void scroll_update_state(t_state *const state, const zf::math::t_v2 offs_to_apply) {
        state->events.scroll_offs += offs_to_apply;
    }

    zf::t_b8 gamepad_check_connected(const t_state *const state, const zf::t_i32 index) {
        ZF_ASSERT(index >= 0 && index < k_gamepad_limit);
        return zf::mem::bitset_check_set(state->gamepads_connected, index);
    }

    zf::t_b8 gamepad_check_button_down(const t_state *const state, const zf::t_i32 gamepad_index, const t_gamepad_button_code btn_code) {
        ZF_ASSERT(gamepad_check_connected(state, gamepad_index));
        return zf::mem::bitset_check_set(state->gamepads[gamepad_index].buttons_down, btn_code);
    }

    zf::t_b8 gamepad_check_button_pressed(const t_state *const state, const zf::t_i32 gamepad_index, const t_gamepad_button_code btn_code) {
        ZF_ASSERT(gamepad_check_connected(state, gamepad_index));
        return zf::mem::bitset_check_set(state->events.gamepads[gamepad_index].buttons_pressed, btn_code);
    }

    zf::t_b8 gamepad_check_button_released(const t_state *const state, const zf::t_i32 gamepad_index, const t_gamepad_button_code btn_code) {
        ZF_ASSERT(gamepad_check_connected(state, gamepad_index));
        return zf::mem::bitset_check_set(state->events.gamepads[gamepad_index].buttons_released, btn_code);
    }

    zf::t_f32 gamepad_get_axis_value_raw(const t_state *const state, const zf::t_i32 gamepad_index, const t_gamepad_axis_code axis_code) {
        ZF_ASSERT(gamepad_check_connected(state, gamepad_index));
        return state->gamepads[gamepad_index].axes[axis_code];
    }

    zf::t_f32 gamepad_get_axis_value_with_deadzone(const t_state *const state, const zf::t_i32 gamepad_index, const t_gamepad_axis_code axis_code) {
        ZF_ASSERT(gamepad_check_connected(state, gamepad_index));

        const zf::t_f32 raw = state->gamepads[gamepad_index].axes[axis_code];
        const zf::t_f32 raw_abs = abs(raw);

        const zf::t_f32 dz = state->gamepad_axis_deadzones[axis_code];

        if (raw_abs <= dz) {
            return 0.0f;
        }

        return static_cast<zf::t_f32>(zf::sign(raw)) * ((raw_abs - dz) / (1.0f - dz));
    }

    void gamepad_update_state(t_state *const state, const zf::t_i32 gamepad_index, const zf::t_b8 connected, const zf::mem::t_static_bitset<ekm_gamepad_button_code_cnt> &btns_down, const zf::t_static_array<zf::t_f32, ekm_gamepad_axis_code_cnt> &axes) {
        if (!connected) {
            ZF_ASSERT(zf::mem::bitset_check_all_unset(btns_down) && zf::array_check_all_equal(zf::array_to_nonstatic(axes), 0.0f));
            return;
        }

        if (!zf::mem::bitset_check_set(state->gamepads_connected, gamepad_index)) {
            zf::mem::bitset_set(state->gamepads_connected, gamepad_index);
            state->gamepads[gamepad_index] = {};
            state->events.gamepads[gamepad_index] = {};
        }

        for (zf::t_i32 i = 0; i < ekm_gamepad_button_code_cnt; i++) {
            if (zf::mem::bitset_check_set(btns_down, i)) {
                if (!zf::mem::bitset_check_set(state->gamepads[i].buttons_down, i)) {
                    zf::mem::bitset_set(state->gamepads[i].buttons_down, i);
                    zf::mem::bitset_set(state->events.gamepads[i].buttons_pressed, i);
                }
            } else {
                if (zf::mem::bitset_check_set(state->gamepads[i].buttons_down, i)) {
                    zf::mem::bitset_unset(state->gamepads[i].buttons_down, i);
                    zf::mem::bitset_set(state->events.gamepads[i].buttons_released, i);
                }
            }
        }

        zf::array_copy(zf::array_to_nonstatic(axes), zf::array_to_nonstatic(state->gamepads[gamepad_index].axes));
    }
}
