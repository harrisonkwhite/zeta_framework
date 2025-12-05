#pragma once

#include <zcl.h>

namespace zf {
    struct s_audio_sys;

    [[nodiscard]] t_b8 InitAudioSys(s_mem_arena& mem_arena, s_audio_sys*& o_as);
    void ShutdownAudioSys(s_audio_sys& as);
    void ProcFinishedSounds(s_audio_sys& as);
}
