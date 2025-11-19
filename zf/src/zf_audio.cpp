#include <zf/zf_audio.h>

#include <miniaudio.h>

namespace zf::audio {
    constexpr t_size g_snd_type_limit = 1024;
    constexpr t_size g_snd_limit = 32;

    static struct {
        t_b8 initted;

        ma_engine eng;
        s_static_activity_array<ma_audio_buffer, g_snd_type_limit> snd_type_bufs;
        s_static_activity_array<ma_sound, g_snd_limit> snds;
    } g_sys;

    static t_b8 IsSoundTypeIDValid(const t_sound_type_id id) {
        return id >= 0 && id < g_snd_type_limit;
    }

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

        for (t_size i = IndexOfFirstActiveSlot(g_sys.snds); i != -1; i = IndexOfFirstActiveSlot(g_sys.snds, i + 1)) {
            ma_sound_uninit(&g_sys.snds[i]);
        }

        ma_engine_uninit(&g_sys.eng);

        g_sys.initted = false;
    }

    void ProcFinishedSounds() {
        ZF_ASSERT(g_sys.initted);

        for (t_size i = IndexOfFirstActiveSlot(g_sys.snds); i != -1; i = IndexOfFirstActiveSlot(g_sys.snds, i + 1)) {
            ma_sound& snd = g_sys.snds[i];

            if (!ma_sound_is_playing(&snd)) {
                ma_sound_uninit(&snd);
                DeactivateSlot(g_sys.snds, i);
            }
        }
    }

    t_b8 RegisterSoundType(const s_sound_data& snd_data, t_sound_type_id& o_id) {
        ZF_ASSERT(g_sys.initted);

        const t_size index = TakeFirstInactiveSlot(g_sys.snd_type_bufs);

        if (index == -1) {
            ZF_REPORT_FAILURE();
            return false;
        }

        ma_audio_buffer_config buf_config = ma_audio_buffer_config_init(ma_format_f32, static_cast<ma_uint32>(snd_data.meta.channel_cnt), static_cast<ma_uint64>(snd_data.meta.frame_cnt), snd_data.pcm.buf_raw, nullptr);
        buf_config.sampleRate = static_cast<ma_uint32>(snd_data.meta.sample_rate);

        if (ma_audio_buffer_init_copy(&buf_config, &g_sys.snd_type_bufs[index]) != MA_SUCCESS) {
            DeactivateSlot(g_sys.snd_type_bufs, index);
            ZF_REPORT_FAILURE();
            return false;
        }

        return true;
    }

    void UnregisterSoundType(const t_sound_type_id id) {
        ZF_ASSERT(g_sys.initted);
        ZF_ASSERT(IsSoundTypeIDValid(id));
        ZF_ASSERT(IsSlotActive(g_sys.snd_type_bufs, id));
        ma_audio_buffer_uninit(&g_sys.snd_type_bufs[id]);
        DeactivateSlot(g_sys.snd_type_bufs, id);
    }

    t_b8 PlaySound(const t_sound_type_id type_id, const t_f32 vol, const t_f32 pan, const t_f32 pitch, const t_b8 loop) {
        ZF_ASSERT(g_sys.initted);
        ZF_ASSERT(IsSoundTypeIDValid(type_id));
        ZF_ASSERT(vol >= 0.0f && vol <= 1.0f);
        ZF_ASSERT(pan >= -1.0f && pan <= 1.0f);
        ZF_ASSERT(pitch > 0.0f);

        // Find and use the first inactive sound slot.
        const t_size index = TakeFirstInactiveSlot(g_sys.snds);

        if (index == -1) {
            ZF_REPORT_FAILURE();
            return false;
        }

        ma_sound& snd = g_sys.snds[index];

        // Bind the buffer to the sound.
        if (ma_sound_init_from_data_source(&g_sys.eng, &g_sys.snd_type_bufs[type_id], 0, nullptr, &snd) != MA_SUCCESS) {
            DeactivateSlot(g_sys.snds, index);
            ZF_REPORT_FAILURE();
            return false;
        }

        // Set properties and play.
        ma_sound_set_volume(&snd, vol);
        ma_sound_set_pan(&snd, pan);
        ma_sound_set_pitch(&snd, pitch);
        ma_sound_set_looping(&snd, loop);

        if (ma_sound_start(&snd) != MA_SUCCESS) {
            ma_sound_uninit(&snd);
            DeactivateSlot(g_sys.snds, index);
            ZF_REPORT_FAILURE();
            return false;
        }

        return true;
    }
}
