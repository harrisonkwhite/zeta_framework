#pragma once

#include <zcl/zcl_io.h>
#include <zcl/zcl_strs.h>

namespace zf::audio {
    struct SoundMeta {
        t_i32 sample_rate;
        t_i32 channel_cnt;
        t_i32 frame_cnt;
    };

    struct SoundDataRdonly {
        SoundMeta meta;
        t_array_rdonly<t_f32> pcm;
    };

    struct SoundDataMut {
        SoundMeta meta;
        t_array_mut<t_f32> pcm;

        operator SoundDataRdonly() const { return {.meta = meta, .pcm = pcm}; }
    };

    inline t_i32 get_sample_cnt(const SoundMeta snd_meta) {
        return snd_meta.channel_cnt * snd_meta.frame_cnt;
    }

    [[nodiscard]] t_b8 load_sound_data_from_raw(const t_str_rdonly file_path, t_arena *const snd_data_arena, t_arena *const temp_arena, SoundDataMut *const o_snd_data);

    [[nodiscard]] t_b8 pack_sound(const t_str_rdonly file_path, const SoundDataMut snd_data, t_arena *const temp_arena);
    [[nodiscard]] t_b8 unpack_sound(const t_str_rdonly file_path, t_arena *const snd_data_arena, t_arena *const temp_arena, SoundDataMut *const o_snd_data);

    [[nodiscard]] t_b8 serialize_sound(s_stream *const stream, const SoundDataMut snd_data);
    [[nodiscard]] t_b8 deserialize_sound(s_stream *const stream, t_arena *const snd_data_arena, SoundDataMut *const o_snd_data);
}
