#include <zgl/zgl_input.h>

namespace zf {
    void RegKeyEvent(s_input_state &is, const e_key_event_type type) {
        switch (act) {
        case GLFW_PRESS:
            SetBit(g_state.input_state->mouse_buttons_down, mbc);
            SetBit(g_state.input_state->events.mouse_buttons_pressed, mbc);
            break;

        case GLFW_RELEASE:
            UnsetBit(g_state.input_state->mouse_buttons_down, mbc);
            SetBit(g_state.input_state->events.mouse_buttons_released, mbc);
            break;
        }
    }

    void RegMouseButtonEvent(s_input_state &is, const e_mouse_button_event_type type) {
    }

    void RegScrollEvent(s_input_state &is, const s_v2 offs) {
    }
}
