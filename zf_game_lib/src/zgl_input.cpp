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

        mem::t_static_bitset<g_gamepad_limit> gamepads_connected;
        t_static_array<t_gamepad, g_gamepad_limit> gamepads;
        t_static_array<t_f32, ecm_gamepad_axis_code_cnt> gamepad_axis_deadzones;

        struct {
            mem::t_static_bitset<ecm_key_code_cnt> keys_pressed;
            mem::t_static_bitset<ecm_key_code_cnt> keys_released;

            mem::t_static_bitset<ecm_mouse_button_code_cnt> mouse_buttons_pressed;
            mem::t_static_bitset<ecm_mouse_button_code_cnt> mouse_buttons_released;

            math::t_v2 scroll_offs;

            t_static_array<t_gamepad_events, g_gamepad_limit> gamepads;
        } events;
    };

    t_state *f_create_state(mem::t_arena *const arena) {
        return mem::f_arena_push_item_zeroed<t_state>(arena);
    }

    void f_clear_events(t_state *const state) {
        mem::f_clear_item(&state->events, 0);
    }

    t_b8 f_is_key_down(const t_state *const state, const t_key_code code) {
        return mem::f_is_bit_set(state->keys_down, code);
    }

    t_b8 f_is_key_pressed(const t_state *const state, const t_key_code code) {
        return mem::f_is_bit_set(state->events.keys_pressed, code);
    }

    t_b8 f_key_is_released(const t_state *const state, const t_key_code code) {
        return mem::f_is_bit_set(state->events.keys_released, code);
    }

    void f_update_key_state(t_state *const state, const t_key_code code, const t_b8 is_down) {
        if (is_down) {
            if (!mem::f_is_bit_set(state->keys_down, code)) {
                mem::f_set_bit(state->keys_down, code);
                mem::f_set_bit(state->events.keys_pressed, code);
            }
        } else {
            if (mem::f_is_bit_set(state->keys_down, code)) {
                mem::f_unset_bit(state->keys_down, code);
                mem::f_set_bit(state->events.keys_released, code);
            }
        }
    }

    t_b8 f_is_mouse_button_down(const t_state *const state, const t_mouse_button_code btn_code) {
        return mem::f_is_bit_set(state->mouse_buttons_down, static_cast<t_i32>(btn_code));
    }

    t_b8 f_is_mouse_button_pressed(const t_state *const state, const t_mouse_button_code btn_code) {
        return mem::f_is_bit_set(state->events.mouse_buttons_pressed, static_cast<t_i32>(btn_code));
    }

    t_b8 f_is_mouse_button_released(const t_state *const state, const t_mouse_button_code btn_code) {
        return mem::f_is_bit_set(state->events.mouse_buttons_released, static_cast<t_i32>(btn_code));
    }

    void f_update_mouse_button_state(t_state *const state, const t_mouse_button_code btn_code, const t_b8 is_down) {
        if (is_down) {
            if (!mem::f_is_bit_set(state->mouse_buttons_down, static_cast<t_i32>(btn_code))) {
                mem::f_set_bit(state->mouse_buttons_down, static_cast<t_i32>(btn_code));
                mem::f_set_bit(state->events.mouse_buttons_pressed, static_cast<t_i32>(btn_code));
            }
        } else {
            if (mem::f_is_bit_set(state->mouse_buttons_down, static_cast<t_i32>(btn_code))) {
                mem::f_unset_bit(state->mouse_buttons_down, static_cast<t_i32>(btn_code));
                mem::f_set_bit(state->events.mouse_buttons_released, static_cast<t_i32>(btn_code));
            }
        }
    }

    math::t_v2 f_get_cursor_pos(const t_state *const state) {
        return state->cursor_pos;
    }

    void f_update_cursor_pos(t_state *const state, const math::t_v2 val) {
        state->cursor_pos = val;
    }

    math::t_v2 f_get_scroll_offs(const t_state *const state) {
        return state->events.scroll_offs;
    }

    void f_update_scroll_offs(t_state *const state, const math::t_v2 offs_to_apply) {
        state->events.scroll_offs += offs_to_apply;
    }

    t_b8 f_is_gamepad_connected(const t_state *const state, const t_i32 index) {
        ZF_ASSERT(index >= 0 && index < g_gamepad_limit);
        return mem::f_is_bit_set(state->gamepads_connected, index);
    }

    t_b8 f_is_gamepad_button_down(const t_state *const state, const t_i32 gamepad_index, const t_gamepad_button_code btn_code) {
        ZF_ASSERT(f_is_gamepad_connected(state, gamepad_index));
        return mem::f_is_bit_set(state->gamepads[gamepad_index].buttons_down, btn_code);
    }

    t_b8 f_is_gamepad_button_pressed(const t_state *const state, const t_i32 gamepad_index, const t_gamepad_button_code btn_code) {
        ZF_ASSERT(f_is_gamepad_connected(state, gamepad_index));
        return mem::f_is_bit_set(state->events.gamepads[gamepad_index].buttons_pressed, btn_code);
    }

    t_b8 f_is_gamepad_button_released(const t_state *const state, const t_i32 gamepad_index, const t_gamepad_button_code btn_code) {
        ZF_ASSERT(f_is_gamepad_connected(state, gamepad_index));
        return mem::f_is_bit_set(state->events.gamepads[gamepad_index].buttons_released, btn_code);
    }

    inline t_f32 f_get_gamepad_axis_value_raw(const t_state *const state, const t_i32 gamepad_index, const t_gamepad_axis_code axis_code) {
        ZF_ASSERT(f_is_gamepad_connected(state, gamepad_index));
        return state->gamepads[gamepad_index].axes[axis_code];
    }

    t_f32 f_get_gamepad_axis_value_with_deadzone(const t_state *const state, const t_i32 gamepad_index, const t_gamepad_axis_code axis_code) {
        ZF_ASSERT(f_is_gamepad_connected(state, gamepad_index));

        const t_f32 raw = state->gamepads[gamepad_index].axes[axis_code];
        const t_f32 raw_abs = f_abs(raw);

        const t_f32 dz = state->gamepad_axis_deadzones[axis_code];

        if (raw_abs <= dz) {
            return 0.0f;
        }

        return static_cast<t_f32>(f_sign(raw)) * ((raw_abs - dz) / (1.0f - dz));
    }

    void f_update_gamepad_state(t_state *const state, const t_i32 gamepad_index, const t_b8 connected, const mem::t_static_bitset<ecm_gamepad_button_code_cnt> &btns_down, const t_static_array<t_f32, ecm_gamepad_axis_code_cnt> &axes) {
        if (!connected) {
            ZF_ASSERT(mem::f_are_all_bits_unset(btns_down) && f_algos_do_all_equal(f_array_get_as_nonstatic(axes), 0.0f));
            return;
        }

        if (!mem::f_is_bit_set(state->gamepads_connected, gamepad_index)) {
            mem::f_set_bit(state->gamepads_connected, gamepad_index);
            state->gamepads[gamepad_index] = {};
            state->events.gamepads[gamepad_index] = {};
        }

        for (t_i32 i = 0; i < ecm_gamepad_button_code_cnt; i++) {
            if (mem::f_is_bit_set(btns_down, i)) {
                if (!mem::f_is_bit_set(state->gamepads[i].buttons_down, i)) {
                    mem::f_set_bit(state->gamepads[i].buttons_down, i);
                    mem::f_set_bit(state->events.gamepads[i].buttons_pressed, i);
                }
            } else {
                if (mem::f_is_bit_set(state->gamepads[i].buttons_down, i)) {
                    mem::f_unset_bit(state->gamepads[i].buttons_down, i);
                    mem::f_set_bit(state->events.gamepads[i].buttons_released, i);
                }
            }
        }

        f_algos_copy_all(f_array_get_as_nonstatic(axes), f_array_get_as_nonstatic(state->gamepads[gamepad_index].axes));
    }
}
