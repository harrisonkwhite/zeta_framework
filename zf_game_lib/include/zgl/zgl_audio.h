#pragma once

#include <zcl.h>

namespace zgl::audio {
    // ============================================================
    // @section: Types and Constants

    struct t_sound_type;

    struct t_sound_type_group {
        zf::mem::t_arena arena;
        t_sound_type *head;
        t_sound_type *tail;
    };

    struct t_sound_id {
        zf::t_i32 index;
        zf::t_i32 version;
    };

    // ============================================================


    // ============================================================
    // @section: Functions

    void module_startup();
    void module_shutdown();

    inline t_sound_type_group sound_type_group_create() {
        return {.arena = zf::mem::arena_create_blockbased()};
    }

    void sound_type_group_destroy(t_sound_type_group *const group);

    t_sound_type *sound_type_create_from_raw(const zf::strs::t_str_rdonly file_path, t_sound_type_group *const group, zf::mem::t_arena *const temp_arena);
    t_sound_type *sound_type_create_from_packed(const zf::strs::t_str_rdonly file_path, t_sound_type_group *const group, zf::mem::t_arena *const temp_arena);

    // Returns true iff the play succeeded. Note that some failure cases will trigger a fatal error instead.
    [[nodiscard]] zf::t_b8 sound_play_and_get_id(const t_sound_type *const type, t_sound_id *const o_id, const zf::t_f32 vol = 1.0f, const zf::t_f32 pan = 0.0f, const zf::t_f32 pitch = 1.0f, const zf::t_b8 loop = false);

    // Returns true iff the play succeeded. Note that some failure cases will trigger a fatal error instead.
    inline zf::t_b8 sound_play(const t_sound_type *const type, const zf::t_f32 vol = 1.0f, const zf::t_f32 pan = 0.0f, const zf::t_f32 pitch = 1.0f, const zf::t_b8 loop = false) {
        t_sound_id id_throwaway;
        return sound_play_and_get_id(type, &id_throwaway, vol, pan, pitch, loop);
    }

    void sound_stop(const t_sound_id id);

    zf::t_b8 sound_check_playing(const t_sound_id id);

    void proc_finished_sounds();

    // ============================================================
}
