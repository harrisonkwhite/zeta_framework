#pragma once

#include <zcl/zcl_io.h>
#include <zcl/zcl_strs.h>

namespace zf::audio {
    struct SoundMeta {
        I32 sample_rate;
        I32 channel_cnt;
        I32 frame_cnt;
    };

    struct SoundDataRdonly {
        SoundMeta meta;
        s_array_rdonly<F32> pcm;
    };

    struct SoundDataMut {
        SoundMeta meta;
        s_array_mut<F32> pcm;

        operator SoundDataRdonly() const { return {.meta = meta, .pcm = pcm}; }
    };

    inline I32 get_sample_cnt(const SoundMeta snd_meta) {
        return snd_meta.channel_cnt * snd_meta.frame_cnt;
    }

    [[nodiscard]] B8 load_sound_data_from_raw(const strs::StrRdonly file_path, s_arena *const snd_data_arena, s_arena *const temp_arena, SoundDataMut *const o_snd_data);

    [[nodiscard]] B8 pack_sound(const strs::StrRdonly file_path, const SoundDataMut snd_data, s_arena *const temp_arena);
    [[nodiscard]] B8 unpack_sound(const strs::StrRdonly file_path, s_arena *const snd_data_arena, s_arena *const temp_arena, SoundDataMut *const o_snd_data);

    [[nodiscard]] B8 serialize_sound(s_stream *const stream, const SoundDataMut snd_data);
    [[nodiscard]] B8 deserialize_sound(s_stream *const stream, s_arena *const snd_data_arena, SoundDataMut *const o_snd_data);
}
