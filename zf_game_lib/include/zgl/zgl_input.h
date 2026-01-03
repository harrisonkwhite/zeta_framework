#pragma once

#include <zcl.h>

namespace zf {
    // ============================================================
    // @section: Types and Globals

    struct s_input_state;

    enum e_key_code : t_i32 {
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

        eks_key_code_cnt
    };

    enum e_mouse_button_code : t_i32 {
        ek_mouse_button_code_left,
        ek_mouse_button_code_right,
        ek_mouse_button_code_middle,

        eks_mouse_button_code_cnt
    };

    inline const t_i32 g_gamepad_limit = 16;

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

    t_b8 KeyIsDown(const s_input_state *const input_state, const e_key_code code);
    t_b8 KeyIsPressed(const s_input_state *const input_state, const e_key_code code);
    t_b8 KeyIsReleased(const s_input_state *const input_state, const e_key_code code);

    t_b8 MouseButtonIsDown(const s_input_state *const input_state, const e_mouse_button_code btn_code);
    t_b8 MouseButtonIsPressed(const s_input_state *const input_state, const e_mouse_button_code btn_code);
    t_b8 MouseButtonIsReleased(const s_input_state *const input_state, const e_mouse_button_code btn_code);

    s_v2 CursorPos(const s_input_state *const input_state);

    s_v2 ScrollOffset(const s_input_state *const input_state);

    t_b8 GamepadIsConnected(const s_input_state *const input_state, const t_i32 index);
    t_b8 GamepadIsButtonDown(const s_input_state *const input_state, const t_i32 gamepad_index, const e_gamepad_button_code btn_code);
    t_b8 GamepadIsButtonPressed(const s_input_state *const input_state, const t_i32 gamepad_index, const e_gamepad_button_code btn_code);
    t_b8 GamepadIsButtonReleased(const s_input_state *const input_state, const t_i32 gamepad_index, const e_gamepad_button_code btn_code);
    t_f32 GamepadAxisValueRaw(const s_input_state *const input_state, const t_i32 gamepad_index, const e_gamepad_axis_code axis_code);
    t_f32 GamepadAxisValueWithDeadzone(const s_input_state *const input_state, const t_i32 gamepad_index, const e_gamepad_axis_code axis_code);

    namespace detail {
        s_input_state *CreateInputState(s_arena *const arena);
        void InputStateClearEvents(s_input_state *const input_state);

        void KeyUpdateState(s_input_state *const input_state, const e_key_code code, const t_b8 is_down);

        void MouseButtonUpdateState(s_input_state *const input_state, const e_mouse_button_code code, const t_b8 is_down);

        void CursorPosUpdate(s_input_state *const input_state, const s_v2 val);

        void ScrollOffsetUpdate(s_input_state *const input_state, const s_v2 offs_to_apply);

        void GamepadUpdateState(s_input_state *const input_state, const t_i32 gamepad_index, const t_b8 connected, const s_static_bit_vec<eks_gamepad_button_code_cnt> &btns_down, const s_static_array<t_f32, eks_gamepad_axis_code_cnt> &axes);
    }

    // ============================================================
}
