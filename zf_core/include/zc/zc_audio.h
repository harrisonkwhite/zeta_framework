#pragma once

#include <zc/zc_strs.h>
#include <zc/zc_allocators.h>

namespace zf {
    struct s_sound_meta {
        t_s32 channel_cnt = 0;
        t_s32 sample_rate = 0;
        t_s64 frame_cnt = 0; // This isn't really needed, and can be deduced from channel count and PCM array length. But it makes the packing and unpacking I/O simpler.

        t_s64 SampleCount() const {
            return channel_cnt * frame_cnt;
        }
    };

    struct s_sound_data_view {
        s_sound_meta meta;
        c_array<const t_f32> pcm;

        s_sound_data_view() = default;
        s_sound_data_view(const s_sound_meta meta, const c_array<const t_f32> pcm)
            : meta(meta), pcm(pcm) {}
    };

    struct s_sound_data {
        s_sound_meta meta;
        c_array<t_f32> pcm;

        s_sound_data() = default;
        s_sound_data(const s_sound_meta meta, const c_array<t_f32> pcm)
            : meta(meta), pcm(pcm) {}

        operator s_sound_data_view() const {
            return {meta, c_array<const t_f32>(pcm)};
        }
    };

    [[nodiscard]] t_b8 LoadSoundFromRaw(const s_str_ro file_path, c_mem_arena& mem_arena, s_sound_data& o_snd_data);
    [[nodiscard]] t_b8 LoadSoundFromPacked(const s_str_ro file_path, c_mem_arena& mem_arena, s_sound_data& o_snd_data);

    [[nodiscard]] t_b8 PackSound(const s_sound_data_view& snd_data, const s_str_ro file_path, c_mem_arena& temp_mem_arena);
}
