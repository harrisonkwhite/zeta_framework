#pragma once

#include <zcl/zcl_io.h>
#include <zcl/zcl_strs.h>

namespace zf {
    struct s_sound_meta {
        t_i32 sample_rate = 0;
        t_i32 channel_cnt = 0;
        t_i64 frame_cnt = 0;

        constexpr t_b8 IsValid() const {
            return (sample_rate > 0 && channel_cnt > 0 && frame_cnt > 0)
                || (sample_rate == 0 && channel_cnt == 0 && frame_cnt == 0);
        }
    };

    struct s_sound_data_rdonly {
    public:
        constexpr s_sound_data_rdonly() = default;

        constexpr s_sound_data_rdonly(const s_sound_meta meta, const s_array_rdonly<t_f32> pcm) : m_meta(meta), m_pcm(pcm) {
            ZF_ASSERT(meta.IsValid() && pcm.Len() == meta.channel_cnt * meta.frame_cnt);
        }

        constexpr s_sound_meta Meta() const {
            return m_meta;
        }

        constexpr s_array_rdonly<t_f32> PCM() const {
            return m_pcm;
        }

    private:
        s_sound_meta m_meta = {};
        s_array_rdonly<t_f32> m_pcm = {};
    };

    struct s_sound_data {
    public:
        constexpr s_sound_data() = default;

        constexpr s_sound_data(const s_sound_meta meta, const s_array<t_f32> pcm) : m_meta(meta), m_pcm(pcm) {
            ZF_ASSERT(meta.IsValid() && pcm.Len() == meta.channel_cnt * meta.frame_cnt);
        }

        constexpr operator s_sound_data_rdonly() const {
            return {m_meta, m_pcm};
        }

        constexpr s_sound_meta Meta() const {
            return m_meta;
        }

        constexpr s_array<t_f32> PCM() const {
            return m_pcm;
        }

    private:
        s_sound_meta m_meta = {};
        s_array<t_f32> m_pcm = {};
    };

    constexpr t_i64 CalcSampleCount(const s_sound_meta snd_meta) {
        return snd_meta.channel_cnt * snd_meta.frame_cnt;
    }

    [[nodiscard]] t_b8 LoadSoundFromRaw(const s_str_rdonly file_path, s_mem_arena &snd_data_mem_arena, s_mem_arena &temp_mem_arena, s_sound_data &o_snd_data);
    [[nodiscard]] t_b8 PackSound(const s_str_rdonly file_path, const s_sound_data snd_data, s_mem_arena &temp_mem_arena);
    [[nodiscard]] t_b8 UnpackSound(const s_str_rdonly file_path, s_mem_arena &snd_data_mem_arena, s_mem_arena &temp_mem_arena, s_sound_data &o_snd_data);
    [[nodiscard]] t_b8 SerializeSound(s_stream &stream, const s_sound_data snd_data);
    [[nodiscard]] t_b8 DeserializeSound(s_stream &stream, s_mem_arena &snd_data_mem_arena, s_sound_data &o_snd_data);
}
