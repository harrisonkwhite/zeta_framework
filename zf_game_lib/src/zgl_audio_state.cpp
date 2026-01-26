#include <zgl/zgl_audio_private.h>

#include <miniaudio.h>

namespace zgl {
    constexpr zcl::t_i32 k_sound_limit = 32;

    enum t_phase {
        ek_phase_inactive,
        ek_phase_active,
        ek_phase_frozen // No audio actually plays, though sound properties can be mutated.
    };

    static struct {
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

    static void SoundIDAssertValid(const t_sound_id id) {
        ZCL_ASSERT(id.index >= 0 && id.index < k_sound_limit);
        ZCL_ASSERT(id.version > 0 && id.version <= g_state.snd_insts.versions[id.index]);
    }

    t_audio_ticket_mut internal::AudioStartup(zcl::t_arena *const arena) {
        ZCL_ASSERT(g_state.phase == ek_phase_inactive);

        g_state.phase = ek_phase_active;

        if (ma_engine_init(nullptr, &g_state.ma_eng) != MA_SUCCESS) {
            ZCL_FATAL();
        }

        return TicketCreate();
    }

    void internal::AudioShutdown(const t_audio_ticket_mut ticket) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        ZCL_ASSERT(TicketCheckValid(ticket));

        ZCL_BITSET_WALK_ALL_SET (g_state.snd_insts.active, i) {
            SoundDestroy(ticket, {i, g_state.snd_insts.versions[i]});
        }

        ma_engine_uninit(&g_state.ma_eng);

        zcl::ZeroClearItem(&g_state);
    }

    void internal::AudioSetFrozen(const t_audio_ticket_mut ticket, const zcl::t_b8 frozen) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        ZCL_ASSERT(TicketCheckValid(ticket));

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

    zcl::t_b8 SoundCreate(const t_audio_ticket_mut audio_ticket, const t_sound_type *const snd_type, t_sound_id *const o_id) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        ZCL_ASSERT(TicketCheckValid(audio_ticket));
        ZCL_ASSERT(snd_type->valid);

        const zcl::t_i32 index = zcl::BitsetFindFirstUnset(g_state.snd_insts.active);

        if (index == -1) {
            // Giving a warning for this instead of fatal error because it's quite an easy thing to hit and can be often recovered from.
            zcl::LogWarning(ZCL_STR_LITERAL("Trying to create a sound, but the limit has been reached!"));
            return false;
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
            if (ma_sound_init_from_file(&g_state.ma_eng, zcl::StrToCStr(snd_type->stream_unbuilt_file_path_terminated), MA_SOUND_FLAG_STREAM, nullptr, nullptr, ma_snd) != MA_SUCCESS) {
                ZCL_FATAL();
            }
        }

        ma_sound_set_volume(ma_snd, 1.0f);
        ma_sound_set_pan(ma_snd, 0.0f);
        ma_sound_set_pitch(ma_snd, 1.0f);
        ma_sound_set_looping(ma_snd, false);

        zcl::BitsetSet(g_state.snd_insts.active, index);
        g_state.snd_insts.versions[index]++;

        *o_id = {index, g_state.snd_insts.versions[index]};

