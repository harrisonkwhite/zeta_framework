#pragma once

#include <zc.h>

namespace zf::audio {
    using t_sound_type_id = t_size;

    [[nodiscard]] t_b8 InitSys();
    void ShutdownSys();
    void ProcFinishedSounds();
    [[nodiscard]] t_b8 RegisterSoundType(const s_sound_data& snd_data, t_sound_type_id& o_id);
    void UnregisterSoundType(const t_sound_type_id id);
    [[nodiscard]] t_b8 PlaySound(const t_sound_type_id type_id, const t_f32 vol = 1.0f, const t_f32 pan = 0.0f, const t_f32 pitch = 1.0f, const t_b8 loop = false);
}
