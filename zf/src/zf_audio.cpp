#include <zf/zf_audio.h>

#include <miniaudio.h>

namespace zf::audio {
    static struct {
        t_b8 initted;

        ma_engine eng;

        s_static_array<ma_sound, g_snd_limit> snds;
        s_static_array<ma_audio_buffer, g_snd_limit> audio_bufs;
        s_static_bit_vector<g_snd_limit> snd_activity;
    } g_audio_sys;

    t_b8 InitSys() {
        g_audio_sys = {};

        if (ma_engine_init(nullptr, &g_audio_sys.eng) != MA_SUCCESS) {
            ZF_REPORT_FAILURE();
            return false;
        }

        return true;
    }

    void ShutdownSys() {
        ZF_ASSERT(g_audio_sys.initted);

        for (t_size i = 0; i < g_snd_limit; i++) {
            if (IsBitSet(g_audio_sys.snd_activity, i)) {
                ma_sound_uninit(&g_audio_sys.snds[i]);
                ma_audio_buffer_uninit(&g_audio_sys.audio_bufs[i]);
            }
        }

        ma_engine_uninit(&g_audio_sys.eng);
    }

    void ProcFinishedSounds() {
        ZF_ASSERT(g_audio_sys.initted);

        for (t_size i = 0; i < g_snd_limit; i++) {
            if (!IsBitSet(g_audio_sys.snd_activity, i)) {
                continue;
            }

            ma_sound* const snd = &g_audio_sys.snds[i];

            if (!ma_sound_is_playing(snd)) {
                ma_sound_uninit(snd);
                ma_audio_buffer_uninit(&g_audio_sys.audio_bufs[i]);

                UnsetBit(g_audio_sys.snd_activity, i);
            }
        }
    }
}
