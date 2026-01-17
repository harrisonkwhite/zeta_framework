#include <zgl/zgl_audio_private.h>

#include <miniaudio.h>

namespace zgl {
    constexpr zcl::t_i32 k_sound_limit = 32;

    enum t_phase {
        ek_phase_inactive,
        ek_phase_active,
        ek_phase_frozen // No audio actually plays, though sound properties can be mutated.
    };

    struct {
        t_phase phase;

        ma_engine ma_eng;

        struct {
            zcl::t_static_array<ma_sound, k_sound_limit> ma_snds;
            zcl::t_static_array<ma_audio_buffer_ref, k_sound_limit> ma_buf_refs;
            zcl::t_static_array<const t_sound_type *, k_sound_limit> types;
            zcl::t_static_array<t_sound_state, k_sound_limit> states;
            zcl::t_static_bitset<k_sound_limit> active;
            zcl::t_static_array<zcl::t_i32, k_sound_limit> versions;
        } snd_insts;
    } g_state;

    static void SoundIDAssertValid(const t_audio_sys *const audio_sys, const t_sound_id id) {
        ZCL_ASSERT(id.index >= 0 && id.index < k_sound_limit);
        ZCL_ASSERT(id.version > 0 && id.version <= g_state.snd_insts.versions[id.index]);
    }

    t_audio_sys *internal::AudioStartup(zcl::t_arena *const arena) {
        ZCL_ASSERT(g_state.phase == ek_phase_inactive);

        g_state.phase = ek_phase_active;

        if (ma_engine_init(nullptr, &g_state.ma_eng) != MA_SUCCESS) {
            ZCL_FATAL();
        }

        return nullptr; // @temp
    }

