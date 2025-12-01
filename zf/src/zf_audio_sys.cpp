#include <zf/zf_audio_sys.h>

#include <miniaudio.h>

namespace zf::audio_sys {
    constexpr t_size g_snd_limit = 32;

    static struct {
        t_b8 initted;

        ma_engine ma_eng;

        struct {
            s_static_array<ma_sound, g_snd_limit> ma_snds;
            s_static_array<ma_audio_buffer_ref, g_snd_limit> ma_buf_refs;
            s_static_bit_vec<g_snd_limit> activity;
        } snd_insts;
    } g_state;

    t_b8 Init() {
        g_state = {};

        if (ma_engine_init(nullptr, &g_state.ma_eng) != MA_SUCCESS) {
            ZF_REPORT_ERROR();
            return false;
        }

        g_state.initted = true;

        return true;
    }

    void Shutdown() {
        ZF_ASSERT(g_state.initted);

        ZF_FOR_EACH_SET_BIT(g_state.snd_insts.activity, i) {
            ma_sound_uninit(&g_state.snd_insts.ma_snds[i]);
            ma_audio_buffer_ref_uninit(&g_state.snd_insts.ma_buf_refs[i]);
        }

        ma_engine_uninit(&g_state.ma_eng);

        g_state.initted = false;
    }

    t_b8 MakeSoundTypeArena(const t_size snd_type_limit, s_mem_arena& mem_arena, s_sound_type_arena& o_arena) {
        o_arena = {
            .mem_arena = &mem_arena
        };

        return MakeArray(mem_arena, snd_type_limit, o_arena.metas)
            && MakeArray(mem_arena, snd_type_limit, o_arena.pcms);
    }

    t_b8 RegisterSoundType(s_sound_type_arena& arena, const s_sound_meta& meta, const s_array_rdonly<t_f32> pcm, t_size& o_index) {
        ZF_ASSERT(arena.mem_arena);

        if (arena.cnt == SoundTypeArenaCap(arena)) {
            return false;
        }

        arena.metas[arena.cnt] = meta;

        if (!MakeArrayClone(*arena.mem_arena, pcm, arena.pcms[arena.cnt])) {
            return false;
        }

        arena.cnt++;

        return true;
    }

    void ProcFinishedSounds() {
        ZF_ASSERT(g_state.initted);

        ZF_FOR_EACH_SET_BIT(g_state.snd_insts.activity, i) {
            ma_sound& snd = g_state.snd_insts.ma_snds[i];

            if (!ma_sound_is_playing(&snd)) {
                ma_sound_uninit(&snd);
                ma_audio_buffer_ref_uninit(&g_state.snd_insts.ma_buf_refs[i]);
                UnsetBit(g_state.snd_insts.activity, i);
            }
        }
    }

    t_b8 PlaySound(const s_sound_type_arena& type_arena, const t_size type_index, const t_f32 vol, const t_f32 pan, const t_f32 pitch, const t_b8 loop) {
        // @todo: It sounds like there's a bit of latency with this - see if it can be reduced.

        ZF_ASSERT(g_state.initted);
        ZF_ASSERT(vol >= 0.0f && vol <= 1.0f);
        ZF_ASSERT(pan >= -1.0f && pan <= 1.0f);
        ZF_ASSERT(pitch > 0.0f);

        t_b8 clean_up = false;

        const s_sound_meta& meta = type_arena.metas[type_index];
        const s_array_rdonly<t_f32> pcm = type_arena.pcms[type_index];

        const t_size index = IndexOfFirstUnsetBit(g_state.snd_insts.activity);

        if (index == -1) {
            ZF_REPORT_ERROR();
            clean_up = true;
            return false;
        }

        ma_sound& ma_snd = g_state.snd_insts.ma_snds[index];
        ma_audio_buffer_ref& ma_buf_ref = g_state.snd_insts.ma_buf_refs[index];

        if (ma_audio_buffer_ref_init(ma_format_f32, static_cast<ma_uint32>(meta.channel_cnt), pcm.buf_raw, static_cast<ma_uint64>(meta.frame_cnt), &ma_buf_ref) != MA_SUCCESS) {
            ZF_REPORT_ERROR();
            clean_up = true;
            return false;
        }

        ZF_DEFER({
            if (clean_up) {
                ma_audio_buffer_ref_uninit(&ma_buf_ref);
            }
        });

        ma_buf_ref.sampleRate = static_cast<ma_uint32>(meta.sample_rate);

        if (ma_sound_init_from_data_source(&g_state.ma_eng, &ma_buf_ref, 0, nullptr, &ma_snd) != MA_SUCCESS) {
            ZF_REPORT_ERROR();
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
            ZF_REPORT_ERROR();
            clean_up = true;
            return false;
        }

        SetBit(g_state.snd_insts.activity, index);

        return true;
    }
}
