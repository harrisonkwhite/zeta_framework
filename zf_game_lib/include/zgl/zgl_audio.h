#pragma once

#include <zcl.h>

namespace zgl {
    struct t_audio_sys;


    // ============================================================
    // @section: Sound Types

    struct t_sound_type_group;
    struct t_sound_type;

    // The lifetime of the given arena's memory must encompass that of the group.
    t_sound_type_group *SoundTypeGroupCreate(t_audio_sys *const audio_sys, zcl::t_arena *const arena);

    void SoundTypeGroupDestroy(t_audio_sys *const audio_sys, t_sound_type_group *const group);

    // Supports WAV, FLAC, and MP3.
    t_sound_type *SoundTypeCreateFromExternal(t_audio_sys *const audio_sys, const zcl::t_str_rdonly file_path, t_sound_type_group *const group, zcl::t_arena *const temp_arena);

    t_sound_type *SoundTypeCreateFromPacked(t_audio_sys *const audio_sys, const zcl::t_str_rdonly file_path, t_sound_type_group *const group, zcl::t_arena *const temp_arena);

    // Supports WAV, FLAC, and MP3.
    // Prefer this for long audio pieces like music and ambience.
    t_sound_type *SoundTypeCreateStreamable(t_audio_sys *const audio_sys, const zcl::t_str_rdonly external_file_path, t_sound_type_group *const group, zcl::t_arena *const temp_arena);

    zcl::t_b8 SoundTypeCheckStreamable(t_audio_sys *const audio_sys, const t_sound_type *const type);

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

    t_sound_id SoundCreate(t_audio_sys *const audio_sys, const t_sound_type *const type);

    void SoundDestroy(t_audio_sys *const audio_sys, const t_sound_id id);

    void SoundStart(t_audio_sys *const audio_sys, const t_sound_id id);

    void SoundPause(t_audio_sys *const audio_sys, const t_sound_id id);

    void SoundResume(t_audio_sys *const audio_sys, const t_sound_id id);

    zcl::t_b8 SoundCheckExists(const t_audio_sys *const audio_sys, const t_sound_id id);

    t_sound_state SoundGetState(const t_audio_sys *const audio_sys, const t_sound_id id);

    zcl::t_f32 SoundGetVolume(t_audio_sys *const audio_sys, const t_sound_id id);

    zcl::t_f32 SoundGetPan(t_audio_sys *const audio_sys, const t_sound_id id);

    zcl::t_f32 SoundGetPitch(t_audio_sys *const audio_sys, const t_sound_id id);

    zcl::t_b8 SoundCheckLooping(t_audio_sys *const audio_sys, const t_sound_id id);

    // Gets track position in seconds.
    zcl::t_f32 SoundGetTrackPosition(t_audio_sys *const audio_sys, const t_sound_id id);

    // Gets track duration in seconds.
    zcl::t_f32 SoundGetTrackDuration(t_audio_sys *const audio_sys, const t_sound_id id);

    void SoundSetVolume(t_audio_sys *const audio_sys, const t_sound_id id, const zcl::t_f32 vol);

    void SoundSetVolumeTransition(t_audio_sys *const audio_sys, const t_sound_id id, const zcl::t_f32 vol_begin, const zcl::t_f32 vol_end, const zcl::t_f32 dur_secs);

    void SoundSetPan(t_audio_sys *const audio_sys, const t_sound_id id, const zcl::t_f32 pan);

    void SoundSetPitch(t_audio_sys *const audio_sys, const t_sound_id id, const zcl::t_f32 pitch);

    void SoundSetLooping(t_audio_sys *const audio_sys, const t_sound_id id, const zcl::t_b8 loop);

    void SoundSetTrackPosition(t_audio_sys *const audio_sys, const t_sound_id id, const zcl::t_f32 pos_secs);

    void SoundsDestroyAll(t_audio_sys *const audio_sys);

    void SoundsPauseAll(t_audio_sys *const audio_sys);

    void SoundsResumeAll(t_audio_sys *const audio_sys);

    // ============================================================


    namespace detail {
        t_audio_sys *AudioStartup(zcl::t_arena *const arena);

        void AudioShutdown(t_audio_sys *const sys);

        void AudioSetMuted(t_audio_sys *const sys, const zcl::t_b8 mute);

        void SoundsProcessFinished(t_audio_sys *const audio_sys);
    };
}
