#include <zgl/zgl_audio.h>

#include <miniaudio.h>

namespace zgl::audio {
    struct t_sound_type {
        zcl::t_b8 valid;
        zcl::t_sound_data_rdonly snd_data;
        t_sound_type *next;
    };

    const zcl::t_i32 g_sound_inst_limit = 32;

    struct {
        zcl::t_b8 active;

        ma_engine ma_eng;

        struct {
            zcl::t_static_array<ma_sound, g_sound_inst_limit> ma_snds;
            zcl::t_static_array<ma_audio_buffer_ref, g_sound_inst_limit> ma_buf_refs;
            zcl::t_static_array<const t_sound_type *, g_sound_inst_limit> types;
            zcl::t_static_bitset<g_sound_inst_limit> activity;
            zcl::t_static_array<zcl::t_i32, g_sound_inst_limit> versions;
        } snd_insts;
    } g_module_state;

    void ModuleStartup() {
        ZCL_REQUIRE(!g_module_state.active);

        g_module_state = {.active = true};

        if (ma_engine_init(nullptr, &g_module_state.ma_eng) != MA_SUCCESS) {
            ZCL_FATAL();
        }
    }

    void ModuleShutdown() {
        ZCL_REQUIRE(g_module_state.active);

        ZCL_BITSET_WALK_ALL_SET (g_module_state.snd_insts.activity, i) {
            ma_sound_stop(&g_module_state.snd_insts.ma_snds[i]);
            ma_sound_uninit(&g_module_state.snd_insts.ma_snds[i]);

            ma_audio_buffer_ref_uninit(&g_module_state.snd_insts.ma_buf_refs[i]);
        }

        ma_engine_uninit(&g_module_state.ma_eng);

        g_module_state = {};
    }

    void SoundTypeGroupDestroy(t_sound_type_group *const group) {
        ZCL_ASSERT(g_module_state.active);

        t_sound_type *snd_type = group->head;

        while (snd_type) {
            ZCL_BITSET_WALK_ALL_SET (g_module_state.snd_insts.activity, i) {
                if (g_module_state.snd_insts.types[i] == snd_type) {
                    SoundStop({i, g_module_state.snd_insts.versions[i]});
                }
            }

            t_sound_type *const snd_type_next = snd_type->next;
            *snd_type = {};
            snd_type = snd_type_next;
        }

        zcl::ArenaDestroy(&group->arena);
        *group = {};
    }

    static t_sound_type *SoundTypeGroupAdd(t_sound_type_group *const group, const zcl::t_sound_data_rdonly snd_data) {
        const auto result = zcl::ArenaPushItem<t_sound_type>(&group->arena);
        result->valid = true;
        result->snd_data = snd_data;

        if (!group->head) {
            group->head = result;
            group->tail = result;
        } else {
            group->tail->next = result;
            group->tail = result;
        }

        return result;
    }

    t_sound_type *SoundTypeCreateFromRaw(const zcl::t_str_rdonly file_path, t_sound_type_group *const group, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(g_module_state.active);

        zcl::t_sound_data_mut snd_data;

        if (!zcl::SoundLoadFromRaw(file_path, &group->arena, temp_arena, &snd_data)) {
            ZCL_FATAL();
        }

        return SoundTypeGroupAdd(group, snd_data);
    }

    t_sound_type *SoundTypeCreateFromPacked(const zcl::t_str_rdonly file_path, t_sound_type_group *const group, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(g_module_state.active);

        zcl::t_file_stream file_stream;

        if (!zcl::FileOpen(file_path, zcl::t_file_access_mode::ek_file_access_mode_read, temp_arena, &file_stream)) {
            ZCL_FATAL();
        }

        zcl::t_sound_data_mut snd_data;

        if (!zcl::DeserializeSound(file_stream, &group->arena, &snd_data)) {
            ZCL_FATAL();
        }

        zcl::FileClose(&file_stream);

        return SoundTypeGroupAdd(group, snd_data);
    }

