#include <zgl/zgl_platform_private.h>

#include <GLFW/glfw3.h> // Only for the input macros, no global state!
#include <zgl/zgl_input.h>

namespace zgl {
    zcl::t_i32 ToGLFWKey(const t_key_code key_code) {
        switch (key_code) {
            case ek_key_code_space: {
                return GLFW_KEY_SPACE;
            }

            case ek_key_code_0: {
                return GLFW_KEY_0;
            }

            case ek_key_code_1: {
                return GLFW_KEY_1;
            }

            case ek_key_code_2: {
                return GLFW_KEY_2;
            }

            case ek_key_code_3: {
                return GLFW_KEY_3;
            }

            case ek_key_code_4: {
                return GLFW_KEY_4;
            }

            case ek_key_code_5: {
                return GLFW_KEY_5;
            }

            case ek_key_code_6: {
                return GLFW_KEY_6;
            }

            case ek_key_code_7: {
                return GLFW_KEY_7;
            }

            case ek_key_code_8: {
                return GLFW_KEY_8;
            }

            case ek_key_code_9: {
                return GLFW_KEY_9;
            }

            case ek_key_code_a: {
                return GLFW_KEY_A;
            }

            case ek_key_code_b: {
                return GLFW_KEY_B;
            }

            case ek_key_code_c: {
                return GLFW_KEY_C;
            }

            case ek_key_code_d: {
                return GLFW_KEY_D;
            }

            case ek_key_code_e: {
                return GLFW_KEY_E;
            }

            case ek_key_code_f: {
                return GLFW_KEY_F;
            }

            case ek_key_code_g: {
                return GLFW_KEY_G;
            }

            case ek_key_code_h: {
                return GLFW_KEY_H;
            }

            case ek_key_code_i: {
                return GLFW_KEY_I;
            }

            case ek_key_code_j: {
                return GLFW_KEY_J;
            }

            case ek_key_code_k: {
                return GLFW_KEY_K;
            }

            case ek_key_code_l: {
                return GLFW_KEY_L;
            }

            case ek_key_code_m: {
                return GLFW_KEY_M;
            }

            case ek_key_code_n: {
                return GLFW_KEY_N;
            }

            case ek_key_code_o: {
                return GLFW_KEY_O;
            }

            case ek_key_code_p: {
                return GLFW_KEY_P;
            }

            case ek_key_code_q: {
                return GLFW_KEY_Q;
            }

            case ek_key_code_r: {
                return GLFW_KEY_R;
            }

            case ek_key_code_s: {
                return GLFW_KEY_S;
            }

            case ek_key_code_t: {
                return GLFW_KEY_T;
            }

            case ek_key_code_u: {
                return GLFW_KEY_U;
            }

            case ek_key_code_v: {
                return GLFW_KEY_V;
            }

            case ek_key_code_w: {
                return GLFW_KEY_W;
            }

            case ek_key_code_x: {
                return GLFW_KEY_X;
            }

            case ek_key_code_y: {
                return GLFW_KEY_Y;
            }

            case ek_key_code_z: {
                return GLFW_KEY_Z;
            }

            case ek_key_code_escape: {
                return GLFW_KEY_ESCAPE;
            }

            case ek_key_code_enter: {
                return GLFW_KEY_ENTER;
            }

            case ek_key_code_backspace: {
                return GLFW_KEY_BACKSPACE;
            }

            case ek_key_code_tab: {
                return GLFW_KEY_TAB;
            }

            case ek_key_code_right: {
                return GLFW_KEY_RIGHT;
            }

            case ek_key_code_left: {
                return GLFW_KEY_LEFT;
            }

            case ek_key_code_down: {
                return GLFW_KEY_DOWN;
            }

            case ek_key_code_up: {
                return GLFW_KEY_UP;
            }

            case ek_key_code_f1: {
                return GLFW_KEY_F1;
            }

            case ek_key_code_f2: {
                return GLFW_KEY_F2;
            }

            case ek_key_code_f3: {
                return GLFW_KEY_F3;
            }

            case ek_key_code_f4: {
                return GLFW_KEY_F4;
            }

            case ek_key_code_f5: {
                return GLFW_KEY_F5;
            }

            case ek_key_code_f6: {
                return GLFW_KEY_F6;
            }

            case ek_key_code_f7: {
                return GLFW_KEY_F7;
            }

            case ek_key_code_f8: {
                return GLFW_KEY_F8;
            }

            case ek_key_code_f9: {
                return GLFW_KEY_F9;
            }

            case ek_key_code_f10: {
                return GLFW_KEY_F10;
            }

            case ek_key_code_f11: {
                return GLFW_KEY_F11;
            }

            case ek_key_code_f12: {
                return GLFW_KEY_F12;
            }

            case ek_key_code_left_shift: {
                return GLFW_KEY_LEFT_SHIFT;
            }

            case ek_key_code_left_control: {
                return GLFW_KEY_LEFT_CONTROL;
            }

            case ek_key_code_left_alt: {
                return GLFW_KEY_LEFT_ALT;
            }

            case ek_key_code_right_shift: {
                return GLFW_KEY_RIGHT_SHIFT;
            }

            case ek_key_code_right_control: {
                return GLFW_KEY_RIGHT_CONTROL;
            }

            case ek_key_code_right_alt: {
                return GLFW_KEY_RIGHT_ALT;
            }

            case ek_key_code_grave: {
                return GLFW_KEY_GRAVE_ACCENT;
            }

            case ek_key_code_minus: {
                return GLFW_KEY_MINUS;
            }

            case ek_key_code_equals: {
                return GLFW_KEY_EQUAL;
            }

            case ek_key_code_left_bracket: {
                return GLFW_KEY_LEFT_BRACKET;
            }

            case ek_key_code_right_bracket: {
                return GLFW_KEY_RIGHT_BRACKET;
            }

            case ek_key_code_backslash: {
                return GLFW_KEY_BACKSLASH;
            }

            case ek_key_code_semicolon: {
                return GLFW_KEY_SEMICOLON;
            }

            case ek_key_code_apostrophe: {
                return GLFW_KEY_APOSTROPHE;
            }

            case ek_key_code_comma: {
                return GLFW_KEY_COMMA;
            }

            case ek_key_code_period: {
                return GLFW_KEY_PERIOD;
            }

            case ek_key_code_slash: {
                return GLFW_KEY_SLASH;
            }

            case ek_key_code_insert: {
                return GLFW_KEY_INSERT;
            }

            case ek_key_code_delete: {
                return GLFW_KEY_DELETE;
            }

            case ek_key_code_home: {
                return GLFW_KEY_HOME;
            }

            case ek_key_code_end: {
                return GLFW_KEY_END;
            }

            case ek_key_code_page_up: {
                return GLFW_KEY_PAGE_UP;
            }

            case ek_key_code_page_down: {
                return GLFW_KEY_PAGE_DOWN;
            }

            case ek_key_code_caps_lock: {
                return GLFW_KEY_CAPS_LOCK;
            }

            case ek_key_code_num_lock: {
                return GLFW_KEY_NUM_LOCK;
            }

            case ek_key_code_scroll_lock: {
                return GLFW_KEY_SCROLL_LOCK;
            }

            case ek_key_code_print_screen: {
                return GLFW_KEY_PRINT_SCREEN;
            }

            case ek_key_code_pause: {
                return GLFW_KEY_PAUSE;
            }

            case ek_key_code_left_super: {
                return GLFW_KEY_LEFT_SUPER;
            }

            case ek_key_code_right_super: {
                return GLFW_KEY_RIGHT_SUPER;
            }

            case ek_key_code_menu: {
                return GLFW_KEY_MENU;
            }

            case ek_key_code_numpad_0: {
                return GLFW_KEY_KP_0;
            }

            case ek_key_code_numpad_1: {
                return GLFW_KEY_KP_1;
            }

            case ek_key_code_numpad_2: {
                return GLFW_KEY_KP_2;
            }

            case ek_key_code_numpad_3: {
                return GLFW_KEY_KP_3;
            }

            case ek_key_code_numpad_4: {
                return GLFW_KEY_KP_4;
            }

            case ek_key_code_numpad_5: {
                return GLFW_KEY_KP_5;
            }

            case ek_key_code_numpad_6: {
                return GLFW_KEY_KP_6;
            }

            case ek_key_code_numpad_7: {
                return GLFW_KEY_KP_7;
            }

            case ek_key_code_numpad_8: {
                return GLFW_KEY_KP_8;
            }

            case ek_key_code_numpad_9: {
                return GLFW_KEY_KP_9;
            }

            case ek_key_code_numpad_decimal: {
                return GLFW_KEY_KP_DECIMAL;
            }

            case ek_key_code_numpad_divide: {
                return GLFW_KEY_KP_DIVIDE;
            }

            case ek_key_code_numpad_multiply: {
                return GLFW_KEY_KP_MULTIPLY;
            }

            case ek_key_code_numpad_subtract: {
                return GLFW_KEY_KP_SUBTRACT;
            }

            case ek_key_code_numpad_add: {
                return GLFW_KEY_KP_ADD;
            }

            case ek_key_code_numpad_enter: {
                return GLFW_KEY_KP_ENTER;
            }

            case ek_key_code_numpad_equals: {
                return GLFW_KEY_KP_EQUAL;
            }

            default: {
                ZCL_UNREACHABLE();
            }
        }
    }

    zcl::t_i32 ToGLFWMouseButton(const t_mouse_button_code btn_code) {
        switch (btn_code) {
            case ek_mouse_button_code_left: {
                return GLFW_MOUSE_BUTTON_LEFT;
            }

            case ek_mouse_button_code_right: {
                return GLFW_MOUSE_BUTTON_RIGHT;
            }

            case ek_mouse_button_code_middle: {
                return GLFW_MOUSE_BUTTON_MIDDLE;
            }

            default: {
                ZCL_UNREACHABLE();
            }
        }
    };
}
