#pragma once

#include <zcl.h>

namespace zgl {
    struct t_audio_ticket_rdonly {
        zcl::t_u64 val;
    };

    struct t_audio_ticket_mut {
        zcl::t_u64 val;

        operator t_audio_ticket_rdonly() const {
            return {val};
        }
    };


    // ============================================================
    // @section: Sound Types

    struct t_sound_type_group;
    struct t_sound_type;

    // The lifetime of the given arena's memory must encompass that of the group.
    t_sound_type_group *SoundTypeGroupCreate(const t_audio_ticket_mut audio_ticket, zcl::t_arena *const arena);

    void SoundTypeGroupDestroy(const t_audio_ticket_mut audio_ticket, t_sound_type_group *const group, zcl::t_arena *const temp_arena);

    t_sound_type *SoundTypeCreateFromBuilt(const t_audio_ticket_mut audio_ticket, const zcl::t_str_rdonly file_path, t_sound_type_group *const group, zcl::t_arena *const temp_arena);

    // Supports WAV, FLAC, and MP3.
    t_sound_type *SoundTypeCreateFromUnbuilt(const t_audio_ticket_mut audio_ticket, const zcl::t_str_rdonly file_path, t_sound_type_group *const group, zcl::t_arena *const temp_arena);

    // Supports WAV, FLAC, and MP3.
    // Prefer this for long audio pieces like music and ambience.
    t_sound_type *SoundTypeCreateStreamable(const t_audio_ticket_mut audio_ticket, const zcl::t_str_rdonly unbuilt_file_path, t_sound_type_group *const group, zcl::t_arena *const temp_arena);

    zcl::t_b8 SoundTypeCheckStreamable(const t_audio_ticket_rdonly audio_ticket, const t_sound_type *const type);

    // ============================================================


    // ============================================================
    // @section: Sound Instances

    enum t_sound_state : zcl::t_i32 {
        ek_sound_state_not_started,
        ek_sound_state_playing,
        ek_sound_state_paused
    };

    struct t_sound_id {
        zcl::t_i32 index;
        zcl::t_i32 version;
    };

    constexpr zcl::t_range k_sound_volume_range = zcl::RangeCreate(0.0f, 1.0f);
    constexpr zcl::t_range k_sound_pan_range = zcl::RangeCreate(-1.0f, 1.0f);
    constexpr zcl::t_range k_sound_pitch_range = zcl::RangeCreateExclLower(0.0f, zcl::k_f32_inf_pos);

    [[nodiscard]] zcl::t_b8 SoundCreate(const t_audio_ticket_mut audio_ticket, const t_sound_type *const type, t_sound_id *const o_id);
    void SoundDestroy(const t_audio_ticket_mut audio_ticket, const t_sound_id id);

    void SoundStart(const t_audio_ticket_mut audio_ticket, const t_sound_id id);
    void SoundPause(const t_audio_ticket_mut audio_ticket, const t_sound_id id);
    void SoundResume(const t_audio_ticket_mut audio_ticket, const t_sound_id id);

    zcl::t_b8 SoundCheckExists(const t_audio_ticket_rdonly audio_ticket, const t_sound_id id);
    t_sound_state SoundGetState(const t_audio_ticket_rdonly audio_ticket, const t_sound_id id);
    const t_sound_type *SoundGetType(const t_audio_ticket_rdonly audio_ticket, const t_sound_id id);
    zcl::t_f32 SoundGetVolume(const t_audio_ticket_rdonly audio_ticket, const t_sound_id id);
    zcl::t_f32 SoundGetPan(const t_audio_ticket_rdonly audio_ticket, const t_sound_id id);
    zcl::t_f32 SoundGetPitch(const t_audio_ticket_rdonly audio_ticket, const t_sound_id id);
    zcl::t_b8 SoundCheckLooping(const t_audio_ticket_rdonly audio_ticket, const t_sound_id id);

    // Gets track position in seconds.
    zcl::t_f32 SoundGetTrackPosition(const t_audio_ticket_rdonly audio_ticket, const t_sound_id id);

    // Gets track duration in seconds.
    zcl::t_f32 SoundGetTrackDuration(const t_audio_ticket_rdonly audio_ticket, const t_sound_id id);

    void SoundSetVolume(const t_audio_ticket_mut audio_ticket, const t_sound_id id, const zcl::t_f32 vol);
    void SoundSetVolumeTransition(const t_audio_ticket_mut audio_ticket, const t_sound_id id, const zcl::t_f32 vol_begin, const zcl::t_f32 vol_end, const zcl::t_f32 dur_secs);
    void SoundSetPan(const t_audio_ticket_mut audio_ticket, const t_sound_id id, const zcl::t_f32 pan);
    void SoundSetPitch(const t_audio_ticket_mut audio_ticket, const t_sound_id id, const zcl::t_f32 pitch);
    void SoundSetLooping(const t_audio_ticket_mut audio_ticket, const t_sound_id id, const zcl::t_b8 looping);
    void SoundSetTrackPosition(const t_audio_ticket_mut audio_ticket, const t_sound_id id, const zcl::t_f32 pos_secs);

    inline zcl::t_b8 SoundFireAndForget(const t_audio_ticket_mut audio_ticket, const t_sound_type *const type, const zcl::t_f32 vol = 1.0f, const zcl::t_f32 pan = 0.0f, const zcl::t_f32 pitch = 1.0f) {
        t_sound_id id;

        if (!SoundCreate(audio_ticket, type, &id)) {
            return false;
        }

        SoundSetVolume(audio_ticket, id, vol);
        SoundSetPan(audio_ticket, id, pan);
        SoundSetPitch(audio_ticket, id, pitch);

        return true;
    }

    zcl::t_array_mut<t_sound_id> SoundsGetExisting(const t_audio_ticket_rdonly audio_ticket, zcl::t_arena *const arena);

    void SoundsDestroyAll(const t_audio_ticket_mut audio_ticket, zcl::t_arena *const temp_arena);
    void SoundsDestroyAllOfType(const t_audio_ticket_mut audio_ticket, const t_sound_type *const snd_type, zcl::t_arena *const temp_arena);

    void SoundsPauseAll(const t_audio_ticket_mut audio_ticket, zcl::t_arena *const temp_arena);
    void SoundsPauseAllOfType(const t_audio_ticket_mut audio_ticket, const t_sound_type *const snd_type, zcl::t_arena *const temp_arena);

    void SoundsResumeAll(const t_audio_ticket_mut audio_ticket, zcl::t_arena *const temp_arena);
    void SoundsResumeAllOfType(const t_audio_ticket_mut audio_ticket, const t_sound_type *const snd_type, zcl::t_arena *const temp_arena);

    // ============================================================


    namespace internal {
        t_audio_ticket_mut AudioStartup(zcl::t_arena *const arena);
        void AudioShutdown(const t_audio_ticket_mut ticket);
        void AudioSetFrozen(const t_audio_ticket_mut ticket, const zcl::t_b8 paused);

        void SoundsProcessFinished(const t_audio_ticket_mut audio_ticket);
    };
}
