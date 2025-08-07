#ifndef ZFW_AUDIO_H
#define ZFW_AUDIO_H

#include <miniaudio.h>
#include <cu.h>

#define ZFW_SND_LIMIT 32

#define ZFW_VOL_DEFAULT 1.0f
#define ZFW_PAN_DEFAULT 0.0f
#define ZFW_PITCH_DEFAULT 1.0f

typedef struct {
    const t_byte* sample_buf;
    int frame_cnt;
    int channel_cnt;
    int sample_rate;
    ma_format format;
} zfw_s_sound_type;

static inline void ZFW_AssertSoundTypeValidity(const zfw_s_sound_type* const snd_type) {
    assert(snd_type);
    assert(snd_type->sample_buf);
    assert(snd_type->frame_cnt > 0);
    assert(snd_type->channel_cnt > 0);
    assert(snd_type->sample_rate > 0);
}

typedef struct {
    const zfw_s_sound_type* buf;
    int cnt;
} zfw_s_sound_types;

static inline void ZFW_AssertSoundTypesValidity(const zfw_s_sound_types* const snd_types) {
    assert(snd_types);
    assert((!snd_types->buf && snd_types->cnt == 0) || (snd_types->buf && snd_types->cnt > 0));

    for (int i = 0; i < snd_types->cnt; i++) {
        ZFW_AssertSoundTypeValidity(&snd_types->buf[i]);
    }
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

zfw_s_sound_types ZFW_LoadSoundTypesFromFiles(s_mem_arena* const mem_arena, const int snd_type_cnt, const char* const* const snd_type_file_paths);

bool ZFW_PlaySound(zfw_s_audio_sys* const audio_sys, const zfw_s_sound_types* const snd_types, const int snd_type_index, const float vol, const float pan, const float pitch);

#endif
