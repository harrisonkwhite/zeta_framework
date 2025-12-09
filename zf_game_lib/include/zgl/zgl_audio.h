#pragma once

#include <zcl.h>

namespace zf {
#if 0
    struct s_audio_sys;

    struct s_sound_type;

    struct s_sound_type_arena {
        s_audio_sys *audio_sys;
        s_mem_arena *mem_arena;
        s_sound_type *head;
        s_sound_type *tail;
    };

    // The memory lifetime of the given memory arena must encompass that of the sound type
    // arena.
    inline s_sound_type_arena CreateSoundTypeArena(s_audio_sys *const as, s_mem_arena *const mem_arena) {
        return {.audio_sys = as, .mem_arena = mem_arena};
    }

    s_sound_type *CreateSoundTypeFromRaw(const s_str_rdonly file_path, s_sound_type_arena *const type_arena, s_mem_arena *const temp_mem_arena);

    void DestroySoundTypes(s_sound_type_arena *const type_arena);

    struct s_sound_id {
        s_audio_sys *audio_sys;

        t_len index;
        t_len version;
    };

    [[nodiscard]] t_b8 PlaySound(const s_sound_type *const type, s_sound_id *const id = nullptr, const t_f32 vol = 1.0f, const t_f32 pan = 0.0f, const t_f32 pitch = 1.0f, const t_b8 loop = false);
    void StopSound(const s_sound_id id);
    t_b8 IsSoundPlaying(const s_sound_id id);

    namespace internal {
        s_audio_sys *CreateAudioSys(s_mem_arena *const mem_arena);
        void DestroyAudioSys(s_audio_sys *const as);
        void ProcFinishedSounds(s_audio_sys *const as);
    }
#endif
}