    void internal::AudioShutdown(t_audio_sys *const sys) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);

        SoundsDestroyAll(sys);

        ma_engine_uninit(&g_state.ma_eng);

        zcl::ZeroClearItem(&g_state);
    }

    void internal::AudioSetFrozen(t_audio_sys *const sys, const zcl::t_b8 frozen) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);

        // Do not touch the per-sound "paused" state, we don't want want to lose that.

        if (frozen && g_state.phase == ek_phase_active) {
            ZCL_BITSET_WALK_ALL_SET (g_state.snd_insts.active, i) {
                ma_sound_stop(&g_state.snd_insts.ma_snds[i]);
            }

            g_state.phase = ek_phase_frozen;
        } else if (!frozen && g_state.phase == ek_phase_frozen) {
            ZCL_BITSET_WALK_ALL_SET (g_state.snd_insts.active, i) {
                ma_sound_start(&g_state.snd_insts.ma_snds[i]);
            }

            g_state.phase = ek_phase_active;
        }
    }

    t_sound_id SoundCreate(t_audio_sys *const audio_sys, const t_sound_type *const snd_type) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        ZCL_ASSERT(snd_type->valid);

        const zcl::t_i32 index = zcl::BitsetFindFirstUnsetBit(g_state.snd_insts.active);

        if (index == -1) {
            ZCL_FATAL();
        }

        g_state.snd_insts.types[index] = snd_type;

        ma_sound *const ma_snd = &g_state.snd_insts.ma_snds[index];

        if (!snd_type->streamable) {
            ma_audio_buffer_ref *const ma_buf_ref = &g_state.snd_insts.ma_buf_refs[index];

            if (ma_audio_buffer_ref_init(ma_format_f32, static_cast<ma_uint32>(snd_type->nonstream_snd_data.meta.channel_cnt), snd_type->nonstream_snd_data.pcm.raw, static_cast<ma_uint64>(snd_type->nonstream_snd_data.meta.frame_cnt), ma_buf_ref) != MA_SUCCESS) {
                ZCL_FATAL();
            }

            ma_buf_ref->sampleRate = static_cast<ma_uint32>(snd_type->nonstream_snd_data.meta.sample_rate);

            if (ma_sound_init_from_data_source(&g_state.ma_eng, ma_buf_ref, 0, nullptr, ma_snd) != MA_SUCCESS) {
                ZCL_FATAL();
            }
        } else {
            if (ma_sound_init_from_file(&g_state.ma_eng, zcl::StrToCStr(snd_type->stream_external_file_path_terminated), MA_SOUND_FLAG_STREAM, nullptr, nullptr, ma_snd) != MA_SUCCESS) {
                ZCL_FATAL();
            }
        }

        ma_sound_set_volume(ma_snd, 1.0f);
        ma_sound_set_pan(ma_snd, 0.0f);
        ma_sound_set_pitch(ma_snd, 1.0f);
        ma_sound_set_looping(ma_snd, false);

        zcl::BitsetSet(g_state.snd_insts.active, index);
        g_state.snd_insts.versions[index]++;

        return {index, g_state.snd_insts.versions[index]};
    }

    void SoundDestroy(t_audio_sys *const audio_sys, const t_sound_id id) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckExists(audio_sys, id));

        ma_sound *const ma_snd = &g_state.snd_insts.ma_snds[id.index];

        if (ma_sound_is_playing(ma_snd)) {
            if (ma_sound_stop(ma_snd) != MA_SUCCESS) {
                ZCL_FATAL();
            }
        }

        ma_sound_uninit(ma_snd);

        if (!g_state.snd_insts.types[id.index]->streamable) {
            ma_audio_buffer_ref_uninit(&g_state.snd_insts.ma_buf_refs[id.index]);
        }

        zcl::BitsetUnset(g_state.snd_insts.active, id.index);
    }

    void SoundStart(t_audio_sys *const audio_sys, const t_sound_id id) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckExists(audio_sys, id));
        ZCL_ASSERT(SoundGetState(audio_sys, id) == ek_sound_state_not_started);

        g_state.snd_insts.states[id.index] = ek_sound_state_playing;

        if (g_state.phase != ek_phase_frozen) {
            if (ma_sound_start(&g_state.snd_insts.ma_snds[id.index]) != MA_SUCCESS) {
                ZCL_FATAL();
            }
        }
    }

    void SoundPause(t_audio_sys *const audio_sys, const t_sound_id id) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckExists(audio_sys, id));
        ZCL_ASSERT(SoundGetState(audio_sys, id) == ek_sound_state_playing);

        g_state.snd_insts.states[id.index] = ek_sound_state_paused;

        if (g_state.phase != ek_phase_frozen) {
            if (ma_sound_stop(&g_state.snd_insts.ma_snds[id.index]) != MA_SUCCESS) {
                ZCL_FATAL();
            }
        }
    }

    void SoundResume(t_audio_sys *const audio_sys, const t_sound_id id) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckExists(audio_sys, id));
        ZCL_ASSERT(SoundGetState(audio_sys, id) == ek_sound_state_paused);

        g_state.snd_insts.states[id.index] = ek_sound_state_playing;

        if (g_state.phase != ek_phase_frozen) {
            if (ma_sound_start(&g_state.snd_insts.ma_snds[id.index]) != MA_SUCCESS) {
                ZCL_FATAL();
            }
        }
    }

    zcl::t_b8 SoundCheckExists(const t_audio_sys *const audio_sys, const t_sound_id id) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        SoundIDAssertValid(audio_sys, id);

        return zcl::BitsetCheckSet(g_state.snd_insts.active, id.index)
            && id.version == g_state.snd_insts.versions[id.index];
    }

    t_sound_state SoundGetState(const t_audio_sys *const audio_sys, const t_sound_id id) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckExists(audio_sys, id));

        return g_state.snd_insts.states[id.index];
    }

    zcl::t_f32 SoundGetVolume(t_audio_sys *const audio_sys, const t_sound_id id) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckExists(audio_sys, id));

        return ma_sound_get_volume(&g_state.snd_insts.ma_snds[id.index]);
    }

    zcl::t_f32 SoundGetPan(t_audio_sys *const audio_sys, const t_sound_id id) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckExists(audio_sys, id));

        return ma_sound_get_pan(&g_state.snd_insts.ma_snds[id.index]);
    }

    zcl::t_f32 SoundGetPitch(t_audio_sys *const audio_sys, const t_sound_id id) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckExists(audio_sys, id));

        return ma_sound_get_pitch(&g_state.snd_insts.ma_snds[id.index]);
    }

    zcl::t_b8 SoundCheckLooping(t_audio_sys *const audio_sys, const t_sound_id id) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckExists(audio_sys, id));

        return ma_sound_is_looping(&g_state.snd_insts.ma_snds[id.index]);
    }

    zcl::t_f32 SoundGetTrackPosition(t_audio_sys *const audio_sys, const t_sound_id id) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckExists(audio_sys, id));

        zcl::t_f32 result;

        if (ma_sound_get_cursor_in_seconds(&g_state.snd_insts.ma_snds[id.index], &result) != MA_SUCCESS) {
            ZCL_FATAL();
        }

        return result;
    }

    zcl::t_f32 SoundGetTrackDuration(t_audio_sys *const audio_sys, const t_sound_id id) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckExists(audio_sys, id));

        zcl::t_f32 result;

        if (ma_sound_get_length_in_seconds(&g_state.snd_insts.ma_snds[id.index], &result) != MA_SUCCESS) {
            ZCL_FATAL();
        }

        return result;
    }

    void SoundSetVolume(t_audio_sys *const audio_sys, const t_sound_id id, const zcl::t_f32 vol) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckExists(audio_sys, id));
        ZCL_ASSERT(zcl::RangeValueCheckWithin(k_sound_volume_range, vol));

        ma_sound_set_volume(&g_state.snd_insts.ma_snds[id.index], vol);
    }

    void SoundSetVolumeTransition(t_audio_sys *const audio_sys, const t_sound_id id, const zcl::t_f32 vol_begin, const zcl::t_f32 vol_end, const zcl::t_f32 dur_secs) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckExists(audio_sys, id));
        ZCL_ASSERT(zcl::RangeValueCheckWithin(k_sound_volume_range, vol_begin));
        ZCL_ASSERT(zcl::RangeValueCheckWithin(k_sound_volume_range, vol_end));
        ZCL_ASSERT(dur_secs >= 0.0f);

        ma_sound_set_fade_in_milliseconds(&g_state.snd_insts.ma_snds[id.index], vol_begin, vol_end, static_cast<ma_uint64>(1000.0f * dur_secs));
    }

    void SoundSetPan(t_audio_sys *const audio_sys, const t_sound_id id, const zcl::t_f32 pan) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckExists(audio_sys, id));
        ZCL_ASSERT(zcl::RangeValueCheckWithin(k_sound_pan_range, pan));

        ma_sound_set_pan(&g_state.snd_insts.ma_snds[id.index], pan);
    }

    void SoundSetPitch(t_audio_sys *const audio_sys, const t_sound_id id, const zcl::t_f32 pitch) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckExists(audio_sys, id));
        ZCL_ASSERT(zcl::RangeValueCheckWithin(k_sound_pitch_range, pitch));

        ma_sound_set_pitch(&g_state.snd_insts.ma_snds[id.index], pitch);
    }

    void SoundSetLooping(t_audio_sys *const audio_sys, const t_sound_id id, const zcl::t_b8 looping) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckExists(audio_sys, id));

        ma_sound_set_looping(&g_state.snd_insts.ma_snds[id.index], looping);
    }

    void SoundSetTrackPosition(t_audio_sys *const audio_sys, const t_sound_id id, const zcl::t_f32 pos_secs) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckExists(audio_sys, id));

        const zcl::t_range pos_secs_range = zcl::RangeCreate(0.0f, SoundGetTrackDuration(audio_sys, id));
        ZCL_ASSERT(zcl::RangeValueCheckWithin(pos_secs_range, pos_secs));

        ma_sound_seek_to_second(&g_state.snd_insts.ma_snds[id.index], pos_secs);
    }

    void SoundsDestroyAll(t_audio_sys *const audio_sys) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);

        ZCL_BITSET_WALK_ALL_SET (g_state.snd_insts.active, i) {
            SoundDestroy(audio_sys, {i, g_state.snd_insts.versions[i]});
        }
    }

    void SoundsDestroyAllOfType(t_audio_sys *const audio_sys, const t_sound_type *const snd_type) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);

        ZCL_BITSET_WALK_ALL_SET (g_state.snd_insts.active, i) {
            if (g_state.snd_insts.types[i] == snd_type) {
                SoundDestroy(audio_sys, {i, g_state.snd_insts.versions[i]});
            }
        }
    }

    void SoundsPauseAll(t_audio_sys *const audio_sys) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);

        ZCL_BITSET_WALK_ALL_SET (g_state.snd_insts.active, i) {
            const t_sound_id id = {i, g_state.snd_insts.versions[i]};

            if (SoundGetState(audio_sys, id) == ek_sound_state_playing) {
                SoundPause(audio_sys, id);
            }
        }
    }

    void SoundsPauseAllOfType(t_audio_sys *const audio_sys, const t_sound_type *const snd_type) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);

        ZCL_BITSET_WALK_ALL_SET (g_state.snd_insts.active, i) {
            const t_sound_id id = {i, g_state.snd_insts.versions[i]};

            if (g_state.snd_insts.types[i] == snd_type
                && SoundGetState(audio_sys, id) == ek_sound_state_playing) {
                SoundPause(audio_sys, id);
            }
        }
    }

    void SoundsResumeAll(t_audio_sys *const audio_sys) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);

        ZCL_BITSET_WALK_ALL_SET (g_state.snd_insts.active, i) {
            const t_sound_id id = {i, g_state.snd_insts.versions[i]};

            if (SoundGetState(audio_sys, id) == ek_sound_state_paused) {
                SoundResume(audio_sys, id);
            }
        }
    }

    void SoundsResumeAllOfType(t_audio_sys *const audio_sys, const t_sound_type *const snd_type) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);

        ZCL_BITSET_WALK_ALL_SET (g_state.snd_insts.active, i) {
            const t_sound_id id = {i, g_state.snd_insts.versions[i]};

            if (g_state.snd_insts.types[i] == snd_type
                && SoundGetState(audio_sys, id) == ek_sound_state_paused) {
                SoundResume(audio_sys, id);
            }
        }
    }

    void internal::SoundsProcessFinished(t_audio_sys *const audio_sys) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);

        if (g_state.phase == ek_phase_frozen) {
            return;
        }

        ZCL_BITSET_WALK_ALL_SET (g_state.snd_insts.active, i) {
            if (g_state.snd_insts.states[i] != ek_sound_state_playing) {
                continue;
            }

            ma_sound *const ma_snd = &g_state.snd_insts.ma_snds[i];

            if (!ma_sound_is_playing(ma_snd)) {
                SoundDestroy(audio_sys, {i, g_state.snd_insts.versions[i]});
            }
        }
    }
}
