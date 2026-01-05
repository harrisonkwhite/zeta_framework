#pragma once

#include <zcl/zcl_io.h>
#include <zcl/zcl_strs.h>

namespace zf {
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

    inline t_i32 f_audio_get_sample_cnt(const t_sound_meta snd_meta) {
        return snd_meta.channel_cnt * snd_meta.frame_cnt;
    }

    [[nodiscard]] t_b8 f_audio_load_sound_data_from_raw(const t_str_rdonly file_path, t_arena *const snd_data_arena, t_arena *const temp_arena, t_sound_data_mut *const o_snd_data);

    [[nodiscard]] t_b8 f_audio_pack_sound(const t_str_rdonly file_path, const t_sound_data_mut snd_data, t_arena *const temp_arena);
    [[nodiscard]] t_b8 f_audio_unpack_sound(const t_str_rdonly file_path, t_arena *const snd_data_arena, t_arena *const temp_arena, t_sound_data_mut *const o_snd_data);

    [[nodiscard]] t_b8 f_audio_serialize_sound(t_stream *const stream, const t_sound_data_mut snd_data);
    [[nodiscard]] t_b8 f_audio_deserialize_sound(t_stream *const stream, t_arena *const snd_data_arena, t_sound_data_mut *const o_snd_data);
}