    zcl::t_b8 SoundPlayAndGetID(const t_sound_type *const type, t_sound_id *const o_id, const zcl::t_f32 vol, const zcl::t_f32 pan, const zcl::t_f32 pitch, const zcl::t_b8 loop) {
        ZCL_ASSERT(g_module_state.active);
        ZCL_ASSERT(type->valid);
        ZCL_ASSERT(vol >= 0.0f && vol <= 1.0f);
        ZCL_ASSERT(pan >= -1.0f && pan <= 1.0f);
        ZCL_ASSERT(pitch > 0.0f);

        const zcl::t_i32 index = zcl::BitsetFindFirstUnsetBit(g_module_state.snd_insts.activity);

        if (index == -1) {
            zcl::LogWarning(ZCL_STR_LITERAL("Trying to play a sound, but the sound instance limit has been reached!"));
            return false;
        }

        ma_sound *const ma_snd = &g_module_state.snd_insts.ma_snds[index];
        ma_audio_buffer_ref *const ma_buf_ref = &g_module_state.snd_insts.ma_buf_refs[index];

        g_module_state.snd_insts.types[index] = type;

        if (ma_audio_buffer_ref_init(ma_format_f32, static_cast<ma_uint32>(type->snd_data.meta.channel_cnt), type->snd_data.pcm.raw, static_cast<ma_uint64>(type->snd_data.meta.frame_cnt), ma_buf_ref) != MA_SUCCESS) {
            ZCL_FATAL();
        }

        ma_buf_ref->sampleRate = static_cast<ma_uint32>(type->snd_data.meta.sample_rate);

        if (ma_sound_init_from_data_source(&g_module_state.ma_eng, ma_buf_ref, 0, nullptr, ma_snd) != MA_SUCCESS) {
            ZCL_FATAL();
        }

        ma_sound_set_volume(ma_snd, vol);
        ma_sound_set_pan(ma_snd, pan);
        ma_sound_set_pitch(ma_snd, pitch);
        ma_sound_set_looping(ma_snd, loop);

        if (ma_sound_start(ma_snd) != MA_SUCCESS) {
            ZCL_FATAL();
        }

        zcl::BitsetSet(g_module_state.snd_insts.activity, index);
        g_module_state.snd_insts.versions[index]++;

        *o_id = {index, g_module_state.snd_insts.versions[index]};

        return true;
    }

    void SoundStop(const t_sound_id id) {
        ZCL_ASSERT(g_module_state.active);
        ZCL_ASSERT(zcl::BitsetCheckSet(g_module_state.snd_insts.activity, id.index) && g_module_state.snd_insts.versions[id.index] == id.version);

        ma_sound_stop(&g_module_state.snd_insts.ma_snds[id.index]);
        ma_sound_uninit(&g_module_state.snd_insts.ma_snds[id.index]);
        ma_audio_buffer_ref_uninit(&g_module_state.snd_insts.ma_buf_refs[id.index]);

        zcl::BitsetUnset(g_module_state.snd_insts.activity, id.index);
    }

    zcl::t_b8 SoundCheckPlaying(const t_sound_id id) {
        ZCL_ASSERT(g_module_state.active);
        ZCL_ASSERT(id.version <= g_module_state.snd_insts.versions[id.index]);

        if (!zcl::BitsetCheckSet(g_module_state.snd_insts.activity, id.index) || id.version != g_module_state.snd_insts.versions[id.index]) {
            return false;
        }

        return ma_sound_is_playing(&g_module_state.snd_insts.ma_snds[id.index]);
    }

    void SoundsProcFinished() {
        ZCL_ASSERT(g_module_state.active);

        ZCL_BITSET_WALK_ALL_SET (g_module_state.snd_insts.activity, i) {
            ma_sound *const ma_snd = &g_module_state.snd_insts.ma_snds[i];

            if (!ma_sound_is_playing(ma_snd)) {
                ma_sound_uninit(ma_snd);
                ma_audio_buffer_ref_uninit(&g_module_state.snd_insts.ma_buf_refs[i]);

                zcl::BitsetUnset(g_module_state.snd_insts.activity, i);
            }
        }
    }
}
