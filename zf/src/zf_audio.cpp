#include <zf/zf_audio.h>

#include <miniaudio.h>

namespace zf::audio {
    constexpr t_size g_snd_limit = 32;

    static struct {
        t_b8 initted;

        ma_engine eng;

        s_static_array<ma_sound, g_snd_limit> snds;
        s_static_array<ma_audio_buffer, g_snd_limit> audio_bufs;
        s_static_bit_vector<g_snd_limit> snd_activity;
    } g_sys;

    t_b8 InitSys() {
        g_sys = {};

        if (ma_engine_init(nullptr, &g_sys.eng) != MA_SUCCESS) {
            ZF_REPORT_FAILURE();
            return false;
        }

        g_sys.initted = true;

        return true;
    }

    void ShutdownSys() {
        ZF_ASSERT(g_sys.initted);

        for (t_size i = 0; i < g_snd_limit; i++) {
            if (IsBitSet(g_sys.snd_activity, i)) {
                ma_sound_uninit(&g_sys.snds[i]);
                ma_audio_buffer_uninit(&g_sys.audio_bufs[i]);
            }
        }

        ma_engine_uninit(&g_sys.eng);

        g_sys.initted = false;
    }

    void ProcFinishedSounds() {
        ZF_ASSERT(g_sys.initted);

        for (t_size i = 0; i < g_snd_limit; i++) {
            if (!IsBitSet(g_sys.snd_activity, i)) {
                continue;
            }

            ma_sound* const snd = &g_sys.snds[i];

            if (!ma_sound_is_playing(snd)) {
                ma_sound_uninit(snd);
                ma_audio_buffer_uninit(&g_sys.audio_bufs[i]);

                UnsetBit(g_sys.snd_activity, i);
            }
        }
    }

    t_b8 PlaySound(const s_sound_data_rdonly& snd_data, const t_f32 vol, const t_f32 pan, const t_f32 pitch) {
        ZF_ASSERT(g_sys.initted);
        ZF_ASSERT(vol >= 0.0f && vol <= 1.0f);
        ZF_ASSERT(pan >= -1.0f && pan <= 1.0f);
        ZF_ASSERT(pitch > 0.0f);

        // Find and use the first inactive sound slot.
        const t_size index = FindFirstUnsetBit(g_sys.snd_activity);

        if (index == -1) {
            ZF_REPORT_FAILURE();
            return false;
        }

        SetBit(g_sys.snd_activity, index);

        ma_sound& snd = g_sys.snds[index];

        // Set up a new audio buffer using the sound type sample buffer.
        ma_audio_buffer_config buf_config = ma_audio_buffer_config_init(ma_format_f32, static_cast<ma_uint32>(snd_data.meta.channel_cnt), static_cast<ma_uint64>(snd_data.meta.frame_cnt), snd_data.pcm.buf_raw, nullptr);
        buf_config.sampleRate = static_cast<ma_uint32>(snd_data.meta.sample_rate);

        if (ma_audio_buffer_init_copy(&buf_config, &g_sys.audio_bufs[index]) != MA_SUCCESS) {
            ZF_REPORT_FAILURE();
            return false;
        }

        // Bind the buffer to the sound.
        if (ma_sound_init_from_data_source(&g_sys.eng, &g_sys.audio_bufs[index], 0, nullptr, &snd) != MA_SUCCESS) {
            ZF_REPORT_FAILURE();
            return false;
        }

        // Set properties and play.
        ma_sound_set_volume(&snd, vol);
        ma_sound_set_pan(&snd, pan);
        ma_sound_set_pitch(&snd, pitch);

        if (ma_sound_start(&snd) != MA_SUCCESS) {
            ZF_REPORT_FAILURE();
            return false;
        }

        return true;
    }
}
