#pragma once

#include <zc/mem/strs.h>

namespace zf {
    struct s_audio_data {
        c_array<t_f32> pcm;
        t_s32 channel_cnt = 0;
        t_s32 sample_rate = 0;

        bool LoadFromRaw(const s_str_view file_path, c_mem_arena& temp_mem_arena);
    };

#if 0
    bool PackAudio(const s_str_view file_path, const c_audio_data audio);
    bool UnpackAudio(c_audio_data& audio, const s_str_view file_path);
#endif
}
