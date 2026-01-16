#include <zgl/zgl_audio.h>

#include <miniaudio.h>

namespace zgl {
    struct t_sound_type_group {
        zcl::t_arena *arena;

        t_sound_type *head;
        t_sound_type *tail;
    };

    struct t_sound_type {
        zcl::t_b8 valid;

        zcl::t_b8 stream;
        zcl::t_sound_data_rdonly nonstream_snd_data;
        zcl::t_str_rdonly stream_external_file_path_terminated; // Terminated since it often needs to be converted to C-string.

        t_sound_type *next;
    };

    constexpr zcl::t_i32 k_sound_limit = 32;

    struct t_audio_sys {
        ma_engine ma_eng;

        struct {
            zcl::t_static_array<ma_sound, k_sound_limit> ma_snds;
            zcl::t_static_array<ma_audio_buffer_ref, k_sound_limit> ma_buf_refs;
            zcl::t_static_array<const t_sound_type *, k_sound_limit> types;
            zcl::t_static_bitset<k_sound_limit> active;
            zcl::t_static_bitset<k_sound_limit> paused;
            zcl::t_static_array<zcl::t_i32, k_sound_limit> versions;
        } snd_insts;
    };

    static zcl::t_b8 g_module_active;

    static void SoundIDAssertValid(const t_audio_sys *const audio_sys, const t_sound_id id) {
        ZCL_ASSERT(id.index >= 0 && id.index < k_sound_limit);
        ZCL_ASSERT(id.version > 0 && id.version <= audio_sys->snd_insts.versions[id.index]);
    }

    static zcl::t_b8 VolumeCheckValid(const zcl::t_f32 vol) {
        return vol >= 0.0f && vol <= 1.0f;
    }

    static zcl::t_b8 PanCheckValid(const zcl::t_f32 pan) {
        return pan >= -1.0f && pan <= 1.0f;
    }

    static zcl::t_b8 PitchCheckValid(const zcl::t_f32 pitch) {
        return pitch > 0.0f;
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

        // @todo: Can be replaced with SoundsStopAll().
        ZCL_BITSET_WALK_ALL_SET (sys->snd_insts.active, i) {
            ma_sound_stop(&sys->snd_insts.ma_snds[i]);
            ma_sound_uninit(&sys->snd_insts.ma_snds[i]);

            // @cleanup
            if (!sys->snd_insts.types[i]->stream) {
                ma_audio_buffer_ref_uninit(&sys->snd_insts.ma_buf_refs[i]);
            }
        }

        ma_engine_uninit(&sys->ma_eng);

        g_module_active = false;
    }

    t_sound_type_group *SoundTypeGroupCreate(t_audio_sys *const audio_sys, zcl::t_arena *const arena) {
        ZCL_ASSERT(g_module_active);

        const auto result = zcl::ArenaPushItem<t_sound_type_group>(arena);
        result->arena = arena;
        return result;
    }

