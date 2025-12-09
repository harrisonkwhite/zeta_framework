#include <zgl/zgl_audio.h>

#include <miniaudio.h>

namespace zf {
    struct s_sound_type {
        s_audio_sys *audio_sys = nullptr;
        s_sound_data snd_data = {};
        s_sound_type *next = nullptr;
    };

    constexpr t_len g_snd_inst_limit = 32;

    struct s_audio_sys {
        ma_engine ma_eng = {};

        struct {
            s_static_array<ma_sound, g_snd_inst_limit> ma_snds;
            s_static_array<ma_audio_buffer_ref, g_snd_inst_limit> ma_buf_refs;
            s_static_array<const s_sound_type *, g_snd_inst_limit> types;
            s_static_bit_vec<g_snd_inst_limit> activity;
            s_static_array<t_len, g_snd_inst_limit> versions;
        } snd_insts = {};
    };

    t_b8 internal::CreateAudioSys(s_mem_arena *const mem_arena, s_audio_sys **const o_as) {
        *o_as = mem_arena->Push<s_audio_sys>();
        const auto as = *o_as;

        if (!as) {
            return false;
        }

        if (ma_engine_init(nullptr, &as->ma_eng) != MA_SUCCESS) {
            return false;
        }

        return true;
    }

    void internal::DestroyAudioSys(s_audio_sys *const as) {
        ZF_FOR_EACH_SET_BIT(as->snd_insts.activity, i) {
            ma_sound_stop(&as->snd_insts.ma_snds[i]);
            ma_sound_uninit(&as->snd_insts.ma_snds[i]);

            ma_audio_buffer_ref_uninit(&as->snd_insts.ma_buf_refs[i]);
        }

        ma_engine_uninit(&as->ma_eng);
    }

    t_b8 CreateSoundTypeFromRaw(const s_str_rdonly file_path, s_sound_type_arena *const type_arena, s_mem_arena *const temp_mem_arena, s_sound_type **const o_type) {
        *o_type = type_arena->mem_arena->Push<s_sound_type>();
        const auto type = *o_type;

        if (!type) {
            return false;
        }

        *type = {
            .audio_sys = type_arena->audio_sys,
        };

        if (!LoadSoundFromRaw(file_path, type_arena->mem_arena, &type->snd_data)) {
            return false;
        }

        if (!type_arena->head) {
            type_arena->head = type;
            type_arena->tail = type;
        } else {
            type_arena->tail->next = type;
            type_arena->tail = type;
        }

        return true;
    }

    void DestroySoundTypes(s_sound_type_arena *const type_arena) {
        s_audio_sys *const as = type_arena->audio_sys;

        const s_sound_type *type = type_arena->head;

        while (type) {
            ZF_FOR_EACH_SET_BIT(as->snd_insts.activity, i) {
                if (as->snd_insts.types[i] == type) {
                    StopSound({as, i, as->snd_insts.versions[i]});
                }
            }

            type = type->next;
        }
    }

    t_b8 PlaySound(const s_sound_type *const type, s_sound_id *const o_id, const t_f32 vol, const t_f32 pan, const t_f32 pitch, const t_b8 loop) {
        ZF_ASSERT(vol >= 0.0f && vol <= 1.0f);
        ZF_ASSERT(pan >= -1.0f && pan <= 1.0f);
        ZF_ASSERT(pitch > 0.0f);

        t_b8 clean_up = false;

        s_audio_sys *const as = type->audio_sys;

        const t_len index = IndexOfFirstUnsetBit(as->snd_insts.activity);

        if (index == -1) {
            clean_up = true;
            return false;
        }

        ma_sound &ma_snd = as->snd_insts.ma_snds[index];
        ma_audio_buffer_ref &ma_buf_ref = as->snd_insts.ma_buf_refs[index];

        as->snd_insts.types[index] = type;

        if (ma_audio_buffer_ref_init(ma_format_f32, static_cast<ma_uint32>(type->snd_data.Meta().channel_cnt), type->snd_data.PCM().Raw(), static_cast<ma_uint64>(type->snd_data.Meta().frame_cnt), &ma_buf_ref) != MA_SUCCESS) {
            clean_up = true;
            return false;
        }

        ZF_DEFER({
            if (clean_up) {
                ma_audio_buffer_ref_uninit(&ma_buf_ref);
            }
        });

        ma_buf_ref.sampleRate = static_cast<ma_uint32>(type->snd_data.Meta().sample_rate);

        if (ma_sound_init_from_data_source(&as->ma_eng, &ma_buf_ref, 0, nullptr, &ma_snd) != MA_SUCCESS) {
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
            clean_up = true;
            return false;
        }

        SetBit(as->snd_insts.activity, index);
        as->snd_insts.versions[index]++;

        if (o_id) {
            *o_id = {as, index, as->snd_insts.versions[index]};
        }

        return true;
    }

    void StopSound(const s_sound_id id) {
        const auto as = id.audio_sys;

        ZF_ASSERT(IsBitSet(as->snd_insts.activity, id.index) && as->snd_insts.versions[id.index] == id.version);

        ma_sound_stop(&as->snd_insts.ma_snds[id.index]);
        ma_sound_uninit(&as->snd_insts.ma_snds[id.index]);
        ma_audio_buffer_ref_uninit(&as->snd_insts.ma_buf_refs[id.index]);

        UnsetBit(as->snd_insts.activity, id.index);
    }

    t_b8 IsSoundPlaying(s_audio_sys *const as, const s_sound_id id) {
        ZF_ASSERT(id.version <= as->snd_insts.versions[id.index]);

        if (!IsBitSet(as->snd_insts.activity, id.index) || id.version != as->snd_insts.versions[id.index]) {
            return false;
        }

        return ma_sound_is_playing(&as->snd_insts.ma_snds[id.index]);
    }

    void internal::ProcFinishedSounds(s_audio_sys *const as) {
        ZF_FOR_EACH_SET_BIT(as->snd_insts.activity, i) {
            ma_sound &snd = as->snd_insts.ma_snds[i];

            if (!ma_sound_is_playing(&snd)) {
                ma_sound_uninit(&snd);
                ma_audio_buffer_ref_uninit(&as->snd_insts.ma_buf_refs[i]);

                UnsetBit(as->snd_insts.activity, i);
            }
        }
    }
}
