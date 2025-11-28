#include <zf/zf_audio.h>

#include <miniaudio.h>

namespace zf::audio {
    constexpr t_size g_snd_type_limit = 1024;
    constexpr t_size g_snd_limit = 32;

    static struct {
        t_b8 initted;

        s_static_array<s_sound_meta, g_snd_type_limit> snd_type_metas;
        s_static_array<s_array<t_f32>, g_snd_type_limit> snd_type_pcms;
        s_static_bit_vec<g_snd_type_limit> snd_type_activity;

        ma_engine ma_eng;

        struct {
            s_static_array<ma_sound, g_snd_limit> ma_snds;
            s_static_array<ma_audio_buffer_ref, g_snd_limit> ma_buf_refs;
            s_static_bit_vec<g_snd_limit> activity;
        } snd_insts;
    } g_sys;

    static t_b8 IsSoundTypeIDValid(const t_sound_type_id id) {
        return id >= 0 && id < g_snd_type_limit;
    }

    t_b8 InitSys() {
        g_sys = {};

        if (ma_engine_init(nullptr, &g_sys.ma_eng) != MA_SUCCESS) {
            ZF_REPORT_FAILURE();
            return false;
        }

        g_sys.initted = true;

        return true;
    }

    void ShutdownSys() {
        ZF_ASSERT(g_sys.initted);

        ZF_FOR_EACH_SET_BIT(g_sys.snd_insts.activity, i) {
            ma_sound_uninit(&g_sys.snd_insts.ma_snds[i]);
            ma_audio_buffer_ref_uninit(&g_sys.snd_insts.ma_buf_refs[i]);
        }

        ZF_FOR_EACH_SET_BIT(g_sys.snd_type_activity, i) {
            LogWarning("Sound type with ID % not released!", i); // @todo: Update this if the ID no longer is an index.
            free(g_sys.snd_type_pcms[i].buf_raw);
            g_sys.snd_type_pcms[i] = {};
        }

        ma_engine_uninit(&g_sys.ma_eng);

        g_sys.initted = false;
    }

    t_b8 RegisterSoundType(const s_sound_meta& snd_meta, const s_array_rdonly<t_f32> snd_pcm, t_sound_type_id& o_id) {
        ZF_ASSERT(g_sys.initted);

        o_id = IndexOfFirstUnsetBit(g_sys.snd_type_activity);

        if (o_id == -1) {
            ZF_REPORT_FAILURE();
            return false;
        }

        const t_size pcm_len = CalcSampleCount(snd_meta);
        const auto pcm_buf_raw = static_cast<t_f32*>(malloc(static_cast<size_t>(ZF_SIZE_OF(t_f32) * pcm_len)));

        if (!pcm_buf_raw) {
            ZF_REPORT_FAILURE();
            return false;
        }

        g_sys.snd_type_pcms[o_id] = {pcm_buf_raw, pcm_len};
        Copy(g_sys.snd_type_pcms[o_id], snd_pcm);

        g_sys.snd_type_metas[o_id] = snd_meta;

        SetBit(g_sys.snd_type_activity, o_id);

        return true;
    }

    void UnregisterSoundType(const t_sound_type_id id) {
        ZF_ASSERT(g_sys.initted);
        ZF_ASSERT(IsSoundTypeIDValid(id));
        ZF_ASSERT(IsBitSet(g_sys.snd_type_activity, id));

        free(g_sys.snd_type_pcms[id].buf_raw);
        g_sys.snd_type_pcms[id] = {};
        UnsetBit(g_sys.snd_type_activity, id);
    }

    void ProcFinishedSounds() {
        ZF_ASSERT(g_sys.initted);

        ZF_FOR_EACH_SET_BIT(g_sys.snd_insts.activity, i) {
            ma_sound& snd = g_sys.snd_insts.ma_snds[i];

            if (!ma_sound_is_playing(&snd)) {
                ma_sound_uninit(&snd);
                ma_audio_buffer_ref_uninit(&g_sys.snd_insts.ma_buf_refs[i]);
                UnsetBit(g_sys.snd_insts.activity, i);
            }
        }
    }

    t_b8 PlaySound(const t_sound_type_id type_id, const t_f32 vol, const t_f32 pan, const t_f32 pitch, const t_b8 loop) {
        // @todo: It sounds like there's a bit of latency with this - see if it can be reduced.

        ZF_ASSERT(g_sys.initted);
        ZF_ASSERT(IsSoundTypeIDValid(type_id));
        ZF_ASSERT(vol >= 0.0f && vol <= 1.0f);
        ZF_ASSERT(pan >= -1.0f && pan <= 1.0f);
        ZF_ASSERT(pitch > 0.0f);

        t_b8 clean_up = false;

        const s_sound_meta& meta = g_sys.snd_type_metas[type_id];
        const s_array_rdonly<t_f32> pcm = g_sys.snd_type_pcms[type_id];

        const t_size index = IndexOfFirstUnsetBit(g_sys.snd_insts.activity);

        if (index == -1) {
            ZF_REPORT_FAILURE();
            clean_up = true;
            return false;
        }

        ma_sound& ma_snd = g_sys.snd_insts.ma_snds[index];
        ma_audio_buffer_ref& ma_buf_ref = g_sys.snd_insts.ma_buf_refs[index];

        if (ma_audio_buffer_ref_init(ma_format_f32, static_cast<ma_uint32>(meta.channel_cnt), pcm.buf_raw, static_cast<ma_uint64>(meta.frame_cnt), &ma_buf_ref) != MA_SUCCESS) {
            ZF_REPORT_FAILURE();
            clean_up = true;
            return false;
        }

        ZF_DEFER({
            if (clean_up) {
                ma_audio_buffer_ref_uninit(&ma_buf_ref);
            }
        });

        ma_buf_ref.sampleRate = static_cast<ma_uint32>(meta.sample_rate);

        if (ma_sound_init_from_data_source(&g_sys.ma_eng, &ma_buf_ref, 0, nullptr, &ma_snd) != MA_SUCCESS) {
            ZF_REPORT_FAILURE();
            clean_up = true;
            return false;
        }

        ZF_DEFER({
            if (clean_up) {
                ma_sound_uninit(&ma_snd);
            }
        });

        ma_sound_set_volume(&ma_snd, vol);
        ma_sound_set_pan(&ma_snd, pan);
        ma_sound_set_pitch(&ma_snd, pitch);
        ma_sound_set_looping(&ma_snd, loop);

        if (ma_sound_start(&ma_snd) != MA_SUCCESS) {
            ZF_REPORT_FAILURE();
            clean_up = true;
            return false;
        }

        SetBit(g_sys.snd_insts.activity, index);

        return true;
    }
}
