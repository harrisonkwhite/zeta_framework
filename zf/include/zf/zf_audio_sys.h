#pragma once

#include <zc.h>

namespace zf::audio_sys {
    [[nodiscard]] t_b8 Init();
    void Shutdown();

    struct s_sound_type_arena {
        t_size cnt = 0;
        s_array<s_sound_meta> metas;
        s_array<s_array<t_f32>> pcms;
        s_mem_arena* mem_arena; // We don't know how large the PCM will need to be for each sound type, so we try pushing it each time here.
    };

    inline t_size SoundTypeArenaCap(const s_sound_type_arena& arena) {
        ZF_ASSERT(arena.metas.len == arena.pcms.len);
        return arena.metas.len;
    }

    [[nodiscard]] t_b8 MakeSoundTypeArena(const t_size snd_type_limit, s_mem_arena& mem_arena, s_sound_type_arena& o_arena);
    [[nodiscard]] t_b8 RegisterSoundType(s_sound_type_arena& arena, const s_sound_meta& meta, const s_array_rdonly<t_f32> pcm, t_size& o_index);

    void ProcFinishedSounds();
    [[nodiscard]] t_b8 PlaySound(const s_sound_type_arena& type_arena, const t_size type_index, const t_f32 vol = 1.0f, const t_f32 pan = 0.0f, const t_f32 pitch = 1.0f, const t_b8 loop = false);
}
