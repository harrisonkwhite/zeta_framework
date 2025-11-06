#pragma once

#include <GLFW/glfw3.h>

#if defined(_WIN32)
    #define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__linux__)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(__APPLE__)
    #define GLFW_EXPOSE_NATIVE_COCOA
#endif

#include <GLFW/glfw3native.h>
#include <zc/debug.h>
#include <zc/math.h>
#include <zc/mem/strs.h>

namespace zf {
    enum e_window_flags {
        ek_window_flags_resizable = 1 << 0,
        ek_window_flags_hide_cursor = 1 << 1
    };

    enum e_key_code {
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

    using t_key_bits = uint64_t;

    static_assert(eks_key_code_cnt < ZF_SIZE_IN_BITS(t_key_bits), "Too many key codes!");

    enum e_mouse_button_code {
        eks_mouse_button_code_none = -1,

        ek_mouse_button_code_left,
        ek_mouse_button_code_right,
        ek_mouse_button_code_middle,

        eks_mouse_button_code_cnt
    };

    using t_mouse_button_bits = t_byte;

    static_assert(eks_mouse_button_code_cnt < ZF_SIZE_IN_BITS(t_mouse_button_bits), "Too many mouse button codes!");

    enum class ec_mouse_scroll_state {
        none,
        down,
        up
    };

    struct s_input_events {
        t_key_bits keys_pressed = 0;
        t_key_bits keys_released = 0;

        t_mouse_button_bits mouse_buttons_pressed = 0;
        t_mouse_button_bits mouse_buttons_released = 0;

        ec_mouse_scroll_state mouse_scroll_state = ec_mouse_scroll_state::none;

        s_static_array<char, 32> unicode_buf;
    };

    static constexpr int ToGLFWKey(const e_key_code kc) {
        switch (kc) {
            case ek_key_code_space: return GLFW_KEY_SPACE;
            case ek_key_code_0: return GLFW_KEY_0;
            case ek_key_code_1: return GLFW_KEY_1;
            case ek_key_code_2: return GLFW_KEY_2;
            case ek_key_code_3: return GLFW_KEY_3;
            case ek_key_code_4: return GLFW_KEY_4;
            case ek_key_code_5: return GLFW_KEY_5;
            case ek_key_code_6: return GLFW_KEY_6;
            case ek_key_code_7: return GLFW_KEY_7;
            case ek_key_code_8: return GLFW_KEY_8;
            case ek_key_code_9: return GLFW_KEY_9;
            case ek_key_code_a: return GLFW_KEY_A;
            case ek_key_code_b: return GLFW_KEY_B;
            case ek_key_code_c: return GLFW_KEY_C;
            case ek_key_code_d: return GLFW_KEY_D;
            case ek_key_code_e: return GLFW_KEY_E;
            case ek_key_code_f: return GLFW_KEY_F;
            case ek_key_code_g: return GLFW_KEY_G;
            case ek_key_code_h: return GLFW_KEY_H;
            case ek_key_code_i: return GLFW_KEY_I;
            case ek_key_code_j: return GLFW_KEY_J;
            case ek_key_code_k: return GLFW_KEY_K;
            case ek_key_code_l: return GLFW_KEY_L;
            case ek_key_code_m: return GLFW_KEY_M;
            case ek_key_code_n: return GLFW_KEY_N;
            case ek_key_code_o: return GLFW_KEY_O;
            case ek_key_code_p: return GLFW_KEY_P;
            case ek_key_code_q: return GLFW_KEY_Q;
            case ek_key_code_r: return GLFW_KEY_R;
            case ek_key_code_s: return GLFW_KEY_S;
            case ek_key_code_t: return GLFW_KEY_T;
            case ek_key_code_u: return GLFW_KEY_U;
            case ek_key_code_v: return GLFW_KEY_V;
            case ek_key_code_w: return GLFW_KEY_W;
            case ek_key_code_x: return GLFW_KEY_X;
            case ek_key_code_y: return GLFW_KEY_Y;
            case ek_key_code_z: return GLFW_KEY_Z;
            case ek_key_code_escape: return GLFW_KEY_ESCAPE;
            case ek_key_code_enter: return GLFW_KEY_ENTER;
            case ek_key_code_backspace: return GLFW_KEY_BACKSPACE;
            case ek_key_code_tab: return GLFW_KEY_TAB;
            case ek_key_code_right: return GLFW_KEY_RIGHT;
            case ek_key_code_left: return GLFW_KEY_LEFT;
            case ek_key_code_down: return GLFW_KEY_DOWN;
            case ek_key_code_up: return GLFW_KEY_UP;
            case ek_key_code_f1: return GLFW_KEY_F1;
            case ek_key_code_f2: return GLFW_KEY_F2;
            case ek_key_code_f3: return GLFW_KEY_F3;
            case ek_key_code_f4: return GLFW_KEY_F4;
            case ek_key_code_f5: return GLFW_KEY_F5;
            case ek_key_code_f6: return GLFW_KEY_F6;
            case ek_key_code_f7: return GLFW_KEY_F7;
            case ek_key_code_f8: return GLFW_KEY_F8;
            case ek_key_code_f9: return GLFW_KEY_F9;
            case ek_key_code_f10: return GLFW_KEY_F10;
            case ek_key_code_f11: return GLFW_KEY_F11;
            case ek_key_code_f12: return GLFW_KEY_F12;
            case ek_key_code_left_shift: return GLFW_KEY_LEFT_SHIFT;
            case ek_key_code_left_control: return GLFW_KEY_LEFT_CONTROL;
            case ek_key_code_left_alt: return GLFW_KEY_LEFT_ALT;
            case ek_key_code_right_shift: return GLFW_KEY_RIGHT_SHIFT;
            case ek_key_code_right_control: return GLFW_KEY_RIGHT_CONTROL;
            case ek_key_code_right_alt: return GLFW_KEY_RIGHT_ALT;

            default: return GLFW_KEY_UNKNOWN;
        }
    }

