#ifndef ZFW_AUDIO_H
#define ZFW_AUDIO_H

#include <miniaudio.h>
#include <cu.h>

#define ZFW_SND_LIMIT 32

#define ZFW_VOL_DEFAULT 1.0f
#define ZFW_PAN_DEFAULT 0.0f
#define ZFW_PITCH_DEFAULT 1.0f

typedef struct {
    const t_u8* sample_buf;
    int frame_cnt;
    int channel_cnt;
    int sample_rate;
    ma_format format;
} zfw_s_sound_type;

typedef struct {
    const zfw_s_sound_type* buf;
    int cnt;
} zfw_s_sound_types;

typedef struct {
    ma_engine eng;

    ma_sound snds[ZFW_SND_LIMIT];
    ma_audio_buffer audio_bufs[ZFW_SND_LIMIT];
    t_u8 snd_activity[BITS_TO_BYTES(ZFW_SND_LIMIT)];
} zfw_s_audio_sys;

bool ZFW_InitAudioSys(zfw_s_audio_sys* const audio_sys);
void ZFW_CleanAudioSys(zfw_s_audio_sys* const audio_sys);
void ZFW_UpdateAudioSys(zfw_s_audio_sys* const audio_sys);

zfw_s_sound_types ZFW_LoadSoundTypesFromFiles(s_mem_arena* const mem_arena, const int snd_type_cnt, const char* const* const snd_type_file_paths);

bool ZFW_PlaySound(zfw_s_audio_sys* const audio_sys, const zfw_s_sound_types* const snd_types, const int snd_type_index, const float vol, const float pan, const float pitch);

#endif
