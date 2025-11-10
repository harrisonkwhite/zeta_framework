#pragma once

#include <zc/mem/strs.h>

namespace zf {
    struct s_sound_data {
        c_array<t_f32> pcm;
        t_s32 channel_cnt = 0;
        t_s32 sample_rate = 0;

        bool LoadFromRaw(c_mem_arena& mem_arena, const s_str_view file_path);
    };

    bool PackSound(const s_str_view file_path, const s_sound_data& snd_data, c_mem_arena& temp_mem_arena);
    bool UnpackSound(s_sound_data& snd_data, const s_str_view file_path);
}
