#pragma once

#include <zcl/zcl_io.h>
#include <zcl/zcl_strs.h>

namespace zf {
    struct s_sound_meta {
        t_s32 channel_cnt;
        t_s32 sample_rate;
        t_s64 frame_cnt;
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

    constexpr t_s64 CalcSampleCount(const s_sound_meta snd_meta) {
        return snd_meta.channel_cnt * snd_meta.frame_cnt;
    }

    [[nodiscard]] t_b8 LoadSoundFromRaw(const s_str_rdonly file_path,
                                        s_sound_data *const snd_data,
                                        s_mem_arena *const snd_data_mem_arena,
                                        s_mem_arena *const temp_mem_arena);
    [[nodiscard]] t_b8 PackSound(const s_str_rdonly file_path, const s_sound_data snd_data,
                                 s_mem_arena *const temp_mem_arena);
    [[nodiscard]] t_b8 UnpackSound(const s_str_rdonly file_path, s_sound_data *const snd_data,
                                   s_mem_arena *const mem_arena,
                                   s_mem_arena *const temp_mem_arena);
    [[nodiscard]] t_b8 SerializeSound(s_stream *const stream, const s_sound_data snd_data);
    [[nodiscard]] t_b8 DeserializeSound(s_stream *const stream, s_sound_data *const snd_data,
                                        s_mem_arena *const snd_data_mem_arena);
}
