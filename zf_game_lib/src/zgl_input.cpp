#include <zgl/zgl_input.h>

namespace zf::input {
    struct t_gamepad {
        mem::t_static_bitset<ecm_gamepad_button_code_cnt> buttons_down;
        t_static_array<t_f32, ecm_gamepad_axis_code_cnt> axes;
    };

    struct t_gamepad_events {
        mem::t_static_bitset<ecm_gamepad_button_code_cnt> buttons_pressed;
        mem::t_static_bitset<ecm_gamepad_button_code_cnt> buttons_released;
    };

    struct t_state {
        mem::t_static_bitset<ecm_key_code_cnt> keys_down;

        mem::t_static_bitset<ecm_mouse_button_code_cnt> mouse_buttons_down;

        math::t_v2 cursor_pos;

        mem::t_static_bitset<k_gamepad_limit> gamepads_connected;
        t_static_array<t_gamepad, k_gamepad_limit> gamepads;
        t_static_array<t_f32, ecm_gamepad_axis_code_cnt> gamepad_axis_deadzones;

        struct {
            mem::t_static_bitset<ecm_key_code_cnt> keys_pressed;
            mem::t_static_bitset<ecm_key_code_cnt> keys_released;

            mem::t_static_bitset<ecm_mouse_button_code_cnt> mouse_buttons_pressed;
            mem::t_static_bitset<ecm_mouse_button_code_cnt> mouse_buttons_released;

            math::t_v2 scroll_offs;

            t_static_array<t_gamepad_events, k_gamepad_limit> gamepads;
        } events;
    };

    t_state *create_state(mem::t_arena *const arena) {
        return mem::arena_push_item<t_state>(arena);
    }

    void clear_events(t_state *const state) {
        mem::zero_clear_item(&state->events);
    }

    t_b8 key_check_down(const t_state *const state, const t_key_code code) {
        return mem::bitset_check_set(state->keys_down, code);
    }

    t_b8 key_check_pressed(const t_state *const state, const t_key_code code) {
        return mem::bitset_check_set(state->events.keys_pressed, code);
    }

    t_b8 key_check_released(const t_state *const state, const t_key_code code) {
        return mem::bitset_check_set(state->events.keys_released, code);
    }

    void key_update_state(t_state *const state, const t_key_code code, const t_b8 is_down) {
        if (is_down) {
            if (!mem::bitset_check_set(state->keys_down, code)) {
                mem::bitset_set(state->keys_down, code);
                mem::bitset_set(state->events.keys_pressed, code);
            }
        } else {
            if (mem::bitset_check_set(state->keys_down, code)) {
                mem::bitset_unset(state->keys_down, code);
                mem::bitset_set(state->events.keys_released, code);
            }
        }
    }

    t_b8 mouse_button_check_down(const t_state *const state, const t_mouse_button_code btn_code) {
        return mem::bitset_check_set(state->mouse_buttons_down, static_cast<t_i32>(btn_code));
    }

    t_b8 mouse_button_check_pressed(const t_state *const state, const t_mouse_button_code btn_code) {
        return mem::bitset_check_set(state->events.mouse_buttons_pressed, static_cast<t_i32>(btn_code));
    }

    t_b8 mouse_button_check_released(const t_state *const state, const t_mouse_button_code btn_code) {
        return mem::bitset_check_set(state->events.mouse_buttons_released, static_cast<t_i32>(btn_code));
    }

    void mouse_button_update_state(t_state *const state, const t_mouse_button_code btn_code, const t_b8 is_down) {
        if (is_down) {
            if (!mem::bitset_check_set(state->mouse_buttons_down, static_cast<t_i32>(btn_code))) {
                mem::bitset_set(state->mouse_buttons_down, static_cast<t_i32>(btn_code));
                mem::bitset_set(state->events.mouse_buttons_pressed, static_cast<t_i32>(btn_code));
            }
        } else {
            if (mem::bitset_check_set(state->mouse_buttons_down, static_cast<t_i32>(btn_code))) {
                mem::bitset_unset(state->mouse_buttons_down, static_cast<t_i32>(btn_code));
                mem::bitset_set(state->events.mouse_buttons_released, static_cast<t_i32>(btn_code));
            }
        }
    }

