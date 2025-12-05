#pragma once

#include <zgl/public/zgl_input.h>

namespace zf {
    [[nodiscard]] t_b8 MakeInputState(s_mem_arena& mem_arena, s_input_state*& o_is);
    void ClearInputEvents(s_input_state& is);

    enum class e_key_action {
        invalid,
        press,
        release
    };

    enum class e_mouse_button_action {
        invalid,
        press,
        release
    };

    void ProcKeyAction(s_input_state& is, const e_key_code code, const e_key_action act);
    void ProcMouseButtonAction(s_input_state& is, const e_mouse_button_code code, const e_mouse_button_action act);
    void ProcCursorMove(s_input_state& is, const s_v2<t_f32> pos);
    void ProcScroll(s_input_state& is, const s_v2<t_f32> scroll);
}
