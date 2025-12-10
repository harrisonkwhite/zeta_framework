#pragma once

#include <zcl.h>

namespace zf {
    struct s_audio_sys;

    [[nodiscard]] t_b8 CreateAudioSys(const s_ptr_nonnull<s_mem_arena> mem_arena, const s_ptr_nonnull<s_ptr<s_audio_sys>> o_as);
    void DestroyAudioSys(const s_ptr_nonnull<s_audio_sys> as);

    struct s_sound_type;

    struct s_sound_type_arena {
        s_ptr<s_audio_sys> audio_sys = nullptr;
        s_ptr<s_mem_arena> mem_arena = nullptr;
        s_ptr<s_sound_type> head = nullptr;
        s_ptr<s_sound_type> tail = nullptr;
    };

    // The memory lifetime of the given memory arena must encompass that of the sound type arena.
    inline s_sound_type_arena CreateSoundTypeArena(const s_ptr_nonnull<s_audio_sys> as, const s_ptr_nonnull<s_mem_arena> mem_arena) {
        return {.audio_sys = as, .mem_arena = mem_arena};
    }

    [[nodiscard]] t_b8 CreateSoundTypeFromRaw(const s_str_rdonly file_path, const s_ptr_nonnull<s_sound_type_arena> type_arena, const s_ptr_nonnull<s_mem_arena> temp_mem_arena, const s_ptr_nonnull<s_ptr<s_sound_type>> o_type);
    void DestroySoundTypes(const s_ptr_nonnull<s_sound_type_arena> type_arena);

    struct s_sound_id {
        s_ptr<s_audio_sys> audio_sys = nullptr;

        t_len index = 0;
        t_len version = 0;
    };

    [[nodiscard]] t_b8 PlaySound(const s_ptr_nonnull<const s_sound_type> type, const s_ptr<s_sound_id> o_id = nullptr, const t_f32 vol = 1.0f, const t_f32 pan = 0.0f, const t_f32 pitch = 1.0f, const t_b8 loop = false);
    void StopSound(const s_sound_id id);
    t_b8 IsSoundPlaying(const s_sound_id id);

    void ProcFinishedSounds(const s_ptr_nonnull<s_audio_sys> as);
}
