#include <zgl/zgl_input.h>

namespace zf {
    struct t_gamepad {
        t_static_bitset<ecm_gamepad_button_code_cnt> buttons_down;
        t_static_array<t_f32, ecm_gamepad_axis_code_cnt> axes;
    };

    struct t_gamepad_events {
        t_static_bitset<ecm_gamepad_button_code_cnt> buttons_pressed;
        t_static_bitset<ecm_gamepad_button_code_cnt> buttons_released;
    };

    struct t_input_state {
        t_static_bitset<ecm_key_code_cnt> keys_down;

        t_static_bitset<ecm_mouse_button_code_cnt> mouse_buttons_down;

        t_v2 cursor_pos;

        t_static_bitset<g_gamepad_limit> gamepads_connected;
        t_static_array<t_gamepad, g_gamepad_limit> gamepads;
        t_static_array<t_f32, ecm_gamepad_axis_code_cnt> gamepad_axis_deadzones;

        struct {
            t_static_bitset<ecm_key_code_cnt> keys_pressed;
            t_static_bitset<ecm_key_code_cnt> keys_released;

            t_static_bitset<ecm_mouse_button_code_cnt> mouse_buttons_pressed;
            t_static_bitset<ecm_mouse_button_code_cnt> mouse_buttons_released;

            t_v2 scroll_offs;

            t_static_array<t_gamepad_events, g_gamepad_limit> gamepads;
        } events;
    };

    t_input_state *f_input_create_state(t_arena *const arena) {
        return f_mem_arena_push_item_zeroed<t_input_state>(arena);
    }

    void f_input_clear_events(t_input_state *const state) {
        f_mem_clear_item(&state->events, 0);
    }

    t_b8 f_input_is_key_down(const t_input_state *const state, const t_key_code code) {
        return f_mem_is_bit_set(state->keys_down, code);
    }

    t_b8 f_input_is_key_pressed(const t_input_state *const state, const t_key_code code) {
        return f_mem_is_bit_set(state->events.keys_pressed, code);
    }

    t_b8 f_input_key_is_released(const t_input_state *const state, const t_key_code code) {
        return f_mem_is_bit_set(state->events.keys_released, code);
    }

    void f_input_update_key_state(t_input_state *const state, const t_key_code code, const t_b8 is_down) {
        if (is_down) {
            if (!f_mem_is_bit_set(state->keys_down, code)) {
                f_mem_set_bit(state->keys_down, code);
                f_mem_set_bit(state->events.keys_pressed, code);
            }
        } else {
            if (f_mem_is_bit_set(state->keys_down, code)) {
                f_mem_unset_bit(state->keys_down, code);
                f_mem_set_bit(state->events.keys_released, code);
            }
        }
    }

    t_b8 f_input_is_mouse_button_down(const t_input_state *const state, const t_mouse_button_code btn_code) {
        return f_mem_is_bit_set(state->mouse_buttons_down, static_cast<t_i32>(btn_code));
    }

    t_b8 f_input_is_mouse_button_pressed(const t_input_state *const state, const t_mouse_button_code btn_code) {
        return f_mem_is_bit_set(state->events.mouse_buttons_pressed, static_cast<t_i32>(btn_code));
    }

    t_b8 f_input_is_mouse_button_released(const t_input_state *const state, const t_mouse_button_code btn_code) {
        return f_mem_is_bit_set(state->events.mouse_buttons_released, static_cast<t_i32>(btn_code));
    }

    void f_input_update_mouse_button_state(t_input_state *const state, const t_mouse_button_code btn_code, const t_b8 is_down) {
        if (is_down) {
            if (!f_mem_is_bit_set(state->mouse_buttons_down, static_cast<t_i32>(btn_code))) {
                f_mem_set_bit(state->mouse_buttons_down, static_cast<t_i32>(btn_code));
                f_mem_set_bit(state->events.mouse_buttons_pressed, static_cast<t_i32>(btn_code));
            }
        } else {
            if (f_mem_is_bit_set(state->mouse_buttons_down, static_cast<t_i32>(btn_code))) {
                f_mem_unset_bit(state->mouse_buttons_down, static_cast<t_i32>(btn_code));
                f_mem_set_bit(state->events.mouse_buttons_released, static_cast<t_i32>(btn_code));
            }
        }
    }

    t_v2 f_input_get_cursor_pos(const t_input_state *const state) {
        return state->cursor_pos;
    }

