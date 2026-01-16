#include <zgl/zgl_audio.h>

#include <miniaudio.h>

namespace zgl {
    struct t_sound_type_group {
        zcl::t_b8 valid;

        zcl::t_arena *arena;

        t_sound_type *head;
        t_sound_type *tail;
    };

    struct t_sound_type {
        zcl::t_b8 valid;

        zcl::t_b8 streamable;
        zcl::t_sound_data_rdonly nonstream_snd_data;
        zcl::t_str_rdonly stream_external_file_path_terminated; // Terminated since it often needs to be converted to C-string.

        t_sound_type *next;
    };

    constexpr zcl::t_i32 k_sound_limit = 32;

    struct t_audio_sys {
        zcl::t_b8 paused;

        ma_engine ma_eng;

        struct {
            zcl::t_static_array<ma_sound, k_sound_limit> ma_snds;
            zcl::t_static_array<ma_audio_buffer_ref, k_sound_limit> ma_buf_refs;
            zcl::t_static_array<const t_sound_type *, k_sound_limit> types;
            zcl::t_static_array<t_sound_state, k_sound_limit> states;
            zcl::t_static_bitset<k_sound_limit> active;
            zcl::t_static_array<zcl::t_i32, k_sound_limit> versions;
        } snd_insts;
    };

    static zcl::t_b8 g_module_active;

    static void SoundIDAssertValid(const t_audio_sys *const audio_sys, const t_sound_id id) {
        ZCL_ASSERT(id.index >= 0 && id.index < k_sound_limit);
        ZCL_ASSERT(id.version > 0 && id.version <= audio_sys->snd_insts.versions[id.index]);
    }

    t_audio_sys *detail::AudioStartup(zcl::t_arena *const arena) {
        ZCL_ASSERT(!g_module_active);

        g_module_active = true;

        const auto sys = zcl::ArenaPushItem<t_audio_sys>(arena);

        if (ma_engine_init(nullptr, &sys->ma_eng) != MA_SUCCESS) {
            ZCL_FATAL();
        }

        return sys;
    }

    void detail::AudioShutdown(t_audio_sys *const sys) {
        ZCL_ASSERT(g_module_active);

        SoundsDestroyAll(sys);

        ma_engine_uninit(&sys->ma_eng);

        g_module_active = false;
    }

    void detail::AudioSetPaused(t_audio_sys *const sys, const zcl::t_b8 paused) {
        ZCL_ASSERT(g_module_active);

        // Do not touch the per-sound "paused" state, we don't want want to lose that.

        if (paused && !sys->paused) {
            ZCL_BITSET_WALK_ALL_SET (sys->snd_insts.active, i) {
                ma_sound_stop(&sys->snd_insts.ma_snds[i]);
            }
        } else if (!paused && sys->paused) {
            ZCL_BITSET_WALK_ALL_SET (sys->snd_insts.active, i) {
                ma_sound_start(&sys->snd_insts.ma_snds[i]);
            }
        }

        sys->paused = paused;
    }

    t_sound_type_group *SoundTypeGroupCreate(t_audio_sys *const audio_sys, zcl::t_arena *const arena) {
        ZCL_ASSERT(g_module_active);

        const auto result = zcl::ArenaPushItem<t_sound_type_group>(arena);
        result->valid = true;
        result->arena = arena;
        return result;
    }

    void SoundTypeGroupDestroy(t_audio_sys *const audio_sys, t_sound_type_group *const group) {
        ZCL_ASSERT(g_module_active);
        ZCL_ASSERT(group->valid);

        // All sound instances of any of the sound types in the group need to be destroyed.
        t_sound_type *snd_type = group->head;

        while (snd_type) {
            ZCL_BITSET_WALK_ALL_SET (audio_sys->snd_insts.active, i) {
                if (audio_sys->snd_insts.types[i] == snd_type) {
                    SoundDestroy(audio_sys, {i, audio_sys->snd_insts.versions[i]});
                }
            }

            t_sound_type *const snd_type_next = snd_type->next;
            *snd_type = {};
            snd_type = snd_type_next;
        }

        *group = {};
    }

