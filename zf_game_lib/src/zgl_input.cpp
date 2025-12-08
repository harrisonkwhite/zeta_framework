#include <zgl/zgl_input.h>

namespace zf {
    void internal::ProcKeyAction(s_input_state *const is, const e_key_code code,
                                 const e_key_action act) {
        if (code == eks_key_code_none) {
            return;
        }

        switch (act) {
        case e_key_action::invalid:
            ZF_ASSERT(false);
            break;

        case e_key_action::press:
            SetBit(is->keys_down, code);
            SetBit(is->events.keys_pressed, code);
            break;

        case e_key_action::release:
            UnsetBit(is->keys_down, code);
            SetBit(is->events.keys_released, code);
            break;
        }
    }

    void internal::ProcMouseButtonAction(s_input_state *const is,
                                         const e_mouse_button_code code,
                                         const e_mouse_button_action act) {
        if (code == eks_mouse_button_code_none) {
            return;
        }

        switch (act) {
        case e_mouse_button_action::invalid:
            ZF_ASSERT(false);
            break;

        case e_mouse_button_action::press:
            SetBit(is->mouse_buttons_down, code);
            SetBit(is->events.mouse_buttons_pressed, code);
            break;

        case e_mouse_button_action::release:
            SetBit(is->mouse_buttons_down, code);
            SetBit(is->events.mouse_buttons_pressed, code);
            break;
        }
    }

    void internal::ProcCursorMove(s_input_state *const is, const s_v2 pos) {
        is->cursor_pos = pos;
    }

    void internal::ProcScroll(s_input_state *const is, const s_v2 scroll) {
        is->events.scroll += scroll;
    }
}