    void SoundTypeGroupDestroy(t_audio_sys *const audio_sys, t_sound_type_group *const group) {
        ZCL_ASSERT(g_module_active);

        t_sound_type *snd_type = group->head;

        while (snd_type) {
            ZCL_BITSET_WALK_ALL_SET (audio_sys->snd_insts.active, i) {
                if (audio_sys->snd_insts.types[i] == snd_type) {
                    SoundStop(audio_sys, {i, audio_sys->snd_insts.versions[i]});
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

        zcl::t_sound_data_mut snd_data;

        if (!zcl::SoundLoadFromExternal(file_path, group->arena, temp_arena, &snd_data)) {
            ZCL_FATAL();
        }

        t_sound_type *const result = SoundTypeGroupAdd(group);
        result->stream = false;
        result->nonstream_snd_data = snd_data;

        return result;
    }

    t_sound_type *SoundTypeCreateFromPacked(t_audio_sys *const audio_sys, const zcl::t_str_rdonly file_path, t_sound_type_group *const group, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(g_module_active);

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
        result->stream = false;
        result->nonstream_snd_data = snd_data;

        return result;
    }

    t_sound_type *SoundTypeCreateStreamable(t_audio_sys *const audio_sys, const zcl::t_str_rdonly external_file_path, t_sound_type_group *const group, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(g_module_active);

        t_sound_type *const result = SoundTypeGroupAdd(group);
        result->stream = true;
        result->stream_external_file_path_terminated = zcl::StrCloneButAddTerminator(external_file_path, temp_arena);

        return result;
    }

    zcl::t_b8 SoundPlayAndGetID(t_audio_sys *const audio_sys, const t_sound_type *const type, t_sound_id *const o_id, const zcl::t_f32 vol, const zcl::t_f32 pan, const zcl::t_f32 pitch, const zcl::t_b8 loop) {
        ZCL_ASSERT(g_module_active);
        ZCL_ASSERT(type->valid);
        ZCL_ASSERT(VolumeCheckValid(vol));
        ZCL_ASSERT(PanCheckValid(pan));
        ZCL_ASSERT(PitchCheckValid(pitch));

        const zcl::t_i32 index = zcl::BitsetFindFirstUnsetBit(audio_sys->snd_insts.active);

        if (index == -1) {
            zcl::LogWarning(ZCL_STR_LITERAL("Trying to play a sound, but the sound instance limit has been reached!"));
            return false;
        }

        audio_sys->snd_insts.types[index] = type;

        ma_sound *const ma_snd = &audio_sys->snd_insts.ma_snds[index];

        if (!type->stream) {
            ma_audio_buffer_ref *const ma_buf_ref = &audio_sys->snd_insts.ma_buf_refs[index];

            if (ma_audio_buffer_ref_init(ma_format_f32, static_cast<ma_uint32>(type->nonstream_snd_data.meta.channel_cnt), type->nonstream_snd_data.pcm.raw, static_cast<ma_uint64>(type->nonstream_snd_data.meta.frame_cnt), ma_buf_ref) != MA_SUCCESS) {
                ZCL_FATAL();
            }

            ma_buf_ref->sampleRate = static_cast<ma_uint32>(type->nonstream_snd_data.meta.sample_rate);

            if (ma_sound_init_from_data_source(&audio_sys->ma_eng, ma_buf_ref, 0, nullptr, ma_snd) != MA_SUCCESS) {
                ZCL_FATAL();
            }
        } else {
            if (ma_sound_init_from_file(&audio_sys->ma_eng, zcl::StrToCStr(type->stream_external_file_path_terminated), MA_SOUND_FLAG_STREAM, nullptr, nullptr, ma_snd) != MA_SUCCESS) {
                ZCL_FATAL();
            }
        }

        ma_sound_set_volume(ma_snd, vol);
        ma_sound_set_pan(ma_snd, pan);
        ma_sound_set_pitch(ma_snd, pitch);
        ma_sound_set_looping(ma_snd, loop);

        if (ma_sound_start(ma_snd) != MA_SUCCESS) {
            ZCL_FATAL();
        }

        zcl::BitsetSet(audio_sys->snd_insts.active, index);
        audio_sys->snd_insts.versions[index]++;

        *o_id = {index, audio_sys->snd_insts.versions[index]};

        return true;
    }

    void SoundStop(t_audio_sys *const audio_sys, const t_sound_id id) {
        ZCL_ASSERT(g_module_active);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckActive(audio_sys, id));

        ma_sound *const ma_snd = &audio_sys->snd_insts.ma_snds[id.index];

        if (ma_sound_is_playing(ma_snd)) {
            ma_sound_stop(ma_snd);
        }

        ma_sound_uninit(ma_snd);

        // @cleanup
        if (!audio_sys->snd_insts.types[id.index]->stream) {
            ma_audio_buffer_ref_uninit(&audio_sys->snd_insts.ma_buf_refs[id.index]);
        }

        zcl::BitsetUnset(audio_sys->snd_insts.active, id.index);
        zcl::BitsetUnset(audio_sys->snd_insts.paused, id.index);
    }

    void SoundPause(t_audio_sys *const audio_sys, const t_sound_id id) {
        ZCL_ASSERT(g_module_active);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckActive(audio_sys, id));
        ZCL_ASSERT(!SoundCheckPaused(audio_sys, id));

        ma_sound_stop(&audio_sys->snd_insts.ma_snds[id.index]);

        zcl::BitsetSet(audio_sys->snd_insts.paused, id.index);
    }

    void SoundResume(t_audio_sys *const audio_sys, const t_sound_id id) {
        ZCL_ASSERT(g_module_active);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckActive(audio_sys, id));
        ZCL_ASSERT(SoundCheckPaused(audio_sys, id));

        ma_sound_start(&audio_sys->snd_insts.ma_snds[id.index]);

        zcl::BitsetUnset(audio_sys->snd_insts.paused, id.index);
    }

    zcl::t_b8 SoundCheckActive(const t_audio_sys *const audio_sys, const t_sound_id id) {
        ZCL_ASSERT(g_module_active);
        SoundIDAssertValid(audio_sys, id);

        return zcl::BitsetCheckSet(audio_sys->snd_insts.active, id.index)
            && id.version == audio_sys->snd_insts.versions[id.index];
    }

    zcl::t_b8 SoundCheckPaused(const t_audio_sys *const audio_sys, const t_sound_id id) {
        ZCL_ASSERT(g_module_active);
        SoundIDAssertValid(audio_sys, id);

        return SoundCheckActive(audio_sys, id) && zcl::BitsetCheckSet(audio_sys->snd_insts.paused, id.index);
    }

    void SoundSetVolume(t_audio_sys *const audio_sys, const t_sound_id id, const zcl::t_f32 vol) {
        ZCL_ASSERT(g_module_active);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckActive(audio_sys, id));
        ZCL_ASSERT(VolumeCheckValid(vol));

        ma_sound_set_volume(&audio_sys->snd_insts.ma_snds[id.index], vol);
    }

    void SoundSetPan(t_audio_sys *const audio_sys, const t_sound_id id, const zcl::t_f32 pan) {
        ZCL_ASSERT(g_module_active);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckActive(audio_sys, id));
        ZCL_ASSERT(PanCheckValid(pan));

        ma_sound_set_pan(&audio_sys->snd_insts.ma_snds[id.index], pan);
    }

    void SoundSetPitch(t_audio_sys *const audio_sys, const t_sound_id id, const zcl::t_f32 pitch) {
        ZCL_ASSERT(g_module_active);
        SoundIDAssertValid(audio_sys, id);
        ZCL_ASSERT(SoundCheckActive(audio_sys, id));
        ZCL_ASSERT(PitchCheckValid(pitch));

        ma_sound_set_pitch(&audio_sys->snd_insts.ma_snds[id.index], pitch);
    }

    void detail::SoundsProcessFinished(t_audio_sys *const audio_sys) {
        ZCL_ASSERT(g_module_active);

        ZCL_BITSET_WALK_ALL_SET (audio_sys->snd_insts.active, i) {
            if (zcl::BitsetCheckSet(audio_sys->snd_insts.paused, i)) {
                continue;
            }

            ma_sound *const ma_snd = &audio_sys->snd_insts.ma_snds[i];

            if (!ma_sound_is_playing(ma_snd)) {
                SoundStop(audio_sys, {i, audio_sys->snd_insts.versions[i]});
            }
        }
    }
}
