#pragma once

#include <zgl.h>

namespace zf {
    // ============================================================
    // @section: Input
    // ============================================================
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

    // ============================================================
    // @section: Audio
    // ============================================================
    [[nodiscard]] t_b8 InitAudioSys(s_mem_arena& mem_arena, s_audio_sys*& o_as);
    void ShutdownAudioSys(s_audio_sys& as);
    void ProcFinishedSounds(s_audio_sys& as);
}