        return true;
    }

    void SoundDestroy(const t_audio_ticket_mut audio_ticket, const t_sound_id id) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        ZCL_ASSERT(TicketCheckValid(audio_ticket));
        SoundIDAssertValid(id);
        ZCL_ASSERT(SoundCheckExists(audio_ticket, id));

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

    void SoundStart(const t_audio_ticket_mut audio_ticket, const t_sound_id id) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        ZCL_ASSERT(TicketCheckValid(audio_ticket));
        SoundIDAssertValid(id);
        ZCL_ASSERT(SoundCheckExists(audio_ticket, id));
        ZCL_ASSERT(SoundGetState(audio_ticket, id) == ek_sound_state_not_started);

        g_state.snd_insts.states[id.index] = ek_sound_state_playing;

        if (g_state.phase != ek_phase_frozen) {
            if (ma_sound_start(&g_state.snd_insts.ma_snds[id.index]) != MA_SUCCESS) {
                ZCL_FATAL();
            }
        }
    }

    void SoundPause(const t_audio_ticket_mut audio_ticket, const t_sound_id id) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        ZCL_ASSERT(TicketCheckValid(audio_ticket));
        SoundIDAssertValid(id);
        ZCL_ASSERT(SoundCheckExists(audio_ticket, id));
        ZCL_ASSERT(SoundGetState(audio_ticket, id) == ek_sound_state_playing);

        g_state.snd_insts.states[id.index] = ek_sound_state_paused;

        if (g_state.phase != ek_phase_frozen) {
            if (ma_sound_stop(&g_state.snd_insts.ma_snds[id.index]) != MA_SUCCESS) {
                ZCL_FATAL();
            }
        }
    }

    void SoundResume(const t_audio_ticket_mut audio_ticket, const t_sound_id id) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        ZCL_ASSERT(TicketCheckValid(audio_ticket));
        SoundIDAssertValid(id);
        ZCL_ASSERT(SoundCheckExists(audio_ticket, id));
        ZCL_ASSERT(SoundGetState(audio_ticket, id) == ek_sound_state_paused);

        g_state.snd_insts.states[id.index] = ek_sound_state_playing;

        if (g_state.phase != ek_phase_frozen) {
            if (ma_sound_start(&g_state.snd_insts.ma_snds[id.index]) != MA_SUCCESS) {
                ZCL_FATAL();
            }
        }
    }

    zcl::t_b8 SoundCheckExists(const t_audio_ticket_rdonly audio_ticket, const t_sound_id id) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        ZCL_ASSERT(TicketCheckValid(audio_ticket));
        SoundIDAssertValid(id);

        return zcl::BitsetCheckSet(g_state.snd_insts.active, id.index)
            && id.version == g_state.snd_insts.versions[id.index];
    }

    t_sound_state SoundGetState(const t_audio_ticket_rdonly audio_ticket, const t_sound_id id) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        ZCL_ASSERT(TicketCheckValid(audio_ticket));
        SoundIDAssertValid(id);
        ZCL_ASSERT(SoundCheckExists(audio_ticket, id));

        return g_state.snd_insts.states[id.index];
    }

    const t_sound_type *SoundGetType(const t_audio_ticket_rdonly audio_ticket, const t_sound_id id) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        ZCL_ASSERT(TicketCheckValid(audio_ticket));
        SoundIDAssertValid(id);
        ZCL_ASSERT(SoundCheckExists(audio_ticket, id));

        return g_state.snd_insts.types[id.index];
    }

    zcl::t_f32 SoundGetVolume(const t_audio_ticket_rdonly audio_ticket, const t_sound_id id) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        ZCL_ASSERT(TicketCheckValid(audio_ticket));
        SoundIDAssertValid(id);
        ZCL_ASSERT(SoundCheckExists(audio_ticket, id));

        return ma_sound_get_volume(&g_state.snd_insts.ma_snds[id.index]);
    }

    zcl::t_f32 SoundGetPan(const t_audio_ticket_rdonly audio_ticket, const t_sound_id id) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        ZCL_ASSERT(TicketCheckValid(audio_ticket));
        SoundIDAssertValid(id);
        ZCL_ASSERT(SoundCheckExists(audio_ticket, id));

        return ma_sound_get_pan(&g_state.snd_insts.ma_snds[id.index]);
    }

    zcl::t_f32 SoundGetPitch(const t_audio_ticket_rdonly audio_ticket, const t_sound_id id) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        ZCL_ASSERT(TicketCheckValid(audio_ticket));
        SoundIDAssertValid(id);
        ZCL_ASSERT(SoundCheckExists(audio_ticket, id));

        return ma_sound_get_pitch(&g_state.snd_insts.ma_snds[id.index]);
    }

    zcl::t_b8 SoundCheckLooping(const t_audio_ticket_rdonly audio_ticket, const t_sound_id id) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        ZCL_ASSERT(TicketCheckValid(audio_ticket));
        SoundIDAssertValid(id);
        ZCL_ASSERT(SoundCheckExists(audio_ticket, id));

        return ma_sound_is_looping(&g_state.snd_insts.ma_snds[id.index]);
    }

    zcl::t_f32 SoundGetTrackPosition(const t_audio_ticket_rdonly audio_ticket, const t_sound_id id) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        ZCL_ASSERT(TicketCheckValid(audio_ticket));
        SoundIDAssertValid(id);
        ZCL_ASSERT(SoundCheckExists(audio_ticket, id));

        zcl::t_f32 result;

        if (ma_sound_get_cursor_in_seconds(&g_state.snd_insts.ma_snds[id.index], &result) != MA_SUCCESS) {
            ZCL_FATAL();
        }

        return result;
    }

    zcl::t_f32 SoundGetTrackDuration(const t_audio_ticket_rdonly audio_ticket, const t_sound_id id) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        ZCL_ASSERT(TicketCheckValid(audio_ticket));
        SoundIDAssertValid(id);
        ZCL_ASSERT(SoundCheckExists(audio_ticket, id));

        zcl::t_f32 result;

        if (ma_sound_get_length_in_seconds(&g_state.snd_insts.ma_snds[id.index], &result) != MA_SUCCESS) {
            ZCL_FATAL();
        }

        return result;
    }

    void SoundSetVolume(const t_audio_ticket_mut audio_ticket, const t_sound_id id, const zcl::t_f32 vol) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        ZCL_ASSERT(TicketCheckValid(audio_ticket));
        SoundIDAssertValid(id);
        ZCL_ASSERT(SoundCheckExists(audio_ticket, id));
        ZCL_ASSERT(zcl::RangeValueCheckWithin(k_sound_volume_range, vol));

        ma_sound_set_volume(&g_state.snd_insts.ma_snds[id.index], vol);
    }

    void SoundSetVolumeTransition(const t_audio_ticket_mut audio_ticket, const t_sound_id id, const zcl::t_f32 vol_begin, const zcl::t_f32 vol_end, const zcl::t_f32 dur_secs) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        ZCL_ASSERT(TicketCheckValid(audio_ticket));
        SoundIDAssertValid(id);
        ZCL_ASSERT(SoundCheckExists(audio_ticket, id));
        ZCL_ASSERT(zcl::RangeValueCheckWithin(k_sound_volume_range, vol_begin));
        ZCL_ASSERT(zcl::RangeValueCheckWithin(k_sound_volume_range, vol_end));
        ZCL_ASSERT(dur_secs >= 0.0f);

        ma_sound_set_fade_in_milliseconds(&g_state.snd_insts.ma_snds[id.index], vol_begin, vol_end, static_cast<ma_uint64>(1000.0f * dur_secs));
    }

    void SoundSetPan(const t_audio_ticket_mut audio_ticket, const t_sound_id id, const zcl::t_f32 pan) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        ZCL_ASSERT(TicketCheckValid(audio_ticket));
        SoundIDAssertValid(id);
        ZCL_ASSERT(SoundCheckExists(audio_ticket, id));
        ZCL_ASSERT(zcl::RangeValueCheckWithin(k_sound_pan_range, pan));

        ma_sound_set_pan(&g_state.snd_insts.ma_snds[id.index], pan);
    }

    void SoundSetPitch(const t_audio_ticket_mut audio_ticket, const t_sound_id id, const zcl::t_f32 pitch) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        ZCL_ASSERT(TicketCheckValid(audio_ticket));
        SoundIDAssertValid(id);
        ZCL_ASSERT(SoundCheckExists(audio_ticket, id));
        ZCL_ASSERT(zcl::RangeValueCheckWithin(k_sound_pitch_range, pitch));

        ma_sound_set_pitch(&g_state.snd_insts.ma_snds[id.index], pitch);
    }

    void SoundSetLooping(const t_audio_ticket_mut audio_ticket, const t_sound_id id, const zcl::t_b8 looping) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        ZCL_ASSERT(TicketCheckValid(audio_ticket));
        SoundIDAssertValid(id);
        ZCL_ASSERT(SoundCheckExists(audio_ticket, id));

        ma_sound_set_looping(&g_state.snd_insts.ma_snds[id.index], looping);
    }

    void SoundSetTrackPosition(const t_audio_ticket_mut audio_ticket, const t_sound_id id, const zcl::t_f32 pos_secs) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        ZCL_ASSERT(TicketCheckValid(audio_ticket));
        SoundIDAssertValid(id);
        ZCL_ASSERT(SoundCheckExists(audio_ticket, id));

        const zcl::t_range pos_secs_range = zcl::RangeCreate(0.0f, SoundGetTrackDuration(audio_ticket, id));
        ZCL_ASSERT(zcl::RangeValueCheckWithin(pos_secs_range, pos_secs));

        ma_sound_seek_to_second(&g_state.snd_insts.ma_snds[id.index], pos_secs);
    }

    zcl::t_array_mut<t_sound_id> SoundsGetExisting(const t_audio_ticket_rdonly audio_ticket, zcl::t_arena *const arena) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        ZCL_ASSERT(TicketCheckValid(audio_ticket));

        const auto ids = zcl::ArenaPushArray<t_sound_id>(arena, zcl::BitsetCountSet(g_state.snd_insts.active));
        zcl::t_i32 ids_index = 0;

        ZCL_BITSET_WALK_ALL_SET (g_state.snd_insts.active, i) {
            ids[ids_index] = {i, g_state.snd_insts.versions[i]};
            ids_index++;
        }

        return ids;
    }

    void internal::SoundsProcessFinished(const t_audio_ticket_mut audio_ticket) {
        ZCL_ASSERT(g_state.phase != ek_phase_inactive);
        ZCL_ASSERT(TicketCheckValid(audio_ticket));

        if (g_state.phase == ek_phase_frozen) {
            return;
        }

        ZCL_BITSET_WALK_ALL_SET (g_state.snd_insts.active, i) {
            if (g_state.snd_insts.states[i] != ek_sound_state_playing) {
                continue;
            }

            ma_sound *const ma_snd = &g_state.snd_insts.ma_snds[i];

            if (!ma_sound_is_playing(ma_snd)) {
                SoundDestroy(audio_ticket, {i, g_state.snd_insts.versions[i]});
            }
        }
    }
}
