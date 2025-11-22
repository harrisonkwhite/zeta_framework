#pragma once

#include <zc/zc_strs.h>
#include <zc/zc_mem.h>

namespace zf {
    struct s_sound_meta {
        t_s32 channel_cnt;
        t_s32 sample_rate;
        t_s64 frame_cnt; // This isn't really needed, and can be deduced from channel count and PCM array length. But it makes the packing and unpacking I/O simpler.
    };

    struct s_sound_data_rdonly {
        s_sound_meta meta;
        s_array_rdonly<t_f32> pcm;
    };

    struct s_sound_data {
        s_sound_meta meta;
        s_array<t_f32> pcm;

        constexpr operator s_sound_data_rdonly() const {
            return {meta, pcm};
        }
    };

    inline t_s64 CalcSampleCount(const s_sound_meta& snd_meta) {
        return snd_meta.channel_cnt * snd_meta.frame_cnt;
    }

    [[nodiscard]] t_b8 LoadSoundFromRaw(const s_str_rdonly file_path, s_mem_arena& mem_arena, s_sound_meta& o_snd_meta, s_array<t_f32>& o_snd_pcm);

    [[nodiscard]] t_b8 PackSound(const s_sound_data_rdonly& snd_data, const s_str_rdonly file_path, s_mem_arena& temp_mem_arena);
    [[nodiscard]] t_b8 UnpackSound(const s_str_rdonly file_path, s_mem_arena& mem_arena, s_sound_data& o_snd_data);
}
