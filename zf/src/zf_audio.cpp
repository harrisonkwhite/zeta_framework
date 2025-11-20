#include <zf/zf_audio.h>

#include <miniaudio.h>

namespace zf::audio {
    constexpr t_size g_snd_type_limit = 1024;
    constexpr t_size g_snd_limit = 32;

    static struct {
        t_b8 initted;

        s_static_array<s_sound_meta, g_snd_type_limit> snd_type_metas;
        s_static_array<s_array<t_f32>, g_snd_type_limit> snd_type_pcms;
        s_static_bit_vector<g_snd_type_limit> snd_type_activity;

        ma_engine ma_eng;
    } g_sys;

    static t_b8 IsSoundTypeIDValid(const t_sound_type_id id) {
        return id >= 0 && id < g_snd_type_limit;
    }

    t_b8 InitSys() {
        g_sys = {};

        if (ma_engine_init(nullptr, &g_sys.ma_eng) != MA_SUCCESS) {
            ZF_REPORT_FAILURE();
            return false;
        }

        g_sys.initted = true;

        return true;
    }

    void ShutdownSys() {
        ZF_ASSERT(g_sys.initted);

        for (t_size i = IndexOfFirstSetBit(g_sys.snd_type_activity); i != -1; i = IndexOfFirstSetBit(g_sys.snd_type_activity, i + 1)) {
            ZF_LOG_WARNING("Sound type with ID %lld not released!", i); // @todo: Update this if the ID no longer is an index.
            free(g_sys.snd_type_pcms[i].buf_raw);
            g_sys.snd_type_pcms[i] = {};
        }

        ma_engine_uninit(&g_sys.ma_eng);

        g_sys.initted = false;
    }

    t_b8 RegisterSoundType(const s_sound_meta& snd_meta, const s_array_rdonly<t_f32> snd_pcm, t_sound_type_id& o_id) {
        ZF_ASSERT(g_sys.initted);

        o_id = IndexOfFirstUnsetBit(g_sys.snd_type_activity);

        if (o_id == -1) {
            ZF_REPORT_FAILURE();
            return false;
        }

        const t_size pcm_len = CalcSampleCount(snd_meta);
        const auto pcm_buf_raw = static_cast<t_f32*>(malloc(static_cast<size_t>(ZF_SIZE_OF(t_f32) * pcm_len)));

        if (!pcm_buf_raw) {
            ZF_REPORT_FAILURE();
            return false;
        }

        g_sys.snd_type_pcms[o_id] = {pcm_buf_raw, pcm_len};
        Copy(g_sys.snd_type_pcms[o_id], snd_pcm);

        g_sys.snd_type_metas[o_id] = snd_meta;

        return true;
    }

    void UnregisterSoundType(const t_sound_type_id id) {
        ZF_ASSERT(g_sys.initted);
        ZF_ASSERT(IsSoundTypeIDValid(id));
        ZF_ASSERT(IsBitSet(g_sys.snd_type_activity, id));

        free(g_sys.snd_type_pcms.buf_raw);
        g_sys.snd_type_pcms = {};
        UnsetBit(g_sys.snd_type_activity, id);
    }

#if 0
    constexpr t_size g_snd_type_limit = 1024;
    constexpr t_size g_snd_limit = 32;

    static struct {
        t_b8 initted;

        s_static_array<s_sound_meta, g_snd_type_limit> snd_type_metas;
        s_static_array<s_array<t_f32>, g_snd_type_limit> snd_type_pcms;
        s_static_bit_vector<g_snd_type_limit> snd_type_activity;

        ma_engine eng;
        s_static_activity_array<ma_sound, g_snd_limit> snds;
    } g_sys;

    static t_b8 IsSoundTypeIDValid(const t_sound_type_id id) {
        return id >= 0 && id < g_snd_type_limit;
    }

    t_b8 InitSys() {
        g_sys = {};

        if (ma_engine_init(nullptr, &g_sys.eng) != MA_SUCCESS) {
            ZF_REPORT_FAILURE();
            return false;
        }

        g_sys.initted = true;

        return true;
    }

    void ShutdownSys() {
        ZF_ASSERT(g_sys.initted);

        for (t_size i = IndexOfFirstActiveSlot(g_sys.snds); i != -1; i = IndexOfFirstActiveSlot(g_sys.snds, i + 1)) {
            ma_sound_uninit(&g_sys.snds[i]);
        }

        ma_engine_uninit(&g_sys.eng);

        g_sys.initted = false;
    }

    t_b8 RegisterSoundType(const s_sound_data_rdonly snd_data, t_sound_type_id& o_id) {

        return true;
    }

