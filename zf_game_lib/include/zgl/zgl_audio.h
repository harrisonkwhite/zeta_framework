#pragma once

#include <zcl.h>

namespace zf
{
    struct s_sound_type;

    struct s_sound_type_arena
    {
        s_mem_arena* mem_arena;
        s_sound_type* head;
        s_sound_type* tail;
    };

    // The memory lifetime of the given memory arena must encompass that of the sound type arena.
    inline s_sound_type_arena MakeSoundTypeArena(s_mem_arena& mem_arena)
    {
        return {.mem_arena = &mem_arena};
    }

    [[nodiscard]] t_b8 CreateSoundTypeFromRaw(const s_str_rdonly file_path,
        s_sound_type_arena& type_arena,
        s_mem_arena& temp_mem_arena,
        s_sound_type*& o_type);

    struct s_audio_sys;

    void DestroySoundTypes(s_audio_sys& as, s_sound_type_arena& type_arena);

    struct s_sound_id;

    [[nodiscard]] t_b8 PlaySound(s_audio_sys& as,
        const s_sound_type* const type,
        s_sound_id* const o_id = nullptr,
        const t_f32 vol = 1.0f,
        const t_f32 pan = 0.0f,
        const t_f32 pitch = 1.0f,
        const t_b8 loop = false);
    void StopSound(s_audio_sys& as, const s_sound_id id);
    t_b8 IsSoundPlaying(s_audio_sys& as, const s_sound_id id);

    [[nodiscard]] t_b8 P_InitAudioSys(s_mem_arena& mem_arena, s_audio_sys*& o_as);
    void P_ShutdownAudioSys(s_audio_sys& as);
    void P_ProcFinishedSounds(s_audio_sys& as);
}
