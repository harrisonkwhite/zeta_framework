#include <miniaudio.h>
#include <zgl/zgl_audio.h>

namespace zf
{
    struct s_sound_type
    {
        s_sound_meta meta;
        s_array_rdonly<t_f32> pcm;

        s_sound_type* next;
    };

    constexpr t_size g_snd_inst_limit = 32;

    struct s_audio_sys
    {
        ma_engine ma_eng;

        struct
        {
            s_static_array<ma_sound, g_snd_inst_limit> ma_snds;
            s_static_array<ma_audio_buffer_ref, g_snd_inst_limit> ma_buf_refs;
            s_static_array<const s_sound_type*, g_snd_inst_limit> types;
            s_static_bit_vec<g_snd_inst_limit> activity;
            s_static_array<t_size, g_snd_inst_limit> versions;
        } snd_insts;
    };

    struct s_sound_id
    {
        t_size index;
        t_size version;
    };

    t_b8 P_InitAudioSys(s_mem_arena& mem_arena, s_audio_sys*& o_as)
    {
        o_as = PushToMemArena<s_audio_sys>(&mem_arena);

        if (!o_as)
        {
            ZF_REPORT_ERROR();
            return false;
        }

        if (ma_engine_init(nullptr, &o_as->ma_eng) != MA_SUCCESS)
        {
            ZF_REPORT_ERROR();
            return false;
        }

        return true;
    }

    void P_ShutdownAudioSys(s_audio_sys& as)
    {
        ZF_FOR_EACH_SET_BIT(as.snd_insts.activity, i)
        {
            ma_sound_stop(&as.snd_insts.ma_snds[i]);
            ma_sound_uninit(&as.snd_insts.ma_snds[i]);

            ma_audio_buffer_ref_uninit(&as.snd_insts.ma_buf_refs[i]);
        }

        ma_engine_uninit(&as.ma_eng);
    }

    t_b8 CreateSoundTypeFromRaw(const s_str_rdonly file_path,
        s_sound_type_arena& type_arena,
        s_mem_arena& temp_mem_arena,
        s_sound_type*& o_type)
    {
        s_sound_meta meta;
        s_array<t_f32> pcm;

        if (!LoadSoundFromRaw(file_path, *type_arena.mem_arena, temp_mem_arena, meta, pcm))
        {
            return false;
        }

        o_type = PushToMemArena<s_sound_type>(type_arena.mem_arena);

        if (!o_type)
        {
            return false;
        }

        *o_type = {.meta = meta, .pcm = pcm};

        if (!type_arena.head)
        {
            type_arena.head = o_type;
            type_arena.tail = o_type;
        }
        else
        {
            type_arena.tail->next = o_type;
            type_arena.tail = o_type;
        }

        return true;
    }

    void DestroySoundTypes(s_audio_sys& as, s_sound_type_arena& type_arena)
    {
        const s_sound_type* type = type_arena.head;

        while (type)
        {
            ZF_FOR_EACH_SET_BIT(as.snd_insts.activity, i)
            {
                if (as.snd_insts.types[i] == type)
                {
                    StopSound(as, {i, as.snd_insts.versions[i]});
                }
            }

            type = type->next;
        }
    }

    t_b8 PlaySound(s_audio_sys& as,
        const s_sound_type* const type,
        s_sound_id* const o_id,
        const t_f32 vol,
        const t_f32 pan,
        const t_f32 pitch,
        const t_b8 loop)
    {
        ZF_ASSERT(vol >= 0.0f && vol <= 1.0f);
        ZF_ASSERT(pan >= -1.0f && pan <= 1.0f);
        ZF_ASSERT(pitch > 0.0f);

        t_b8 clean_up = false;

        const t_size index = IndexOfFirstUnsetBit(as.snd_insts.activity);

        if (index == -1)
        {
            ZF_REPORT_ERROR();
            clean_up = true;
            return false;
        }

        ma_sound& ma_snd = as.snd_insts.ma_snds[index];
        ma_audio_buffer_ref& ma_buf_ref = as.snd_insts.ma_buf_refs[index];

        as.snd_insts.types[index] = type;

        if (ma_audio_buffer_ref_init(ma_format_f32, static_cast<ma_uint32>(type->meta.channel_cnt),
                type->pcm.buf_raw, static_cast<ma_uint64>(type->meta.frame_cnt),
                &ma_buf_ref) != MA_SUCCESS)
        {
            ZF_REPORT_ERROR();
            clean_up = true;
            return false;
        }

        ZF_DEFER({
            if (clean_up)
            {
                ma_audio_buffer_ref_uninit(&ma_buf_ref);
            }
        });

        ma_buf_ref.sampleRate = static_cast<ma_uint32>(type->meta.sample_rate);

        if (ma_sound_init_from_data_source(&as.ma_eng, &ma_buf_ref, 0, nullptr, &ma_snd) !=
            MA_SUCCESS)
        {
            ZF_REPORT_ERROR();
            clean_up = true;
            return false;
        }

        ZF_DEFER({
            if (clean_up)
            {
                ma_sound_uninit(&ma_snd);
            }
        });

        ma_sound_set_volume(&ma_snd, vol);
        ma_sound_set_pan(&ma_snd, pan);
        ma_sound_set_pitch(&ma_snd, pitch);
        ma_sound_set_looping(&ma_snd, loop);

        if (ma_sound_start(&ma_snd) != MA_SUCCESS)
        {
            ZF_REPORT_ERROR();
            clean_up = true;
            return false;
        }

        SetBit(as.snd_insts.activity, index);
        as.snd_insts.versions[index]++;

        if (o_id)
        {
            *o_id = {index, as.snd_insts.versions[index]};
        }

        return true;
    }

    void StopSound(s_audio_sys& as, const s_sound_id id)
    {
        ZF_ASSERT(IsBitSet(as.snd_insts.activity, id.index) &&
                  as.snd_insts.versions[id.index] == id.version);

        ma_sound_stop(&as.snd_insts.ma_snds[id.index]);
        ma_sound_uninit(&as.snd_insts.ma_snds[id.index]);
        ma_audio_buffer_ref_uninit(&as.snd_insts.ma_buf_refs[id.index]);

        UnsetBit(as.snd_insts.activity, id.index);
    }

    t_b8 IsSoundPlaying(s_audio_sys& as, const s_sound_id id)
    {
        ZF_ASSERT(id.version <= as.snd_insts.versions[id.index]);

        if (!IsBitSet(as.snd_insts.activity, id.index) ||
            id.version != as.snd_insts.versions[id.index])
        {
            return false;
        }

        return ma_sound_is_playing(&as.snd_insts.ma_snds[id.index]);
    }

    void P_ProcFinishedSounds(s_audio_sys& as)
    {
        ZF_FOR_EACH_SET_BIT(as.snd_insts.activity, i)
        {
            ma_sound& snd = as.snd_insts.ma_snds[i];

            if (!ma_sound_is_playing(&snd))
            {
                ma_sound_uninit(&snd);
                ma_audio_buffer_ref_uninit(&as.snd_insts.ma_buf_refs[i]);

                UnsetBit(as.snd_insts.activity, i);
            }
        }
    }
}
