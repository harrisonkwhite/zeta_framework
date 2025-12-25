#pragma once

#include <zcl.h>

namespace zf {
    void InitAudio();
    void ShutdownAudio();

    struct s_sound_type;

    struct s_sound_type_arena {
    public:
        [[nodiscard]] t_b8 AddFromRaw(const s_str_rdonly file_path, s_mem_arena &temp_mem_arena, s_ptr<s_sound_type> &o_type);
        [[nodiscard]] t_b8 AddFromPacked(const s_str_rdonly file_path, s_mem_arena &temp_mem_arena, s_ptr<s_sound_type> &o_type);
        void Release();

        t_i32 Version() const {
            return m_version;
        }

    private:
        t_i32 m_version = 0;

        s_mem_arena m_mem_arena = {};
        s_ptr<s_sound_type> m_head = nullptr;
        s_ptr<s_sound_type> m_tail = nullptr;
    };

    struct s_sound_id {
        t_i32 index = 0;
        t_i32 version = 0;
    };

    [[nodiscard]] t_b8 PlaySound(const s_sound_type &type, const s_ptr<s_sound_id> o_id = nullptr, const t_f32 vol = 1.0f, const t_f32 pan = 0.0f, const t_f32 pitch = 1.0f, const t_b8 loop = false);
    void StopSound(const s_sound_id id);
    t_b8 IsSoundPlaying(const s_sound_id id);

    void ProcFinishedSounds();
}
