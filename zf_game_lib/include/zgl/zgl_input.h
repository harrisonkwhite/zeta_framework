#pragma once

#include <zcl.h>

namespace zf::input {
    // ============================================================
    // @section: Types and Globals

    struct State;

    enum KeyCode : t_i32 {
        ec_key_code_space,
        ec_key_code_0,
        ec_key_code_1,
        ec_key_code_2,
        ec_key_code_3,
        ec_key_code_4,
        ec_key_code_5,
        ec_key_code_6,
        ec_key_code_7,
        ec_key_code_8,
        ec_key_code_9,
        ec_key_code_a,
        ec_key_code_b,
        ec_key_code_c,
        ec_key_code_d,
        ec_key_code_e,
        ec_key_code_f,
        ec_key_code_g,
        ec_key_code_h,
        ec_key_code_i,
        ec_key_code_j,
        ec_key_code_k,
        ec_key_code_l,
        ec_key_code_m,
        ec_key_code_n,
        ec_key_code_o,
        ec_key_code_p,
        ec_key_code_q,
        ec_key_code_r,
        ec_key_code_s,
        ec_key_code_t,
        ec_key_code_u,
        ec_key_code_v,
        ec_key_code_w,
        ec_key_code_x,
        ec_key_code_y,
        ec_key_code_z,
        ec_key_code_escape,
        ec_key_code_enter,
        ec_key_code_backspace,
        ec_key_code_tab,
        ec_key_code_right,
        ec_key_code_left,
        ec_key_code_down,
        ec_key_code_up,
        ec_key_code_f1,
        ec_key_code_f2,
        ec_key_code_f3,
        ec_key_code_f4,
        ec_key_code_f5,
        ec_key_code_f6,
        ec_key_code_f7,
        ec_key_code_f8,
        ec_key_code_f9,
        ec_key_code_f10,
        ec_key_code_f11,
        ec_key_code_f12,
        ec_key_code_left_shift,
        ec_key_code_left_control,
        ec_key_code_left_alt,
        ec_key_code_right_shift,
        ec_key_code_right_control,
        ec_key_code_right_alt,

        ecm_key_code_cnt
    };

    enum MouseButtonCode : t_i32 {
        ec_mouse_button_code_left,
        ec_mouse_button_code_right,
        ec_mouse_button_code_middle,

        ecm_mouse_button_code_cnt
    };

    constexpr t_i32 g_gamepad_limit = 16;

    enum e_gamepad_button_code : t_i32 {
        ec_gamepad_button_code_a,
        ec_gamepad_button_code_b,
        ec_gamepad_button_code_x,
        ec_gamepad_button_code_y,
        ec_gamepad_button_code_left_bumper,
        ec_gamepad_button_code_right_bumper,
        ec_gamepad_button_code_back,
        ec_gamepad_button_code_start,
        ec_gamepad_button_code_guide,
        ec_gamepad_button_code_left_thumb,
        ec_gamepad_button_code_right_thumb,
        ec_gamepad_button_code_dpad_up,
        ec_gamepad_button_code_dpad_right,
        ec_gamepad_button_code_dpad_down,
        ec_gamepad_button_code_dpad_left,

        ecm_gamepad_button_code_cnt
    };

    enum e_gamepad_axis_code : t_i32 {
        ec_gamepad_axis_code_left_x,
        ec_gamepad_axis_code_left_y,
        ec_gamepad_axis_code_right_x,
        ec_gamepad_axis_code_right_y,
        ec_gamepad_axis_code_left_trigger,
        ec_gamepad_axis_code_right_trigger,

        ecm_gamepad_axis_code_cnt
    };

    // ============================================================


    // ============================================================
    // @section: Functions

    State *create_state(s_arena *const arena);

    void clear_events(State *const state);

    B8 get_key_is_down(const State *const state, const KeyCode code);
    B8 get_key_is_pressed(const State *const state, const KeyCode code);
    B8 get_key_is_released(const State *const state, const KeyCode code);
    void update_key_state(State *const state, const KeyCode code, const B8 is_down);

    B8 get_mouse_button_is_down(const State *const state, const MouseButtonCode btn_code);
    B8 get_mouse_button_is_pressed(const State *const state, const MouseButtonCode btn_code);
    B8 get_mouse_button_is_released(const State *const state, const MouseButtonCode btn_code);
    void update_mouse_button_state(State *const state, const MouseButtonCode code, const B8 is_down);

    s_v2 get_cursor_pos(const State *const state);
    void update_cursor_pos(State *const state, const s_v2 val);

    s_v2 get_scroll_offs(const State *const state);
    void update_scroll_offs(State *const state, const s_v2 offs_to_apply);

    B8 get_gamepad_is_connected(const State *const state, const t_i32 index);
    B8 get_gamepad_button_is_down(const State *const state, const t_i32 gamepad_index, const e_gamepad_button_code btn_code);
    B8 get_gamepad_button_is_pressed(const State *const state, const t_i32 gamepad_index, const e_gamepad_button_code btn_code);
    B8 get_gamepad_button_is_released(const State *const state, const t_i32 gamepad_index, const e_gamepad_button_code btn_code);
    t_f32 get_gamepad_axis_value_raw(const State *const state, const t_i32 gamepad_index, const e_gamepad_axis_code axis_code);
    t_f32 calc_gamepad_axis_value_with_deadzone(const State *const state, const t_i32 gamepad_index, const e_gamepad_axis_code axis_code);
    void update_gamepad_state(State *const state, const t_i32 gamepad_index, const B8 connected, const s_static_bit_vec<ecm_gamepad_button_code_cnt> &btns_down, const s_static_array<t_f32, ecm_gamepad_axis_code_cnt> &axes);

    // ============================================================
}
