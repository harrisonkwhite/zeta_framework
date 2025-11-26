#pragma once

#include <zc.h>

namespace zf {
    enum e_window_flags : t_u8 {
        ek_window_flags_none = 0,
        ek_window_flags_resizable = 1 << 0,
        ek_window_flags_hide_cursor = 1 << 1
    };

    enum e_key_code : t_s32 {
        eks_key_code_none = -1,

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

    enum e_mouse_button_code : t_s32 {
        eks_mouse_button_code_none = -1,

        ek_mouse_button_code_left,
        ek_mouse_button_code_right,
        ek_mouse_button_code_middle,

        eks_mouse_button_code_cnt
    };

    enum e_mouse_scroll_state : t_s32 {
        ek_mouse_scroll_state_none,
        ek_mouse_scroll_state_down,
        ek_mouse_scroll_state_up
    };

    t_b8 InitWindow(const s_v2<t_s32> size, const s_str_rdonly title, const e_window_flags flags, s_mem_arena& temp_mem_arena);
    void ReleaseWindow();

    void* GetNativeWindowHandle();
    void* GetNativeDisplayHandle();

    void PollEvents();
    void SwapBuffers();

    void ShowWindow();
    t_b8 ShouldWindowClose();
    void SetWindowShouldClose(const t_b8 close);

    s_v2<t_s32> GetWindowSize();
    s_v2<t_s32> GetFramebufferSize();

    t_f64 GetTime();

    void ClearInputEvents();

    t_b8 IsKeyDown(const e_key_code kc);
    t_b8 IsKeyPressed(const e_key_code kc);
    t_b8 IsKeyReleased(const e_key_code kc);

    t_b8 IsMouseButtonDown(const e_mouse_button_code mbc);
    t_b8 IsMouseButtonPressed(const e_mouse_button_code mbc);
    t_b8 IsMouseButtonReleased(const e_mouse_button_code mbc);

    s_v2<t_f64> GetMousePos();
    e_mouse_scroll_state GetMouseScrollState();
}
