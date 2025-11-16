#pragma once

#include <zc/zc_strs.h>
#include <zc/zc_allocators.h>

namespace zf {
    struct s_sound_meta {
        t_s32 channel_cnt = 0;
        t_s32 sample_rate = 0;
        t_s64 frame_cnt = 0; // This isn't really needed, and can be deduced from channel count and PCM array length. But it makes the packing and unpacking I/O simpler.
    };

    struct s_sound_data {
        s_sound_meta meta;
        c_array<t_f32> pcm;

        s_sound_data() = default;
        s_sound_data(const s_sound_meta meta, const c_array<t_f32> pcm)
            : meta(meta), pcm(pcm) {}
    };

    inline t_s64 CalcSampleCount(const s_sound_data& snd_data) {
        return snd_data.meta.channel_cnt * snd_data.meta.frame_cnt;
    }

    [[nodiscard]] t_b8 LoadSoundFromRaw(const s_str_rdonly file_path, c_mem_arena& mem_arena, s_sound_data& o_snd_data);
    [[nodiscard]] t_b8 LoadSoundFromPacked(const s_str_rdonly file_path, c_mem_arena& mem_arena, s_sound_data& o_snd_data);

    [[nodiscard]] t_b8 PackSound(const s_sound_data& snd_data, const s_str_rdonly file_path, c_mem_arena& temp_mem_arena);
}
