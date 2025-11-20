#pragma once

#include <zc.h>

namespace zf::audio {
    using t_sound_type_id = t_size; // @todo: Consider a versioning system for error prevention?

#if 0
    struct s_resource_arena {
        s_mem_arena mem_arena; // @todo: This could be made into just a pointer. If there is no other operation of resource arena release, just join it with an existing arena.

        t_size snd_type_cnt;
        s_array<s_sound_meta> snd_type_metas;
        s_array<s_array<t_f32>> snd_type_pcms;

        constexpr t_size Cap() const {
            return snd_type_metas.len;
        }
    };

    [[nodiscard]] t_b8 MakeResourceArena(const t_size snd_type_limit, s_resource_arena& o_res_arena);
    void ReleaseResourceArena(s_resource_arena& res_arena);
    [[nodiscard]] t_b8 RegisterSoundTypeFromRaw(s_resource_arena& res_arena, const s_str_rdonly file_path, t_sound_type_id& o_id);
#endif

    [[nodiscard]] t_b8 InitSys();
    void ShutdownSys();
    [[nodiscard]] t_b8 RegisterSoundType(const s_sound_meta& snd_meta, const s_array_rdonly<t_f32> snd_pcm, t_sound_type_id& o_id);
    void UnregisterSoundType(const t_sound_type_id id);
    void ProcFinishedSounds();
    [[nodiscard]] t_b8 PlaySound(const t_sound_type_id type_id, const t_f32 vol = 1.0f, const t_f32 pan = 0.0f, const t_f32 pitch = 1.0f, const t_b8 loop = false);
}
