#pragma once

#include <zcl.h>

namespace zf::platform {
    // ============================================================
    // @section: General
    // ============================================================

    // Gives the time in seconds since the platform module was initialised.
    t_f64 Time();

    namespace internal {
        // OpenGL version chosen for maximum compatibility.
        constexpr t_i32 g_gl_version_major = 3;
        constexpr t_i32 g_gl_version_minor = 3;
        constexpr t_b8 g_gl_core_profile = true;

        [[nodiscard]] t_b8 Init(const s_v2_i init_window_size);
        void Shutdown();

        using t_gl_proc = void (*)();
        using t_get_gl_proc_addr_func = t_gl_proc (*)(const char *name);
        t_get_gl_proc_addr_func GetGLProcAddrFunc();

        void PollOSEvents();
    }

    // ============================================================
    // @section: Display
    // ============================================================

    // Returns true iff the operation succeeded.
    [[nodiscard]] t_b8 SetWindowTitle(const s_str_rdonly title, s_mem_arena &temp_mem_arena);

    // Sets the LOGICAL window size. The actual new framebuffer size MIGHT be larger if there is DPI scaling.
    void SetWindowSize(const s_v2_i size);

    // Set the LOGICAL window size limits. If you don't want to limit a particular dimension, leave -1.
    void SetWindowSizeLimits(const t_i32 min_width, const t_i32 min_height, const t_i32 max_width, const t_i32 max_height);

    void SetWindowResizability(const t_b8 resizable);

    s_v2_i WindowFramebufferSizeCache();

    // Returns the size in pixels of whichever monitor the window most resides in.
    s_v2_i CalcMonitorPixelSize();

    // Returns the size (accounting for DPI scaling) of whichever monitor the window most resides in.
    s_v2_i CalcMonitorLogicalSize();

    t_b8 IsFullscreen();
    void SetFullscreen(const t_b8 fs);

    inline void ToggleFullscreen() {
        SetFullscreen(!IsFullscreen());
    }

    namespace internal {
        void ShowWindow();
        t_b8 ShouldWindowClose();
        void SwapWindowBuffers();
    }

    // ============================================================
    // @section: Input
    // ============================================================
    enum e_key_code : t_i32 {
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

    t_b8 IsKeyDown(const e_key_code kc);
    t_b8 IsKeyPressed(const e_key_code kc);
    t_b8 IsKeyReleased(const e_key_code kc);

    enum e_mouse_button_code : t_i32 {
        eks_mouse_button_code_none = -1,

        ek_mouse_button_code_left,
        ek_mouse_button_code_right,
        ek_mouse_button_code_middle,

        eks_mouse_button_code_cnt
    };

    t_b8 IsMouseButtonDown(const e_mouse_button_code mbc);
    t_b8 IsMouseButtonPressed(const e_mouse_button_code mbc);
    t_b8 IsMouseButtonReleased(const e_mouse_button_code mbc);

    s_v2 CursorPos();
    void SetCursorVisibility(const t_b8 visible);

    // +Y: Scroll up / away from you
    // -Y: Scroll down / towards you
    // +X: Scroll right
    // +X: Scroll left
    s_v2 ScrollOffset();

    namespace internal {
        void ClearInputEvents();
    }
}
