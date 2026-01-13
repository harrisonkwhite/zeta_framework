#pragma once

#include <zcl.h>

// @todo: Consider making global.
// @todo: Also consider bringing back detail subnamespace, it's actually useful.

namespace zgl::input {
    // ============================================================
    // @section: Types and Constants

    struct t_state;

    enum t_key_code : zcl::t_i32 {
        ek_key_code_space,
        ek_key_code_0,
        ek_key_code_1,
        ek_key_code_2,
        ek_key_code_3,
        ek_key_code_4,
        ek_key_code_5,
        ek_key_code_6,
        ek_key_code_7,
        ek_key_code_8,
        ek_key_code_9,
        ek_key_code_a,
        ek_key_code_b,
        ek_key_code_c,
        ek_key_code_d,
        ek_key_code_e,
        ek_key_code_f,
        ek_key_code_g,
        ek_key_code_h,
        ek_key_code_i,
        ek_key_code_j,
        ek_key_code_k,
        ek_key_code_l,
        ek_key_code_m,
        ek_key_code_n,
        ek_key_code_o,
        ek_key_code_p,
        ek_key_code_q,
        ek_key_code_r,
        ek_key_code_s,
        ek_key_code_t,
        ek_key_code_u,
        ek_key_code_v,
        ek_key_code_w,
        ek_key_code_x,
        ek_key_code_y,
        ek_key_code_z,
        ek_key_code_escape,
        ek_key_code_enter,
        ek_key_code_backspace,
        ek_key_code_tab,
        ek_key_code_right,
        ek_key_code_left,
        ek_key_code_down,
        ek_key_code_up,
        ek_key_code_f1,
        ek_key_code_f2,
        ek_key_code_f3,
        ek_key_code_f4,
        ek_key_code_f5,
        ek_key_code_f6,
        ek_key_code_f7,
        ek_key_code_f8,
        ek_key_code_f9,
        ek_key_code_f10,
        ek_key_code_f11,
        ek_key_code_f12,
        ek_key_code_left_shift,
        ek_key_code_left_control,
        ek_key_code_left_alt,
        ek_key_code_right_shift,
        ek_key_code_right_control,
        ek_key_code_right_alt,

        ekm_key_code_cnt
    };

    enum t_mouse_button_code : zcl::t_i32 {
        ek_mouse_button_code_left,
        ek_mouse_button_code_right,
        ek_mouse_button_code_middle,

        ekm_mouse_button_code_cnt
    };

    constexpr zcl::t_i32 k_gamepad_limit = 16;

    enum t_gamepad_button_code : zcl::t_i32 {
        ek_gamepad_button_code_a,
        ek_gamepad_button_code_b,
        ek_gamepad_button_code_x,
        ek_gamepad_button_code_y,
        ek_gamepad_button_code_left_bumper,
        ek_gamepad_button_code_right_bumper,
        ek_gamepad_button_code_back,
        ek_gamepad_button_code_start,
        ek_gamepad_button_code_guide,
        ek_gamepad_button_code_left_thumb,
        ek_gamepad_button_code_right_thumb,
        ek_gamepad_button_code_dpad_up,
        ek_gamepad_button_code_dpad_right,
        ek_gamepad_button_code_dpad_down,
        ek_gamepad_button_code_dpad_left,

        ekm_gamepad_button_code_cnt
    };

    enum t_gamepad_axis_code : zcl::t_i32 {
        ek_gamepad_axis_code_left_x,
        ek_gamepad_axis_code_left_y,
        ek_gamepad_axis_code_right_x,
        ek_gamepad_axis_code_right_y,
        ek_gamepad_axis_code_left_trigger,
        ek_gamepad_axis_code_right_trigger,

        ekm_gamepad_axis_code_cnt
    };

    // ============================================================


    // ============================================================
    // @section: Functions

    t_state *create_state(zcl::t_arena *const arena);

    void clear_events(t_state *const state);

    zcl::t_b8 key_check_down(const t_state *const state, const t_key_code code);
    zcl::t_b8 key_check_pressed(const t_state *const state, const t_key_code code);
    zcl::t_b8 key_check_released(const t_state *const state, const t_key_code code);
    void key_update_state(t_state *const state, const t_key_code code, const zcl::t_b8 is_down);

    zcl::t_b8 mouse_button_check_down(const t_state *const state, const t_mouse_button_code btn_code);
    zcl::t_b8 mouse_button_check_pressed(const t_state *const state, const t_mouse_button_code btn_code);
    zcl::t_b8 mouse_button_check_released(const t_state *const state, const t_mouse_button_code btn_code);
    void mouse_button_update_state(t_state *const state, const t_mouse_button_code code, const zcl::t_b8 is_down);

    zcl::t_v2 cursor_get_pos(const t_state *const state);
    void cursor_update_state(t_state *const state, const zcl::t_v2 pos);

    zcl::t_v2 scroll_get_offset(const t_state *const state);
    void scroll_update_state(t_state *const state, const zcl::t_v2 offs_to_apply);

    zcl::t_b8 gamepad_check_connected(const t_state *const state, const zcl::t_i32 index);
    zcl::t_b8 gamepad_check_button_down(const t_state *const state, const zcl::t_i32 gamepad_index, const t_gamepad_button_code btn_code);
    zcl::t_b8 gamepad_check_button_pressed(const t_state *const state, const zcl::t_i32 gamepad_index, const t_gamepad_button_code btn_code);
    zcl::t_b8 gamepad_check_button_released(const t_state *const state, const zcl::t_i32 gamepad_index, const t_gamepad_button_code btn_code);
    zcl::t_f32 gamepad_get_axis_value_raw(const t_state *const state, const zcl::t_i32 gamepad_index, const t_gamepad_axis_code axis_code);
    zcl::t_f32 gamepad_get_axis_value_with_deadzone(const t_state *const state, const zcl::t_i32 gamepad_index, const t_gamepad_axis_code axis_code);
    void gamepad_update_state(t_state *const state, const zcl::t_i32 gamepad_index, const zcl::t_b8 connected, const zcl::t_static_bitset<ekm_gamepad_button_code_cnt> &btns_down, const zcl::t_static_array<zcl::t_f32, ekm_gamepad_axis_code_cnt> &axes);

    // @todo: The function names below are bad.

    zcl::t_array_rdonly<zcl::strs::t_code_pt> text_get_code_pts(const t_state *const state);

    // Returns true iff there is enough room for the code point and it is added.
    zcl::t_b8 text_submit_code_point(t_state *const state, const zcl::strs::t_code_pt cp);

    // ============================================================
}
