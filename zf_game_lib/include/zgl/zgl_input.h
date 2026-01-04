#pragma once

#include <zcl.h>

namespace zf {
    // ============================================================
    // @section: Types and Globals

    struct zf_input_state;

    enum zf_input_key_code : t_i32 {
        zf_input_key_code_space,
        zf_input_key_code_0,
        zf_input_key_code_1,
        zf_input_key_code_2,
        zf_input_key_code_3,
        zf_input_key_code_4,
        zf_input_key_code_5,
        zf_input_key_code_6,
        zf_input_key_code_7,
        zf_input_key_code_8,
        zf_input_key_code_9,
        zf_input_key_code_a,
        zf_input_key_code_b,
        zf_input_key_code_c,
        zf_input_key_code_d,
        zf_input_key_code_e,
        zf_input_key_code_f,
        zf_input_key_code_g,
        zf_input_key_code_h,
        zf_input_key_code_i,
        zf_input_key_code_j,
        zf_input_key_code_k,
        zf_input_key_code_l,
        zf_input_key_code_m,
        zf_input_key_code_n,
        zf_input_key_code_o,
        zf_input_key_code_p,
        zf_input_key_code_q,
        zf_input_key_code_r,
        zf_input_key_code_s,
        zf_input_key_code_t,
        zf_input_key_code_u,
        zf_input_key_code_v,
        zf_input_key_code_w,
        zf_input_key_code_x,
        zf_input_key_code_y,
        zf_input_key_code_z,
        zf_input_key_code_escape,
        zf_input_key_code_enter,
        zf_input_key_code_backspace,
        zf_input_key_code_tab,
        zf_input_key_code_right,
        zf_input_key_code_left,
        zf_input_key_code_down,
        zf_input_key_code_up,
        zf_input_key_code_f1,
        zf_input_key_code_f2,
        zf_input_key_code_f3,
        zf_input_key_code_f4,
        zf_input_key_code_f5,
        zf_input_key_code_f6,
        zf_input_key_code_f7,
        zf_input_key_code_f8,
        zf_input_key_code_f9,
        zf_input_key_code_f10,
        zf_input_key_code_f11,
        zf_input_key_code_f12,
        zf_input_key_code_left_shift,
        zf_input_key_code_left_control,
        zf_input_key_code_left_alt,
        zf_input_key_code_right_shift,
        zf_input_key_code_right_control,
        zf_input_key_code_right_alt,

        zf_input_key_code_cnt
    };

    enum e_mouse_button_code : t_i32 {
        ek_mouse_button_code_left,
        ek_mouse_button_code_right,
        ek_mouse_button_code_middle,

        eks_mouse_button_code_cnt
    };

    constexpr t_i32 g_gamepad_limit = 16;

    enum e_gamepad_button_code : t_i32 {
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

        eks_gamepad_button_code_cnt
    };

    enum e_gamepad_axis_code : t_i32 {
        ek_gamepad_axis_code_left_x,
        ek_gamepad_axis_code_left_y,
        ek_gamepad_axis_code_right_x,
        ek_gamepad_axis_code_right_y,
        ek_gamepad_axis_code_left_trigger,
        ek_gamepad_axis_code_right_trigger,

        eks_gamepad_axis_code_cnt
    };

    // ============================================================


    // ============================================================
    // @section: Functions

    zf_input_state *zf_input_create_state(s_arena *const arena);

    void zf_input_clear_events(zf_input_state *const state);

    B8 zf_input_get_key_is_down(const zf_input_state *const state, const zf_input_key_code code);
    B8 zf_input_get_key_is_pressed(const zf_input_state *const state, const zf_input_key_code code);
    B8 zf_input_get_key_is_released(const zf_input_state *const state, const zf_input_key_code code);
    void zf_input_update_key_state(zf_input_state *const state, const zf_input_key_code code, const B8 is_down);

    B8 IsMouseButtonDown(const zf_input_state *const state, const e_mouse_button_code btn_code);
    B8 IsMouseButtonPressed(const zf_input_state *const state, const e_mouse_button_code btn_code);
    B8 IsMouseButtonReleased(const zf_input_state *const state, const e_mouse_button_code btn_code);
    void UpdateMouseButtonState(zf_input_state *const state, const e_mouse_button_code code, const B8 is_down);

    s_v2 CursorPos(const zf_input_state *const state);
    void UpdateCursorPos(zf_input_state *const state, const s_v2 val);

    s_v2 zf_input_get_scroll_offset(const zf_input_state *const state);
    void zf_input_update_scroll_offset(zf_input_state *const state, const s_v2 offs_to_apply);

    B8 IsGamepadConnected(const zf_input_state *const state, const t_i32 index);
    B8 IsGamepadButtonDown(const zf_input_state *const state, const t_i32 gamepad_index, const e_gamepad_button_code btn_code);
    B8 IsGamepadButtonPressed(const zf_input_state *const state, const t_i32 gamepad_index, const e_gamepad_button_code btn_code);
    B8 IsGamepadButtonReleased(const zf_input_state *const state, const t_i32 gamepad_index, const e_gamepad_button_code btn_code);
    t_f32 zf_input_get_gamepad_axis_value_raw(const zf_input_state *const state, const t_i32 gamepad_index, const e_gamepad_axis_code axis_code);
    t_f32 zf_input_calc_gamepad_axis_value_with_deadzone(const zf_input_state *const state, const t_i32 gamepad_index, const e_gamepad_axis_code axis_code);
    void UpdateGamepadState(zf_input_state *const state, const t_i32 gamepad_index, const B8 connected, const s_static_bit_vec<eks_gamepad_button_code_cnt> &btns_down, const s_static_array<t_f32, eks_gamepad_axis_code_cnt> &axes);

    // ============================================================
}