    void ProcFinishedSounds() {
        ZF_ASSERT(g_sys.initted);

        for (t_size i = IndexOfFirstActiveSlot(g_sys.snds); i != -1; i = IndexOfFirstActiveSlot(g_sys.snds, i + 1)) {
            ma_sound& snd = g_sys.snds[i];

            if (!ma_sound_is_playing(&snd)) {
                ma_sound_uninit(&snd);
                DeactivateSlot(g_sys.snds, i);
            }
        }
    }

    t_b8 RegisterSoundType(const s_sound_meta& snd_meta, const s_array<t_f32> snd_pcm, t_sound_type_id& o_id) {
        ZF_ASSERT(g_sys.initted);

        const t_size index = IndexOfFirstUnsetBit(g_sys.snd_type_activity);

        if (index == -1) {
            ZF_REPORT_FAILURE();
            return false;
        }

        const t_size pcm_size = CalcSampleCount(snd_meta);
        const auto pcm_buf_raw = static_cast<t_f32*>(malloc(static_cast<size_t>(ZF_SIZE_OF(t_f32) * pcm_size)));

        if (!pcm_buf_raw) {
            ZF_REPORT_FAILURE();
            return false;
        }

        g_sys.snd_type_pcms[index] = {pcm_buf_raw, pcm_size};
        Copy(g_sys.snd_type_pcms[index], snd_pcm);

        g_sys.snd_type_metas[index] = snd_meta;

        return true;
    }

    void UnregisterSoundType(const t_sound_type_id id) {
        ZF_ASSERT(g_sys.initted);
        ZF_ASSERT(IsSoundTypeIDValid(id));
        ZF_ASSERT(IsSlotActive(g_sys.snd_type_bufs, id));
        ma_audio_buffer_uninit(&g_sys.snd_type_bufs[id]);
        DeactivateSlot(g_sys.snd_type_bufs, id);
    }

    t_b8 PlaySound(const t_sound_type_id type_id, const t_f32 vol, const t_f32 pan, const t_f32 pitch, const t_b8 loop) {
        ZF_ASSERT(g_sys.initted);
        ZF_ASSERT(IsSoundTypeIDValid(type_id));
        ZF_ASSERT(vol >= 0.0f && vol <= 1.0f);
        ZF_ASSERT(pan >= -1.0f && pan <= 1.0f);
        ZF_ASSERT(pitch > 0.0f);

        // Find and use the first inactive sound slot.
        const t_size index = TakeFirstInactiveSlot(g_sys.snds);

        if (index == -1) {
            ZF_REPORT_FAILURE();
            return false;
        }

        ma_sound& snd = g_sys.snds[index];

        // Bind the buffer to the sound.
        if (ma_sound_init_from_data_source(&g_sys.eng, &g_sys.snd_type_bufs[type_id], 0, nullptr, &snd) != MA_SUCCESS) {
            DeactivateSlot(g_sys.snds, index);
            ZF_REPORT_FAILURE();
            return false;
        }

        // Set properties and play.
        ma_sound_set_volume(&snd, vol);
        ma_sound_set_pan(&snd, pan);
        ma_sound_set_pitch(&snd, pitch);
        ma_sound_set_looping(&snd, loop);

        if (ma_sound_start(&snd) != MA_SUCCESS) {
            ma_sound_uninit(&snd);
            DeactivateSlot(g_sys.snds, index);
            ZF_REPORT_FAILURE();
            return false;
        }

        return true;
    }
#endif

#if 0
    t_b8 MakeResourceArena(const t_size snd_type_limit, s_resource_arena& o_res_arena) {
        o_res_arena = {};

        if (!MakeMemArena(o_res_arena.mem_arena)) {
            return false;
        }

        if (!MakeArray(o_res_arena.mem_arena, snd_type_limit, o_res_arena.snd_type_metas)) {
            return false;
        }

        if (!MakeArray(o_res_arena.mem_arena, snd_type_limit, o_res_arena.snd_type_pcms)) {
            return false;
        }

        return true;
    }

    void ReleaseResourceArena(s_resource_arena& res_arena) {
        ReleaseMemArena(res_arena.mem_arena);
        res_arena = {};
    }

    [[nodiscard]] t_b8 RegisterSoundTypeFromRaw(s_resource_arena& res_arena, const s_str_rdonly file_path, t_sound_type_id& o_id) {
        if (res_arena.snd_type_cnt == res_arena.Cap()) {
            ZF_REPORT_FAILURE();
            return false;
        }

        s_sound_meta& meta = res_arena.snd_type_metas[res_arena.snd_type_cnt];
        s_array<t_f32>& pcm = res_arena.snd_type_pcms[res_arena.snd_type_cnt];

        if (!LoadSoundFromRaw(file_path, res_arena.mem_arena, meta, pcm)) {
            ZF_REPORT_FAILURE();
            return false;
        }

        res_arena.snd_type_cnt++;

        return true;
    }
#endif
}