    static t_sound_type *SoundTypeGroupAdd(t_sound_type_group *const group) {
        const auto result = zcl::ArenaPushItem<t_sound_type>(group->arena);
        result->valid = true;

        if (!group->head) {
            group->head = result;
            group->tail = result;
        } else {
            group->tail->next = result;
            group->tail = result;
        }

        return result;
    }

    t_sound_type *SoundTypeCreateFromExternal(t_audio_sys *const audio_sys, const zcl::t_str_rdonly file_path, t_sound_type_group *const group, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(g_module_active);
        ZCL_ASSERT(group->valid);

        zcl::t_sound_data_mut snd_data;

        if (!zcl::SoundLoadFromExternal(file_path, group->arena, temp_arena, &snd_data)) {
            ZCL_FATAL();
        }

        t_sound_type *const result = SoundTypeGroupAdd(group);
        result->streamable = false;
        result->nonstream_snd_data = snd_data;

        return result;
    }

    t_sound_type *SoundTypeCreateFromPacked(t_audio_sys *const audio_sys, const zcl::t_str_rdonly file_path, t_sound_type_group *const group, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(g_module_active);
        ZCL_ASSERT(group->valid);

        zcl::t_file_stream file_stream;

        if (!zcl::FileOpen(file_path, zcl::t_file_access_mode::ek_file_access_mode_read, temp_arena, &file_stream)) {
            ZCL_FATAL();
        }

        zcl::t_sound_data_mut snd_data;

        if (!zcl::DeserializeSound(file_stream, group->arena, &snd_data)) {
            ZCL_FATAL();
        }

        zcl::FileClose(&file_stream);

        t_sound_type *const result = SoundTypeGroupAdd(group);
        result->streamable = false;
        result->nonstream_snd_data = snd_data;

        return result;
    }

    t_sound_type *SoundTypeCreateStreamable(t_audio_sys *const audio_sys, const zcl::t_str_rdonly external_file_path, t_sound_type_group *const group, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(g_module_active);
        ZCL_ASSERT(group->valid);

        t_sound_type *const result = SoundTypeGroupAdd(group);
        result->streamable = true;
        result->stream_external_file_path_terminated = zcl::StrCloneButAddTerminator(external_file_path, temp_arena);

        return result;
    }

    zcl::t_b8 SoundTypeCheckStreamable(t_audio_sys *const audio_sys, const t_sound_type *const type) {
        ZCL_ASSERT(g_module_active);
        ZCL_ASSERT(type->valid);

        return type->streamable;
    }

    t_sound_id SoundCreate(t_audio_sys *const audio_sys, const t_sound_type *const snd_type) {
        ZCL_ASSERT(g_module_active);
        ZCL_ASSERT(!audio_sys->paused);
        ZCL_ASSERT(snd_type->valid);

        const zcl::t_i32 index = zcl::BitsetFindFirstUnsetBit(audio_sys->snd_insts.active);

        if (index == -1) {
            ZCL_FATAL();
        }

        audio_sys->snd_insts.types[index] = snd_type;

        ma_sound *const ma_snd = &audio_sys->snd_insts.ma_snds[index];

        if (!snd_type->streamable) {
            ma_audio_buffer_ref *const ma_buf_ref = &audio_sys->snd_insts.ma_buf_refs[index];

            if (ma_audio_buffer_ref_init(ma_format_f32, static_cast<ma_uint32>(snd_type->nonstream_snd_data.meta.channel_cnt), snd_type->nonstream_snd_data.pcm.raw, static_cast<ma_uint64>(snd_type->nonstream_snd_data.meta.frame_cnt), ma_buf_ref) != MA_SUCCESS) {
                ZCL_FATAL();
            }

            ma_buf_ref->sampleRate = static_cast<ma_uint32>(snd_type->nonstream_snd_data.meta.sample_rate);

            if (ma_sound_init_from_data_source(&audio_sys->ma_eng, ma_buf_ref, 0, nullptr, ma_snd) != MA_SUCCESS) {
                ZCL_FATAL();
            }
        } else {
            if (ma_sound_init_from_file(&audio_sys->ma_eng, zcl::StrToCStr(snd_type->stream_external_file_path_terminated), MA_SOUND_FLAG_STREAM, nullptr, nullptr, ma_snd) != MA_SUCCESS) {
                ZCL_FATAL();
            }
        }

        // @todo: Could these be default anyway?
        ma_sound_set_volume(ma_snd, 1.0f);
        ma_sound_set_pan(ma_snd, 0.0f);
        ma_sound_set_pitch(ma_snd, 1.0f);
        ma_sound_set_looping(ma_snd, false);

        zcl::BitsetSet(audio_sys->snd_insts.active, index);
        audio_sys->snd_insts.versions[index]++;

        return {index, audio_sys->snd_insts.versions[index]};
    }

