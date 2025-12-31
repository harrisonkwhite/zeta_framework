#pragma once

#include <zcl/zcl_io.h>
#include <zcl/zcl_strs.h>

namespace zf {
    struct s_sound_meta {
        t_i32 sample_rate;
        t_i32 channel_cnt;
        t_i32 frame_cnt;
    };

    struct s_sound_data_rdonly {
        s_sound_meta meta;
        s_array_rdonly<t_f32> pcm;
    };

    struct s_sound_data_mut {
        s_sound_meta meta;
        s_array_mut<t_f32> pcm;

        operator s_sound_data_rdonly() const { return {.meta = meta, .pcm = pcm}; }
    };

    inline t_i32 SampleCount(const s_sound_meta snd_meta) {
        return snd_meta.channel_cnt * snd_meta.frame_cnt;
    }

    [[nodiscard]] t_b8 LoadSoundDataFromRaw(const s_str_rdonly file_path, c_arena *const snd_data_mem_arena, c_arena *const temp_mem_arena, s_sound_data_mut *const o_snd_data);

    [[nodiscard]] t_b8 PackSound(const s_str_rdonly file_path, const s_sound_data_mut snd_data, c_arena *const temp_mem_arena);
    [[nodiscard]] t_b8 UnpackSound(const s_str_rdonly file_path, c_arena *const snd_data_mem_arena, c_arena *const temp_mem_arena, s_sound_data_mut *const o_snd_data);

    [[nodiscard]] t_b8 SerializeSound(c_stream *const stream, const s_sound_data_mut snd_data);
    [[nodiscard]] t_b8 DeserializeSound(c_stream *const stream, c_arena *const snd_data_mem_arena, s_sound_data_mut *const o_snd_data);
}
