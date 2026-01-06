#pragma once

#include <zcl.h>

namespace zf::audio_sys {
    void module_startup();
    void module_shutdown();

    struct t_sound_type;

    struct t_sound_type_group {
        mem::t_arena arena;
        t_sound_type *head;
        t_sound_type *tail;
    };

    t_sound_type_group sound_type_group_create();
    void sound_type_group_destroy();

    t_sound_type *sound_type_create_from_raw(const strs::t_str_rdonly file_path, t_sound_type_group *const group, mem::t_arena *const temp_arena);
    t_sound_type *sound_type_create_from_packed(const strs::t_str_rdonly file_path, t_sound_type_group *const group, mem::t_arena *const temp_arena);

    struct t_sound_id {
        t_i32 index;
        t_i32 version;
    };

    t_sound_id sound_play(const t_sound_type *const type, const t_f32 vol = 1.0f, const t_f32 pan = 0.0f, const t_f32 pitch = 1.0f, const t_b8 loop = false);
    void sound_stop(const t_sound_id id);
    t_b8 sound_is_playing(const t_sound_id id);

#if 0
    void StartupAudioModule();
    void ShutdownAudioModule();

    struct s_sound_type;

    struct s_sound_type_arena {
    public:
        s_sound_type_arena() = default;
        s_sound_type_arena(const s_sound_type_arena &) = delete;
        s_sound_type &operator=(const s_sound_type_arena &) = delete;

        void Release();

        [[nodiscard]] t_b8 AddFromRaw(const s_str_rdonly file_path, s_arena &temp_arena, s_ptr<s_sound_type> &o_type);
        [[nodiscard]] t_b8 AddFromPacked(const s_str_rdonly file_path, s_arena &temp_arena, s_ptr<s_sound_type> &o_type);

        t_i32 Version() const {
            return m_version;
        }

    private:
        void Add(const s_sound_data snd_data, s_ptr<s_sound_type> &o_type);

        t_i32 m_version = 0;

        s_arena m_arena = {};
        s_ptr<s_sound_type> m_head = nullptr;
        s_ptr<s_sound_type> m_tail = nullptr;
    };

    struct s_sound_id {
    public:
        s_sound_id() = default;

        s_sound_id(const t_i32 index, const t_i32 version) : valid(true), m_index(index), m_version(version) {
            ZF_ASSERT(index >= 0 && version >= 0);
        }

        t_i32 Index() const { return m_index; }
        t_i32 Version() const { return m_version; }

    private:
        t_b8 valid = false;
        t_i32 m_index = 0;
        t_i32 m_version = 0;
    };

    s_sound_id PlaySound(const s_sound_type &type, const t_f32 vol = 1.0f, const t_f32 pan = 0.0f, const t_f32 pitch = 1.0f, const t_b8 loop = false);
    void StopSound(const s_sound_id id);
    t_b8 IsSoundPlaying(const s_sound_id id);

    void ProcFinishedSounds();
#endif
}
