#pragma once

#include <zcl.h>

namespace zgl::audio {
    void ModuleStartup();
    void ModuleShutdown();

    struct t_sound_type_group;

    // The lifetime of the given arena's memory must encompass that of the group.
    t_sound_type_group *SoundTypeGroupCreate(zcl::t_arena *const arena);

    void SoundTypeGroupDestroy(t_sound_type_group *const group);

    struct t_sound_type;

    t_sound_type *SoundTypeCreateFromRaw(const zcl::t_str_rdonly file_path, t_sound_type_group *const group, zcl::t_arena *const temp_arena);
    t_sound_type *SoundTypeCreateFromPacked(const zcl::t_str_rdonly file_path, t_sound_type_group *const group, zcl::t_arena *const temp_arena);
    t_sound_type *SoundTypeCreateStreamable(const zcl::t_str_rdonly file_path, t_sound_type_group *const group, zcl::t_arena *const temp_arena);

    struct t_sound_id {
        zcl::t_i32 index;
        zcl::t_i32 version;
    };

    // Returns true iff the play succeeded. Note that some failure cases will trigger a fatal error instead.
    [[nodiscard]] zcl::t_b8 SoundPlayAndGetID(const t_sound_type *const type, t_sound_id *const o_id, const zcl::t_f32 vol = 1.0f, const zcl::t_f32 pan = 0.0f, const zcl::t_f32 pitch = 1.0f, const zcl::t_b8 loop = false);

    // Returns true iff the play succeeded. Note that some failure cases will trigger a fatal error instead.
    inline zcl::t_b8 SoundPlay(const t_sound_type *const type, const zcl::t_f32 vol = 1.0f, const zcl::t_f32 pan = 0.0f, const zcl::t_f32 pitch = 1.0f, const zcl::t_b8 loop = false) {
        t_sound_id id_throwaway;
        return SoundPlayAndGetID(type, &id_throwaway, vol, pan, pitch, loop);
    }

    void SoundStop(const t_sound_id id);

    // @todo: Functions for modifying volume, etc.

    zcl::t_b8 SoundCheckPlaying(const t_sound_id id);

    void SoundsProcFinished();
}
