#pragma once

#include <zcl/zcl_strs.h>
#include <zcl/io/zcl_streams.h>

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

        operator t_sound_data_rdonly() const { return {.meta = meta, .pcm = pcm}; }
    };

    constexpr t_i32 sound_calc_sample_cnt(const t_sound_meta snd_meta) {
        return snd_meta.channel_cnt * snd_meta.frame_cnt;
    }

    [[nodiscard]] t_b8 sound_load_from_raw(const t_str_rdonly file_path, t_arena *const snd_data_arena, t_arena *const temp_arena, t_sound_data_mut *const o_snd_data);

    [[nodiscard]] t_b8 sound_pack(const t_str_rdonly file_path, const t_sound_data_rdonly snd_data, t_arena *const temp_arena);
    [[nodiscard]] t_b8 sound_unpack(const t_str_rdonly file_path, t_arena *const snd_data_arena, t_arena *const temp_arena, t_sound_data_mut *const o_snd_data);

    [[nodiscard]] t_b8 sound_serialize(const t_sound_data_rdonly snd_data, const t_stream stream);
    [[nodiscard]] t_b8 sound_deserialize(const t_stream stream, t_arena *const snd_data_arena, t_sound_data_mut *const o_snd_data);
}
