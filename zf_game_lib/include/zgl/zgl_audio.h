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

    // @todo: I don't know, maybe sound instances could start in preinit state, then a playing/paused alternation?
    // Mainly think this because SoundsPlayAll() is a bit strange because it will start non-setup sounds. SoundsResumeAll() makes more sense.
    // Probably don't use multiple bit vectors. Simpler to just have a state enum. Because bit vectors would encode/imply invalid states e.g. inactive but started.

    struct t_sound_id {
        zcl::t_i32 index;
        zcl::t_i32 version;
    };

    t_sound_id SoundCreate(t_audio_sys *const audio_sys, const t_sound_type *const type);

    void SoundDestroy(t_audio_sys *const audio_sys, const t_sound_id id);

    zcl::t_b8 SoundCheckExists(const t_audio_sys *const audio_sys, const t_sound_id id);

    // The sound must not already be playing when calling this.
    void SoundPlay(t_audio_sys *const audio_sys, const t_sound_id id);

    // The sound must be playing when calling this.
    void SoundStop(t_audio_sys *const audio_sys, const t_sound_id id);

    // Also returns false for non-existent sounds.
    zcl::t_b8 SoundCheckPlaying(const t_audio_sys *const audio_sys, const t_sound_id id);

    void SoundsDestroyAll(t_audio_sys *const audio_sys);

#if 0
    // Returns true iff the play succeeded. Note that some failure cases will trigger a fatal error instead.
    [[nodiscard]] zcl::t_b8 SoundPlayAndGetID(t_audio_sys *const audio_sys, const t_sound_type *const snd_type, t_sound_id *const o_id, const zcl::t_f32 vol = 1.0f, const zcl::t_f32 pan = 0.0f, const zcl::t_f32 pitch = 1.0f, const zcl::t_b8 loop = false);

    // @todo: Helper for checking how many sound slots are left.

    // @todo: Maybe split up playing vs. setup phase.

    // @todo: Sound restart function.

    // Returns true iff the play succeeded. Note that some failure cases will trigger a fatal error instead.
    inline zcl::t_b8 SoundPlay(t_audio_sys *const audio_sys, const t_sound_type *const snd_type, const zcl::t_f32 vol = 1.0f, const zcl::t_f32 pan = 0.0f, const zcl::t_f32 pitch = 1.0f, const zcl::t_b8 loop = false) {
        t_sound_id id_throwaway;
        return SoundPlayAndGetID(audio_sys, snd_type, &id_throwaway, vol, pan, pitch, loop);
    }

    // Completely deactivates the sound instance, including if it is paused.
    void SoundStop(t_audio_sys *const audio_sys, const t_sound_id id);

    void SoundPause(t_audio_sys *const audio_sys, const t_sound_id id);

    void SoundResume(t_audio_sys *const audio_sys, const t_sound_id id);

    // Note that this still returns true if the sound is paused.
    zcl::t_b8 SoundCheckActive(const t_audio_sys *const audio_sys, const t_sound_id id);

    // Also returns false if the sound is inactive.
    zcl::t_b8 SoundCheckPaused(const t_audio_sys *const audio_sys, const t_sound_id id);

    // This is fine to call when the sound is paused.
    zcl::t_f32 SoundGetVolume(t_audio_sys *const audio_sys, const t_sound_id id);

    // This is fine to call when the sound is paused.
    zcl::t_f32 SoundGetPan(t_audio_sys *const audio_sys, const t_sound_id id);

    // This is fine to call when the sound is paused.
    zcl::t_f32 SoundGetPitch(t_audio_sys *const audio_sys, const t_sound_id id);

    // Gets track position in seconds.
    // This is fine to call when the sound is paused.
    zcl::t_f32 SoundGetTrackPosition(t_audio_sys *const audio_sys, const t_sound_id id);

    // Gets track duration in seconds.
    // This is fine to call when the sound is paused.
    zcl::t_f32 SoundGetTrackDuration(t_audio_sys *const audio_sys, const t_sound_id id);

    // This is fine to call when the sound is paused.
    void SoundSetVolume(t_audio_sys *const audio_sys, const t_sound_id id, const zcl::t_f32 vol);

    #if 0
    // This is fine to call when the sound is paused.
    void SoundSetVolumeTransition(t_audio_sys *const audio_sys, const t_sound_id id, const zcl::t_f32 vol_begin, const zcl::t_f32 vol_end, const zcl::t_f32 dur_secs);
    #endif

    // This is fine to call when the sound is paused.
    void SoundSetPan(t_audio_sys *const audio_sys, const t_sound_id id, const zcl::t_f32 pan);

    // This is fine to call when the sound is paused.
    void SoundSetPitch(t_audio_sys *const audio_sys, const t_sound_id id, const zcl::t_f32 pitch);

    // This is fine to call when the sound is paused.
    void SoundSetTrackPosition(t_audio_sys *const audio_sys, const t_sound_id id, const zcl::t_f32 pos_secs);

    void SoundsStopAll(t_audio_sys *const audio_sys);

    void SoundsPauseAll(t_audio_sys *const audio_sys);

    void SoundsResumeAll(t_audio_sys *const audio_sys);
#endif

    // ============================================================


    namespace detail {
        t_audio_sys *AudioStartup(zcl::t_arena *const arena);

        void AudioShutdown(t_audio_sys *const sys);

        void SoundsProcessFinished(t_audio_sys *const audio_sys);
    };
}
