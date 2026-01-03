#include <zgl/zgl_audio.h>

#include <miniaudio.h>

namespace zf {
#if 0
    struct s_sound_type {
        const s_sound_type_arena &group;
        t_i32 group_version = 0;

        s_sound_data snd_data = {};

        s_sound_type *next = nullptr;

        s_sound_type(s_sound_type_arena &group, const t_i32 group_version) : group(group), group_version(group_version) {}
    };

    const t_i32 g_snd_inst_limit = 32;

    struct {
        t_b8 initted = false;

        ma_engine ma_eng = {};

        struct {
            s_static_array<ma_sound, g_snd_inst_limit> ma_snds;
            s_static_array<ma_audio_buffer_ref, g_snd_inst_limit> ma_buf_refs;
            s_static_array<const s_sound_type *, g_snd_inst_limit> types;
            s_static_bit_vec<g_snd_inst_limit> activity;
            s_static_array<t_i32, g_snd_inst_limit> versions;
        } snd_insts = {};
    } g_state;

    void StartupAudioModule() {
        ZF_REQUIRE(!g_state.initted);

        g_state.initted = true;

        if (ma_engine_init(nullptr, &g_state.ma_eng) != MA_SUCCESS) {
            ZF_FATAL();
        }
    }

    void ShutdownAudioModule() {
        ZF_REQUIRE(g_state.initted);

        ZF_WALK_SET_BITS(g_state.snd_insts.activity, i) {
            ma_sound_stop(&g_state.snd_insts.ma_snds[i]);
            ma_sound_uninit(&g_state.snd_insts.ma_snds[i]);

            ma_audio_buffer_ref_uninit(&g_state.snd_insts.ma_buf_refs[i]);
        }

        ma_engine_uninit(&g_state.ma_eng);

        g_state = {};
    }

    void s_sound_type_arena::Release() {
        ZF_ASSERT(g_state.initted);

        auto type = m_head;

        while (type) {
            ZF_WALK_SET_BITS(g_state.snd_insts.activity, i) {
                if (g_state.snd_insts.types[i] == type) {
                    StopSound({i, g_state.snd_insts.versions[i]});
                }
            }

            type = type->next;
        }

        m_arena.Release();
        m_head = nullptr;
        m_tail = nullptr;

        m_version++;
    }

    t_b8 s_sound_type_arena::AddFromRaw(const s_str_rdonly file_path, s_arena &temp_arena, s_ptr<s_sound_type> &o_type) {
        ZF_ASSERT(g_state.initted);

        s_sound_data snd_data = {};

        if (!LoadSoundDataFromRaw(file_path, m_arena, temp_arena, snd_data)) {
            return false;
        }

        Add(snd_data, o_type);

        return true;
    }

    t_b8 s_sound_type_arena::AddFromPacked(const s_str_rdonly file_path, s_arena &temp_arena, s_ptr<s_sound_type> &o_type) {
        ZF_ASSERT(g_state.initted);

        s_sound_data snd_data = {};

        if (!UnpackSound(file_path, m_arena, temp_arena, snd_data)) {
            return false;
        }

        Add(snd_data, o_type);

        return true;
    }

    void s_sound_type_arena::Add(const s_sound_data snd_data, s_ptr<s_sound_type> &o_type) {
        o_type = Alloc<s_sound_type>(&m_arena, *this, m_version);
        o_type->snd_data = snd_data;

        if (!m_head) {
            m_head = o_type;
            m_tail = o_type;
        } else {
            m_tail->next = o_type;
            m_tail = o_type;
        }
    }

    s_sound_id PlaySound(const s_sound_type &type, const t_f32 vol, const t_f32 pan, const t_f32 pitch, const t_b8 loop) {
        ZF_ASSERT(g_state.initted);
        ZF_ASSERT(type.group_version == type.group.Version());
        ZF_ASSERT(vol >= 0.0f && vol <= 1.0f);
        ZF_ASSERT(pan >= -1.0f && pan <= 1.0f);
        ZF_ASSERT(pitch > 0.0f);

        const t_i32 index = IndexOfFirstUnsetBit(g_state.snd_insts.activity);

        if (index == -1) {
            LogWarning("Trying to play a sound, but the sound instance limit has been reached!");
            return {};
        }

        ma_sound &ma_snd = g_state.snd_insts.ma_snds[index];
        ma_audio_buffer_ref &ma_buf_ref = g_state.snd_insts.ma_buf_refs[index];

        g_state.snd_insts.types[index] = &type;

        if (ma_audio_buffer_ref_init(ma_format_f32, static_cast<ma_uint32>(type.snd_data.Meta().channel_cnt), type.snd_data.PCM().Ptr(), static_cast<ma_uint64>(type.snd_data.Meta().frame_cnt), &ma_buf_ref) != MA_SUCCESS) {
            ZF_FATAL();
        }

        ma_buf_ref.sampleRate = static_cast<ma_uint32>(type.snd_data.Meta().sample_rate);

        if (ma_sound_init_from_data_source(&g_state.ma_eng, &ma_buf_ref, 0, nullptr, &ma_snd) != MA_SUCCESS) {
            ZF_FATAL();
        }

        ma_sound_set_volume(&ma_snd, vol);
        ma_sound_set_pan(&ma_snd, pan);
        ma_sound_set_pitch(&ma_snd, pitch);
        ma_sound_set_looping(&ma_snd, loop);

        if (ma_sound_start(&ma_snd) != MA_SUCCESS) {
            ZF_FATAL();
        }

        SetBit(g_state.snd_insts.activity, index);
        g_state.snd_insts.versions[index]++;

        return {index, g_state.snd_insts.versions[index]};
    }

    void StopSound(const s_sound_id id) {
        ZF_ASSERT(g_state.initted);

        ZF_ASSERT(IsBitSet(g_state.snd_insts.activity, id.Index()) && g_state.snd_insts.versions[id.Index()] == id.Version());

        ma_sound_stop(&g_state.snd_insts.ma_snds[id.Index()]);
        ma_sound_uninit(&g_state.snd_insts.ma_snds[id.Index()]);
        ma_audio_buffer_ref_uninit(&g_state.snd_insts.ma_buf_refs[id.Index()]);

        UnsetBit(g_state.snd_insts.activity, id.Index());
    }

    t_b8 IsSoundPlaying(const s_sound_id id) {
        ZF_ASSERT(g_state.initted);
        ZF_ASSERT(id.Version() <= g_state.snd_insts.versions[id.Index()]);

        if (!IsBitSet(g_state.snd_insts.activity, id.Index()) || id.Version() != g_state.snd_insts.versions[id.Index()]) {
            return false;
        }

        return ma_sound_is_playing(&g_state.snd_insts.ma_snds[id.Index()]);
    }

    void ProcFinishedSounds() {
        ZF_ASSERT(g_state.initted);

        ZF_WALK_SET_BITS(g_state.snd_insts.activity, i) {
            ma_sound &ma_snd = g_state.snd_insts.ma_snds[i];

            if (!ma_sound_is_playing(&ma_snd)) {
                ma_sound_uninit(&ma_snd);
                ma_audio_buffer_ref_uninit(&g_state.snd_insts.ma_buf_refs[i]);

                UnsetBit(g_state.snd_insts.activity, i);
            }
        }
    }
#endif
}
