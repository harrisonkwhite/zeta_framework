#include <zgl/zgl_audio.h>

#include <miniaudio.h>

namespace zf::audio_sys {
    struct t_sound_type {
        t_b8 valid;
        audio::t_sound_data_rdonly snd_data;
        t_sound_type *next;
    };

    const t_i32 g_sound_inst_limit = 32;

    struct {
        t_b8 active;

        ma_engine ma_eng;

        struct {
            t_static_array<ma_sound, g_sound_inst_limit> ma_snds;
            t_static_array<ma_audio_buffer_ref, g_sound_inst_limit> ma_buf_refs;
            t_static_array<const t_sound_type *, g_sound_inst_limit> types;
            mem::t_static_bitset<g_sound_inst_limit> activity;
            t_static_array<t_i32, g_sound_inst_limit> versions;
        } snd_insts;
    } g_module_state;

    void module_startup() {
        ZF_REQUIRE(!g_module_state.active);

        g_module_state.active = true;

        if (ma_engine_init(nullptr, &g_module_state.ma_eng) != MA_SUCCESS) {
            ZF_FATAL();
        }
    }

    void module_shutdown() {
        ZF_REQUIRE(g_module_state.active);

        ZF_WALK_SET_BITS (g_module_state.snd_insts.activity, i) {
            ma_sound_stop(&g_module_state.snd_insts.ma_snds[i]);
            ma_sound_uninit(&g_module_state.snd_insts.ma_snds[i]);

            ma_audio_buffer_ref_uninit(&g_module_state.snd_insts.ma_buf_refs[i]);
        }

        ma_engine_uninit(&g_module_state.ma_eng);

        g_module_state = {};
    }

    void sound_type_group_destroy(t_sound_type_group *const group) {
        ZF_ASSERT(g_module_state.active);

        t_sound_type *snd_type = group->head;

        while (snd_type) {
            ZF_WALK_SET_BITS (g_module_state.snd_insts.activity, i) {
                if (g_module_state.snd_insts.types[i] == snd_type) {
                    sound_stop({i, g_module_state.snd_insts.versions[i]});
                }
            }

            snd_type->valid = false;
            snd_type = snd_type->next;
        }

        mem::arena_destroy(&group->arena);
        group->head = nullptr;
        group->tail = nullptr;
    }

    static t_sound_type *sound_type_group_add(t_sound_type_group *const group, const audio::t_sound_data_rdonly snd_data) {
        const auto result = mem::arena_push_item<t_sound_type>(&group->arena);
        result->valid = true;
        result->snd_data = snd_data;
        result->next = nullptr;

        if (!group->head) {
            group->head = result;
            group->tail = result;
        } else {
            group->tail->next = result;
            group->tail = result;
        }

        return result;
    }

    t_sound_type *sound_type_create_from_raw(const strs::t_str_rdonly file_path, t_sound_type_group *const group, mem::t_arena *const temp_arena) {
        ZF_ASSERT(g_module_state.active);

        audio::t_sound_data_mut snd_data;

        if (!audio::sound_load_from_raw(file_path, &group->arena, temp_arena, &snd_data)) {
            ZF_FATAL();
        }

        return sound_type_group_add(group, snd_data);
    }

    t_sound_type *sound_type_create_from_packed(const strs::t_str_rdonly file_path, t_sound_type_group *const group, mem::t_arena *const temp_arena) {
        ZF_ASSERT(g_module_state.active);

        audio::t_sound_data_mut snd_data;

        if (!audio::sound_unpack(file_path, &group->arena, temp_arena, &snd_data)) {
            ZF_FATAL();
        }

        return sound_type_group_add(group, snd_data);
    }

    t_b8 sound_play_and_get_id(const t_sound_type *const type, t_sound_id *const o_id, const t_f32 vol, const t_f32 pan, const t_f32 pitch, const t_b8 loop) {
        ZF_ASSERT(g_module_state.active);
        ZF_ASSERT(type->valid);
        ZF_ASSERT(vol >= 0.0f && vol <= 1.0f);
        ZF_ASSERT(pan >= -1.0f && pan <= 1.0f);
        ZF_ASSERT(pitch > 0.0f);

        const t_i32 index = mem::bitset_find_first_unset_bit(g_module_state.snd_insts.activity);

        if (index == -1) {
            io::log_warning(ZF_STR_LITERAL("Trying to play a sound, but the sound instance limit has been reached!"));
            return false;
        }

        ma_sound *const ma_snd = &g_module_state.snd_insts.ma_snds[index];
        ma_audio_buffer_ref *const ma_buf_ref = &g_module_state.snd_insts.ma_buf_refs[index];

        g_module_state.snd_insts.types[index] = type;

        if (ma_audio_buffer_ref_init(ma_format_f32, static_cast<ma_uint32>(type->snd_data.meta.channel_cnt), type->snd_data.pcm.raw, static_cast<ma_uint64>(type->snd_data.meta.frame_cnt), ma_buf_ref) != MA_SUCCESS) {
            ZF_FATAL();
        }

        ma_buf_ref->sampleRate = static_cast<ma_uint32>(type->snd_data.meta.sample_rate);

        if (ma_sound_init_from_data_source(&g_module_state.ma_eng, ma_buf_ref, 0, nullptr, ma_snd) != MA_SUCCESS) {
            ZF_FATAL();
        }

        ma_sound_set_volume(ma_snd, vol);
        ma_sound_set_pan(ma_snd, pan);
        ma_sound_set_pitch(ma_snd, pitch);
        ma_sound_set_looping(ma_snd, loop);

        if (ma_sound_start(ma_snd) != MA_SUCCESS) {
            ZF_FATAL();
        }

        mem::bitset_set(g_module_state.snd_insts.activity, index);
        g_module_state.snd_insts.versions[index]++;

        *o_id = {index, g_module_state.snd_insts.versions[index]};

        return true;
    }

    void sound_stop(const t_sound_id id) {
        ZF_ASSERT(g_module_state.active);
        ZF_ASSERT(mem::bitset_check_set(g_module_state.snd_insts.activity, id.index) && g_module_state.snd_insts.versions[id.index] == id.version);

        ma_sound_stop(&g_module_state.snd_insts.ma_snds[id.index]);
        ma_sound_uninit(&g_module_state.snd_insts.ma_snds[id.index]);
        ma_audio_buffer_ref_uninit(&g_module_state.snd_insts.ma_buf_refs[id.index]);

        mem::bitset_unset(g_module_state.snd_insts.activity, id.index);
    }

    t_b8 sound_check_playing(const t_sound_id id) {
        ZF_ASSERT(g_module_state.active);
        ZF_ASSERT(id.version <= g_module_state.snd_insts.versions[id.index]);

        if (!mem::bitset_check_set(g_module_state.snd_insts.activity, id.index) || id.version != g_module_state.snd_insts.versions[id.index]) {
            return false;
        }

        return ma_sound_is_playing(&g_module_state.snd_insts.ma_snds[id.index]);
    }

    void proc_finished_sounds() {
        ZF_ASSERT(g_module_state.active);

        ZF_WALK_SET_BITS (g_module_state.snd_insts.activity, i) {
            ma_sound *const ma_snd = &g_module_state.snd_insts.ma_snds[i];

            if (!ma_sound_is_playing(ma_snd)) {
                ma_sound_uninit(ma_snd);
                ma_audio_buffer_ref_uninit(&g_module_state.snd_insts.ma_buf_refs[i]);

                mem::bitset_unset(g_module_state.snd_insts.activity, i);
            }
        }
    }
}
