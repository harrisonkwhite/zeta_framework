#pragma once

#include <zc.h>

namespace zf::audio {
    t_b8 InitSys();
    void ShutdownSys();
    void ProcFinishedSounds();

    [[nodiscard]] t_b8 PlaySound(const s_sound_data_rdonly& snd_data, const t_f32 vol = 1.0f, const t_f32 pan = 0.0f, const t_f32 pitch = 1.0f, const t_b8 loop = false);
}
