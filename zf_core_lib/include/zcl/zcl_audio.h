#pragma once

#include <zcl/zcl_strs.h>
#include <zcl/zcl_streams.h>

namespace zcl {
    struct t_sound_meta {
        t_i32 sample_rate;
        t_i32 channel_cnt;
        t_i32 frame_cnt;
    };

    struct t_sound_data_rdonly {
        t_sound_meta meta;
        t_array_rdonly<t_f32> pcm;
    };

    struct t_sound_data_mut {
        t_sound_meta meta;
        t_array_mut<t_f32> pcm;

        operator t_sound_data_rdonly() const {
            return {.meta = meta, .pcm = pcm};
        }
    };

    constexpr t_i32 SoundCalcSampleCount(const t_sound_meta snd_meta) {
        return snd_meta.channel_cnt * snd_meta.frame_cnt;
    }

    [[nodiscard]] t_b8 SoundLoadFromRaw(const t_str_rdonly file_path, t_arena *const snd_data_arena, t_arena *const temp_arena, t_sound_data_mut *const o_snd_data);
}
