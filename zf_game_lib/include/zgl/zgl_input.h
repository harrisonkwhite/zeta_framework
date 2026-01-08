#pragma once

#include <zcl.h>

namespace zf::input {
    // ============================================================
    // @section: Types and Globals

    struct t_state;

    enum t_key_code : t_i32 {
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

    enum t_mouse_button_code : t_i32 {
        ec_mouse_button_code_left,
        ec_mouse_button_code_right,
        ec_mouse_button_code_middle,

        ecm_mouse_button_code_cnt
    };

    constexpr t_i32 k_gamepad_limit = 16;

    enum t_gamepad_button_code : t_i32 {
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

    enum t_gamepad_axis_code : t_i32 {
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

    t_state *create_state(mem::t_arena *const arena);

    void clear_events(t_state *const state);

    t_b8 key_check_down(const t_state *const state, const t_key_code code);
    t_b8 key_check_pressed(const t_state *const state, const t_key_code code);
    t_b8 key_check_released(const t_state *const state, const t_key_code code);
    void key_update_state(t_state *const state, const t_key_code code, const t_b8 is_down);

    t_b8 mouse_button_check_down(const t_state *const state, const t_mouse_button_code btn_code);
    t_b8 mouse_button_check_pressed(const t_state *const state, const t_mouse_button_code btn_code);
    t_b8 mouse_button_check_released(const t_state *const state, const t_mouse_button_code btn_code);
    void mouse_button_update_state(t_state *const state, const t_mouse_button_code code, const t_b8 is_down);

    math::t_v2 cursor_get_pos(const t_state *const state);
    void cursor_update_state(t_state *const state, const math::t_v2 pos);

    math::t_v2 scroll_get_offset(const t_state *const state);
    void scroll_update_state(t_state *const state, const math::t_v2 offs_to_apply);

    t_b8 gamepad_check_connected(const t_state *const state, const t_i32 index);
    t_b8 gamepad_check_button_down(const t_state *const state, const t_i32 gamepad_index, const t_gamepad_button_code btn_code);
    t_b8 gamepad_check_button_pressed(const t_state *const state, const t_i32 gamepad_index, const t_gamepad_button_code btn_code);
    t_b8 gamepad_check_button_released(const t_state *const state, const t_i32 gamepad_index, const t_gamepad_button_code btn_code);
    t_f32 gamepad_get_axis_value_raw(const t_state *const state, const t_i32 gamepad_index, const t_gamepad_axis_code axis_code);
    t_f32 gamepad_get_axis_value_with_deadzone(const t_state *const state, const t_i32 gamepad_index, const t_gamepad_axis_code axis_code);
    void gamepad_update_state(t_state *const state, const t_i32 gamepad_index, const t_b8 connected, const mem::t_static_bitset<ecm_gamepad_button_code_cnt> &btns_down, const t_static_array<t_f32, ecm_gamepad_axis_code_cnt> &axes);

    // ============================================================
}
