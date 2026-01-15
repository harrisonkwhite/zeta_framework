#pragma once

#include <zcl.h>

namespace zgl {
    struct t_audio_sys;
    struct t_sound_type_group;
    struct t_sound_type;

    // The lifetime of the given arena's memory must encompass that of the group.
    t_sound_type_group *SoundTypeGroupCreate(t_audio_sys *const audio_sys, zcl::t_arena *const arena);

    void SoundTypeGroupDestroy(t_audio_sys *const audio_sys, t_sound_type_group *const group);

    // @todo: Could give a warning or something to use streamable instead if PCM is really huge.
    t_sound_type *SoundTypeCreateFromExternal(t_audio_sys *const audio_sys, const zcl::t_str_rdonly file_path, t_sound_type_group *const group, zcl::t_arena *const temp_arena);
    t_sound_type *SoundTypeCreateFromPacked(t_audio_sys *const audio_sys, const zcl::t_str_rdonly file_path, t_sound_type_group *const group, zcl::t_arena *const temp_arena);

    // Prefer this for long audio pieces like music and ambience.
    t_sound_type *SoundTypeCreateStreamable(t_audio_sys *const audio_sys, const zcl::t_str_rdonly external_file_path, t_sound_type_group *const group, zcl::t_arena *const temp_arena);

    struct t_sound_id {
        zcl::t_i32 index;
        zcl::t_i32 version;
    };

    // Returns true iff the play succeeded. Note that some failure cases will trigger a fatal error instead.
    [[nodiscard]] zcl::t_b8 SoundPlayAndGetID(t_audio_sys *const audio_sys, const t_sound_type *const type, t_sound_id *const o_id, const zcl::t_f32 vol = 1.0f, const zcl::t_f32 pan = 0.0f, const zcl::t_f32 pitch = 1.0f, const zcl::t_b8 loop = false);

    // Returns true iff the play succeeded. Note that some failure cases will trigger a fatal error instead.
    inline zcl::t_b8 SoundPlay(t_audio_sys *const audio_sys, const t_sound_type *const type, const zcl::t_f32 vol = 1.0f, const zcl::t_f32 pan = 0.0f, const zcl::t_f32 pitch = 1.0f, const zcl::t_b8 loop = false) {
        t_sound_id id_throwaway;
        return SoundPlayAndGetID(audio_sys, type, &id_throwaway, vol, pan, pitch, loop);
    }

    void SoundStop(t_audio_sys *const audio_sys, const t_sound_id id);

    // @todo: Functions for modifying volume, etc.

    zcl::t_b8 SoundCheckPlaying(const t_audio_sys *const audio_sys, const t_sound_id id);

    namespace detail {
        t_audio_sys *AudioStartup(zcl::t_arena *const arena);
        void AudioShutdown(t_audio_sys *const sys);

        void SoundsProcessFinished(t_audio_sys *const audio_sys);
    };
}
