#include <zgl/zgl_input.h>

namespace zf {
    void internal::ProcKeyAction(s_input_state &is, const e_key_code code, const e_key_action act) {
        if (code == eks_key_code_none) {
            return;
        }

        switch (act) {
        case ek_key_action_invalid:
            ZF_ASSERT(false);
            break;

        case ek_key_action_press:
            SetBit(is.keys_down, code);
            SetBit(is.events.keys_pressed, code);
            break;

        case ek_key_action_release:
            UnsetBit(is.keys_down, code);
            SetBit(is.events.keys_released, code);
            break;
        }
    }

    void internal::ProcMouseButtonAction(s_input_state &is, const e_mouse_button_code code, const e_mouse_button_action act) {
        if (code == eks_mouse_button_code_none) {
            return;
        }

        switch (act) {
        case ek_mouse_button_action_invalid:
            ZF_ASSERT(false);
            break;

        case ek_mouse_button_action_press:
            SetBit(is.mouse_buttons_down, code);
            SetBit(is.events.mouse_buttons_pressed, code);
            break;

        case ek_mouse_button_action_release:
            SetBit(is.mouse_buttons_down, code);
            SetBit(is.events.mouse_buttons_pressed, code);
            break;
        }
    }

    void internal::ProcCursorMove(s_input_state &is, const s_v2 pos) {
        is.cursor_pos = pos;
    }

    void internal::ProcScroll(s_input_state &is, const s_v2 scroll) {
        is.events.scroll += scroll;
    }
}