    static constexpr int ToGLFWMouseButton(const e_mouse_button_code mbc) {
        switch (mbc) {
            case ek_mouse_button_code_left: return GLFW_MOUSE_BUTTON_LEFT;
            case ek_mouse_button_code_right: return GLFW_MOUSE_BUTTON_RIGHT;
            case ek_mouse_button_code_middle: return GLFW_MOUSE_BUTTON_MIDDLE;

            default: return -1;
        }
    }

    class c_window {
    public:
        c_window() = delete;
        c_window(const c_window&) = delete;
        c_window& operator=(const c_window&) = delete;

        [[nodiscard]] static bool Init(const s_v2<int> size, const s_str_view title, const e_window_flags flags);
        static void Clean();

        static void* GetNativeWindowHandle() {
#if defined(_WIN32)
            return glfwGetWin32Window(sm_glfw_window);
#elif defined(__linux__)
            return glfwGetX11Window(sm_glfw_window);
#elif defined(__APPLE__)
            return glfwGetCocoaWindow(sm_glfw_window);
#endif
        }

        static void* GetNativeDisplayHandle() {
#if defined(_WIN32)
            return nullptr;
#elif defined(__linux__)
            return glfwGetX11Display();
#elif defined(__APPLE__)
            return nullptr;
#endif
        }

        static void Show() {
            glfwShowWindow(sm_glfw_window);
        }

        static void PollEvents() {
            glfwPollEvents();
        }

        static bool ShouldClose() {
            return glfwWindowShouldClose(sm_glfw_window);
        }

        static void SetWindowShouldClose(const bool close) {
            glfwSetWindowShouldClose(sm_glfw_window, close);
        }

        static double GetTime() {
            return glfwGetTime();
        }

        static s_v2<int> GetSize() {
            int w, h;
            glfwGetWindowSize(sm_glfw_window, &w, &h);
            return {w, h};
        }

        static s_v2<int> GetFramebufferSize() {
            int w, h;
            glfwGetFramebufferSize(sm_glfw_window, &w, &h);
            return {w, h};
        }

        static void ClearInputEvents() {
            sm_input_events = {};
        }

        static bool IsKeyDown(const e_key_code kc) {
            ZF_ASSERT(kc != eks_key_code_none);
            return glfwGetKey(sm_glfw_window, ToGLFWKey(kc)) != GLFW_RELEASE;
        }

        static bool IsKeyPressed(const e_key_code kc) {
            ZF_ASSERT(kc != eks_key_code_none);

            const t_key_bits key_mask = static_cast<t_key_bits>(1) << kc;
            return (sm_input_events.keys_pressed & key_mask) != 0;
        }

        static bool IsKeyReleased(const e_key_code kc) {
            ZF_ASSERT(kc != eks_key_code_none);

            const t_key_bits key_mask = static_cast<t_key_bits>(1) << kc;
            return (sm_input_events.keys_released & key_mask) != 0;
        }

        static bool IsMouseButtonDown(const e_mouse_button_code mbc) {
            ZF_ASSERT(mbc != eks_mouse_button_code_none);
            return glfwGetMouseButton(sm_glfw_window, ToGLFWMouseButton(mbc)) != GLFW_RELEASE;
        }

        static bool IsMouseButtonPressed(const e_mouse_button_code mbc) {
            ZF_ASSERT(mbc != eks_mouse_button_code_none);

            const t_mouse_button_bits mb_mask = static_cast<t_mouse_button_bits>(1) << mbc;
            return (sm_input_events.mouse_buttons_pressed & mb_mask) != 0;
        }

        static bool IsMouseButtonReleased(const e_mouse_button_code mbc) {
            ZF_ASSERT(mbc != eks_mouse_button_code_none);

            const t_mouse_button_bits mb_mask = static_cast<t_mouse_button_bits>(1) << mbc;
            return (sm_input_events.mouse_buttons_released & mb_mask) != 0;
        }

        template<co_floating_point tp_type>
        static s_v2<tp_type> GetMousePos() {
            double mx, my;
            glfwGetCursorPos(sm_glfw_window, &mx, &my);
            return {static_cast<tp_type>(mx), static_cast<tp_type>(my)};
        }

    private:
        static inline GLFWwindow* sm_glfw_window = nullptr;
        static inline s_input_events sm_input_events; // Events such as key presses and mouse wheel scrolls are recordeded here in callbacks.
    };
}