    void SoundDestroy(t_audio_sys *const audio_sys, const t_sound_id id) {
        ZCL_ASSERT(g_module_active);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckExists(audio_sys, id));

        ma_sound *const ma_snd = &audio_sys->snd_insts.ma_snds[id.index];

        if (ma_sound_is_playing(ma_snd)) {
            if (ma_sound_stop(ma_snd) != MA_SUCCESS) {
                ZCL_FATAL();
            }
        }

        ma_sound_uninit(ma_snd);

        if (!audio_sys->snd_insts.types[id.index]->streamable) {
            ma_audio_buffer_ref_uninit(&audio_sys->snd_insts.ma_buf_refs[id.index]);
        }

        zcl::BitsetUnset(audio_sys->snd_insts.active, id.index);
    }

    void SoundStart(t_audio_sys *const audio_sys, const t_sound_id id) {
        ZCL_ASSERT(g_module_active);
        ZCL_ASSERT(!audio_sys->paused);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckExists(audio_sys, id));
        ZCL_ASSERT(SoundGetState(audio_sys, id) == ek_sound_state_not_started);

        if (ma_sound_start(&audio_sys->snd_insts.ma_snds[id.index]) != MA_SUCCESS) {
            ZCL_FATAL();
        }

        audio_sys->snd_insts.states[id.index] = ek_sound_state_playing;
    }

    void SoundPause(t_audio_sys *const audio_sys, const t_sound_id id) {
        ZCL_ASSERT(g_module_active);
        ZCL_ASSERT(!audio_sys->paused);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckExists(audio_sys, id));
        ZCL_ASSERT(SoundGetState(audio_sys, id) == ek_sound_state_playing);

        if (ma_sound_stop(&audio_sys->snd_insts.ma_snds[id.index]) != MA_SUCCESS) {
            ZCL_FATAL();
        }

        audio_sys->snd_insts.states[id.index] = ek_sound_state_paused;
    }

    void SoundResume(t_audio_sys *const audio_sys, const t_sound_id id) {
        ZCL_ASSERT(g_module_active);
        ZCL_ASSERT(!audio_sys->paused);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckExists(audio_sys, id));
        ZCL_ASSERT(SoundGetState(audio_sys, id) == ek_sound_state_paused);

        if (ma_sound_start(&audio_sys->snd_insts.ma_snds[id.index]) != MA_SUCCESS) {
            ZCL_FATAL();
        }

        audio_sys->snd_insts.states[id.index] = ek_sound_state_playing;
    }

    zcl::t_b8 SoundCheckExists(const t_audio_sys *const audio_sys, const t_sound_id id) {
        ZCL_ASSERT(g_module_active);
        SoundIDAssertValid(audio_sys, id);

        return zcl::BitsetCheckSet(audio_sys->snd_insts.active, id.index)
            && id.version == audio_sys->snd_insts.versions[id.index];
    }

    t_sound_state SoundGetState(const t_audio_sys *const audio_sys, const t_sound_id id) {
        ZCL_ASSERT(g_module_active);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckExists(audio_sys, id));

        return audio_sys->snd_insts.states[id.index];
    }

    zcl::t_f32 SoundGetVolume(t_audio_sys *const audio_sys, const t_sound_id id) {
        ZCL_ASSERT(g_module_active);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckExists(audio_sys, id));

        return ma_sound_get_volume(&audio_sys->snd_insts.ma_snds[id.index]);
    }

    zcl::t_f32 SoundGetPan(t_audio_sys *const audio_sys, const t_sound_id id) {
        ZCL_ASSERT(g_module_active);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckExists(audio_sys, id));

        return ma_sound_get_pan(&audio_sys->snd_insts.ma_snds[id.index]);
    }

    zcl::t_f32 SoundGetPitch(t_audio_sys *const audio_sys, const t_sound_id id) {
        ZCL_ASSERT(g_module_active);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckExists(audio_sys, id));

        return ma_sound_get_pitch(&audio_sys->snd_insts.ma_snds[id.index]);
    }

    zcl::t_b8 SoundCheckLooping(t_audio_sys *const audio_sys, const t_sound_id id) {
        ZCL_ASSERT(g_module_active);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckExists(audio_sys, id));

        return ma_sound_is_looping(&audio_sys->snd_insts.ma_snds[id.index]);
    }

    zcl::t_f32 SoundGetTrackPosition(t_audio_sys *const audio_sys, const t_sound_id id) {
        ZCL_ASSERT(g_module_active);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckExists(audio_sys, id));

        zcl::t_f32 result;

        if (ma_sound_get_cursor_in_seconds(&audio_sys->snd_insts.ma_snds[id.index], &result) != MA_SUCCESS) {
            ZCL_FATAL();
        }

        return result;
    }

    zcl::t_f32 SoundGetTrackDuration(t_audio_sys *const audio_sys, const t_sound_id id) {
        ZCL_ASSERT(g_module_active);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckExists(audio_sys, id));

        zcl::t_f32 result;

        if (ma_sound_get_length_in_seconds(&audio_sys->snd_insts.ma_snds[id.index], &result) != MA_SUCCESS) {
            ZCL_FATAL();
        }

        return result;
    }

    void SoundSetVolume(t_audio_sys *const audio_sys, const t_sound_id id, const zcl::t_f32 vol) {
        ZCL_ASSERT(g_module_active);
        ZCL_ASSERT(!audio_sys->paused);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckExists(audio_sys, id));
        ZCL_ASSERT(zcl::RangeValueCheckWithin(k_sound_volume_range, vol));

        ma_sound_set_volume(&audio_sys->snd_insts.ma_snds[id.index], vol);
    }

    void SoundSetVolumeTransition(t_audio_sys *const audio_sys, const t_sound_id id, const zcl::t_f32 vol_begin, const zcl::t_f32 vol_end, const zcl::t_f32 dur_secs) {
        ZCL_ASSERT(g_module_active);
        ZCL_ASSERT(!audio_sys->paused);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckExists(audio_sys, id));
        ZCL_ASSERT(zcl::RangeValueCheckWithin(k_sound_volume_range, vol_begin));
        ZCL_ASSERT(zcl::RangeValueCheckWithin(k_sound_volume_range, vol_end));
        ZCL_ASSERT(dur_secs >= 0.0f);

        ma_sound_set_fade_in_milliseconds(&audio_sys->snd_insts.ma_snds[id.index], vol_begin, vol_end, static_cast<ma_uint64>(1000.0f * dur_secs));
    }

    void SoundSetPan(t_audio_sys *const audio_sys, const t_sound_id id, const zcl::t_f32 pan) {
        ZCL_ASSERT(g_module_active);
        ZCL_ASSERT(!audio_sys->paused);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckExists(audio_sys, id));
        ZCL_ASSERT(zcl::RangeValueCheckWithin(k_sound_pan_range, pan));

        ma_sound_set_pan(&audio_sys->snd_insts.ma_snds[id.index], pan);
    }

    void SoundSetPitch(t_audio_sys *const audio_sys, const t_sound_id id, const zcl::t_f32 pitch) {
        ZCL_ASSERT(g_module_active);
        ZCL_ASSERT(!audio_sys->paused);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckExists(audio_sys, id));
        ZCL_ASSERT(zcl::RangeValueCheckWithin(k_sound_pitch_range, pitch));

        ma_sound_set_pitch(&audio_sys->snd_insts.ma_snds[id.index], pitch);
    }

    void SoundSetLooping(t_audio_sys *const audio_sys, const t_sound_id id, const zcl::t_b8 looping) {
        ZCL_ASSERT(g_module_active);
        ZCL_ASSERT(!audio_sys->paused);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckExists(audio_sys, id));

        ma_sound_set_looping(&audio_sys->snd_insts.ma_snds[id.index], looping);
    }

    void SoundSetTrackPosition(t_audio_sys *const audio_sys, const t_sound_id id, const zcl::t_f32 pos_secs) {
        ZCL_ASSERT(g_module_active);
        ZCL_ASSERT(!audio_sys->paused);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckExists(audio_sys, id));

        const zcl::t_range pos_secs_range = zcl::RangeCreate(0.0f, SoundGetTrackDuration(audio_sys, id));
        ZCL_ASSERT(zcl::RangeValueCheckWithin(pos_secs_range, pos_secs));

        ma_sound_seek_to_second(&audio_sys->snd_insts.ma_snds[id.index], pos_secs);
    }

    void SoundsDestroyAll(t_audio_sys *const audio_sys) {
        ZCL_ASSERT(g_module_active);

        ZCL_BITSET_WALK_ALL_SET (audio_sys->snd_insts.active, i) {
            SoundDestroy(audio_sys, {i, audio_sys->snd_insts.versions[i]});
        }
    }

    void SoundsPauseAll(t_audio_sys *const audio_sys) {
        ZCL_ASSERT(g_module_active);
        ZCL_ASSERT(!audio_sys->paused);

        ZCL_BITSET_WALK_ALL_SET (audio_sys->snd_insts.active, i) {
            const t_sound_id id = {i, audio_sys->snd_insts.versions[i]};

            if (SoundGetState(audio_sys, id) == ek_sound_state_playing) {
                SoundPause(audio_sys, id);
            }
        }
    }

    void SoundsResumeAll(t_audio_sys *const audio_sys) {
        ZCL_ASSERT(g_module_active);
        ZCL_ASSERT(!audio_sys->paused);

        ZCL_BITSET_WALK_ALL_SET (audio_sys->snd_insts.active, i) {
            const t_sound_id id = {i, audio_sys->snd_insts.versions[i]};

            if (SoundGetState(audio_sys, id) == ek_sound_state_paused) {
                SoundResume(audio_sys, id);
            }
        }
    }

    void detail::SoundsProcessFinished(t_audio_sys *const audio_sys) {
        ZCL_ASSERT(g_module_active);
        ZCL_ASSERT(!audio_sys->paused);

        ZCL_BITSET_WALK_ALL_SET (audio_sys->snd_insts.active, i) {
            if (audio_sys->snd_insts.states[i] != ek_sound_state_playing) {
                continue;
            }

            ma_sound *const ma_snd = &audio_sys->snd_insts.ma_snds[i];

            if (!ma_sound_is_playing(ma_snd)) {
                SoundDestroy(audio_sys, {i, audio_sys->snd_insts.versions[i]});
            }
        }
    }
}
