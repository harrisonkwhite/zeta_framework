#include <zgl_private.h>

namespace zf {
    t_b8 IsKeyDown(const s_input_state& is, const e_key_code kc) {
        return IsBitSet(is.keys_down, kc);
    }

    t_b8 IsKeyPressed(const s_input_state& is, const e_key_code kc) {
        return IsBitSet(is.events.keys_pressed, kc);
    }

    t_b8 IsKeyReleased(const s_input_state& is, const e_key_code kc) {
        return IsBitSet(is.events.keys_released, kc);
    }

    t_b8 IsMouseButtonDown(const s_input_state& is, const e_mouse_button_code mbc) {
        return IsBitSet(is.mouse_buttons_down, mbc);
    }

    t_b8 IsMouseButtonPressed(const s_input_state& is, const e_mouse_button_code mbc) {
        return IsBitSet(is.events.mouse_buttons_pressed, mbc);
    }

    t_b8 IsMouseButtonReleased(const s_input_state& is, const e_mouse_button_code mbc) {
        return IsBitSet(is.events.mouse_buttons_released, mbc);
    }

    s_v2<t_f32> MousePos(const s_input_state& is) {
        return is.mouse_pos;
    }

    void ProcKeyAction(s_input_state& is, const e_key_code code, const e_key_action act) {
        if (code == eks_key_code_none) {
            return;
        }

        switch (act) {
            case e_key_action::invalid:
                ZF_ASSERT(false);
                break;

            case e_key_action::press:
                SetBit(is.keys_down, code);
                SetBit(is.events.keys_pressed, code);
                break;

            case e_key_action::release:
                UnsetBit(is.keys_down, code);
                SetBit(is.events.keys_released, code);
                break;
        }
    }

    void ProcMouseButtonAction(s_input_state& is, const e_mouse_button_code code, const e_mouse_button_action act) {
        if (code == eks_mouse_button_code_none) {
            return;
        }

        switch (act) {
            case e_mouse_button_action::invalid:
                ZF_ASSERT(false);
                break;

            case e_mouse_button_action::press:
                SetBit(is.mouse_buttons_down, code);
                SetBit(is.events.mouse_buttons_pressed, code);
                break;

            case e_mouse_button_action::release:
                SetBit(is.mouse_buttons_down, code);
                SetBit(is.events.mouse_buttons_pressed, code);
                break;
        }
    }
}
