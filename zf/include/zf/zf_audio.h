#pragma once

#include <zc.h>

namespace zf::audio {
    constexpr t_size g_snd_limit = 32;

    struct s_sound {
    };

    t_b8 InitSys();
    void ShutdownSys();
    void ProcFinishedSounds();

    void PlaySound(const s_sound& snd, const t_f32 vol = 1.0f, const t_f32 pan = 0.0f, const t_f32 pitch = 1.0f);
}

#if 0
#ifndef ZFW_AUDIO_H
#define ZFW_AUDIO_H

#include <miniaudio.h>
#include <cu.h>

#define ZFW_SND_LIMIT 32

#define ZFW_VOL_DEFAULT 1.0f
#define ZFW_PAN_DEFAULT 0.0f
#define ZFW_PITCH_DEFAULT 1.0f

typedef struct {
    s_byte_array samples;
    int frame_cnt;
    int channel_cnt;
    int sample_rate;
    ma_format format;
} zfw_s_sound_type;

DECLARE_ARRAY_TYPE(zfw_s_sound_type, sound_type, SoundType);

static inline void ZFW_AssertSoundTypeValidity(const zfw_s_sound_type* const snd_type) {
    assert(snd_type);
    ByteArray_AssertValidity(snd_type->samples);
    assert(snd_type->frame_cnt > 0);
    assert(snd_type->channel_cnt > 0);
    assert(snd_type->sample_rate > 0);
}

DECLARE_STATIC_BITSET_TYPE(ZFW_SND_LIMIT, snd_activity, SndActivity);

typedef struct {
    ma_engine eng;

    ma_sound snds[ZFW_SND_LIMIT];
    ma_audio_buffer audio_bufs[ZFW_SND_LIMIT];
    t_snd_activity snd_activity;
} zfw_s_audio_sys;

bool ZFW_InitAudioSys(zfw_s_audio_sys* const audio_sys);
void ZFW_CleanAudioSys(zfw_s_audio_sys* const audio_sys);
void ZFW_UpdateAudioSys(zfw_s_audio_sys* const audio_sys);

s_sound_type_array ZFW_LoadSoundTypesFromFiles(s_mem_arena* const mem_arena, const int snd_type_cnt, const char* const* const snd_type_file_paths);

bool ZFW_PlaySound(zfw_s_audio_sys* const audio_sys, const s_sound_type_array snd_types, const int snd_type_index, const float vol, const float pan, const float pitch);

#endif
#endif
