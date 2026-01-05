#pragma once

#include <zcl/zcl_io.h>
#include <zcl/zcl_strs.h>

namespace zf::audio {
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

    [[nodiscard]] t_b8 f_sound_load_from_raw(const t_str_rdonly file_path, mem::t_arena *const snd_data_arena, mem::t_arena *const temp_arena, t_sound_data_mut *const o_snd_data);

    [[nodiscard]] t_b8 f_sound_pack(const t_str_rdonly file_path, const t_sound_data_rdonly snd_data, mem::t_arena *const temp_arena);
    [[nodiscard]] t_b8 f_sound_unpack(const t_str_rdonly file_path, mem::t_arena *const snd_data_arena, mem::t_arena *const temp_arena, t_sound_data_mut *const o_snd_data);

    [[nodiscard]] t_b8 f_sound_serialize(const t_sound_data_rdonly snd_data, t_io_stream *const stream);
    [[nodiscard]] t_b8 f_sound_deserialize(t_io_stream *const stream, mem::t_arena *const snd_data_arena, t_sound_data_mut *const o_snd_data);

    inline t_i32 f_sound_get_sample_cnt(const t_sound_meta snd_meta) {
        return snd_meta.channel_cnt * snd_meta.frame_cnt;
    }
}