    void f_input_update_cursor_pos(t_input_state *const state, const t_v2 val) {
        state->cursor_pos = val;
    }

    t_v2 f_input_get_scroll_offs(const t_input_state *const state) {
        return state->events.scroll_offs;
    }

    void f_input_update_scroll_offs(t_input_state *const state, const t_v2 offs_to_apply) {
        state->events.scroll_offs += offs_to_apply;
    }

    t_b8 f_input_is_gamepad_connected(const t_input_state *const state, const t_i32 index) {
        ZF_ASSERT(index >= 0 && index < g_gamepad_limit);
        return f_mem_is_bit_set(state->gamepads_connected, index);
    }

    t_b8 f_input_is_gamepad_button_down(const t_input_state *const state, const t_i32 gamepad_index, const t_gamepad_button_code btn_code) {
        ZF_ASSERT(f_input_is_gamepad_connected(state, gamepad_index));
        return f_mem_is_bit_set(state->gamepads[gamepad_index].buttons_down, btn_code);
    }

    t_b8 f_input_is_gamepad_button_pressed(const t_input_state *const state, const t_i32 gamepad_index, const t_gamepad_button_code btn_code) {
        ZF_ASSERT(f_input_is_gamepad_connected(state, gamepad_index));
        return f_mem_is_bit_set(state->events.gamepads[gamepad_index].buttons_pressed, btn_code);
    }

    t_b8 f_input_is_gamepad_button_released(const t_input_state *const state, const t_i32 gamepad_index, const t_gamepad_button_code btn_code) {
        ZF_ASSERT(f_input_is_gamepad_connected(state, gamepad_index));
        return f_mem_is_bit_set(state->events.gamepads[gamepad_index].buttons_released, btn_code);
    }

    inline t_f32 f_input_get_gamepad_axis_value_raw(const t_input_state *const state, const t_i32 gamepad_index, const t_gamepad_axis_code axis_code) {
        ZF_ASSERT(f_input_is_gamepad_connected(state, gamepad_index));
        return state->gamepads[gamepad_index].axes[axis_code];
    }

    t_f32 f_input_get_gamepad_axis_value_with_deadzone(const t_input_state *const state, const t_i32 gamepad_index, const t_gamepad_axis_code axis_code) {
        ZF_ASSERT(f_input_is_gamepad_connected(state, gamepad_index));

        const t_f32 raw = state->gamepads[gamepad_index].axes[axis_code];
        const t_f32 raw_abs = f_abs(raw);

        const t_f32 dz = state->gamepad_axis_deadzones[axis_code];

        if (raw_abs <= dz) {
            return 0.0f;
        }

        return static_cast<t_f32>(f_sign(raw)) * ((raw_abs - dz) / (1.0f - dz));
    }

    void f_input_update_gamepad_state(t_input_state *const state, const t_i32 gamepad_index, const t_b8 connected, const t_static_bitset<ecm_gamepad_button_code_cnt> &btns_down, const t_static_array<t_f32, ecm_gamepad_axis_code_cnt> &axes) {
        if (!connected) {
            ZF_ASSERT(f_mem_are_all_bits_unset(btns_down) && f_algos_do_all_equal(f_array_get_as_nonstatic(axes), 0.0f));
            return;
        }

        if (!f_mem_is_bit_set(state->gamepads_connected, gamepad_index)) {
            f_mem_set_bit(state->gamepads_connected, gamepad_index);
            state->gamepads[gamepad_index] = {};
            state->events.gamepads[gamepad_index] = {};
        }

        for (t_i32 i = 0; i < ecm_gamepad_button_code_cnt; i++) {
            if (f_mem_is_bit_set(btns_down, i)) {
                if (!f_mem_is_bit_set(state->gamepads[i].buttons_down, i)) {
                    f_mem_set_bit(state->gamepads[i].buttons_down, i);
                    f_mem_set_bit(state->events.gamepads[i].buttons_pressed, i);
                }
            } else {
                if (f_mem_is_bit_set(state->gamepads[i].buttons_down, i)) {
                    f_mem_unset_bit(state->gamepads[i].buttons_down, i);
                    f_mem_set_bit(state->events.gamepads[i].buttons_released, i);
                }
            }
        }

        f_algos_copy_all(f_array_get_as_nonstatic(axes), f_array_get_as_nonstatic(state->gamepads[gamepad_index].axes));
    }
}
