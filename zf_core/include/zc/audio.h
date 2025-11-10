#pragma once

#include <zc/mem/strs.h>

namespace zf {
#if 0
    class c_audio_data {
        bool LoadFromRaw(const s_str_view file_path, c_mem_arena& temp_mem_arena);

        c_array<const t_f32> pcm;
        t_s32 channel_cnt = 0;
        t_s32 sample_rate = 0;
    };

    bool PackAudio(const s_str_view file_path, const c_audio_data audio);
    bool UnpackAudio(c_audio_data& audio, const s_str_view file_path);
#endif
}
