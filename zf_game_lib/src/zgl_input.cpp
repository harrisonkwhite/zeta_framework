#include <zgl/private/zgl_input.h>

namespace zf {
    struct s_input_state {
        s_static_bit_vec<eks_key_code_cnt> keys_down;
        s_static_bit_vec<eks_mouse_button_code_cnt> mouse_buttons_down;

        s_v2<t_f32> cursor_pos;

        struct {
            s_static_bit_vec<eks_key_code_cnt> keys_pressed;
            s_static_bit_vec<eks_key_code_cnt> keys_released;

            s_static_bit_vec<eks_mouse_button_code_cnt> mouse_buttons_pressed;
            s_static_bit_vec<eks_mouse_button_code_cnt> mouse_buttons_released;

            s_v2<t_f32> scroll;
        } events;
    };

    [[nodiscard]] t_b8 MakeInputState(s_mem_arena& mem_arena, s_input_state*& o_is) {
        o_is = PushToMemArena<s_input_state>(mem_arena);
        return o_is != nullptr;
    }

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

    s_v2<t_f32> CursorPos(const s_input_state& is) {
        return is.cursor_pos;
    }

    s_v2<t_f32> GetScroll(const s_input_state& is) {
        return is.events.scroll;
    }

    void ClearInputEvents(s_input_state& is) {
        ZeroOut(is.events);
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

    void ProcCursorMove(s_input_state& is, const s_v2<t_f32> pos) {
        is.cursor_pos = pos;
    }

    void ProcScroll(s_input_state& is, const s_v2<t_f32> scroll) {
        is.events.scroll += scroll;
    }
}
