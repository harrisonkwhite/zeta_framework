#include <zgl/zgl_input.h>

namespace zf {
    void RegKeyEvent(s_input_state &is, const e_key_code kc, const e_key_event_type type) {
        switch (type) {
        case ek_key_event_type_press:
            SetBit(is.keys_down, kc);
            SetBit(is.events.keys_pressed, kc);
            return;

        case ek_key_event_type_release:
            UnsetBit(is.keys_down, kc);
            SetBit(is.events.keys_released, kc);
            return;

        case ek_key_event_type_repeat: return;
        }

        ZF_UNREACHABLE();
    }

    void RegMouseButtonEvent(s_input_state &is, const e_mouse_button_code mbc, const e_mouse_button_event_type type) {
        switch (type) {
        case ek_mouse_button_event_type_press:
            SetBit(is.mouse_buttons_down, mbc);
            SetBit(is.events.mouse_buttons_pressed, mbc);
            return;

        case ek_mouse_button_event_type_release:
            UnsetBit(is.mouse_buttons_down, mbc);
            SetBit(is.events.mouse_buttons_released, mbc);
            return;
        }

        ZF_UNREACHABLE();
    }

    void RegScrollEvent(s_input_state &is, const s_v2 offs) {
        is.events.scroll += offs;
    }
}