    math::t_v2 cursor_get_pos(const t_state *const state) {
        return state->cursor_pos;
    }

    void cursor_update_state(t_state *const state, const math::t_v2 pos) {
        state->cursor_pos = pos;
    }

    math::t_v2 scroll_get_offset(const t_state *const state) {
        return state->events.scroll_offs;
    }

    void scroll_update_state(t_state *const state, const math::t_v2 offs_to_apply) {
        state->events.scroll_offs += offs_to_apply;
    }

    t_b8 gamepad_check_connected(const t_state *const state, const t_i32 index) {
        ZF_ASSERT(index >= 0 && index < k_gamepad_limit);
        return mem::bitset_check_set(state->gamepads_connected, index);
    }

    t_b8 gamepad_check_button_down(const t_state *const state, const t_i32 gamepad_index, const t_gamepad_button_code btn_code) {
        ZF_ASSERT(gamepad_check_connected(state, gamepad_index));
        return mem::bitset_check_set(state->gamepads[gamepad_index].buttons_down, btn_code);
    }

    t_b8 gamepad_check_button_pressed(const t_state *const state, const t_i32 gamepad_index, const t_gamepad_button_code btn_code) {
        ZF_ASSERT(gamepad_check_connected(state, gamepad_index));
        return mem::bitset_check_set(state->events.gamepads[gamepad_index].buttons_pressed, btn_code);
    }

    t_b8 gamepad_check_button_released(const t_state *const state, const t_i32 gamepad_index, const t_gamepad_button_code btn_code) {
        ZF_ASSERT(gamepad_check_connected(state, gamepad_index));
        return mem::bitset_check_set(state->events.gamepads[gamepad_index].buttons_released, btn_code);
    }

    t_f32 gamepad_get_axis_value_raw(const t_state *const state, const t_i32 gamepad_index, const t_gamepad_axis_code axis_code) {
        ZF_ASSERT(gamepad_check_connected(state, gamepad_index));
        return state->gamepads[gamepad_index].axes[axis_code];
    }

    t_f32 gamepad_get_axis_value_with_deadzone(const t_state *const state, const t_i32 gamepad_index, const t_gamepad_axis_code axis_code) {
        ZF_ASSERT(gamepad_check_connected(state, gamepad_index));

        const t_f32 raw = state->gamepads[gamepad_index].axes[axis_code];
        const t_f32 raw_abs = abs(raw);

        const t_f32 dz = state->gamepad_axis_deadzones[axis_code];

        if (raw_abs <= dz) {
            return 0.0f;
        }

        return static_cast<t_f32>(sign(raw)) * ((raw_abs - dz) / (1.0f - dz));
    }

    void gamepad_update_state(t_state *const state, const t_i32 gamepad_index, const t_b8 connected, const mem::t_static_bitset<ecm_gamepad_button_code_cnt> &btns_down, const t_static_array<t_f32, ecm_gamepad_axis_code_cnt> &axes) {
        if (!connected) {
            ZF_ASSERT(mem::bitset_check_all_unset(btns_down) && array_check_all_equal(array_to_nonstatic(axes), 0.0f));
            return;
        }

        if (!mem::bitset_check_set(state->gamepads_connected, gamepad_index)) {
            mem::bitset_set(state->gamepads_connected, gamepad_index);
            state->gamepads[gamepad_index] = {};
            state->events.gamepads[gamepad_index] = {};
        }

        for (t_i32 i = 0; i < ecm_gamepad_button_code_cnt; i++) {
            if (mem::bitset_check_set(btns_down, i)) {
                if (!mem::bitset_check_set(state->gamepads[i].buttons_down, i)) {
                    mem::bitset_set(state->gamepads[i].buttons_down, i);
                    mem::bitset_set(state->events.gamepads[i].buttons_pressed, i);
                }
            } else {
                if (mem::bitset_check_set(state->gamepads[i].buttons_down, i)) {
                    mem::bitset_unset(state->gamepads[i].buttons_down, i);
                    mem::bitset_set(state->events.gamepads[i].buttons_released, i);
                }
            }
        }

        array_copy(array_to_nonstatic(axes), array_to_nonstatic(state->gamepads[gamepad_index].axes));
    }
}
